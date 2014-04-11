#######################################################################################
#  (c) ST-Ericsson, 2010 - All rights reserved
#  Reproduction and Communication of this document is strictly prohibited
#  unless specifically authorized in writing by ST-Ericsson
#
#  \brief   Android makefile for STE Video Renderer to be used by Stagefright framework
#  \author  ST-Ericsson
#######################################################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

$(shell mkdir -p $(TARGET_OUT_INTERMEDIATES)/SHARED_LIBRARIES/libstelpcutils_intermediates)

$(shell touch $(TARGET_OUT_INTERMEDIATES)/SHARED_LIBRARIES/libstelpcutils_intermediates/export_includes)

LOCAL_SRC_FILES = \
    STECodecsPlugin.cpp

LOCAL_CFLAGS += -DBOARD_USES_OVERLAY=1
LOCAL_CFLAGS += -DDBGT_CONFIG_DEBUG -DDBGT_CONFIG_AUTOVAR -DLOG_TAG=\"libstagefrighthw\"

LOCAL_SHARED_LIBRARIES:= \
	libutils \
	libbinder \
	libui \
	libcutils \
	libstagefright \
	libstagefright_foundation \
	libsurfaceflinger \
	libdl \
	liblog \
	libstelpcutils

LOCAL_C_INCLUDES += \
	$(TOP)/frameworks/native/include/media/hardware \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/av/include/media/stagefright/foundation \
	$(LOCAL_PATH)/include

LOCAL_MODULE:= libstagefrighthw
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
