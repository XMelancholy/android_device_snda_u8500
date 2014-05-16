/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/mgmt.h"
#include "src/shared/mgmt.h"
#include "src/glib-helper.h"
#include "src/eir.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "src/sdp-client.h"
#include "src/sdpd.h"
#include "log.h"
#include "hal-msg.h"
#include "ipc.h"
#include "utils.h"
#include "bluetooth.h"

#define DEFAULT_ADAPTER_NAME "BlueZ for Android"

#define DUT_MODE_FILE "/sys/kernel/debug/bluetooth/hci%u/dut_mode"

#define DEVICE_ID_SOURCE	0x0002	/* USB */
#define DEVICE_ID_VENDOR	0x1d6b	/* Linux Foundation */
#define DEVICE_ID_PRODUCT	0x0247	/* BlueZ for Android */

#define ADAPTER_MAJOR_CLASS 0x02 /* Phone */
#define ADAPTER_MINOR_CLASS 0x03 /* Smartphone */

/* Default to DisplayYesNo */
#define DEFAULT_IO_CAPABILITY 0x01

/* Default discoverable timeout 120sec as in Android */
#define DEFAULT_DISCOVERABLE_TIMEOUT 120

#define BASELEN_PROP_CHANGED (sizeof(struct hal_ev_adapter_props_changed) \
					+ (sizeof(struct hal_property)))

static uint16_t option_index = MGMT_INDEX_NONE;

#define BASELEN_REMOTE_DEV_PROP (sizeof(struct hal_ev_remote_device_props) \
					+ sizeof(struct hal_property))

struct device {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;

	int bond_state;

	char *name;
	char *friendly_name;

	uint32_t class;
	int32_t rssi;

	uint32_t timestamp;

	GSList *uuids;

	bool found; /* if device is found in current discovery session */
};

struct browse_req {
	bdaddr_t bdaddr;
	GSList *uuids;
	int search_uuid;
	int reconnect_attempt;
};

static struct {
	uint16_t index;

	bdaddr_t bdaddr;
	uint32_t dev_class;

	char *name;

	uint32_t current_settings;

	bool discovering;
	uint32_t discoverable_timeout;

	GSList *uuids;
} adapter = {
	.index = MGMT_INDEX_NONE,
	.dev_class = 0,
	.name = NULL,
	.current_settings = 0,
	.discovering = false,
	.discoverable_timeout = DEFAULT_DISCOVERABLE_TIMEOUT,
	.uuids = NULL,
};

static const uint16_t uuid_list[] = {
	L2CAP_UUID,
	PNP_INFO_SVCLASS_ID,
	PUBLIC_BROWSE_GROUP,
	0
};

static struct mgmt *mgmt_if = NULL;
static GSList *devices = NULL;

/* This list contains addresses which are asked for records */
static GSList *browse_reqs;

static void store_adapter_config(void)
{
	GKeyFile *key_file;
	gsize length = 0;
	char addr[18];
	char *data;

	key_file = g_key_file_new();

	g_key_file_load_from_file(key_file, ANDROID_STORAGEDIR"/settings", 0,
									NULL);

	ba2str(&adapter.bdaddr, addr);

	g_key_file_set_string(key_file, "General", "Address", addr);
	g_key_file_set_string(key_file, "General", "Name", adapter.name);
	g_key_file_set_integer(key_file, "General", "DiscoverableTimeout",
						adapter.discoverable_timeout);

	data = g_key_file_to_data(key_file, &length, NULL);

	g_file_set_contents(ANDROID_STORAGEDIR"/settings", data, length, NULL);

	g_free(data);
	g_key_file_free(key_file);
}

static void load_adapter_config(void)
{
	GError *gerr = NULL;
	GKeyFile *key_file;
	char *str;

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, ANDROID_STORAGEDIR"/settings", 0,
									NULL);

	str = g_key_file_get_string(key_file, "General", "Address", NULL);
	if (!str) {
		g_key_file_free(key_file);
		return;
	}

	str2ba(str, &adapter.bdaddr);
	g_free(str);

	adapter.name = g_key_file_get_string(key_file, "General", "Name", NULL);

	adapter.discoverable_timeout = g_key_file_get_integer(key_file,
				"General", "DiscoverableTimeout", &gerr);
	if (gerr) {
		adapter.discoverable_timeout = DEFAULT_DISCOVERABLE_TIMEOUT;
		g_error_free(gerr);
		gerr = NULL;
	}

	g_key_file_free(key_file);
}

static void store_device_info(struct device *dev)
{
	GKeyFile *key_file;
	char addr[18];
	gsize length = 0;
	char **uuids = NULL;
	char *str;

	/* We only store bonded devices and need to modify the storage
	 * if the state is either NONE or BONDED.
	 */
	if (dev->bond_state != HAL_BOND_STATE_BONDED &&
					dev->bond_state != HAL_BOND_STATE_NONE)
		return;

	ba2str(&dev->bdaddr, addr);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, ANDROID_STORAGEDIR"/devices", 0,
									NULL);

	if (dev->bond_state == HAL_BOND_STATE_NONE) {
		g_key_file_remove_group(key_file, addr, NULL);
		goto done;
	}

	g_key_file_set_integer(key_file, addr, "Type", dev->bdaddr_type);

	g_key_file_set_string(key_file, addr, "Name", dev->name);

	if (dev->friendly_name)
		g_key_file_set_string(key_file, addr, "FriendlyName",
							dev->friendly_name);
	else
		g_key_file_remove_key(key_file, addr, "FriendlyName", NULL);

	if (dev->class)
		g_key_file_set_integer(key_file, addr, "Class", dev->class);
	else
		g_key_file_remove_key(key_file, addr, "Class", NULL);

	g_key_file_set_integer(key_file, addr, "Timestamp", dev->timestamp);

	if (dev->uuids) {
		GSList *l;
		int i;

		uuids = g_new0(char *, g_slist_length(dev->uuids) + 1);

		for (i = 0, l = dev->uuids; l; l = g_slist_next(l), i++) {
			int j;
			uint8_t *u = l->data;
			char *uuid_str = g_malloc0(33);

			for (j = 0; j < 16; j++)
				sprintf(uuid_str + (j * 2), "%2.2X", u[j]);

			uuids[i] = uuid_str;
		}

		g_key_file_set_string_list(key_file, addr, "Services",
						(const char **)uuids, i);
	} else {
		g_key_file_remove_key(key_file, addr, "Services", NULL);
	}

done:
	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(ANDROID_STORAGEDIR"/devices", str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
	g_strfreev(uuids);
}

static int device_match(gconstpointer a, gconstpointer b)
{
	const struct device *dev = a;
	const bdaddr_t *bdaddr = b;

	return bacmp(&dev->bdaddr, bdaddr);
}

static struct device *find_device(const bdaddr_t *bdaddr)
{
	GSList *l;

	l = g_slist_find_custom(devices, bdaddr, device_match);
	if (l)
		return l->data;

	return NULL;
}

static struct device *create_device(const bdaddr_t *bdaddr, uint8_t type)
{
	struct device *dev;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("%s", addr);

	dev = g_new0(struct device, 1);

	bacpy(&dev->bdaddr, bdaddr);
	dev->bdaddr_type = type;
	dev->bond_state = HAL_BOND_STATE_NONE;
	dev->timestamp = time(NULL);

	/* use address for name, will be change if one is present
	 * eg. in EIR or set by set_property. */
	dev->name = g_strdup(addr);
	devices = g_slist_prepend(devices, dev);

	return dev;
}

static void free_device(struct device *dev)
{
	g_free(dev->name);
	g_free(dev->friendly_name);
	g_slist_free_full(dev->uuids, g_free);
	g_free(dev);
}

static struct device *get_device(const bdaddr_t *bdaddr, uint8_t type)
{
	struct device *dev;

	dev = find_device(bdaddr);
	if (dev)
		return dev;

	return create_device(bdaddr, type);
}

static  void send_adapter_property(uint8_t type, uint16_t len, const void *val)
{
	uint8_t buf[BASELEN_PROP_CHANGED + len];
	struct hal_ev_adapter_props_changed *ev = (void *) buf;

	ev->status = HAL_STATUS_SUCCESS;
	ev->num_props = 1;
	ev->props[0].type = type;
	ev->props[0].len = len;
	memcpy(ev->props[0].val, val, len);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_ADAPTER_PROPS_CHANGED,
							sizeof(buf), buf);
}

static void adapter_name_changed(const uint8_t *name)
{
	/* Android expects string value without NULL terminator */
	send_adapter_property(HAL_PROP_ADAPTER_NAME,
					strlen((const char *) name), name);
}

static void adapter_set_name(const uint8_t *name)
{
	if (!g_strcmp0(adapter.name, (const char *) name))
		return;

	DBG("%s", name);

	g_free(adapter.name);
	adapter.name = g_strdup((const char *) name);

	store_adapter_config();

	adapter_name_changed(name);
}

static void mgmt_local_name_changed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_cp_set_local_name *rp = param;

	if (length < sizeof(*rp)) {
		error("Wrong size of local name changed parameters");
		return;
	}

	adapter_set_name(rp->name);

	/* TODO Update services if needed */
}

static void powered_changed(void)
{
	struct hal_ev_adapter_state_changed ev;

	ev.state = (adapter.current_settings & MGMT_SETTING_POWERED) ?
						HAL_POWER_ON : HAL_POWER_OFF;

	DBG("%u", ev.state);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_ADAPTER_STATE_CHANGED,
							sizeof(ev), &ev);
}

static uint8_t settings2scan_mode(void)
{
	bool connectable, discoverable;

	connectable = adapter.current_settings & MGMT_SETTING_CONNECTABLE;
	discoverable = adapter.current_settings & MGMT_SETTING_DISCOVERABLE;

	if (connectable && discoverable)
		return HAL_ADAPTER_SCAN_MODE_CONN_DISC;

	if (connectable)
		return HAL_ADAPTER_SCAN_MODE_CONN;

	return HAL_ADAPTER_SCAN_MODE_NONE;
}

