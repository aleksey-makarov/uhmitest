LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := UHMITest
LOCAL_SRC_FILES := $(call all-java-files-under, java)
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_JNI_SHARED_LIBRARIES := libnativeegl

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

LOCAL_MODULE := libnativeegl
LOCAL_MULTILIB := first
LOCAL_SRC_FILES := jni/jniapi.cpp jni/renderer.cpp
LOCAL_CC_FLAGS += -Wall -Werror -Wextra -pedantic -O2
LOCAL_SHARED_LIBRARIES := libuhmigl libes2gears libandroid liblog

include $(BUILD_SHARED_LIBRARY)
