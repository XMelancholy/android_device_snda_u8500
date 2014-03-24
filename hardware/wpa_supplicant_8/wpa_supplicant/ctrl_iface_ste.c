/*
 * ST-Ericsson hostap/wpa_supplicant changes and additions.
 * Control interface additions.
 *
 * Copyright (c) ST-Ericsson SA 2011
 *
 * License terms: Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following conditions
 * are met:
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   - Neither the name of ST-Ericsson S.A. nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * If the command is handled in this file, a value != 0 has to be returned.
 * Returning a value != 0 means that the command will be handled in ctrl_iface.c
 */

#include "includes.h"

#include "common.h"
#include "rsn_supp/wpa.h"
#include "config_ssid.h"
#include "wpa_supplicant_i.h"
#include "wpa_supplicant_ste.h"
#include "common/wpa_ctrl.h"
#include "driver_i.h"
#include "bss.h"

#include "config.h"
#ifdef CONFIG_RSSI_FILTER
#include "scan.h"
#endif

#define max_args 30

static int wpa_ste_get_country(struct wpa_supplicant *wpa_s,
				char *reply, size_t reply_size)
{
	int ret;
	char *pos, *end;

	pos = reply;
	end = pos + reply_size;

	if (reply_size <= 3)
		return -1;

	if (wpa_supplicant_ste_get_country(wpa_s, reply))
		return -1;

	ret = os_snprintf(pos, end - pos, "%c%c", reply[0], reply[1]);
	if (ret < 0 || ret >= end - pos)
		return pos - reply;
	pos += ret;

	return pos - reply;
}

static int wpa_ste_set_country(struct wpa_supplicant *wpa_s, const char *buf)
{
	return wpa_supplicant_ste_set_country(wpa_s, buf);
}

#ifdef CONFIG_RSSI_FILTER
/* RSSI Filter */
static int wpa_ste_set_rssi_filter(struct wpa_supplicant *wpa_s, const char *buf)
{
	const char *value = buf;
	int rssi_filter_value =0;

	wpa_printf(MSG_DEBUG, ">%s buf %s", __func__, buf);
	if ((*value == '-' ) || (*value == '+' )) {
		value++;
	}

	while (*value != '\0') {
		if ((*value < '0') || (*value > '9')) {
			wpa_printf(MSG_ERROR, "%s : Invalid RSSI_FILTER value", __func__);
			return -1;
		}
		value++;
	}

	rssi_filter_value = atoi(buf);
	if ((rssi_filter_value < -128) || (rssi_filter_value > 127)) {
		wpa_printf(MSG_ERROR, "%s : Invalid RSSI_FILTER value", __func__);
		return -1;
	}

	wpa_s->rssi_filter_value = rssi_filter_value;
	wpa_supplicant_req_scan(wpa_s, 0, 0);
	return 0;
}
#endif

#ifdef CONFIG_DATA_PKT_FILTER
static int tokenize_cmd(char *cmd, char *argv[])
{
	char *pos;
	int argc = 0;

	pos = cmd;
	for (;;) {
		while (*pos == ' ')
			pos++;
		if (*pos == '\0')
			break;
		argv[argc] = pos;
		argc++;
		if (argc == max_args)
			break;
		while (*pos != '\0' && *pos != ' ')
			pos++;
		if (*pos == ' ')
			*pos++ = '\0';
	}
	return argc;
}