static void scan_mode_changed(void)
{
	uint8_t mode;

	mode = settings2scan_mode();

	DBG("mode %u", mode);

	send_adapter_property(HAL_PROP_ADAPTER_SCAN_MODE, sizeof(mode), &mode);
}

static void adapter_class_changed(void)
{
	send_adapter_property(HAL_PROP_ADAPTER_CLASS, sizeof(adapter.dev_class),
							&adapter.dev_class);
}

static void settings_changed(uint32_t settings)
{
	uint32_t changed_mask;
	uint32_t scan_mode_mask;

	changed_mask = adapter.current_settings ^ settings;

	adapter.current_settings = settings;

	DBG("0x%08x", changed_mask);

	if (changed_mask & MGMT_SETTING_POWERED)
		powered_changed();


	scan_mode_mask = MGMT_SETTING_CONNECTABLE |
					MGMT_SETTING_DISCOVERABLE;

	/*
	 * Only when powered, the connectable and discoverable
	 * state changes should be communicated.
	 */
	if (adapter.current_settings & MGMT_SETTING_POWERED)
		if (changed_mask & scan_mode_mask)
			scan_mode_changed();
}

static void new_settings_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	uint32_t settings;

	if (length < sizeof(settings)) {
		error("Wrong size of new settings parameters");
		return;
	}

	settings = bt_get_le32(param);

	DBG("settings: 0x%8.8x -> 0x%8.8x", adapter.current_settings,
								settings);

	if (settings == adapter.current_settings)
		return;

	settings_changed(settings);
}

static void mgmt_dev_class_changed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_cod *rp = param;
	uint32_t dev_class;

	if (length < sizeof(*rp)) {
		error("Wrong size of class of device changed parameters");
		return;
	}

	dev_class = rp->val[0] | (rp->val[1] << 8) | (rp->val[2] << 16);

	if (dev_class == adapter.dev_class)
		return;

	DBG("Class: 0x%06x", dev_class);

	adapter.dev_class = dev_class;

	adapter_class_changed();

	/* TODO: Gatt attrib set*/
}

static void store_link_key(const bdaddr_t *dst, const uint8_t *key,
					uint8_t type, uint8_t pin_length)
{
	GKeyFile *key_file;
	char key_str[33];
	gsize length = 0;
	char addr[18];
	char *data;
	int i;

	key_file = g_key_file_new();

	if (!g_key_file_load_from_file(key_file, ANDROID_STORAGEDIR"/devices",
								0, NULL))
		return;

	ba2str(dst, addr);

	DBG("%s type %u pin_len %u", addr, type, pin_length);

	for (i = 0; i < 16; i++)
		sprintf(key_str + (i * 2), "%2.2X", key[i]);

	g_key_file_set_string(key_file, addr, "LinkKey", key_str);
	g_key_file_set_integer(key_file, addr, "LinkKeyType", type);
	g_key_file_set_integer(key_file, addr, "LinkKeyPinLength", pin_length);

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(ANDROID_STORAGEDIR"/devices", data, length, NULL);
	g_free(data);

	g_key_file_free(key_file);
}

static void send_bond_state_change(const bdaddr_t *addr, uint8_t status,
								uint8_t state)
{
	struct hal_ev_bond_state_changed ev;

	ev.status = status;
	ev.state = state;
	bdaddr2android(addr, ev.bdaddr);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_BOND_STATE_CHANGED,
							sizeof(ev), &ev);
}

static void set_device_bond_state(const bdaddr_t *addr, uint8_t status,
								int state)
{
	struct device *dev;

	dev = find_device(addr);
	if (!dev)
		return;

	if (dev->bond_state != state) {
		dev->bond_state = state;
		send_bond_state_change(&dev->bdaddr, status, state);

		store_device_info(dev);
	}
}

static  void send_device_property(const bdaddr_t *bdaddr, uint8_t type,
						uint16_t len, const void *val)
{
	uint8_t buf[BASELEN_REMOTE_DEV_PROP + len];
	struct hal_ev_remote_device_props *ev = (void *) buf;

	ev->status = HAL_STATUS_SUCCESS;
	bdaddr2android(bdaddr, ev->bdaddr);
	ev->num_props = 1;
	ev->props[0].type = type;
	ev->props[0].len = len;
	memcpy(ev->props[0].val, val, len);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_REMOTE_DEVICE_PROPS,
							sizeof(buf), buf);
}

static void send_device_uuids_notif(struct device *dev)
{
	uint8_t buf[sizeof(uint128_t) * g_slist_length(dev->uuids)];
	uint8_t *ptr = buf;
	GSList *l;

	for (l = dev->uuids; l; l = g_slist_next(l)) {
		memcpy(ptr, l->data, sizeof(uint128_t));
		ptr += sizeof(uint128_t);
	}

	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_UUIDS, sizeof(buf),
									buf);
}

static void set_device_uuids(struct device *dev, GSList *uuids)
{
	g_slist_free_full(dev->uuids, g_free);
	dev->uuids = uuids;

	store_device_info(dev);

	send_device_uuids_notif(dev);
}

static void browse_req_free(struct browse_req *req)
{
	g_slist_free_full(req->uuids, g_free);
	g_free(req);
}

static int uuid_128_cmp(gconstpointer a, gconstpointer b)
{
	return memcmp(a, b, sizeof(uint128_t));
}

static void update_records(struct browse_req *req, sdp_list_t *recs)
{
	for (; recs; recs = recs->next) {
		sdp_record_t *rec = (sdp_record_t *) recs->data;
		sdp_list_t *svcclass = NULL;
		uuid_t uuid128;
		uuid_t *tmp;
		uint8_t *new_uuid;

		if (!rec)
			break;

		if (sdp_get_service_classes(rec, &svcclass) < 0)
			continue;

		if (!svcclass)
			continue;

		tmp = svcclass->data;

		switch (tmp->type) {
		case SDP_UUID16:
			sdp_uuid16_to_uuid128(&uuid128, tmp);
			break;
		case SDP_UUID32:
			sdp_uuid32_to_uuid128(&uuid128, tmp);
			break;
		case SDP_UUID128:
			memcpy(&uuid128, tmp, sizeof(uuid_t));
			break;
		default:
			sdp_list_free(svcclass, free);
			continue;
		}

		new_uuid = g_malloc(16);/* size of 128 bit uuid */
		memcpy(new_uuid, &uuid128.value.uuid128,
				sizeof(uuid128.value.uuid128));

		/* Check if uuid is already added */
		if (g_slist_find_custom(req->uuids, new_uuid, uuid_128_cmp))
			g_free(new_uuid);
		else
			req->uuids = g_slist_append(req->uuids, new_uuid);

		sdp_list_free(svcclass, free);
	}
}

static void browse_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct browse_req *req = user_data;
	struct device *dev;
	uuid_t uuid;

	/* If we have a valid response and req->search_uuid == 2, then L2CAP
	 * UUID & PNP searching was successful -- we are done */
	if (err < 0 || req->search_uuid == 2) {
		if (err == -ECONNRESET && req->reconnect_attempt < 1) {
			req->search_uuid--;
			req->reconnect_attempt++;
		} else {
			goto done;
		}
	}

	update_records(req, recs);

	/* Search for mandatory uuids */
	if (uuid_list[req->search_uuid]) {
		sdp_uuid16_create(&uuid, uuid_list[req->search_uuid++]);
		bt_search_service(&adapter.bdaddr, &req->bdaddr, &uuid,
						browse_cb, user_data, NULL, 0);
		return;
	}

done:
	dev = find_device(&req->bdaddr);
	if (dev) {
		set_device_uuids(dev, req->uuids);
		req->uuids = NULL;
	}

	browse_reqs = g_slist_remove(browse_reqs, req);
	browse_req_free(req);
}

static int req_cmp(gconstpointer a, gconstpointer b)
{
	const struct browse_req *req = a;
	const bdaddr_t *bdaddr = b;

	return bacmp(&req->bdaddr, bdaddr);
}

static uint8_t browse_remote_sdp(const bdaddr_t *addr)
{
	struct browse_req *req;
	uuid_t uuid;

	if (g_slist_find_custom(browse_reqs, addr, req_cmp))
		return HAL_STATUS_SUCCESS;

	req = g_new0(struct browse_req, 1);
	bacpy(&req->bdaddr, addr);
	sdp_uuid16_create(&uuid, uuid_list[req->search_uuid++]);

	if (bt_search_service(&adapter.bdaddr,
			&req->bdaddr, &uuid, browse_cb, req, NULL , 0) < 0) {
		browse_req_free(req);
		return HAL_STATUS_FAILED;
	}

	browse_reqs = g_slist_append(browse_reqs, req);

	return HAL_STATUS_SUCCESS;
}

static void new_link_key_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_link_key *ev = param;
	const struct mgmt_addr_info *addr = &ev->key.addr;
	char dst[18];

	if (length < sizeof(*ev)) {
		error("Too small new link key event");
		return;
	}

	ba2str(&addr->bdaddr, dst);

	DBG("new key for %s type %u pin_len %u",
					dst, ev->key.type, ev->key.pin_len);

	if (ev->key.pin_len > 16) {
		error("Invalid PIN length (%u) in new_key event",
							ev->key.pin_len);
		return;
	}

	set_device_bond_state(&addr->bdaddr, HAL_STATUS_SUCCESS,
							HAL_BOND_STATE_BONDED);

	if (ev->store_hint) {
		const struct mgmt_link_key_info *key = &ev->key;

		store_link_key(&addr->bdaddr, key->val, key->type,
								key->pin_len);
	}

	browse_remote_sdp(&addr->bdaddr);
}

static uint8_t get_device_name(struct device *dev)
{
	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_NAME,
						strlen(dev->name), dev->name);

	return HAL_STATUS_SUCCESS;
}

