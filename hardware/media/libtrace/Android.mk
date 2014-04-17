# build Author : XianGxin

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
   trace.c

LOCAL_LDLIBS := -lpthread
LOCAL_CFLAGS += -DTRACE_DEBUG

LOCAL_MODULE:= libtrace
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
#LOCAL_SHARED_LIBRARIES:=

MULTIMEDIA_PATH := $(LOCAL_PATH)/../../multimedia

LOCAL_C_INCLUDES += \
	$(MULTIMEDIA_PATH)/linux/trace/api

include $(BUILD_SHARED_LIBRARY)
