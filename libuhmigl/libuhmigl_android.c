/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>

#include <android/native_window.h>

#include <glad/egl.h>
#include <glad/gles2.h>

#include "egl_helpers.h"
#define LOG_TAG "libuhmigl"
#include "pr.h"

static const char * android_dl_path_default = "libEGL.so";
static const char * android_dl_path_env_var_name = "LIBUHMIGL_ANDROID_DL_PATH";
static void *android_dl_handle;
static void *eglGetProcAddress_ptr;

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

int  libuhmigl_android_init(struct ANativeWindow *window, uint16_t *h, uint16_t *v)
{
	static const EGLint attribs[] =
		{ EGL_SURFACE_TYPE,    EGL_WINDOW_BIT
		, EGL_RED_SIZE,        8
		, EGL_GREEN_SIZE,      8
		, EGL_BLUE_SIZE,       8
		, EGL_ALPHA_SIZE,      8
		, EGL_DEPTH_SIZE,      16
		, EGL_CONFORMANT,      EGL_OPENGL_ES2_BIT
		, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT
		, EGL_NONE
		};

	EGLConfig config;
	EGLint num_config;
	EGLint format;

	int version;

	assert(window);

	const char * android_dl_path = getenv(android_dl_path_env_var_name);

	if (!android_dl_path)
		android_dl_path = android_dl_path_default;

	pr_info("dlopen(%s)", android_dl_path);
	android_dl_handle = dlopen(android_dl_path, RTLD_NOW);
	if (!android_dl_handle) {
		pr_err("dlopen(%s): %s", android_dl_path, dlerror());
		goto error;
	}

	// clear any existing error
	dlerror();

	pr_info("dlsym(\"eglGetProcAddress\")");
	eglGetProcAddress_ptr = dlsym(android_dl_handle, "eglGetProcAddress");
	if (!eglGetProcAddress_ptr) {
		char *dlerror_string = dlerror();
		pr_err("dlsym(\"eglGetProcAddress\"): %s", dlerror_string ? dlerror_string : "???");
		goto error_dlclose;
	}

	pr_info("gladLoadEGL(EGL_NO_DISPLAY, gl_eglGetProcAddress)");
	version = gladLoadEGL(EGL_NO_DISPLAY, eglGetProcAddress_ptr);
	if (version == 0) {
		pr_err("gladLoadEGL(EGL_NO_DISPLAY, gl_eglGetProcAddress)");
		goto error_dlclose;
	}

	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	pr_info("eglGetDisplay()");
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY)
		EGL_CHECK_ERROR("eglGetDisplay()", error_dlclose);

	pr_info("eglInitialize()");
	EGLint major, minor;
	EGL_RET(eglInitialize(display, &major, &minor), error_egl_terminate);
	pr_info("EGL: %d.%d", (int)major, (int)minor);

	pr_info("gladLoadEGL(display, gl_eglGetProcAddress)");
	version = gladLoadEGL(display, eglGetProcAddress_ptr);
	if (version == 0) {
		pr_err("gladLoadEGL(display, gl_eglGetProcAddress)");
		goto error_egl_terminate;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	pr_info("eglBindAPI(EGL_OPENGL_ES_API)");
	EGL_RET(eglBindAPI(EGL_OPENGL_ES_API), error_egl_terminate);

	pr_info("eglChooseConfig()");
	EGL_RET(eglChooseConfig(display, attribs, &config, 1, &num_config), error_egl_release_thread);
	if (num_config < 1) {
		pr_err("num_config < 1");
		goto error_egl_release_thread;
	}

	pr_info("eglGetConfigAttrib()");
	EGL_RET(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format), error_egl_release_thread);

	ANativeWindow_setBuffersGeometry(window, 0, 0, format);

	pr_info("eglCreateWindowSurface()");
	surface = eglCreateWindowSurface(display, config, window, NULL);
	if (surface == EGL_NO_SURFACE)
		EGL_CHECK_ERROR("eglCreateWindowSurface()", error_egl_release_thread);


	static const EGLint context_attrs[] =
		{ EGL_CONTEXT_CLIENT_VERSION, 2
		, EGL_NONE
		};

	pr_info("eglCreateContext()");
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attrs);
	if (context == EGL_NO_CONTEXT)
		EGL_CHECK_ERROR("eglCreateContext()", error_egl_destroy_surface);

	pr_info("eglMakeCurrent()");
	EGL_RET(eglMakeCurrent(display, surface, surface, context), error_egl_destroy_context);

	pr_info("gladLoadGLES2(eglGetProcAddress)");
	version = gladLoadGLES2(eglGetProcAddress);
	if (version == 0) {
		pr_err("gladLoadGLES2(eglGetProcAddress)");
		goto error_egl_make_current;
	}

	// Successfully loaded OpenGLES2
	pr_info("Loaded OpenGLES2 %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	if (h) {
		EGLint width;
		pr_info("eglQuerySurface(width)");
		EGL_RET(eglQuerySurface(display, surface, EGL_WIDTH, &width), error_egl_make_current);
		*h = width;
	}

	if (v) {
		EGLint height;
		pr_info("eglQuerySurface(height)");
		EGL_RET(eglQuerySurface(display, surface, EGL_HEIGHT, &height), error_egl_make_current);
		*v = height;
	}

	return 0;

error_egl_make_current:
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

error_egl_destroy_context:
	eglDestroyContext(display, context);

error_egl_destroy_surface:
	eglDestroySurface(display, surface);

error_egl_release_thread:
	eglReleaseThread();

error_egl_terminate:
	eglTerminate(display);

error_dlclose:
	dlclose(android_dl_handle);

error:
	return -1;
}

void libuhmigl_android_done(void)
{
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(display, context);
	eglDestroySurface(display, surface);
	eglReleaseThread();
	eglTerminate(display);
	dlclose(android_dl_handle);
}

int  libuhmigl_android_update(void)
{
	EGL_RET(eglSwapBuffers(display, surface), error);
	return 0;
error:
	return -1;
}

int libuhmigl_android_load(void)
{
	int version;

	pr_info("gladLoadEGL(display, gl_eglGetProcAddress)");
	version = gladLoadEGL(display, eglGetProcAddress_ptr);
	if (version == 0) {
		pr_err("gladLoadEGL(display, gl_eglGetProcAddress)");
		goto error;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	pr_info("gladLoadGLES2(eglGetProcAddress)");
	version = gladLoadGLES2(eglGetProcAddress);
	if (version == 0) {
		pr_err("gladLoadGLES2(eglGetProcAddress)");
		goto error;
	}

	return 0;
error:
	return -1;
}