static void pin_code_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_pin_code_request *ev = param;
	struct hal_ev_pin_request hal_ev;
	struct device *dev;
	char dst[18];

	if (length < sizeof(*ev)) {
		error("Too small PIN code request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, dst);

	dev = get_device(&ev->addr.bdaddr, BDADDR_BREDR);

	/* Workaround for Android Bluetooth.apk issue: send remote
	 * device property */
	get_device_name(dev);

	set_device_bond_state(&ev->addr.bdaddr, HAL_STATUS_SUCCESS,
						HAL_BOND_STATE_BONDING);

	DBG("%s type %u secure %u", dst, ev->addr.type, ev->secure);

	/* Name already sent in remote device prop */
	memset(&hal_ev, 0, sizeof(hal_ev));
	bdaddr2android(&ev->addr.bdaddr, hal_ev.bdaddr);
	hal_ev.class_of_dev = dev->class;

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_PIN_REQUEST,
						sizeof(hal_ev), &hal_ev);
}

static void send_ssp_request(const bdaddr_t *addr, uint8_t variant,
							uint32_t passkey)
{
	struct hal_ev_ssp_request ev;

	/* It is ok to have empty name and CoD of remote devices here since
	* those information has been already provided on device_connected event
	* or during device scaning. Android will use that instead.
	*/
	memset(&ev, 0, sizeof(ev));
	bdaddr2android(addr, ev.bdaddr);
	ev.pairing_variant = variant;
	ev.passkey = passkey;

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_SSP_REQUEST,
							sizeof(ev), &ev);
}

static void user_confirm_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_user_confirm_request *ev = param;
	char dst[18];

	if (length < sizeof(*ev)) {
		error("Too small user confirm request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, dst);
	DBG("%s confirm_hint %u", dst, ev->confirm_hint);

	set_device_bond_state(&ev->addr.bdaddr, HAL_STATUS_SUCCESS,
						HAL_BOND_STATE_BONDING);

	if (ev->confirm_hint)
		send_ssp_request(&ev->addr.bdaddr, HAL_SSP_VARIANT_CONSENT, 0);
	else
		send_ssp_request(&ev->addr.bdaddr, HAL_SSP_VARIANT_CONFIRM,
								ev->value);
}

static void user_passkey_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_user_passkey_request *ev = param;
	char dst[18];

	if (length < sizeof(*ev)) {
		error("Too small passkey request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, dst);
	DBG("%s", dst);

	set_device_bond_state(&ev->addr.bdaddr, HAL_STATUS_SUCCESS,
						HAL_BOND_STATE_BONDING);

	send_ssp_request(&ev->addr.bdaddr, HAL_SSP_VARIANT_ENTRY, 0);
}

static void user_passkey_notify_callback(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	const struct mgmt_ev_passkey_notify *ev = param;
	char dst[18];

	if (length < sizeof(*ev)) {
		error("Too small passkey notify event");
		return;
	}

	ba2str(&ev->addr.bdaddr, dst);
	DBG("%s entered %u", dst, ev->entered);

	/* HAL seems to not support entered characters */
	if (ev->entered)
		return;

	set_device_bond_state(&ev->addr.bdaddr, HAL_STATUS_SUCCESS,
						HAL_BOND_STATE_BONDING);

	send_ssp_request(&ev->addr.bdaddr, HAL_SSP_VARIANT_NOTIF, ev->passkey);
}

static void clear_device_found(gpointer data, gpointer user_data)
{
	struct device *dev = data;

	dev->found = false;
}

static void mgmt_discovering_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_discovering *ev = param;
	struct hal_ev_discovery_state_changed cp;

	if (length < sizeof(*ev)) {
		error("Too small discovering event");
		return;
	}

	DBG("hci%u type %u discovering %u", index, ev->type,
							ev->discovering);

	if (adapter.discovering == !!ev->discovering)
		return;

	adapter.discovering = !!ev->discovering;

	DBG("new discovering state %u", ev->discovering);

	if (adapter.discovering) {
		cp.state = HAL_DISCOVERY_STATE_STARTED;
	} else {
		g_slist_foreach(devices, clear_device_found, NULL);
		cp.state = HAL_DISCOVERY_STATE_STOPPED;
	}

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH,
			HAL_EV_DISCOVERY_STATE_CHANGED, sizeof(cp), &cp);
}

static void confirm_device_name(const bdaddr_t *addr, uint8_t addr_type)
{
	struct mgmt_cp_confirm_name cp;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, addr);
	cp.addr.type = addr_type;

	if (mgmt_reply(mgmt_if, MGMT_OP_CONFIRM_NAME, adapter.index,
				sizeof(cp), &cp, NULL, NULL, NULL) == 0)
		error("Failed to send confirm name request");
}

static int fill_hal_prop(void *buf, uint8_t type, uint16_t len,
							const void *val)
{
	struct hal_property *prop = buf;

	prop->type = type;
	prop->len = len;
	memcpy(prop->val, val, len);

	return sizeof(*prop) + len;
}

static uint8_t bdaddr_type2android(uint8_t type)
{
	if (type == BDADDR_BREDR)
		return HAL_TYPE_BREDR;

	return HAL_TYPE_LE;
}

static void update_found_device(const bdaddr_t *bdaddr, uint8_t bdaddr_type,
					int8_t rssi, bool confirm,
					const uint8_t *data, uint8_t data_len)
{
	uint8_t buf[BLUEZ_HAL_MTU];
	struct eir_data eir;
	struct device *dev;
	uint8_t *num_prop;
	uint8_t opcode;
	int size = 0;

	memset(buf, 0, sizeof(buf));
	memset(&eir, 0, sizeof(eir));

	eir_parse(&eir, data, data_len);

	dev = find_device(bdaddr);

	/*
	 * Device found event needs to be send also for known device if this is
	 * new discovery session. Otherwise framework will ignore it.
	 */
	if (!dev || !dev->found) {
		struct hal_ev_device_found *ev = (void *) buf;
		bdaddr_t android_bdaddr;
		uint8_t android_type;

		if (!dev)
			dev = create_device(bdaddr, bdaddr_type);

		dev->found = true;

		size += sizeof(*ev);

		num_prop = &ev->num_props;
		opcode = HAL_EV_DEVICE_FOUND;

		bdaddr2android(bdaddr, &android_bdaddr);

		size += fill_hal_prop(buf + size, HAL_PROP_DEVICE_ADDR,
							sizeof(android_bdaddr),
							&android_bdaddr);
		(*num_prop)++;

		android_type = bdaddr_type2android(dev->bdaddr_type);
		size += fill_hal_prop(buf + size, HAL_PROP_DEVICE_TYPE,
					sizeof(android_type), &android_type);
		(*num_prop)++;
	} else {
		struct hal_ev_remote_device_props *ev = (void *) buf;

		size += sizeof(*ev);

		num_prop = &ev->num_props;
		opcode = HAL_EV_REMOTE_DEVICE_PROPS;

		ev->status = HAL_STATUS_SUCCESS;
		bdaddr2android(bdaddr, ev->bdaddr);

		dev->timestamp = time(NULL);
	}

	if (eir.class) {
		dev->class = eir.class;

		size += fill_hal_prop(buf + size, HAL_PROP_DEVICE_CLASS,
						sizeof(eir.class), &eir.class);
		(*num_prop)++;
	}

	if (rssi) {
		dev->rssi = rssi;

		size += fill_hal_prop(buf + size, HAL_PROP_DEVICE_RSSI,
						sizeof(dev->rssi), &dev->rssi);
		(*num_prop)++;
	}

	if (eir.name) {
		g_free(dev->name);
		dev->name = g_strdup(eir.name);

		size += fill_hal_prop(buf + size, HAL_PROP_DEVICE_NAME,
						strlen(eir.name), eir.name);
		(*num_prop)++;
	}

	if (*num_prop)
		ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, opcode, size, buf);

	if (confirm) {
		char addr[18];

		ba2str(bdaddr, addr);
		info("Device %s needs name confirmation.", addr);
		confirm_device_name(bdaddr, bdaddr_type);
	}

	eir_data_free(&eir);
}

static void mgmt_device_found_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_found *ev = param;
	const uint8_t *eir;
	uint16_t eir_len;
	uint32_t flags;
	bool confirm_name;
	char addr[18];

	if (length < sizeof(*ev)) {
		error("Too short device found event (%u bytes)", length);
		return;
	}

	eir_len = btohs(ev->eir_len);
	if (length != sizeof(*ev) + eir_len) {
		error("Device found event size mismatch (%u != %zu)",
					length, sizeof(*ev) + eir_len);
		return;
	}

	if (eir_len == 0)
		eir = NULL;
	else
		eir = ev->eir;

	flags = btohl(ev->flags);

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u addr %s, rssi %d flags 0x%04x eir_len %u",
				index, addr, ev->rssi, flags, eir_len);

	confirm_name = flags & MGMT_DEV_FOUND_CONFIRM_NAME;

	update_found_device(&ev->addr.bdaddr, ev->addr.type, ev->rssi,
						confirm_name, eir, eir_len);
}

static void mgmt_device_connected_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_connected *ev = param;
	struct hal_ev_acl_state_changed hal_ev;

	if (length < sizeof(*ev)) {
		error("Too short device connected event (%u bytes)", length);
		return;
	}

	update_found_device(&ev->addr.bdaddr, ev->addr.type, 0, false,
					&ev->eir[0], btohs(ev->eir_len));

	hal_ev.status = HAL_STATUS_SUCCESS;
	hal_ev.state = HAL_ACL_STATE_CONNECTED;
	bdaddr2android(&ev->addr.bdaddr, hal_ev.bdaddr);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_ACL_STATE_CHANGED,
						sizeof(hal_ev), &hal_ev);
}

static void mgmt_device_disconnected_event(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	const struct mgmt_ev_device_disconnected *ev = param;
	struct hal_ev_acl_state_changed hal_ev;

	if (length < sizeof(*ev)) {
		error("Too short device disconnected event (%u bytes)", length);
		return;
	}

	hal_ev.status = HAL_STATUS_SUCCESS;
	hal_ev.state = HAL_ACL_STATE_DISCONNECTED;
	bdaddr2android(&ev->addr.bdaddr, hal_ev.bdaddr);

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_ACL_STATE_CHANGED,
						sizeof(hal_ev), &hal_ev);
}

