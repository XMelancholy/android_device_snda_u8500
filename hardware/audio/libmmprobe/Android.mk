#
# Copyright (C) ST-Ericsson SA 2012. All rights reserved.
# This code is ST-Ericsson proprietary and confidential.
# Any use of the code for whatever purpose is subject to
# specific written permission of ST-Ericsson SA.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=  \
        src/client.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

LOCAL_MODULE           := libmmprobe
LOCAL_MODULE_TAGS      := optional
LOCAL_PRELINK_MODULE   := false

LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_SHARED_LIBRARY)
