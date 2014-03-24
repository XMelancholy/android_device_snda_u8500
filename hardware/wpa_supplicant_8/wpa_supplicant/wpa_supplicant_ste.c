/*
 * ST-Ericsson hostap/wpa_supplicant changes and additions.
 *
 * Copyright (C) ST-Ericsson SA 2011
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
 */

#include "includes.h"

#include "common.h"
#include "rsn_supp/wpa.h"
#include "wpa_supplicant_i.h"
#include "wpa_supplicant_ste.h"
#include "config.h"
#include "driver_i.h"

#ifdef CONFIG_STE_AUTOMATIC_REGULATORY_DOMAIN

#include <sys/socket.h>
#include <sys/un.h>
#include "mcc2alpha2_ste.h"

//TODO: Socket path should be included, not explicitly defined here
#define ATC_SOCKET_PATH "/dev/socket/at_core"

#define MAX_AT_RESP_LENGHT    100
#define MAX_AT_CONN_ATTEMPTS  1

#define AT_CMD_COPS_PARAM_SET "AT+COPS=3,2\r"
#define AT_CMD_COPS_READ      "AT+COPS?\r"

#define AT_RESP_OK            "OK"

/* -------------------------- local prototypes ----------------------------- */

static int  wpas_ste_open_at_connection(void);
static void wpas_ste_close_at_connection(int fd);
static int  wpas_ste_send_at_command(int fd, char* cmd);
static int  wpas_ste_get_at_response(int fd, char* buf);
static int  wpas_ste_get_mcc(char* buf);
static int  wpas_ste_compare_and_set_country(struct wpa_supplicant *wpa_s, char* alpha2);
static int  wpas_ste_handle_at_message(int index, char* buf);

#endif /* CONFIG_STE_AUTOMATIC_REGULATORY_DOMAIN */

/* ------------------------- exported functions ---------------------------- */

#ifdef CONFIG_DATA_PKT_FILTER
/**
 * wpa_supplicant_ste_set_filter
 * @wpa_s: wpa_supplicant structure for a network interface
 * @filter: filter parameter structure
 * Returns: 0 if succeed or -1  driver returns an error
 *
 * This function is called to set data packet filter on device .
 */
int wpa_supplicant_ste_set_filter(struct wpa_supplicant *wpa_s, struct set_filter *filter)
{
	int i, ret;

	ret = wpa_drv_set_data_filter(wpa_s, (const u8 *) filter,
						sizeof(struct set_filter));
	if (ret){
		wpa_printf(MSG_DEBUG, "Failed to set filter\n");
		return -1;
	}
	wpa_printf(MSG_DEBUG, "filter command sent successfully\n");
	return 0;
}
#endif

/**
 * wpa_supplicant_ste_get_country - Get country code for interface
 * @wpa_s: wpa_supplicant structure for a network interface
 * @alpha2: pointer to country code
 * Returns: 0 if succeed or -1 if alpha2 has an invalid value
 *
 * This function is called to get country code for interface.
 */
int wpa_supplicant_ste_get_country(struct wpa_supplicant *wpa_s, char *alpha2)
{
	if (wpa_drv_get_country(wpa_s, alpha2)) {
		wpa_printf(MSG_DEBUG, "Failed to get country");
		return -1;
	}
	return 0;
}

/**
 * wpa_supplicant_ste_set_country - Set country code for interface
 * @wpa_s: wpa_supplicant structure for a network interface
 * @alpha2: pointer to country code
 * Returns: 0 if succeed or -1 if alpha2 has an invalid value or driver returns an error
 *
 * This function is called to set country code for interface.
 */
int wpa_supplicant_ste_set_country(struct wpa_supplicant *wpa_s, const char *alpha2)
{
	if (!alpha2[0] || !alpha2[1]) {
		wpa_printf(MSG_DEBUG, "Invalid country set");
		return -1;
	}

	if (wpa_drv_set_country(wpa_s, alpha2)) {
		wpa_printf(MSG_DEBUG, "Failed to set country");
		return -1;
	}
	wpa_s->conf->country[0] = alpha2[0];
	wpa_s->conf->country[1] = alpha2[1];

	wpa_printf(MSG_DEBUG, "Setting country='%c%c'",
	wpa_s->conf->country[0], wpa_s->conf->country[1]);

	return 0;
}

/**
 * wpa_supplicant_ste_get_sta_data - Retrieves station link infomation
 * @wpa_s: Pointer to wpa_supplicant data
 * @data:  hostap driver status information
 * Returns: 0 if succeed or -1 error
 *
 * This function is called to retrieve station link information.
 */
int wpa_supplicant_ste_get_sta_data(struct wpa_supplicant *wpa_s, struct hostap_sta_driver_data *data)
{
	return(wpa_drv_get_sta(wpa_s, data));
}

