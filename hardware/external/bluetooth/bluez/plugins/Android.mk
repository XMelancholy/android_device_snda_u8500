LOCAL_PATH:= $(call my-dir)

#
# libplugin
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	autopair.c \
	external-dummy.c \
	hostname.c \
	neard.c \
	policy.c \
	wiimote.c \

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-DBLUETOOTH_PLUGIN_BUILTIN \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../btio \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../src \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib \
	$(LOCAL_PATH)/../../../dbus/dbus

LOCAL_SHARED_LIBRARIES := \
	libbluetooth \
	libcutils \
	libdbus \
	libglib \

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/bluez-plugin
LOCAL_UNSTRIPPED_PATH := $(TARGET_OUT_SHARED_LIBRARIES_UNSTRIPPED)/bluez-plugin
LOCAL_MODULE:=libbuiltinplugin

include $(BUILD_STATIC_LIBRARY)
