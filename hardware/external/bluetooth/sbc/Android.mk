LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	sbc/sbc.c \
	sbc/sbc_primitives.c \
	sbc/sbc_primitives_mmx.c \
	sbc/sbc_primitives_iwmmxt.c \
	sbc/sbc_primitives_neon.c \
	sbc/sbc_primitives_armv6.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/ \
	$(LOCAL_PATH)/src/

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libsbc

include $(BUILD_SHARED_LIBRARY)
