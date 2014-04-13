A2DP_USES_STANDARD_ANDROID_PATH := true

MULTIMEDIA_PATH := $(LOCAL_PATH)/../../multimedia

$(shell mkdir -p $(TARGET_OUT_INTERMEDIATES)/SHARED_LIBRARIES/libste_adm_intermediates)
$(shell touch $(TARGET_OUT_INTERMEDIATES)/SHARED_LIBRARIES/libste_adm_intermediates/export_includes)

LOCAL_PATH := $(call my-dir)

SRC_FOLDER_AHI := src/ahi
SRC_FOLDER_DBG := src
SRC_FILES_ANM := \
	$(SRC_FOLDER_AHI)/ste_anm_ahi.c \
	$(SRC_FOLDER_AHI)/ste_anm_ahi_output.c \
	$(SRC_FOLDER_AHI)/ste_anm_ahi_input.c \
	$(SRC_FOLDER_AHI)/ste_anm_ahi_admbase.c \
	$(SRC_FOLDER_DBG)/ste_anm_dbg.c \
	$(SRC_FOLDER_DBG)/ste_anm_util.c

SRC_FOLDER_AP := src/policy

SRC_FILES_AP := \
	$(SRC_FOLDER_AP)/ste_anm_ap.c \
	$(SRC_FOLDER_AP)/ste_anm_ext_hal.c \
	$(SRC_FOLDER_AP)/ste_hal_a2dp.c \
	$(SRC_FOLDER_AP)/ste_hal_usb.c \
	$(SRC_FOLDER_DBG)/ste_anm_dbg.c \
	$(SRC_FOLDER_DBG)/ste_anm_util.c

CFLAGS_COMMON_ANM := -DLOG_WARNINGS \
	-DLOG_ERRORS \
    -DSTE_VIDEO_CALL \
	-D_POSIX_SOURCE \
	-DUSE_CACHE_MECHANISM \
	-DALLOW_DUPLICATION

ifeq ($(A2DP_USES_STANDARD_ANDROID_PATH),true)
CFLAGS_COMMON_ANM += -DSTD_A2DP_MNGT
endif

EXTERNAL_INCLUDES_ANM := \
	$(MULTIMEDIA_PATH)/audio/adm/include \
	$(MULTIMEDIA_PATH)/shared/utils/include \
	hardware/libhardware/include \
	$(call include-path-for, audio-effects)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_policy.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libste_adm \
	libcutils \
	libstelpcutils \
	libutils \
	libmedia \
	libdl \
	libc

LOCAL_SRC_FILES := $(SRC_FILES_AP)

LOCAL_CFLAGS += $(CFLAGS_COMMON_ANM)
ifeq ($(BOARD_HAVE_BLUETOOTH),true)
  LOCAL_CFLAGS += -DWITH_BLUETOOTH -DWITH_A2DP
endif

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(EXTERNAL_INCLUDES_ANM)

LOCAL_STATIC_LIBRARIES := \
	libmedia_helper

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := \
	libste_adm \
	libcutils \
	libstelpcutils \
	libutils \
	libmedia \
	libhardware \
	liblog \
	libasound \
	libdl

LOCAL_SRC_FILES += \
	$(SRC_FILES_ANM)

LOCAL_CFLAGS += \
	$(CFLAGS_COMMON_ANM) \

LOCAL_C_INCLUDES += \
	$(EXTERNAL_INCLUDES_ANM) \
	$(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES += \
	libmedia_helper

include $(BUILD_SHARED_LIBRARY)
