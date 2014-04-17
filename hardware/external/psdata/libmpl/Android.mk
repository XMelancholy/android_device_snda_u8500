LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libmpl
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
   mpl_config.c \
   mpl_list.c \
   mpl_msg.c \
   mpl_param.c

LOCAL_PRELINK_MODULE:= false
include $(BUILD_SHARED_LIBRARY)