static int wpa_ste_set_filter(struct wpa_supplicant *wpa_s, char *buf)
{
	char *argv[max_args];
	int argc, i, nrfilter;
	struct set_filter filter;

	/*Driver expects all values initialized to zero*/
	os_memset((u8 *)&filter, 0, sizeof(struct set_filter));

	wpa_printf(MSG_DEBUG, "%s : %s\n", __func__, buf);

	argc = tokenize_cmd(buf, argv);

	filter.filter_id = atoi(argv[0]);
	wpa_printf(MSG_DEBUG, "filter.filter_id:%d\n", filter.filter_id);

	switch(filter.filter_id){

		case IPV4_FILTER:
				filter.filterdata.ipv4.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.ipv4.action_mode:%d\n",
					filter.filterdata.ipv4.action_mode);
				if (filter.filterdata.ipv4.action_mode == 0)
					break;
				nrfilter = filter.filterdata.ipv4.nrfilter = atoi(argv[2]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.ipv4.nrfilter:%d\n",
					filter.filterdata.ipv4.nrfilter);
				if(nrfilter <= 0 || nrfilter > 8){
					wpa_printf(MSG_DEBUG, "invalid no. of ip addresses\n");
					return -1;
				}
				for(i = 0; i < nrfilter; i++) {
					if (ipaddr_aton(argv[3 + i * 3],
							filter.filterdata.ipv4.ipv4addr[i].addr)){
						wpa_printf(MSG_DEBUG, "invalid ip address\n");
						return -1;
					}
					filter.filterdata.ipv4.ipv4addr[i].addr_mode =
						atoi(argv[4 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.ipv4.ipv4addr[%d]"
							".addr_mode:%d\n", i,
							filter.filterdata.ipv4.ipv4addr[i].addr_mode);
					filter.filterdata.ipv4.ipv4addr[i].filter_mode =
						atoi(argv[5 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.ipv4.ipv4addr[%d]."
							"filter_mode:%d\n", i,
							filter.filterdata.ipv4.ipv4addr[i].filter_mode);
				}
				break;

		case IPV6_FILTER:
				filter.filterdata.ipv6.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.ipv6.action_mode:%d\n",
					filter.filterdata.ipv6.action_mode);
				if (filter.filterdata.ipv6.action_mode == 0)
					break;
				nrfilter = filter.filterdata.ipv6.nrfilter = atoi(argv[2]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.ipv6.nrfilter:%d\n",
					filter.filterdata.ipv6.nrfilter);
				if(nrfilter <= 0 || nrfilter > 4){
					wpa_printf(MSG_DEBUG, "invalid no. of ip addresses\n");
					return -1;
				}
				for(i = 0; i < nrfilter; i++) {
					if (ip6addr_aton(argv[3 + i * 3],
							filter.filterdata.ipv6.ipv6addr[i].addr)){
						wpa_printf(MSG_DEBUG, "invalid ip address\n");
						return -1;
					}
					filter.filterdata.ipv6.ipv6addr[i].addr_mode =
						atoi(argv[4 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.ipv6.ipv6addr[%d]"
							".addr_mode:%d\n", i,
							filter.filterdata.ipv6.ipv6addr[i].addr_mode);
					filter.filterdata.ipv6.ipv6addr[i].filter_mode=
						atoi(argv[5 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.ipv6.ipv6addr[%d]."
							"filter_mode:%d\n", i,
							filter.filterdata.ipv6.ipv6addr[i].filter_mode);
				}
				break;

		case MAC_FILTER:
				filter.filterdata.mac.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.mac.action_mode:%d\n",
					filter.filterdata.mac.action_mode);
				if (filter.filterdata.mac.action_mode == 0)
					break;
				nrfilter = filter.filterdata.mac.nrfilter = atoi(argv[2]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.mac.nrfilter:%d\n",
					filter.filterdata.mac.nrfilter);
				if(nrfilter <= 0 || nrfilter > 8){
					wpa_printf(MSG_DEBUG, "invalid no. of mac addresses\n");
					return -1;
				}
				for(i = 0; i < nrfilter; i++) {
					if (hwaddr_aton(argv[3 + i * 3],
							filter.filterdata.mac.macaddr[i].addr)){
						wpa_printf(MSG_DEBUG, "invalid mac address\n");
						return -1;
					}
					wpa_printf(MSG_DEBUG, "filter.filterdata.mac"
						".macaddr[%d].addr:" MACSTR, i,
						MAC2STR(filter.filterdata.mac.macaddr[i].addr));
					filter.filterdata.mac.macaddr[i].addr_mode =
						atoi(argv[4 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.mac"
						".macaddr[%d].addr_mode:%d\n", i,
						filter.filterdata.mac.macaddr[i].addr_mode);
					filter.filterdata.mac.macaddr[i].filter_mode =
						atoi(argv[5 + i * 3]);
					wpa_printf(MSG_DEBUG, "filter.filterdata.mac.macaddr[%d]."
							"filter_mode:%d\n", i,
							filter.filterdata.ipv4.ipv4addr[i].filter_mode);
				}
				break;

		case GROUPMAC_FILTER:
				filter.filterdata.groupmac.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.groupmac.action_mode:%d\n",
					filter.filterdata.groupmac.action_mode);
				if (filter.filterdata.groupmac.action_mode == 0 ||
							filter.filterdata.groupmac.action_mode == 2)
					break;
				nrfilter = filter.filterdata.groupmac.nrfilter = atoi(argv[2]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.groupmac.nrfilter:%d\n",
					filter.filterdata.groupmac.nrfilter);
				if(nrfilter <= 0 || nrfilter > 8){
					wpa_printf(MSG_DEBUG, "invalid no. of mac addresses\n");
					return -1;
				}
				for(i = 0; i < nrfilter; i++) {
					if (hwaddr_aton(argv[3 + i],
							filter.filterdata.groupmac.groupmacaddr[i])){
						wpa_printf(MSG_DEBUG, "invalid groupmac address\n");
						return -1;
					}
					wpa_printf(MSG_DEBUG, "filter.filterdata.groupmac."
						"groupmacaddr[%d]:" MACSTR, i,
						MAC2STR(filter.filterdata.groupmac.groupmacaddr[i]));
				}
				break;

		case DHCP_FILTER:
				filter.filterdata.dhcp.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.dhcp.action_mode:%d\n",
					filter.filterdata.dhcp.action_mode);
				break;

		case SSDP_FILTER:
				filter.filterdata.ssdp.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.ssdp.action_mode:%d\n",
					filter.filterdata.ssdp.action_mode);
				break;

		case DISABLEALL_FILTER:
				filter.filterdata.disableall.action_mode = atoi(argv[1]);
				wpa_printf(MSG_DEBUG, "filter.filterdata.disableall.action_mode:%d\n",
					filter.filterdata.disableall.action_mode);
				break;

		default:
				wpa_printf(MSG_DEBUG, "invalid filter id\n");
				return -1;
		}

	return wpa_supplicant_ste_set_filter(wpa_s, &filter);
}
#endif

