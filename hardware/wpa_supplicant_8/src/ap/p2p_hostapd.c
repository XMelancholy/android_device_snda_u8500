/*
 * hostapd / P2P integration
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "utils/includes.h"

#include "utils/common.h"
#include "common/ieee802_11_defs.h"
#include "p2p/p2p.h"
#include "hostapd.h"
#include "ap_config.h"
#include "ap_drv_ops.h"
#include "sta_info.h"
#include "p2p_hostapd.h"


#ifdef CONFIG_P2P

int hostapd_p2p_get_mib_sta(struct hostapd_data *hapd, struct sta_info *sta,
			    char *buf, size_t buflen)
{
	if (sta->p2p_ie == NULL)
		return 0;

	return p2p_ie_text(sta->p2p_ie, buf, buf + buflen);
}


int hostapd_p2p_set_noa(struct hostapd_data *hapd, u8 count, int start,
			int duration, int interval)
{
	wpa_printf(MSG_DEBUG, "P2P: Set NoA parameters: count=%u start=%d "
		   "duration=%d interval=%d", count, start, duration, interval);

	if (count == 0) {
		hapd->noa_enabled = 0;
		hapd->noa_start = 0;
		hapd->noa_duration = 0;
		hapd->noa_interval = 0;
	}

	if (count != 255) {
		wpa_printf(MSG_DEBUG, "P2P: Non-periodic NoA - set "
			   "NoA parameters");
		return hostapd_driver_set_noa(hapd, count, start, duration, interval);
	}

	hapd->noa_enabled = 1;
	hapd->noa_start = start;
	hapd->noa_duration = duration;
	hapd->noa_interval = interval;

	if (hapd->num_sta_no_p2p == 0) {
		wpa_printf(MSG_DEBUG, "P2P: No legacy STAs connected - update "
			   "periodic NoA parameters");
		return hostapd_driver_set_noa(hapd, count, start, duration, interval);
	}

	wpa_printf(MSG_DEBUG, "P2P: Legacy STA(s) connected - do not enable "
		   "periodic NoA");

	return 0;
}

int hostapd_set_p2p_powersave(struct hostapd_data *hapd, int legacy_ps,
			int opp_ps, int ctwindow)
{
	wpa_printf(MSG_DEBUG, "P2P: Set P2P power save: legacy=%d, opportunistic=%d, "
			   "ctwindow=%d", legacy_ps, opp_ps, ctwindow);

	hapd->legacy_ps = legacy_ps;
	hapd->opp_ps = opp_ps;
	hapd->ctwindow = ctwindow;

	if (hapd->num_sta_no_p2p == 0) {
		wpa_printf(MSG_DEBUG, "P2P: No legacy STAs connected - update "
			   "P2P power save parameters");
		return hostapd_driver_set_p2p_powersave(hapd, legacy_ps, opp_ps, ctwindow);
	}

	wpa_printf(MSG_DEBUG, "P2P: Legacy STA(s) connected - do not enable "
		   "P2P power save");

	return 0;
}

void hostapd_p2p_non_p2p_sta_connected(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "P2P: First non-P2P device connected");

	if (hapd->noa_enabled) {
		wpa_printf(MSG_DEBUG, "P2P: Disable periodic NoA");
		hostapd_driver_set_noa(hapd, 0, 0, 0, 0);
	}
	if (hapd->opp_ps == 1) {
		wpa_printf(MSG_DEBUG, "P2P: Disable P2P power save");
		hostapd_driver_set_p2p_powersave(hapd, -1, 0, 0);
	}
}


void hostapd_p2p_non_p2p_sta_disconnected(struct hostapd_data *hapd)
{
	wpa_printf(MSG_DEBUG, "P2P: Last non-P2P device disconnected");

	if (hapd->noa_enabled) {
		wpa_printf(MSG_DEBUG, "P2P: Enable periodic NoA");
		hostapd_driver_set_noa(hapd, 255, hapd->noa_start,
				       hapd->noa_duration, hapd->noa_interval);
	}
	if (hapd->opp_ps == 1) {
		wpa_printf(MSG_DEBUG, "P2P: Enable P2P power save");
		hostapd_driver_set_p2p_powersave(hapd, hapd->legacy_ps, hapd->opp_ps, hapd->ctwindow);
	}
}

#endif /* CONFIG_P2P */


#ifdef CONFIG_P2P_MANAGER
u8 * hostapd_eid_p2p_manage(struct hostapd_data *hapd, u8 *eid)
{
	u8 bitmap;
	*eid++ = WLAN_EID_VENDOR_SPECIFIC;
	*eid++ = 4 + 3 + 1;
	WPA_PUT_BE24(eid, OUI_WFA);
	eid += 3;
	*eid++ = P2P_OUI_TYPE;

	*eid++ = P2P_ATTR_MANAGEABILITY;
	WPA_PUT_LE16(eid, 1);
	eid += 2;
	bitmap = P2P_MAN_DEVICE_MANAGEMENT;
	if (hapd->conf->p2p & P2P_ALLOW_CROSS_CONNECTION)
		bitmap |= P2P_MAN_CROSS_CONNECTION_PERMITTED;
	bitmap |= P2P_MAN_COEXISTENCE_OPTIONAL;
	*eid++ = bitmap;

	return eid;
}
#endif /* CONFIG_P2P_MANAGER */
