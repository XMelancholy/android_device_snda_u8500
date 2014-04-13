LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_STATIC_JAVA_LIBRARIES := \
   android-support-v13 \
   android-support-v4

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := FileExplorer

#LOCAL_JAVA_LIBRARIES := javax.obex

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
