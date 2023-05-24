LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := libuhmigl
LOCAL_SRC_FILES := drm_state.c libuhmigl.c src/gles2.c src/egl.c rproxy_properties.c libuhmigl_android.c egl_helpers.c pr.c loader.c
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include $(LOCAL_PATH)/include_android
LOCAL_C_FLAGS += -Wall -Wextra -Werror -O2
LOCAL_SHARED_LIBRARIES += libdrm libgbm libnativewindow libandroid liblog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH) $(LOCAL_PATH)/include $(LOCAL_PATH)/include_android
# LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)
