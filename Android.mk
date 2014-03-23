LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),u8500)
    include $(call first-makefiles-under,$(LOCAL_PATH))
endif