static uint8_t status_mgmt2hal(uint8_t mgmt)
{
	switch (mgmt) {
	case MGMT_STATUS_SUCCESS:
		return HAL_STATUS_SUCCESS;
	case MGMT_STATUS_NO_RESOURCES:
		return HAL_STATUS_NOMEM;
	case MGMT_STATUS_BUSY:
		return HAL_STATUS_BUSY;
	case MGMT_STATUS_NOT_SUPPORTED:
		return HAL_STATUS_UNSUPPORTED;
	case MGMT_STATUS_INVALID_PARAMS:
		return HAL_STATUS_INVALID;
	case MGMT_STATUS_AUTH_FAILED:
		return HAL_STATUS_AUTH_FAILURE;
	case MGMT_STATUS_NOT_CONNECTED:
		return HAL_STATUS_REMOTE_DEVICE_DOWN;
	default:
		return HAL_STATUS_FAILED;
	}
}

static void mgmt_connect_failed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_connect_failed *ev = param;

	DBG("");

	/* In case security mode 3 pairing we will get connect failed event
	* in case e.g wrong PIN code entered. Let's check if device is
	* bonding, if so update bond state */
	set_device_bond_state(&ev->addr.bdaddr, status_mgmt2hal(ev->status),
							HAL_BOND_STATE_NONE);
}

static void mgmt_auth_failed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_auth_failed *ev = param;

	DBG("");

	set_device_bond_state(&ev->addr.bdaddr, status_mgmt2hal(ev->status),
							HAL_BOND_STATE_NONE);
}

static void mgmt_device_unpaired_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	DBG("");
}

static void register_mgmt_handlers(void)
{
	mgmt_register(mgmt_if, MGMT_EV_NEW_SETTINGS, adapter.index,
					new_settings_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_CLASS_OF_DEV_CHANGED, adapter.index,
				mgmt_dev_class_changed_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_LOCAL_NAME_CHANGED, adapter.index,
				mgmt_local_name_changed_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_NEW_LINK_KEY, adapter.index,
					new_link_key_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_PIN_CODE_REQUEST, adapter.index,
					pin_code_request_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_USER_CONFIRM_REQUEST, adapter.index,
				user_confirm_request_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_USER_PASSKEY_REQUEST, adapter.index,
				user_passkey_request_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_PASSKEY_NOTIFY, adapter.index,
				user_passkey_notify_callback, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_DISCOVERING, adapter.index,
					mgmt_discovering_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_DEVICE_FOUND, adapter.index,
					mgmt_device_found_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_DEVICE_CONNECTED, adapter.index,
				mgmt_device_connected_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_DEVICE_DISCONNECTED, adapter.index,
				mgmt_device_disconnected_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_CONNECT_FAILED, adapter.index,
					mgmt_connect_failed_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_AUTH_FAILED, adapter.index,
					mgmt_auth_failed_event, NULL, NULL);

	mgmt_register(mgmt_if, MGMT_EV_DEVICE_UNPAIRED, adapter.index,
				mgmt_device_unpaired_event, NULL, NULL);
}

static void load_link_keys_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	bt_bluetooth_ready cb = user_data;
	int err;

	if (status) {
		error("Failed to load link keys for index %u: %s (0x%02x)",
				adapter.index, mgmt_errstr(status), status);
		err = -EIO;
		goto failed;
	}

	DBG("status %u", status);

	cb(0, &adapter.bdaddr);
	return;

failed:
	cb(err, NULL);
}

static void load_link_keys(GSList *keys, bt_bluetooth_ready cb)
{
	struct mgmt_cp_load_link_keys *cp;
	struct mgmt_link_key_info *key;
	size_t key_count, cp_size;
	unsigned int id;

	key_count = g_slist_length(keys);

	DBG("keys %zu ", key_count);

	cp_size = sizeof(*cp) + (key_count * sizeof(*key));

	cp = g_malloc0(cp_size);

	/*
	 * Even if the list of stored keys is empty, it is important to
	 * load an empty list into the kernel. That way it is ensured
	 * that no old keys from a previous daemon are present.
	 */
	cp->key_count = htobs(key_count);

	for (key = cp->keys; keys != NULL; keys = g_slist_next(keys), key++)
		memcpy(key, keys->data, sizeof(*key));

	id = mgmt_send(mgmt_if, MGMT_OP_LOAD_LINK_KEYS, adapter.index,
			cp_size, cp, load_link_keys_complete, cb, NULL);

	g_free(cp);

	if (id == 0) {
		error("Failed to load link keys");
		cb(-EIO, NULL);
	}
}

/* output uint128 is in host order */
static void uuid16_to_uint128(uint16_t uuid, uint128_t *u128)
{
	uuid_t uuid16, uuid128;

	sdp_uuid16_create(&uuid16, uuid);
	sdp_uuid16_to_uuid128(&uuid128, &uuid16);

	ntoh128(&uuid128.value.uuid128, u128);
}

static uint8_t get_adapter_uuids(void)
{
	struct hal_ev_adapter_props_changed *ev;
	GSList *list = adapter.uuids;
	unsigned int uuid_count = g_slist_length(list);
	int len = uuid_count * sizeof(uint128_t);
	uint8_t buf[BASELEN_PROP_CHANGED + len];
	uint8_t *p;
	int i;

	memset(buf, 0, sizeof(buf));
	ev = (void *) buf;

	ev->num_props = 1;
	ev->status = HAL_STATUS_SUCCESS;

	ev->props[0].type = HAL_PROP_ADAPTER_UUIDS;
	ev->props[0].len = len;
	p = ev->props->val;

	for (; list; list = g_slist_next(list)) {
		uint16_t uuid = GPOINTER_TO_UINT(list->data);
		uint128_t uint128;

		uuid16_to_uint128(uuid, &uint128);

		/* Android expects swapped bytes in uuid */
		for (i = 0; i < 16; i++)
			p[15 - i] = uint128.data[i];

		p += sizeof(uint128_t);
	}

	ipc_send_notif(HAL_SERVICE_ID_BLUETOOTH, HAL_EV_ADAPTER_PROPS_CHANGED,
							sizeof(buf), ev);

	return HAL_STATUS_SUCCESS;
}

static void remove_uuid_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to remove UUID: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	mgmt_dev_class_changed_event(adapter.index, length, param, NULL);

	get_adapter_uuids();
}

static void remove_uuid(uint16_t uuid)
{
	uint128_t uint128;
	struct mgmt_cp_remove_uuid cp;

	uuid16_to_uint128(uuid, &uint128);
	htob128(&uint128, (uint128_t *) cp.uuid);

	mgmt_send(mgmt_if, MGMT_OP_REMOVE_UUID, adapter.index, sizeof(cp), &cp,
					remove_uuid_complete, NULL, NULL);
}

static void add_uuid_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to add UUID: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	mgmt_dev_class_changed_event(adapter.index, length, param, NULL);

	get_adapter_uuids();
}

static void add_uuid(uint8_t svc_hint, uint16_t uuid)
{
	uint128_t uint128;
	struct mgmt_cp_add_uuid cp;

	uuid16_to_uint128(uuid, &uint128);

	htob128(&uint128, (uint128_t *) cp.uuid);
	cp.svc_hint = svc_hint;

	mgmt_send(mgmt_if, MGMT_OP_ADD_UUID, adapter.index, sizeof(cp), &cp,
						add_uuid_complete, NULL, NULL);
}

int bt_adapter_add_record(sdp_record_t *rec, uint8_t svc_hint)
{
	uint16_t uuid;

	/* TODO support all types? */
	if (rec->svclass.type != SDP_UUID16) {
		warn("Ignoring unsupported UUID type");
		return -EINVAL;
	}

	uuid = rec->svclass.value.uuid16;

	if (g_slist_find(adapter.uuids, GUINT_TO_POINTER(uuid))) {
		DBG("UUID 0x%x already added", uuid);
		return -EALREADY;
	}

	adapter.uuids = g_slist_prepend(adapter.uuids, GUINT_TO_POINTER(uuid));

	add_uuid(svc_hint, uuid);

	return add_record_to_server(&adapter.bdaddr, rec);
}

void bt_adapter_remove_record(uint32_t handle)
{
	sdp_record_t *rec;
	GSList *uuid_found;
	uint16_t uuid;

	rec = sdp_record_find(handle);
	if (!rec)
		return;

	uuid = rec->svclass.value.uuid16;

	uuid_found = g_slist_find(adapter.uuids, GUINT_TO_POINTER(uuid));
	if (uuid_found) {
		remove_uuid(uuid);

		adapter.uuids = g_slist_remove(adapter.uuids,
							uuid_found->data);
	}

	remove_record_from_server(handle);
}

static void set_mode_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to set mode: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	new_settings_callback(adapter.index, length, param, NULL);
}

static bool set_mode(uint16_t opcode, uint8_t mode)
{
	struct mgmt_mode cp;

	memset(&cp, 0, sizeof(cp));
	cp.val = mode;

	DBG("opcode=0x%x mode=0x%x", opcode, mode);

	if (mgmt_send(mgmt_if, opcode, adapter.index, sizeof(cp), &cp,
					set_mode_complete, NULL, NULL) > 0)
		return true;

	error("Failed to set mode");

	return false;
}

static void set_io_capability(void)
{
	struct mgmt_cp_set_io_capability cp;

	memset(&cp, 0, sizeof(cp));
	cp.io_capability = DEFAULT_IO_CAPABILITY;

	if (mgmt_send(mgmt_if, MGMT_OP_SET_IO_CAPABILITY, adapter.index,
				sizeof(cp), &cp, NULL, NULL, NULL) == 0)
		error("Failed to set IO capability");
}

static void set_device_id(void)
{
	struct mgmt_cp_set_device_id cp;
	uint8_t major, minor;
	uint16_t version;

	if (sscanf(VERSION, "%hhu.%hhu", &major, &minor) != 2)
		return;

	version = major << 8 | minor;

	memset(&cp, 0, sizeof(cp));
	cp.source = htobs(DEVICE_ID_SOURCE);
	cp.vendor = htobs(DEVICE_ID_VENDOR);
	cp.product = htobs(DEVICE_ID_PRODUCT);
	cp.version = htobs(version);

	if (mgmt_send(mgmt_if, MGMT_OP_SET_DEVICE_ID, adapter.index,
				sizeof(cp), &cp, NULL, NULL, NULL) == 0)
		error("Failed to set device id");

	register_device_id(DEVICE_ID_SOURCE, DEVICE_ID_VENDOR,
						DEVICE_ID_PRODUCT, version);

	bt_adapter_add_record(sdp_record_find(0x10000), 0x00);
}

