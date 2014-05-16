LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	bluetooth.c \
	hci.c \
	sdp.c \
	uuid.c \

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/bluetooth \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	liblog \

LOCAL_MODULE:=libbluetooth

LOCAL_CFLAGS += -O3 -Wno-missing-field-initializers -Wno-pointer-arith -DHAVE_CONFIG_H

include $(BUILD_SHARED_LIBRARY)
