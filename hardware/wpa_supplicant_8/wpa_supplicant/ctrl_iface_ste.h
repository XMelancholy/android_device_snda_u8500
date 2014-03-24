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

#ifndef CTRL_IFACE_STE_H
#define CTRL_IFACE_STE_H

#ifdef CONFIG_CTRL_IFACE

/**
 * wpa_supplicant_ctrl_iface_process_ste_commands - Process ctrl_iface command
 * @wpa_s: Pointer to wpa_supplicant data
 * @buf: Received command buffer (nul terminated string)
 * @reply: Reply buffer
 * @reply_size: Variable to be set to the reply buffer size
 * Returns: Response (*resp_len bytes) or 0 if command is not handled
 *
 * wpa_supplicant_ctrl_iface_process() call this function. It checks
 * if the message received by control interface backends is ST-Ericsson specific.
 */
int wpa_supplicant_ctrl_iface_process_ste_commands(struct wpa_supplicant *wpa_s,
					 char *buf, char *reply, const size_t reply_size);

#endif /* CONFIG_CTRL_IFACE */
#endif /* CTRL_IFACE_STE_H */