static void set_adapter_name_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_cp_set_local_name *rp = param;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to set name: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	adapter_set_name(rp->name);
}

static uint8_t set_adapter_name(const uint8_t *name, uint16_t len)
{
	struct mgmt_cp_set_local_name cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(cp.name, name, len);

	if (mgmt_send(mgmt_if, MGMT_OP_SET_LOCAL_NAME, adapter.index,
				sizeof(cp), &cp, set_adapter_name_complete,
				NULL, NULL) > 0)
		return HAL_STATUS_SUCCESS;

	error("Failed to set name");

	return HAL_STATUS_FAILED;
}

static uint8_t set_adapter_discoverable_timeout(const void *buf, uint16_t len)
{
	const uint32_t *timeout = buf;

	if (len != sizeof(*timeout)) {
		error("Invalid set disc timeout size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return HAL_STATUS_FAILED;
	}

	/* Android handles discoverable timeout in Settings app.
	 * There is no need to use kernel feature for that.
	 * Just need to store this value here */

	memcpy(&adapter.discoverable_timeout, timeout, sizeof(uint32_t));

	store_adapter_config();

	send_adapter_property(HAL_PROP_ADAPTER_DISC_TIMEOUT,
					sizeof(adapter.discoverable_timeout),
					&adapter.discoverable_timeout);

	return HAL_STATUS_SUCCESS;
}

static void clear_uuids(void)
{
	struct mgmt_cp_remove_uuid cp;

	memset(&cp, 0, sizeof(cp));

	mgmt_send(mgmt_if, MGMT_OP_REMOVE_UUID, adapter.index,
					sizeof(cp), &cp, NULL, NULL, NULL);
}

static void create_device_from_info(GKeyFile *key_file, const char *peer)
{
	struct device *dev;
	uint8_t type;
	bdaddr_t bdaddr;
	char **uuids;
	char *str;

	DBG("%s", peer);

	type = g_key_file_get_integer(key_file, peer, "Type", NULL);

	str2ba(peer, &bdaddr);
	dev = create_device(&bdaddr, type);

	dev->bond_state = HAL_BOND_STATE_BONDED;

	str = g_key_file_get_string(key_file, peer, "Name", NULL);
	if (str) {
		g_free(dev->name);
		dev->name = str;
	}

	str = g_key_file_get_string(key_file, peer, "FriendlyName", NULL);
	if (str) {
		g_free(dev->friendly_name);
		dev->friendly_name = str;
	}

	dev->class = g_key_file_get_integer(key_file, peer, "Class", NULL);

	dev->timestamp = g_key_file_get_integer(key_file, peer, "Timestamp",
									NULL);

	uuids = g_key_file_get_string_list(key_file, peer, "Services", NULL,
									NULL);
	if (uuids) {
		char **uuid;

		for (uuid = uuids; *uuid; uuid++) {
			uint8_t *u = g_malloc0(16);
			int i;

			for (i = 0; i < 16; i++)
				sscanf((*uuid) + (i * 2), "%02hhX", &u[i]);

			dev->uuids = g_slist_append(dev->uuids, u);
		}

		g_strfreev(uuids);
	}
}

static struct mgmt_link_key_info *get_key_info(GKeyFile *key_file, const char *peer)
{
	struct mgmt_link_key_info *info = NULL;
	char *str;
	unsigned int i;

	str = g_key_file_get_string(key_file, peer, "LinkKey", NULL);
	if (!str || strlen(str) != 32)
		goto failed;

	info = g_new0(struct mgmt_link_key_info, 1);

	str2ba(peer, &info->addr.bdaddr);

	info->addr.type = g_key_file_get_integer(key_file, peer, "Type", NULL);

	for (i = 0; i < sizeof(info->val); i++)
		sscanf(str + (i * 2), "%02hhX", &info->val[i]);

	info->type = g_key_file_get_integer(key_file, peer, "LinkKeyType",
									NULL);
	info->pin_len = g_key_file_get_integer(key_file, peer,
						"LinkKeyPinLength", NULL);

failed:
	g_free(str);

	return info;
}

static void load_devices_info(bt_bluetooth_ready cb)
{
	GKeyFile *key_file;
	gchar **devs;
	gsize len = 0;
	unsigned int i;
	GSList *keys = NULL;

	key_file = g_key_file_new();

	g_key_file_load_from_file(key_file, ANDROID_STORAGEDIR"/devices", 0,
									NULL);

	devs = g_key_file_get_groups(key_file, &len);

	for (i = 0; i < len; i++) {
		struct mgmt_link_key_info *key_info;

		create_device_from_info(key_file, devs[i]);

		key_info = get_key_info(key_file, devs[i]);
		if (key_info)
			keys = g_slist_prepend(keys, key_info);

		/* TODO ltk */
	}

	load_link_keys(keys, cb);
	g_strfreev(devs);
	g_slist_free_full(keys, g_free);
	g_key_file_free(key_file);
}

static void set_adapter_class(void)
{
	struct mgmt_cp_set_dev_class cp;

	memset(&cp, 0, sizeof(cp));

	/*
	 * kernel assign the major and minor numbers straight to dev_class[0]
	 * and dev_class[1] without considering the proper bit shifting.
	 */
	cp.major = ADAPTER_MAJOR_CLASS & 0x1f;
	cp.minor = ADAPTER_MINOR_CLASS << 2;

	if (mgmt_send(mgmt_if, MGMT_OP_SET_DEV_CLASS, adapter.index,
					sizeof(cp), &cp, NULL, NULL, NULL) > 0)
		return;

	error("Failed to set class of device");
}

static void read_info_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_info *rp = param;
	bt_bluetooth_ready cb = user_data;
	uint32_t missing_settings, supported_settings;
	int err;

	DBG("");

	if (status) {
		error("Failed to read info for index %u: %s (0x%02x)",
				adapter.index, mgmt_errstr(status), status);
		err = -EIO;
		goto failed;
	}

	if (length < sizeof(*rp)) {
		error("Too small read info complete response");
		err = -EIO;
		goto failed;
	}

	if (!bacmp(&rp->bdaddr, BDADDR_ANY)) {
		error("No Bluetooth address");
		err = -ENODEV;
		goto failed;
	}

	load_adapter_config();

	if (!bacmp(&adapter.bdaddr, BDADDR_ANY)) {
		bacpy(&adapter.bdaddr, &rp->bdaddr);
		adapter.name = g_strdup(DEFAULT_ADAPTER_NAME);
		store_adapter_config();
	} else if (bacmp(&adapter.bdaddr, &rp->bdaddr)) {
		error("Bluetooth address mismatch");
		err = -ENODEV;
		goto failed;
	}

	if (g_strcmp0(adapter.name, (const char *) rp->name))
		set_adapter_name((uint8_t *)adapter.name, strlen(adapter.name));

	set_adapter_class();

	/* Store adapter information */
	adapter.dev_class = rp->dev_class[0] | (rp->dev_class[1] << 8) |
						(rp->dev_class[2] << 16);

	supported_settings = btohs(rp->supported_settings);
	adapter.current_settings = btohs(rp->current_settings);

	/* TODO: Register all event notification handlers */
	register_mgmt_handlers();

	clear_uuids();

	load_devices_info(cb);

	set_io_capability();
	set_device_id();

	missing_settings = adapter.current_settings ^ supported_settings;

	if (missing_settings & MGMT_SETTING_SSP)
		set_mode(MGMT_OP_SET_SSP, 0x01);

	if (missing_settings & MGMT_SETTING_SECURE_CONN)
		set_mode(MGMT_OP_SET_SECURE_CONN, 0x01);

	if (missing_settings & MGMT_SETTING_PAIRABLE)
		set_mode(MGMT_OP_SET_PAIRABLE, 0x01);

	return;

failed:
	cb(err, NULL);
}

static void mgmt_index_added_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	bt_bluetooth_ready cb = user_data;

	DBG("index %u", index);

	if (adapter.index != MGMT_INDEX_NONE) {
		DBG("skip event for index %u", index);
		return;
	}

	if (option_index != MGMT_INDEX_NONE && option_index != index) {
		DBG("skip event for index %u (option %u)", index, option_index);
		return;
	}

	adapter.index = index;

	if (mgmt_send(mgmt_if, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_complete, cb, NULL) == 0) {
		cb(-EIO, NULL);
		return;
	}
}

static void mgmt_index_removed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	DBG("index %u", index);

	if (index != adapter.index)
		return;

	error("Adapter was removed. Exiting.");
	raise(SIGTERM);
}

static void read_index_list_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	bt_bluetooth_ready cb = user_data;
	uint16_t num;
	int i;

	DBG("");

	if (status) {
		error("%s: Failed to read index list: %s (0x%02x)",
					__func__, mgmt_errstr(status), status);
		goto failed;
	}

	if (length < sizeof(*rp)) {
		error("%s: Wrong size of read index list response", __func__);
		goto failed;
	}

	num = btohs(rp->num_controllers);

	DBG("Number of controllers: %u", num);

	if (num * sizeof(uint16_t) + sizeof(*rp) != length) {
		error("%s: Incorrect pkt size for index list rsp", __func__);
		goto failed;
	}

	if (adapter.index != MGMT_INDEX_NONE)
		return;

	for (i = 0; i < num; i++) {
		uint16_t index = btohs(rp->index[i]);

		if (option_index != MGMT_INDEX_NONE && option_index != index)
			continue;

		if (mgmt_send(mgmt_if, MGMT_OP_READ_INFO, index, 0, NULL,
					read_info_complete, cb, NULL) == 0)
			goto failed;

		adapter.index = index;
		return;
	}

	return;

