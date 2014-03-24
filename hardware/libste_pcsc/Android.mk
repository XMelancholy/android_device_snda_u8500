LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	ste_pcsc.c \
	ste_pcsc_utils.c \
	modem_utils_ril.c

LOCAL_CFLAGS += -DRIL

LOCAL_LDLIBS += -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_MODULE    := libste_pcsc
LOCAL_MODULE_TAGS := optional
LOCAL_COPY_HEADERS := ste_pcsc.h
include $(BUILD_STATIC_LIBRARY)

