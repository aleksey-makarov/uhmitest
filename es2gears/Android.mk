LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libes2gears
LOCAL_SRC_FILES := es2gears.c
LOCAL_C_FLAGS += -Wall -Wextra -pedantic -Werror -O2
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := libuhmigl

include $(BUILD_SHARED_LIBRARY)
