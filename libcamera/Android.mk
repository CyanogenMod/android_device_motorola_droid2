LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= Droid2CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES:= libdl libutils libcutils libcamera_client
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libcamera
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