static int wpa_ste_get_num_channels(struct wpa_supplicant *wpa_s,
					char *reply, size_t reply_size)
{
	int num_channels = 13, ret = 0;
	char *pos, *end;
	char buf[3];

	pos = reply;
	end = pos + reply_size;

	if (reply_size < 3)
		return -1;

	if (wpa_supplicant_ste_get_country(wpa_s, buf))
		return -1;

	if (os_strncmp(buf, "US", 2) == 0)
		num_channels = 11;
	else if (os_strncmp(buf, "JP", 2) == 0)
		num_channels = 14;

	ret = os_snprintf(pos, end - pos, "%i", num_channels);
	if (ret < 0 || ret >= end - pos)
		return pos - reply;
	pos += ret;

	return pos - reply;
}

static int wpa_ste_set_num_channels(struct wpa_supplicant *wpa_s, const char *buf)
{
	int num_channels = atoi(buf);
	char alpha2[3];

	switch (num_channels) {
		case 11:
			strcpy(alpha2, "US");
			break;
		case 13:
			strcpy(alpha2, "EU");
			break;
		case 14:
			strcpy(alpha2, "JP");
			break;
		default:
			wpa_printf(MSG_WARNING, "Unsupported number of channels: %i", num_channels);
		return -1;
	}

	return wpa_supplicant_ste_set_country(wpa_s, alpha2);
}

static int wpa_ste_get_rssi(struct wpa_supplicant *wpa_s, char *reply, const size_t reply_size, int *reply_len)
{
	struct wpa_ssid *ssid = wpa_s->current_ssid;
	struct hostap_sta_driver_data data;

	if(ssid && (wpa_supplicant_ste_get_sta_data(wpa_s, &data))>=0) {
		*reply_len = os_snprintf(reply, reply_size, "%s rssi %d\n", wpa_ssid_txt(ssid->ssid, ssid->ssid_len), data.last_rssi);
		return 0;
	}
	else {
		return 0;
	}
}

