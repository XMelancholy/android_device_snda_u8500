/*
 * WPA Supplicant - background scan and roaming interface
 * Copyright (c) 2009-2010, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef BGSCAN_H
#define BGSCAN_H

struct wpa_supplicant;
struct wpa_ssid;

#define BGSCAN_PENALTY_TIME 30
#define BGSCAN_TEMPORARY_OFFSET 5

struct bgscan_ops {
	const char *name;

	void * (*init)(struct wpa_supplicant *wpa_s, const char *params,
		       const struct wpa_ssid *ssid);
	void (*deinit)(void *priv);

	int (*notify_scan)(void *priv, struct wpa_scan_results *scan_res);
	void (*notify_beacon_loss)(void *priv);
	void (*notify_tx_failure)(void *priv);
	void (*notify_signal_change)(void *priv, int above,
				     int current_signal,
				     int current_noise,
				     int current_txrate);
#ifdef ENABLE_STE_CHANGES
	int (*check_quick_scan)(void *priv);
	void (*set_quick_scan)(void *priv, int quick_scan);
#endif /* ENABLE_STE_CHANGES */
};

#ifdef CONFIG_BGSCAN

int bgscan_init(struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
void bgscan_deinit(struct wpa_supplicant *wpa_s);
int bgscan_notify_scan(struct wpa_supplicant *wpa_s,
		       struct wpa_scan_results *scan_res);
#ifdef ENABLE_STE_CHANGES
int bgscan_check_quick_scan(struct wpa_supplicant *wpa_s);
void bgscan_set_quick_scan(struct wpa_supplicant *wpa_s, int quick_scan);
#endif /* ENABLE_STE_CHANGES */
void bgscan_notify_beacon_loss(struct wpa_supplicant *wpa_s);
void bgscan_notify_tx_failure(struct wpa_supplicant *wpa_s);
void bgscan_notify_signal_change(struct wpa_supplicant *wpa_s, int above,
				 int current_signal, int current_noise,
				 int current_txrate);

#else /* CONFIG_BGSCAN */

static inline int bgscan_init(struct wpa_supplicant *wpa_s,
			      struct wpa_ssid *ssid)
{
	return 0;
}

static inline void bgscan_deinit(struct wpa_supplicant *wpa_s)
{
}

static inline int bgscan_notify_scan(struct wpa_supplicant *wpa_s,
				     struct wpa_scan_results *scan_res)
{
	return 0;
}

#ifdef ENABLE_STE_CHANGES
static inline int bgscan_check_quick_scan(struct wpa_supplicant *wpa_s)
{
	return 0;
}

static inline void bgscan_set_quick_scan(void *priv, int quick_scan)
{
}
#endif /* ENABLE_STE_CHANGES */

static inline void bgscan_notify_beacon_loss(struct wpa_supplicant *wpa_s)
{
}

static inline int bgscan_notify_tx_failure(struct wpa_supplicant *wpa_s)
{
	return 0;
}

static inline void bgscan_notify_signal_change(struct wpa_supplicant *wpa_s,
					       int above, int current_signal,
					       int current_noise,
					       int current_txrate)
{
}

#endif /* CONFIG_BGSCAN */

#endif /* BGSCAN_H */