failed:
	cb(-EIO, NULL);
}

static void read_version_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_version *rp = param;
	uint8_t mgmt_version, mgmt_revision;
	bt_bluetooth_ready cb = user_data;

	DBG("");

	if (status) {
		error("Failed to read version information: %s (0x%02x)",
						mgmt_errstr(status), status);
		goto failed;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size response");
		goto failed;
	}

	mgmt_version = rp->version;
	mgmt_revision = btohs(rp->revision);

	info("Bluetooth management interface %u.%u initialized",
						mgmt_version, mgmt_revision);

	if (MGMT_VERSION(mgmt_version, mgmt_revision) < MGMT_VERSION(1, 3)) {
		error("Version 1.3 or later of management interface required");
		goto failed;
	}

	mgmt_register(mgmt_if, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					mgmt_index_added_event, cb, NULL);
	mgmt_register(mgmt_if, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					mgmt_index_removed_event, NULL, NULL);

	if (mgmt_send(mgmt_if, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0,
				NULL, read_index_list_complete, cb, NULL) > 0)
		return;

	error("Failed to read controller index list");

failed:
	cb(-EIO, NULL);
}

bool bt_bluetooth_start(int index, bt_bluetooth_ready cb)
{
	DBG("index %d", index);

	mgmt_if = mgmt_new_default();
	if (!mgmt_if) {
		error("Failed to access management interface");
		return false;
	}

	if (mgmt_send(mgmt_if, MGMT_OP_READ_VERSION, MGMT_INDEX_NONE, 0, NULL,
				read_version_complete, cb, NULL) == 0) {
		error("Error sending READ_VERSION mgmt command");

		mgmt_unref(mgmt_if);
		mgmt_if = NULL;

		return false;
	}

	if (index >= 0)
		option_index = index;

	return true;
}

static void shutdown_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	bt_bluetooth_stopped cb = user_data;

	if (status != MGMT_STATUS_SUCCESS)
		error("Clean controller shutdown failed");

	cb();
}

bool bt_bluetooth_stop(bt_bluetooth_stopped cb)
{
	struct mgmt_mode cp;

	if (adapter.index == MGMT_INDEX_NONE)
		return false;

	info("Switching controller off");

	memset(&cp, 0, sizeof(cp));

	return mgmt_send(mgmt_if, MGMT_OP_SET_POWERED, adapter.index,
				sizeof(cp), &cp, shutdown_complete, (void *)cb,
				NULL) > 0;
}

void bt_bluetooth_cleanup(void)
{
	g_free(adapter.name);
	adapter.name = NULL;

	mgmt_unref(mgmt_if);
	mgmt_if = NULL;
}

static bool set_discoverable(uint8_t mode, uint16_t timeout)
{
	struct mgmt_cp_set_discoverable cp;

	memset(&cp, 0, sizeof(cp));
	cp.val = mode;
	cp.timeout = htobs(timeout);

	DBG("mode %u timeout %u", mode, timeout);

	if (mgmt_send(mgmt_if, MGMT_OP_SET_DISCOVERABLE, adapter.index,
			sizeof(cp), &cp, set_mode_complete, NULL, NULL) > 0)
		return true;

	error("Failed to set mode discoverable");

	return false;
}

static uint8_t get_adapter_address(void)
{
	uint8_t buf[6];

	bdaddr2android(&adapter.bdaddr, buf);

	send_adapter_property(HAL_PROP_ADAPTER_ADDR, sizeof(buf), buf);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_adapter_name(void)
{
	if (!adapter.name)
		return HAL_STATUS_FAILED;

	adapter_name_changed((uint8_t *) adapter.name);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_adapter_class(void)
{
	DBG("");

	adapter_class_changed();

	return HAL_STATUS_SUCCESS;
}

static uint8_t settings2type(void)
{
	bool bredr, le;

	bredr = adapter.current_settings & MGMT_SETTING_BREDR;
	le = adapter.current_settings & MGMT_SETTING_LE;

	if (bredr && le)
		return HAL_TYPE_DUAL;

	if (bredr && !le)
		return HAL_TYPE_BREDR;

	if (!bredr && le)
		return HAL_TYPE_LE;

	return 0;
}

static uint8_t get_adapter_type(void)
{
	uint8_t type;

	DBG("");

	type = settings2type();

	if (!type)
		return HAL_STATUS_FAILED;

	send_adapter_property(HAL_PROP_ADAPTER_TYPE, sizeof(type), &type);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_adapter_service_rec(void)
{
	DBG("Not implemented");

	/* TODO: Add implementation */

	return HAL_STATUS_FAILED;
}

static uint8_t get_adapter_scan_mode(void)
{
	DBG("");

	scan_mode_changed();

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_adapter_bonded_devices(void)
{
	uint8_t buf[sizeof(bdaddr_t) * g_slist_length(devices)];
	int i = 0;
	GSList *l;

	DBG("");

	for (l = devices; l; l = g_slist_next(l)) {
		struct device *dev = l->data;

		if (dev->bond_state != HAL_BOND_STATE_BONDED)
			continue;

		bdaddr2android(&dev->bdaddr, buf + (i * sizeof(bdaddr_t)));
		i++;
	}

	send_adapter_property(HAL_PROP_ADAPTER_BONDED_DEVICES,
						i * sizeof(bdaddr_t), buf);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_adapter_discoverable_timeout(void)
{
	send_adapter_property(HAL_PROP_ADAPTER_DISC_TIMEOUT,
					sizeof(adapter.discoverable_timeout),
					&adapter.discoverable_timeout);

	return HAL_STATUS_SUCCESS;
}

static void handle_get_adapter_prop_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_get_adapter_prop *cmd = buf;
	uint8_t status;

	switch (cmd->type) {
	case HAL_PROP_ADAPTER_ADDR:
		status = get_adapter_address();
		break;
	case HAL_PROP_ADAPTER_NAME:
		status = get_adapter_name();
		break;
	case HAL_PROP_ADAPTER_UUIDS:
		status = get_adapter_uuids();
		break;
	case HAL_PROP_ADAPTER_CLASS:
		status = get_adapter_class();
		break;
	case HAL_PROP_ADAPTER_TYPE:
		status = get_adapter_type();
		break;
	case HAL_PROP_ADAPTER_SERVICE_REC:
		status = get_adapter_service_rec();
		break;
	case HAL_PROP_ADAPTER_SCAN_MODE:
		status = get_adapter_scan_mode();
		break;
	case HAL_PROP_ADAPTER_BONDED_DEVICES:
		status = get_adapter_bonded_devices();
		break;
	case HAL_PROP_ADAPTER_DISC_TIMEOUT:
		status = get_adapter_discoverable_timeout();
		break;
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		error("Failed to get adapter property (type %u status %u)",
							cmd->type, status);

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_ADAPTER_PROP, status);
}

static void get_adapter_properties(void)
{
	get_adapter_address();
	get_adapter_name();
	get_adapter_uuids();
	get_adapter_class();
	get_adapter_type();
	get_adapter_service_rec();
	get_adapter_scan_mode();
	get_adapter_bonded_devices();
	get_adapter_discoverable_timeout();
}

static bool start_discovery(void)
{
	struct mgmt_cp_start_discovery cp;
	uint8_t type = 1 << BDADDR_BREDR;

	if (adapter.current_settings & type)
		cp.type = type;
	else
		cp.type = 0;

	DBG("type=0x%x", type);

	if (mgmt_send(mgmt_if, MGMT_OP_START_DISCOVERY, adapter.index,
					sizeof(cp), &cp, NULL, NULL, NULL) > 0)
		return true;

	error("Failed to start discovery");
	return false;
}

static bool stop_discovery(void)
{
	struct mgmt_cp_stop_discovery cp;
	uint8_t type = 1 << BDADDR_BREDR;

	if (adapter.current_settings & type)
		cp.type = type;
	else
		cp.type = 0;

	DBG("type=0x%x", type);

	if (mgmt_send(mgmt_if, MGMT_OP_STOP_DISCOVERY, adapter.index,
					sizeof(cp), &cp, NULL, NULL, NULL) > 0)
		return true;

	error("Failed to stop discovery");
	return false;
}

static uint8_t set_adapter_scan_mode(const void *buf, uint16_t len)
{
	const uint8_t *mode = buf;
	bool conn, disc, cur_conn, cur_disc;

	if (len != sizeof(*mode)) {
		error("Invalid set scan mode size (%u bytes), terminating",
								len);
		raise(SIGTERM);
		return HAL_STATUS_FAILED;
	}

	cur_conn = adapter.current_settings & MGMT_SETTING_CONNECTABLE;
	cur_disc = adapter.current_settings & MGMT_SETTING_DISCOVERABLE;

	DBG("connectable %u discoverable %d mode %u", cur_conn, cur_disc,
								*mode);

	switch (*mode) {
	case HAL_ADAPTER_SCAN_MODE_NONE:
		if (!cur_conn && !cur_disc)
			goto done;

		conn = false;
		disc = false;
		break;
	case HAL_ADAPTER_SCAN_MODE_CONN:
		if (cur_conn && !cur_disc)
			goto done;

		conn = true;
		disc = false;
		break;
	case HAL_ADAPTER_SCAN_MODE_CONN_DISC:
		if (cur_conn && cur_disc)
			goto done;

		conn = true;
		disc = true;
		break;
	default:
		return HAL_STATUS_FAILED;
	}

	if (cur_conn != conn) {
		if (!set_mode(MGMT_OP_SET_CONNECTABLE, conn ? 0x01 : 0x00))
			return HAL_STATUS_FAILED;
	}

	if (cur_disc != disc && conn) {
		if (!set_discoverable(disc ? 0x01 : 0x00, 0))
			return HAL_STATUS_FAILED;
	}

	return HAL_STATUS_SUCCESS;

done:
	/* Android expects property changed callback */
	scan_mode_changed();

	return HAL_STATUS_SUCCESS;
}

static void handle_set_adapter_prop_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_set_adapter_prop *cmd = buf;
	uint8_t status;

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid set adapter prop cmd (0x%x), terminating",
								cmd->type);
		raise(SIGTERM);
		return;
	}

	switch (cmd->type) {
	case HAL_PROP_ADAPTER_SCAN_MODE:
		status = set_adapter_scan_mode(cmd->val, cmd->len);
		break;
	case HAL_PROP_ADAPTER_NAME:
		status = set_adapter_name(cmd->val, cmd->len);
		break;
	case HAL_PROP_ADAPTER_DISC_TIMEOUT:
		status = set_adapter_discoverable_timeout(cmd->val, cmd->len);
		break;
	default:
		DBG("Unhandled property type 0x%x", cmd->type);
		status = HAL_STATUS_FAILED;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		error("Failed to set adapter property (type %u status %u)",
							cmd->type, status);

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_SET_ADAPTER_PROP, status);
}

static void pair_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_pair_device *rp = param;

	DBG("status %u", status);

	/* On success bond state change will be send when new link key event
	 * is received */
	if (status == MGMT_STATUS_SUCCESS)
		return;

	set_device_bond_state(&rp->addr.bdaddr, status_mgmt2hal(status),
							HAL_BOND_STATE_NONE);
}

