###########################################################
#                                                         #
# Copyright (C) ST-Ericsson SA 2011. All rights reserved. #
# This code is ST-Ericsson proprietary and confidential.  #
# Any use of the code for whatever purpose is subject to  #
# specific written permission of ST-Ericsson SA.          #
#                                                         #
############################################################

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_COPY_HEADERS_TO := ste_mad
#LOCAL_COPY_HEADERS := include/mad/mad.h
LOCAL_SRC_FILES := \
	src/log_client.c \
        src/dbus_handler.c \
        src/dbus_method_table.c \
        src/main.c \
        src/backend/at_handler.c \
        src/backend/tx_bo.c \
        src/backend/fd_handler.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/internal \
	$(LOCAL_PATH)/../common/include \
	$(LOCAL_PATH)/../../external/dbus

LOCAL_CFLAGS := -fno-short-enums -Wall -DCFG_USE_ANDROID_LOG
LOCAL_MODULE := ste_mad
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libutils libdbus
LOCAL_STATIC_LIBRARIES := libaccsutil_log libaccsutil_mainloop libaccsutil_security

include $(BUILD_EXECUTABLE)