/**
 * wpa_supplicant_ste_powermode - Enable / disable powersave
 * @wpa_s wpa_supplicant structure for a network interface
 * @mode: 1 = active power mode (full power), 0 = auto power mode (power save)
 * Returns: 0 on success
 *
 * This function is called to enable or disable powersave
 */
int wpa_supplicant_ste_powermode(struct wpa_supplicant *wpa_s, int mode)
{
	return wpa_drv_set_p2p_powersave(wpa_s, !mode, -1, -1);
}

/**
 * wpa_supplicant_ste_drv_start - Start wlan driver
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is called to start wlan driver.
 */
void wpa_supplicant_ste_drv_start(struct wpa_supplicant *wpa_s)
{
	wpa_drv_resume(wpa_s);
}

/**
 * wpa_supplicant_ste_drv_stop - Stop wlan driver
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is called to stop wlan driver.
 */
void wpa_supplicant_ste_drv_stop(struct wpa_supplicant *wpa_s)
{
	wpa_drv_suspend(wpa_s);
}

#ifdef CONFIG_STE_AUTOMATIC_REGULATORY_DOMAIN
/**
 * wpa_supplicant_ste_get_country_code_from_at - Retrieves MCC from AT module
 * @wpa_s: wpa_supplicant structure for a network interface
 * Returns: 0 if succeed or -1 an error has occurred
 *
 * Map MCC to alpha2 code and set in supplicant's structure for network interface.
 * This information can be then send to the driver to fetch regulatory settings
 * for given country.
 */
int wpa_supplicant_ste_get_country_code_from_at(struct wpa_supplicant *wpa_s)
{
	int fd = 0;
	int country_code = 0;
	int result = -1;

	char buf[MAX_AT_RESP_LENGHT];
	char alpha2[2];

	//Connecting to AT_Core module
	fd = wpas_ste_open_at_connection();
	if (fd < 0) {
		goto error;
	}

	//Send AT+COPS=3,2 command to set response format
	if (wpas_ste_send_at_command(fd, AT_CMD_COPS_PARAM_SET) == 0) {
		//Read the response, and check if OK
		if (wpas_ste_get_at_response(fd, buf) != 0) {
			goto error;
		}
	} else {
		goto error;
	}

	//Send AT+COPS? to read the current settings
	if (wpas_ste_send_at_command(fd, AT_CMD_COPS_READ) == 0) {
		//Read the response, and check if OK
		if (wpas_ste_get_at_response(fd, buf) != 0) {
			goto error;
		}
	} else {
		goto error;
	}

	//Parse the AT+COPS response to get MCC
	country_code = wpas_ste_get_mcc(buf);
	if (country_code < 0) {
		goto error;
	}

	//Map MCC to alpha2 code
	if (mcc2alpha2_get(country_code, alpha2) == 0) {
		//If success, fill the wpa_s structure
		result = wpas_ste_compare_and_set_country(wpa_s, alpha2);
	}

error:
	wpas_ste_close_at_connection(fd);
	return result;
}

/* ---------------------------- Local functions ---------------------------- */

/*
 * wpas_ste_compare_and_set_country
 *
 * Compare country codes. If codes are different,
 * update the driver and wpa_s structure
 * Returns: 0 if succeed or -1 an error has occurred
 */
static int wpas_ste_compare_and_set_country(struct wpa_supplicant *wpa_s, char* alpha2)
{
	int result = 0;

	if (wpa_s->conf->country[0] != alpha2[0] ||
		wpa_s->conf->country[0] != alpha2[0]) {

		if (wpa_drv_set_country(wpa_s, alpha2)) {
			wpa_printf(MSG_DEBUG, "Failed to set country");
			result = -1;
		} else {
			wpa_s->conf->country[0] = alpha2[0];
			wpa_s->conf->country[1] = alpha2[1];

			wpa_printf(MSG_DEBUG, "Setting country='%c%c'",
			wpa_s->conf->country[0], wpa_s->conf->country[1]);
		}
	}

	return result;
}


/*
 * wpas_ste_open_at_connection
 *
 * Open socket connection to AT_Core.
 * Returns: socket descriptor when succed, -1 when failed
 */
static int wpas_ste_open_at_connection(void)
{
	int fd;
	int result = -1;

	if ((fd = socket(AF_UNIX, SOCK_SEQPACKET, 0)) < 0) {
		wpa_printf(MSG_DEBUG, "Socket failed (%s).", strerror(errno));
	} else {
		int conn_attempts = MAX_AT_CONN_ATTEMPTS;
		struct sockaddr_un remote;

		memset(&remote, 0, sizeof(struct sockaddr_un));
		remote.sun_family = AF_UNIX;
		strncpy(remote.sun_path, ATC_SOCKET_PATH, strlen(ATC_SOCKET_PATH));

		while (result < 0 && conn_attempts--) {
			result = connect(fd, (struct sockaddr *)&remote, sizeof(struct sockaddr_un));

			if (result < 0) {
				wpa_printf(MSG_DEBUG,
				"Connection failed on socket %s, retrying.. (%s)",
				remote.sun_path,
				strerror(errno));
				sleep(1);
			}
		}

		//If unable to connect. Return -1
		if(result < 0) {
			close(fd);
			fd = -1;
		}
	}

	return fd;
}