static void handle_create_bond_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_create_bond *cmd = buf;
	uint8_t status;
	struct mgmt_cp_pair_device cp;

	cp.io_cap = DEFAULT_IO_CAPABILITY;
	cp.addr.type = BDADDR_BREDR;
	android2bdaddr(cmd->bdaddr, &cp.addr.bdaddr);

	if (mgmt_send(mgmt_if, MGMT_OP_PAIR_DEVICE, adapter.index, sizeof(cp),
				&cp, pair_device_complete, NULL, NULL) == 0) {
		status = HAL_STATUS_FAILED;
		goto fail;
	}

	status = HAL_STATUS_SUCCESS;

	set_device_bond_state(&cp.addr.bdaddr, HAL_STATUS_SUCCESS,
						HAL_BOND_STATE_BONDING);

fail:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_CREATE_BOND, status);
}

static void handle_cancel_bond_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_cancel_bond *cmd = buf;
	struct mgmt_addr_info cp;
	uint8_t status;

	cp.type = BDADDR_BREDR;
	android2bdaddr(cmd->bdaddr, &cp.bdaddr);

	if (mgmt_reply(mgmt_if, MGMT_OP_CANCEL_PAIR_DEVICE, adapter.index,
					sizeof(cp), &cp, NULL, NULL, NULL) > 0)
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_CANCEL_BOND, status);
}

static void unpair_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_unpair_device *rp = param;

	DBG("status %u", status);

	if (status != MGMT_STATUS_SUCCESS)
		return;

	set_device_bond_state(&rp->addr.bdaddr, HAL_STATUS_SUCCESS,
							HAL_BOND_STATE_NONE);
}

static void handle_remove_bond_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_remove_bond *cmd = buf;
	struct mgmt_cp_unpair_device cp;
	uint8_t status;

	cp.disconnect = 1;
	cp.addr.type = BDADDR_BREDR;
	android2bdaddr(cmd->bdaddr, &cp.addr.bdaddr);

	if (mgmt_send(mgmt_if, MGMT_OP_UNPAIR_DEVICE, adapter.index,
				sizeof(cp), &cp, unpair_device_complete,
				NULL, NULL) > 0)
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_REMOVE_BOND, status);
}

static void handle_pin_reply_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_pin_reply *cmd = buf;
	uint8_t status;
	bdaddr_t bdaddr;
	char addr[18];

	android2bdaddr(cmd->bdaddr, &bdaddr);
	ba2str(&bdaddr, addr);

	DBG("%s accept %u pin_len %u", addr, cmd->accept, cmd->pin_len);

	if (!cmd->accept && cmd->pin_len) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	if (cmd->accept) {
		struct mgmt_cp_pin_code_reply rp;

		memset(&rp, 0, sizeof(rp));

		bacpy(&rp.addr.bdaddr, &bdaddr);
		rp.addr.type = BDADDR_BREDR;
		rp.pin_len = cmd->pin_len;
		memcpy(rp.pin_code, cmd->pin_code, rp.pin_len);

		if (mgmt_reply(mgmt_if, MGMT_OP_PIN_CODE_REPLY, adapter.index,
				sizeof(rp), &rp, NULL, NULL, NULL) == 0) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}
	} else {
		struct mgmt_cp_pin_code_neg_reply rp;

		bacpy(&rp.addr.bdaddr, &bdaddr);
		rp.addr.type = BDADDR_BREDR;

		if (mgmt_reply(mgmt_if, MGMT_OP_PIN_CODE_NEG_REPLY,
						adapter.index, sizeof(rp), &rp,
						NULL, NULL, NULL) == 0) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}
	}

	status = HAL_STATUS_SUCCESS;
failed:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_PIN_REPLY, status);
}

static uint8_t user_confirm_reply(const bdaddr_t *bdaddr, bool accept)
{
	struct mgmt_addr_info cp;
	uint16_t opcode;

	if (accept)
		opcode = MGMT_OP_USER_CONFIRM_REPLY;
	else
		opcode = MGMT_OP_USER_CONFIRM_NEG_REPLY;

	bacpy(&cp.bdaddr, bdaddr);
	cp.type = BDADDR_BREDR;

	if (mgmt_reply(mgmt_if, opcode, adapter.index, sizeof(cp), &cp,
							NULL, NULL, NULL) > 0)
		return HAL_STATUS_SUCCESS;

	return HAL_STATUS_FAILED;
}

static uint8_t user_passkey_reply(const bdaddr_t *bdaddr, bool accept,
							uint32_t passkey)
{
	unsigned int id;

	if (accept) {
		struct mgmt_cp_user_passkey_reply cp;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = BDADDR_BREDR;
		cp.passkey = htobl(passkey);

		id = mgmt_reply(mgmt_if, MGMT_OP_USER_PASSKEY_REPLY,
						adapter.index, sizeof(cp), &cp,
						NULL, NULL, NULL);
	} else {
		struct mgmt_cp_user_passkey_neg_reply cp;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = BDADDR_BREDR;

		id = mgmt_reply(mgmt_if, MGMT_OP_USER_PASSKEY_NEG_REPLY,
						adapter.index, sizeof(cp), &cp,
						NULL, NULL, NULL);
	}

	if (id == 0)
		return HAL_STATUS_FAILED;

	return HAL_STATUS_SUCCESS;
}

static void handle_ssp_reply_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_ssp_reply *cmd = buf;
	bdaddr_t bdaddr;
	uint8_t status;
	char addr[18];

	/* TODO should parameters sanity be verified here? */

	android2bdaddr(cmd->bdaddr, &bdaddr);
	ba2str(&bdaddr, addr);

	DBG("%s variant %u accept %u", addr, cmd->ssp_variant, cmd->accept);

	switch (cmd->ssp_variant) {
	case HAL_SSP_VARIANT_CONFIRM:
	case HAL_SSP_VARIANT_CONSENT:
		status = user_confirm_reply(&bdaddr, cmd->accept);
		break;
	case HAL_SSP_VARIANT_ENTRY:
		status = user_passkey_reply(&bdaddr, cmd->accept,
								cmd->passkey);
		break;
	case HAL_SSP_VARIANT_NOTIF:
		status = HAL_STATUS_SUCCESS;
		break;
	default:
		status = HAL_STATUS_INVALID;
		break;
	}

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_SSP_REPLY, status);
}

static void handle_get_remote_services_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_get_remote_services *cmd = buf;
	uint8_t status;
	bdaddr_t addr;

	android2bdaddr(&cmd->bdaddr, &addr);

	status = browse_remote_sdp(&addr);

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_REMOTE_SERVICES,
									status);
}

static uint8_t get_device_uuids(struct device *dev)
{
	send_device_uuids_notif(dev);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_device_class(struct device *dev)
{
	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_CLASS,
					sizeof(dev->class), &dev->class);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_device_type(struct device *dev)
{
	uint8_t type = bdaddr_type2android(dev->bdaddr_type);

	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_TYPE,
							sizeof(type), &type);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_device_service_rec(struct device *dev)
{
	DBG("Not implemented");

	/* TODO */

	return HAL_STATUS_FAILED;
}

static uint8_t get_device_friendly_name(struct device *dev)
{
	if (!dev->friendly_name)
		return HAL_STATUS_FAILED;

	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_FRIENDLY_NAME,
				strlen(dev->friendly_name), dev->friendly_name);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_device_rssi(struct device *dev)
{
	if (!dev->rssi)
		return HAL_STATUS_FAILED;

	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_RSSI,
						sizeof(dev->rssi), &dev->rssi);

	return HAL_STATUS_SUCCESS;
}

static uint8_t get_device_version_info(struct device *dev)
{
	DBG("Not implemented");

	/* TODO */

	return HAL_STATUS_FAILED;
}

static uint8_t get_device_timestamp(struct device *dev)
{
	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_TIMESTAMP,
				sizeof(dev->timestamp), &dev->timestamp);

	return HAL_STATUS_SUCCESS;
}

static void get_remote_device_props(struct device *dev)
{
	get_device_name(dev);
	get_device_uuids(dev);
	get_device_class(dev);
	get_device_type(dev);
	get_device_service_rec(dev);
	get_device_friendly_name(dev);
	get_device_rssi(dev);
	get_device_version_info(dev);
	get_device_timestamp(dev);
}

static void send_bonded_devices_props(void)
{
	GSList *l;

	for (l = devices; l; l = g_slist_next(l)) {
		struct device *dev = l->data;

		if (dev->bond_state == HAL_BOND_STATE_BONDED)
			get_remote_device_props(dev);
	}
}

