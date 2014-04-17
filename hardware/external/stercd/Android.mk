LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stercd
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := stercd.c sterc_handler.c sterc_runscript.c sterc_pscc.c sterc_if_ppp.c sterc_log_client.c
LOCAL_SHARED_LIBRARIES := libstecom libmpl libsterc liblog libpscc libutils
LOCAL_STATIC_LIBRARIES := libaccsutil_security
LOCAL_CFLAGS := -DSTERC_SW_VARIANT_ANDROID -DCFG_USE_ANDROID_LOG
LOCAL_C_INCLUDES := \
  $(LOCAL_PATH)/../libstecom \
  $(LOCAL_PATH)/../libmpl \
  $(LOCAL_PATH)/../libnlcom \
  $(LOCAL_PATH)/../libpscc \
  $(LOCAL_PATH)/../libsterc \
  $(LOCAL_PATH)/../../modem/common/include

include $(BUILD_EXECUTABLE)
