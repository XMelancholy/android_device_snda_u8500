# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libomxil-bellagio
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES := \
   src/st_static_component_loader.c \
   src/omxcore.c \
   src/omx_create_loaders_linux.c \
   src/tsemaphore.c \
   src/queue.c \
   src/utils.c \
   src/common.c \
   src/content_pipe_inet.c \
   src/content_pipe_file.c \
   src/omx_reference_resource_manager.c \
   src/getline.c \
   src/base/omx_base_component.c \
   src/base/omx_base_port.c \
   src/base/omx_base_filter.c \
   src/base/omx_base_sink.c \
   src/base/omx_base_source.c \
   src/base/omx_base_audio_port.c \
   src/base/omx_base_video_port.c \
   src/base/omx_base_image_port.c \
   src/base/omx_base_clock_port.c \
   src/base/OMXComponentRMExt.c

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/include \
   $(LOCAL_PATH)/src/base \
   $(LOCAL_PATH)/src

LOCAL_CFLAGS += -DRELEASE -D__RELEASE
LOCAL_SHARED_LIBRARIES := \
   libdl \
   liblog

include $(BUILD_SHARED_LIBRARY)