static void handle_enable_cmd(const void *buf, uint16_t len)
{
	uint8_t status;

	/* Framework expects all properties to be emitted while
	 * enabling adapter */
	get_adapter_properties();

	/* Sent also properties of bonded devices */
	send_bonded_devices_props();

	if (adapter.current_settings & MGMT_SETTING_POWERED) {
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	if (!set_mode(MGMT_OP_SET_POWERED, 0x01)) {
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	status = HAL_STATUS_SUCCESS;
reply:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_ENABLE, status);
}

static void handle_disable_cmd(const void *buf, uint16_t len)
{
	uint8_t status;

	if (!(adapter.current_settings & MGMT_SETTING_POWERED)) {
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	if (!set_mode(MGMT_OP_SET_POWERED, 0x00)) {
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	status = HAL_STATUS_SUCCESS;
reply:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_DISABLE, status);
}

static void handle_get_adapter_props_cmd(const void *buf, uint16_t len)
{
	get_adapter_properties();

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_ADAPTER_PROPS,
							HAL_STATUS_SUCCESS);
}

static void handle_get_remote_device_props_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_get_remote_device_props *cmd = buf;
	struct device *dev;
	uint8_t status;
	bdaddr_t addr;

	android2bdaddr(cmd->bdaddr, &addr);

	dev = find_device(&addr);
	if (!dev) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	get_remote_device_props(dev);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_REMOTE_DEVICE_PROPS,
									status);
}

static void handle_get_remote_device_prop_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_get_remote_device_prop *cmd = buf;
	struct device *dev;
	uint8_t status;
	bdaddr_t addr;

	android2bdaddr(cmd->bdaddr, &addr);

	dev = find_device(&addr);
	if (!dev) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	switch (cmd->type) {
	case HAL_PROP_DEVICE_NAME:
		status = get_device_name(dev);
		break;
	case HAL_PROP_DEVICE_UUIDS:
		status = get_device_uuids(dev);
		break;
	case HAL_PROP_DEVICE_CLASS:
		status = get_device_class(dev);
		break;
	case HAL_PROP_DEVICE_TYPE:
		status = get_device_type(dev);
		break;
	case HAL_PROP_DEVICE_SERVICE_REC:
		status = get_device_service_rec(dev);
		break;
	case HAL_PROP_DEVICE_FRIENDLY_NAME:
		status = get_device_friendly_name(dev);
		break;
	case HAL_PROP_DEVICE_RSSI:
		status = get_device_rssi(dev);
		break;
	case HAL_PROP_DEVICE_VERSION_INFO:
		status = get_device_version_info(dev);
		break;
	case HAL_PROP_DEVICE_TIMESTAMP:
		status = get_device_timestamp(dev);
		break;
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		error("Failed to get device property (type %u status %u)",
							cmd->type, status);

failed:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_REMOTE_DEVICE_PROP,
								status);
}

static uint8_t set_device_friendly_name(struct device *dev, const uint8_t *val,
								uint16_t len)
{
	DBG("");

	g_free(dev->friendly_name);
	dev->friendly_name = g_strndup((const char *) val, len);

	store_device_info(dev);

	send_device_property(&dev->bdaddr, HAL_PROP_DEVICE_FRIENDLY_NAME,
				strlen(dev->friendly_name), dev->friendly_name);

	return HAL_STATUS_SUCCESS;
}

static uint8_t set_device_version_info(struct device *dev)
{
	DBG("Not implemented");

	/* TODO */

	return HAL_STATUS_FAILED;
}

static void handle_set_remote_device_prop_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_set_remote_device_prop *cmd = buf;
	struct device *dev;
	uint8_t status;
	bdaddr_t addr;

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid set remote device prop cmd (0x%x), terminating",
								cmd->type);
		raise(SIGTERM);
		return;
	}

	android2bdaddr(cmd->bdaddr, &addr);

	dev = find_device(&addr);
	if (!dev) {
		status = HAL_STATUS_INVALID;
		goto failed;
	}

	switch (cmd->type) {
	case HAL_PROP_DEVICE_FRIENDLY_NAME:
		status = set_device_friendly_name(dev, cmd->val, cmd->len);
		break;
	case HAL_PROP_DEVICE_VERSION_INFO:
		status = set_device_version_info(dev);
		break;
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

	if (status != HAL_STATUS_SUCCESS)
		error("Failed to set device property (type %u status %u)",
							cmd->type, status);

failed:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_SET_REMOTE_DEVICE_PROP,
									status);
}

static void handle_get_remote_service_rec_cmd(const void *buf, uint16_t len)
{
	/* TODO */

	error("get_remote_service_record not supported");

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_GET_REMOTE_SERVICE_REC,
							HAL_STATUS_FAILED);
}

static void handle_start_discovery_cmd(const void *buf, uint16_t len)
{
	uint8_t status;

	if (adapter.discovering) {
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	if (!(adapter.current_settings & MGMT_SETTING_POWERED)) {
		status = HAL_STATUS_NOT_READY;
		goto reply;
	}

	if (!start_discovery()) {
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	status = HAL_STATUS_SUCCESS;
reply:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_START_DISCOVERY, status);
}

static void handle_cancel_discovery_cmd(const void *buf, uint16_t len)
{
	uint8_t status;

	if (!adapter.discovering) {
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	if (!(adapter.current_settings & MGMT_SETTING_POWERED)) {
		status = HAL_STATUS_NOT_READY;
		goto reply;
	}

	if (!stop_discovery()) {
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	status = HAL_STATUS_SUCCESS;

reply:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_CANCEL_DISCOVERY, status);
}

static void handle_dut_mode_conf_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_dut_mode_conf *cmd = buf;
	char path[FILENAME_MAX];
	uint8_t status;
	int fd, ret;

	DBG("enable %u", cmd->enable);

	snprintf(path, sizeof(path), DUT_MODE_FILE, adapter.index);

	fd = open(path, O_WRONLY);
	if (fd < 0) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (cmd->enable)
		ret = write(fd, "1", sizeof("1"));
	else
		ret = write(fd, "0", sizeof("0"));

	if (ret < 0)
		status = HAL_STATUS_FAILED;
	else
		status = HAL_STATUS_SUCCESS;

	close(fd);

failed:
	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_DUT_MODE_CONF, status);
}

static void handle_dut_mode_send_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_dut_mode_send *cmd = buf;

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid dut mode send cmd, terminating");
		raise(SIGTERM);
		return;
	}

	error("dut_mode_send not supported (cmd opcode %u)", cmd->opcode);

	/* TODO */

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_DUT_MODE_SEND,
							HAL_STATUS_FAILED);
}

static void handle_le_test_mode_cmd(const void *buf, uint16_t len)
{
	const struct hal_cmd_le_test_mode *cmd = buf;

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid le test mode cmd, terminating");
		raise(SIGTERM);
		return;
	}

	error("le_test_mode not supported (cmd opcode %u)", cmd->opcode);

	/* TODO */

	ipc_send_rsp(HAL_SERVICE_ID_BLUETOOTH, HAL_OP_LE_TEST_MODE,
							HAL_STATUS_FAILED);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_ENABLE */
	{ handle_enable_cmd, false, 0 },
	/* HAL_OP_DISABLE */
	{ handle_disable_cmd, false, 0 },
	/* HAL_OP_GET_ADAPTER_PROPS */
	{ handle_get_adapter_props_cmd, false, 0 },
	/* HAL_OP_GET_ADAPTER_PROP */
	{ handle_get_adapter_prop_cmd, false,
				sizeof(struct hal_cmd_get_adapter_prop) },
	/* HAL_OP_SET_ADAPTER_PROP */
	{ handle_set_adapter_prop_cmd, true,
				sizeof(struct hal_cmd_set_adapter_prop) },
	/* HAL_OP_GET_REMOTE_DEVICE_PROPS */
	{ handle_get_remote_device_props_cmd, false,
			sizeof(struct hal_cmd_get_remote_device_props) },
	/* HAL_OP_GET_REMOTE_DEVICE_PROP */
	{ handle_get_remote_device_prop_cmd, false,
				sizeof(struct hal_cmd_get_remote_device_prop) },
	/* HAL_OP_SET_REMOTE_DEVICE_PROP */
	{ handle_set_remote_device_prop_cmd, true,
				sizeof(struct hal_cmd_set_remote_device_prop) },
	/* HAL_OP_GET_REMOTE_SERVICE_REC */
	{ handle_get_remote_service_rec_cmd, false,
				sizeof(struct hal_cmd_get_remote_service_rec) },
	/* HAL_OP_GET_REMOTE_SERVICES */
	{ handle_get_remote_services_cmd, false,
				sizeof(struct hal_cmd_get_remote_services) },
	/* HAL_OP_START_DISCOVERY */
	{ handle_start_discovery_cmd, false, 0 },
	/* HAL_OP_CANCEL_DISCOVERY */
	{ handle_cancel_discovery_cmd, false, 0 },
	/* HAL_OP_CREATE_BOND */
	{ handle_create_bond_cmd, false, sizeof(struct hal_cmd_create_bond) },
	/* HAL_OP_REMOVE_BOND */
	{ handle_remove_bond_cmd, false, sizeof(struct hal_cmd_remove_bond) },
	/* HAL_OP_CANCEL_BOND */
	{handle_cancel_bond_cmd, false, sizeof(struct hal_cmd_cancel_bond) },
	/* HAL_OP_PIN_REPLY */
	{ handle_pin_reply_cmd, false, sizeof(struct hal_cmd_pin_reply) },
	/* HAL_OP_SSP_REPLY */
	{ handle_ssp_reply_cmd, false, sizeof(struct hal_cmd_ssp_reply) },
	/* HAL_OP_DUT_MODE_CONF */
	{ handle_dut_mode_conf_cmd, false,
					sizeof(struct hal_cmd_dut_mode_conf) },
	/* HAL_OP_DUT_MODE_SEND */
	{ handle_dut_mode_send_cmd, true,
					sizeof(struct hal_cmd_dut_mode_send) },
	/* HAL_OP_LE_TEST_MODE */
	{ handle_le_test_mode_cmd, true, sizeof(struct hal_cmd_le_test_mode) },
};

void bt_bluetooth_register(void)
{
	DBG("");

	ipc_register(HAL_SERVICE_ID_BLUETOOTH, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));
}

void bt_bluetooth_unregister(void)
{
	DBG("");

	g_slist_free_full(devices, (GDestroyNotify) free_device);
	devices = NULL;

	ipc_unregister(HAL_SERVICE_ID_CORE);
}
