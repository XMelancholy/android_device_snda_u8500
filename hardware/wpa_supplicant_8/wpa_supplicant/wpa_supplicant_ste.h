/*
 * ST-Ericsson hostap/wpa_supplicant changes and additions.
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
 */

#ifndef _WPA_SUPPLICANT_STE_H_
#define _WPA_SUPPLICANT_STE_H_

struct hostap_sta_driver_data;
/**
 * wpa_supplicant_ste_get_country - Get country code for interface
 * @wpa_s: wpa_supplicant structure for a network interface
 * @alpha2: pointer to country code
 * Returns: 0 if succeed or -1 if alpha2 has an invalid value
 *
 */
int wpa_supplicant_ste_get_country(struct wpa_supplicant *wpa_s, char *alpha2);

/**
 * wpa_supplicant_ste_set_country - Set country code for interface
 * @wpa_s: wpa_supplicant structure for a network interface
 * @pos: pointer to country code
 * Returns: 0 if succeed or -1 if alpha2 has an invalid value
 */
int wpa_supplicant_ste_set_country(struct wpa_supplicant *wpa_s, const char *pos);

/**
 * wpa_supplicant_ste_get_sta_data - Retrieves station link infomation
 * @wpa_s: Pointer to wpa_supplicant data
 * @data:  hostap driver status information
 * Returns: 0 if succeed or -1 error
 *
 * This function is called to retrieve station link information.
 */
int wpa_supplicant_ste_get_sta_data(struct wpa_supplicant *wpa_s, struct hostap_sta_driver_data *data);

/**
 * wpa_supplicant_ste_drv_start - Start wlan driver
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is called to start wlan driver.
 */
void wpa_supplicant_ste_drv_start(struct wpa_supplicant *wpa_s);

/**
 * wpa_supplicant_ste_drv_stop - Stop wlan driver
 * @wpa_s: Pointer to wpa_supplicant data
 *
 * This function is called to stop wlan driver.
 */
void wpa_supplicant_ste_drv_stop(struct wpa_supplicant *wpa_s);

/**
 * wpa_supplicant_ste_get_country_code_from_at - Retrieves MCC from AT module
 * @wpa_s: wpa_supplicant structure for a network interface
 * Returns: 0 if succeed or -1 an error has occurred
 *
 * Map MCC to alpha2 code and set in supplicant's structure for network interface.
 */
int wpa_supplicant_ste_get_country_code_from_at(struct wpa_supplicant *wpa_s);

#ifdef CONFIG_DATA_PKT_FILTER
struct set_filter;
/**
 * wpa_supplicant_ste_set_filter
 * @wpa_s: wpa_supplicant structure for a network interface
 * @filter: filter parameter structure
 * Returns: 0 if succeed or -1  driver returns an error
 *
 * This function is called to set data filter on device .
 */
int wpa_supplicant_ste_set_filter(struct wpa_supplicant *wpa_s, struct set_filter *filter);
#endif
/**
 * wpa_supplicant_ste_powermode - Enable / disable powersave
 * @wpa_s wpa_supplicant structure for a network interface
 * @mode: 1 = active power mode (full power), 0 = auto power mode (power save)
 * Returns: 0 on success
 *
 * This function is called to enable or disable powersave
 */
int wpa_supplicant_ste_powermode(struct wpa_supplicant *wpa_s, int mode);

#endif /* _WPA_SUPPLICANT_STE_H_ */