static int wpa_ste_get_linkspeed(struct wpa_supplicant *wpa_s, char *reply, const size_t reply_size, int *reply_len)
{
	struct wpa_ssid *ssid = wpa_s->current_ssid;
	struct hostap_sta_driver_data data;

	if(ssid && (wpa_supplicant_ste_get_sta_data(wpa_s, &data))>=0) {
		*reply_len = os_snprintf(reply, reply_size, "LinkSpeed %lu\n", data.current_tx_rate / 10);
		return 0;
	}
	else {
		*reply_len = os_snprintf(reply, reply_size, "LinkSpeed 0\n");
		return 0;
	}
}

static int wpa_ste_driverstart(struct wpa_supplicant *wpa_s, Boolean interface_up)
{
	if(interface_up) {
		wpa_supplicant_ste_drv_start(wpa_s);
		wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_DRIVER_STATE "STARTED");
	}
	else {
		wpa_supplicant_ste_drv_stop(wpa_s);
		wpa_msg(wpa_s, MSG_INFO, WPA_EVENT_DRIVER_STATE "STOPPED");
	}

	return 0;
}

static int wpa_ste_powermode(struct wpa_supplicant *wpa_s, char *cmd)
{
	int powermode = atoi(cmd);

	return wpa_supplicant_ste_powermode(wpa_s, powermode);
}

int wpa_supplicant_ctrl_iface_process_ste_commands(struct wpa_supplicant *wpa_s,
					 char *buf, char *reply, const size_t reply_size)
{
	int reply_len = 3;
	os_memcpy(reply, "OK\n", 3);

	wpa_printf(MSG_DEBUG, "Command received: %s", buf);

	if (os_strncmp(buf, "DRIVER MACADDR", 14) == 0)	{
		reply_len = os_snprintf(reply, reply_size, "Macaddr = %02x:%02x:%02x:%02x:%02x:%02x\n",
		MAC2STR(wpa_s->own_addr));
	}
	else if (os_strncmp(buf, "DRIVER RSSI", 11) == 0 ||
		os_strncmp(buf, "DRIVER RSSI-APPROX", 18) == 0) {
		if(wpa_ste_get_rssi(wpa_s, reply, reply_size, &reply_len)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER LINKSPEED", 16) == 0) {
		if(wpa_ste_get_linkspeed(wpa_s, reply, reply_size, &reply_len)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER SCAN-CHANNELS ", 21) == 0 && strlen(buf) == 23) {
		if (wpa_ste_set_num_channels(wpa_s, buf + 21)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER START", 12) == 0) {
		if (wpa_ste_driverstart(wpa_s, TRUE)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER STOP", 11) == 0) {
		if (wpa_ste_driverstart(wpa_s, FALSE)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER SCAN-CHANNELS", 20) == 0) {
		wpa_printf(MSG_DEBUG, "Get num channels");
		return wpa_ste_get_num_channels(wpa_s, reply, reply_size);
	}
	else if (os_strncmp(buf, "GET COUNTRY", 11) == 0) {
		return wpa_ste_get_country(wpa_s, reply, reply_size);
	}
	else if (os_strncmp(buf, "SET COUNTRY ", 12) == 0) {
		if (wpa_ste_set_country(wpa_s, buf + 12)) {
			reply_len = -1;
		}
	}
	else if (os_strncmp(buf, "DRIVER COUNTRY ", 15) == 0) {
		if (wpa_ste_set_country(wpa_s, buf + 15)) {
			reply_len = -1;
		}
	}
#ifdef CONFIG_RSSI_FILTER
	else if (os_strncmp(buf, "RSSI_FILTER ", 12) == 0) {
		if (wpa_ste_set_rssi_filter(wpa_s, buf + 12)) {
			reply_len = -1;
                }
        }
#endif
#ifdef CONFIG_DATA_PKT_FILTER
	else if (os_strncmp(buf, "SET_FILTER ", 11) == 0) {
		if (wpa_ste_set_filter(wpa_s, buf + 11)) {
			reply_len = -1;
		}
	}
#endif
	else if (os_strncmp(buf, "DRIVER POWERMODE ", 17) == 0) {
		if (wpa_ste_powermode(wpa_s, buf + 17)){
			reply_len = -1;
		}
	}
	else {
		// Not an STE command - handle in ctrl_iface.c
		return 0;
	}

	// If we failed, make sure we reply FAIL
	if (reply_len < 0) {
		os_memcpy(reply, "FAIL\n", 5);
		reply_len = 5;
	}

	return reply_len;
}
