LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := uhmitest
LOCAL_SRC_FILES := uhmitest.c
LOCAL_C_FLAGS += -Wall -Wextra -pedantic -Werror -O2
LOCAL_SHARED_LIBRARIES += libuhmigl libes2gears
LOCAL_VENDOR_MODULE := false

include $(BUILD_EXECUTABLE)
