#/************************************************************************
# *                                                                      *
# *  Copyright (C) 2010 ST-Ericsson                                      *
# *                                                                      *
# *  Author: Joakim AXELSSON <joakim.axelsson AT stericsson.com>         *
# *  Author: Sebastian RASMUSSEN <sebastian.rasmussen AT stericsson.com> *
# *                                                                      *
# ************************************************************************/

# Make sure that statements below are included if only MSA is being built

LOCAL_PATH:= $(call my-dir)

#-ffunction-sections -fdata-sections will section the functions and data during compilation
#-Wl,--gc-sections will remove the sections not used during linking
common_cflags := -Wall -Werror -std=gnu99 -ffunction-sections -fdata-sections -Wl,--gc-sections

include $(CLEAR_VARS)

LOCAL_CFLAGS += $(common_cflags)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mal/libshmnetlnk/include

LOCAL_SHARED_LIBRARIES += libshmnetlnk

LOCAL_SRC_FILES := \
	main.c \
	config.c \
	log.c \
	fsa.c \
	posix.c \
	convenience.c \
	process.c \
	state.c \
	wakelock.c

LOCAL_MODULE := msa
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