/*
 * wpas_ste_close_at_connection
 *
 * Close connection to AT_Core
 */
static void wpas_ste_close_at_connection(int fd)
{
	if (fd > 0) {
		close(fd);
	}
}

/*
 * wpas_ste_get_mcc
 *
 * Retrieves mobile country code from AT response.
 * Returns: Mobile Country Code (numeric) when succed, -1 error
 */
static int wpas_ste_get_mcc(char* buf)
{
	char* index;
	char  format;
	int   country_code = -1;

	if (NULL != (index = strchr(buf, ','))) {
		index++;
		while(!isdigit(*index)) {
			index++;
		}
		format  = *index;

		switch (format) {
		case '0' :
				wpa_printf(MSG_DEBUG, "long format");
				break;
		case '1' :
			wpa_printf(MSG_DEBUG, "short format");
			break;
		case '2' :
			/* Response from AT+COPS has numeric format which consists of a
			  * three BCD digit country code coded as in ITU-T E.212 Annex A [5],
			  * plus a two BCD digit network code */
			wpa_printf(MSG_DEBUG, "numeric format");
			if (NULL != (index = strchr(index,'"'))) {
				index++;

				/* Three digit BDC country code */
				country_code = (index[0]-48)*100 + (index[1]-48)*10 + (index[2]-48);
				wpa_printf(MSG_DEBUG,"MCC from AT %d\n", country_code);
			} else {
				wpa_printf(MSG_DEBUG, "ERROR while parsing PLMN code");
			}
			break;
		default:
			wpa_printf(MSG_DEBUG, "Internal parse error");
		}
	} else {
		wpa_printf(MSG_DEBUG, "Not registered to any network!");
	}

	return country_code;
}

/*
 * wpas_ste_send_at_command
 *
 * Send AT command
 * Returns: 0 if succeed or -1 an error has occurred
 */
static int wpas_ste_send_at_command(int fd, char* cmd)
{
	int result = 0;

	wpa_printf(MSG_DEBUG, "Sending AT command: %s", cmd);

	if (write(fd, cmd, strlen(cmd)) < 0) {
		result = -1;
	}

	return result;
}

/*
 * wpas_ste_get_at_response
 *
 * Get AT response
 * Returns: 0 if succeed or -1 an error has occurred
 */
static int wpas_ste_get_at_response(int fd, char* buf)
{
	int    index  = 0;
	int    result = 0;
	int    buf_length;
	int    max_len = MAX_AT_RESP_LENGHT;
	int    ret;
	fd_set set;

	//Set timeout value
	struct timeval timeout = { .tv_sec = 0, .tv_usec = 50000 };

	FD_ZERO(&set);
	FD_SET(fd, &set);

	while(1) {
		ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

		if (ret > 0 && FD_ISSET(fd, &set)) {
			buf_length = recv(fd, &buf[index], max_len - index, 0);

			if (buf_length > 0) {
				index += buf_length;
			} else {
				wpa_printf(MSG_DEBUG, "recv() error");
				result = -1;
				break;
			}
		} else if (ret == 0) {
			//Nothing to read. break
			break;
		} else {
			wpa_printf(MSG_DEBUG, "select() failed");
			result = -1;
			break;
		}
	}

	if (result == 0) {
		result = wpas_ste_handle_at_message(index, buf);
	}

	return result;
}

/*
 * wpas_ste_handle_at_message
 *
 * Handle the message
 * Check if we read anything and  the buf max size
 *
 */
static int wpas_ste_handle_at_message(int index, char* buf)
{
	int result = 0;

	if (index > 0) {
		if (index == MAX_AT_RESP_LENGHT) {
			index = MAX_AT_RESP_LENGHT - 1;
		}
		buf[index] = '\0';

		while(--index >= 0) {
			buf[index] = (char)toupper(buf[index]);
		}

		//Print message
		wpa_printf(MSG_DEBUG, "AT response: %s", buf);

		if (NULL == strstr(buf, AT_RESP_OK)) {
			wpa_printf(MSG_DEBUG, "Response not OK");
			result = -1;
		}
	} else {
		wpa_printf(MSG_DEBUG, "No response from AT. Timeout");
		result = -1;
	}

	return result;
}

#endif /* CONFIG_STE_AUTOMATIC_REGULATORY_DOMAIN */
