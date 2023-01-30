/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include <stdlib.h>
#include <gbm.h>
#include <xf86drmMode.h>
#include <glad/egl.h>
#include <glad/gles2.h>
#include <dlfcn.h>

#include <assert.h>

#include "animated_pointer.h"
#include "drm_state.h"
#include "egl_helpers.h"
#define LOG_TAG "libuhmigl"
#include "pr.h"

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;
static void *dl_handle;
static void *eglGetProcAddress_ptr;

#ifdef __ANDROID__
static const char * dl_path_default = "/system/lib64/drm/libGLES_mesa.so";
#else
#error "FIXME"
#endif
static const char * dl_path_env_var_name = "LIBUHMIGL_DL_PATH";

int libuhmigl_init(uint16_t *h, uint16_t *v)
{
	EGLConfig config;
	EGLint num_config = 0;
	int err;
	int version;

	pr_info("drm_state_init()");
	err = drm_state_init();
	if (err) {
		pr_err("drm_state_init()");
		goto error;
	}

	assert(drm_state_gbm_device);
	assert(drm_state_gbm_surface);
	assert(drm_state_mode);

	if (h)
		*h = drm_state_mode->hdisplay;
	if (v)
		*v = drm_state_mode->vdisplay;

	const char * dl_path = getenv(dl_path_env_var_name);

	if (!dl_path)
		dl_path = dl_path_default;

	pr_info("dlopen(%s)", dl_path);
	dl_handle = dlopen(dl_path, RTLD_NOW);
	if (!dl_handle) {
		pr_err("dlopen(%s): %s", dl_path, dlerror());
		goto error_drm_state_done;
	}

	// clear any existing error
	dlerror();

	pr_info("dlsym(\"eglGetProcAddress\")");
	eglGetProcAddress_ptr = dlsym(dl_handle, "eglGetProcAddress");
	if (!eglGetProcAddress_ptr) {
		char *dlerror_string = dlerror();
		pr_err("dlsym(\"eglGetProcAddress\"): %s", dlerror_string ? dlerror_string : "???");
		goto error_dlclose;
	}

	pr_info("gladLoadEGL(EGL_NO_DISPLAY, gl_eglGetProcAddress)");
	version = gladLoadEGL(EGL_NO_DISPLAY, ANIMATED_POINTER(GLADloadfunc, eglGetProcAddress_ptr));
	if (version == 0) {
		pr_err("gladLoadEGL(EGL_NO_DISPLAY, gl_eglGetProcAddress)");
		goto error_dlclose;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, drm_state_gbm_device, NULL);
	if (display == EGL_NO_DISPLAY)
		EGL_CHECK_ERROR("eglGetPlatformDisplayEXT()", error_dlclose);

	pr_info("eglInitialize()");
	EGLint major, minor;
	EGL_RET(eglInitialize(display, &major, &minor), error_egl_terminate);
	pr_info("EGL: %d.%d", (int)major, (int)minor);

	pr_info("gladLoadEGL(display, gl_eglGetProcAddress)");
	version = gladLoadEGL(display, ANIMATED_POINTER(GLADloadfunc, eglGetProcAddress_ptr));
	if (version == 0) {
		pr_err("gladLoadEGL(display, gl_eglGetProcAddress)");
		goto error_egl_terminate;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	pr_info("eglBindAPI(EGL_OPENGL_ES_API)");
	EGL_RET(eglBindAPI(EGL_OPENGL_ES_API), error_egl_terminate);

	static const EGLint config_attrs[] =
		{ EGL_SURFACE_TYPE,  EGL_WINDOW_BIT
		, EGL_RED_SIZE,        8
		, EGL_GREEN_SIZE,      8
		, EGL_BLUE_SIZE,       8
		, EGL_ALPHA_SIZE,      8
		, EGL_DEPTH_SIZE,      16
		, EGL_CONFORMANT,      EGL_OPENGL_ES2_BIT
		, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT
		, EGL_NONE
		};

	pr_info("eglChooseConfig()");
	EGL_RET(eglChooseConfig(display, config_attrs, &config, 1, &num_config), error_egl_release_thread);
	if (num_config < 1) {
		pr_err("num_config < 1");
		goto error_egl_release_thread;
	}

	pr_info("eglCreateWindowSurface()");
	surface = eglCreateWindowSurface(display, config, drm_state_gbm_surface, NULL);
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
	pr_info("Loaded OpenGLES2 %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

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
	dlclose(dl_handle);

error_drm_state_done:
	pr_info("drm_state_done()");
	drm_state_done();
error:
	return -1;
}

void libuhmigl_done(void)
{
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(display, context);
	eglDestroySurface(display, surface);
	eglReleaseThread();
	eglTerminate(display);
	dlclose(dl_handle);
	drm_state_done();
}

int libuhmigl_update(void)
{
	int err;

	EGL_RET(eglSwapBuffers(display, surface), error);

	err = drm_state_flip();
	if (err) {
		pr_err("drm_state_flip()");
		goto error;
	}

	return 0;

error:
	return -1;
}

int libuhmigl_load(void)
{
	int version;

	pr_info("gladLoadEGL(display, gl_eglGetProcAddress)");
	version = gladLoadEGL(display, ANIMATED_POINTER(GLADloadfunc, eglGetProcAddress_ptr));
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
	pr_info("Loaded OpenGLES2 %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	return 0;
error:
	return -1;
}
