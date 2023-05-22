/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include <libuhmigl.h>

#include <stdlib.h>
#include <gbm.h>
#include <xf86drmMode.h>
#include <string.h>

#ifdef GL_STATIC_LINKING
# include <EGL/egl.h>
# include <EGL/eglext.h>
# include <EGL/eglplatform.h>
#else
# include <glad/egl.h>
#endif

#include <assert.h>

#include "drm_state.h"
#include "egl_helpers.h"
#include "loader.h"
#define LOG_TAG "libuhmigl"
#include "pr.h"

static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;

int libuhmigl_init(uint16_t *h, uint16_t *v)
{
	EGLConfig config;
	EGLint num_config = 0;
	int err;

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

	pr_info("loader_load_egl(EGL_NO_DISPLAY)");
	err = loader_load_egl(EGL_NO_DISPLAY);
	if (err) {
		pr_err("loader_load_egl()");
		goto error_drm_state_done;
	}

#ifdef GL_STATIC_LINKING
	pr_info("eglQueryString(EGL_EXTENSIONS)");
	char const * const supported_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

	pr_info("is EGL_EXT_platform_base supported?");
	if (supported_extensions && strstr(supported_extensions, "EGL_EXT_platform_base")) {

		pr_info("eglGetProcAddress(\"eglGetPlatformDisplayEXT\")");
		typedef EGLDisplay eglGetPlatformDisplayEXT_t(EGLenum platform, void *native_display, const EGLint *attrib_list);
		eglGetPlatformDisplayEXT_t *get_platform_display_ext = (eglGetPlatformDisplayEXT_t *)eglGetProcAddress("eglGetPlatformDisplayEXT");
		if (get_platform_display_ext) {

			pr_info("eglGetPlatformDisplayEXT()");
			display = get_platform_display_ext(EGL_PLATFORM_GBM_KHR, drm_state_gbm_device, NULL);
			if (!display) {
				pr_err("eglGetPlatformDisplayEXT(), 0x%x", (unsigned)eglGetError());
			}

		} else {
			pr_err("eglGetProcAddress(\"eglGetPlatformDisplayEXT\")");
		}
       }

	if (!display) {
		pr_info("eglGetDisplay()");
		display = eglGetDisplay(drm_state_gbm_device);
		if (!display)
			EGL_CHECK_ERROR("eglGetDisplay()", error_loader_done);
	}

#else

	pr_info("eglGetPlatformDisplayEXT()");
	display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, drm_state_gbm_device, NULL);
	if (display == EGL_NO_DISPLAY)
		EGL_CHECK_ERROR("eglGetPlatformDisplayEXT()", error_loader_done);

#endif

	pr_info("eglInitialize()");
	EGLint major, minor;
	EGL_RET(eglInitialize(display, &major, &minor), error_egl_terminate);
	pr_info("EGL: %d.%d", (int)major, (int)minor);

	pr_info("loader_load_egl(display)");
	err = loader_load_egl(display);
	if (err) {
		pr_err("loader_load_egl()");
		goto error_egl_terminate;
	}

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

	pr_info("loader_load_gles()");
	err = loader_load_gles();
	if (err) {
		pr_err("loader_load_gles()");
		goto error_egl_make_current;
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

error_loader_done:
	loader_done();

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
	loader_done();
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
	int err;

	pr_info("loader_load_egl(display)");
	err = loader_load_egl(display);
	if (err) {
		pr_err("loader_load_egl()");
		goto error;
	}


	pr_info("loader_load_gles()");
	err = loader_load_gles();
	if (err) {
		pr_err("loader_load_gles()");
		goto error;
	}


	return 0;
error:
	return -1;
}
