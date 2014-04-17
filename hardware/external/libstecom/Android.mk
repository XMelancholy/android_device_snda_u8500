LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
   stecom.c \
   subscribe.c

LOCAL_MODULE := libstecom
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false

LOCAL_CFLAGS := -DSW_VARIANT_ANDROID

LOCAL_SHARED_LIBRARIES := \
   libutils \
   libcutils \
   liblog

include $(BUILD_SHARED_LIBRARY)
