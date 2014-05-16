LOCAL_PATH:= $(call my-dir)

#
# hciattach
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	hciattach.c \
	hciattach_st.c \
	hciattach_ti.c \
	hciattach_tialt.c \
	hciattach_ath3k.c \
	hciattach_qualcomm.c \
	hciattach_intel.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=hciattach

include $(BUILD_EXECUTABLE)

#
# hciconfig
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	hciconfig.c \
	csr.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools \
	$(LOCAL_PATH)/../src

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=hciconfig

include $(BUILD_EXECUTABLE)

#
# hcitool
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	hcitool.c \
	../src/oui.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-DSTORAGEDIR=\"/data/misc/bluetoothd\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools \
	$(LOCAL_PATH)/../src \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib

LOCAL_SHARED_LIBRARIES := \
	libbluetooth libglib

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=hcitool

include $(BUILD_EXECUTABLE)

#
# hcidump
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	hcidump.c \
	parser/parser.c \
	parser/lmp.c \
	parser/hci.c \
	parser/l2cap.c \
	parser/amp.c \
	parser/smp.c \
	parser/att.c \
	parser/sdp.c \
	parser/rfcomm.c \
	parser/bnep.c \
	parser/cmtp.c \
	parser/hidp.c \
	parser/hcrp.c \
	parser/avdtp.c \
	parser/avctp.c \
	parser/avrcp.c \
	parser/sap.c \
	parser/obex.c \
	parser/capi.c \
	parser/ppp.c \
	parser/tcpip.c \
	parser/ericsson.c \
	parser/csr.c \
	parser/bpa.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=hcidump

include $(BUILD_EXECUTABLE)

#
# rfcomm
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rfcomm.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=rfcomm

include $(BUILD_EXECUTABLE)

#
# rctest
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	rctest.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=rctest

include $(BUILD_EXECUTABLE)

#
# l2ping
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	l2ping.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=l2ping

include $(BUILD_EXECUTABLE)

#
# sdptool
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	sdptool.c \
	../src/sdp-xml.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../src \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools \
	$(call include-path-for, glib) \
	$(call include-path-for, glib)/glib

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=sdptool

include $(BUILD_EXECUTABLE)

#
# ciptool
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	ciptool.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=ciptool

include $(BUILD_EXECUTABLE)

#
# bccmd
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	bccmd.c \
	csr.c \
	csr_hci.c \
	csr_usb.c \
	csr_h4.c \
	csr_3wire.c \
	csr_bcsp.c \
	ubcsp.c

LOCAL_CFLAGS:= \
	-DVERSION=\"5.14\" \
	-Wno-missing-field-initializers \
	-Wno-pointer-arith

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH)/../lib \
	$(LOCAL_PATH)/../tools

LOCAL_SHARED_LIBRARIES := \
	libbluetooth

LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:=bccmd

include $(BUILD_EXECUTABLE)
