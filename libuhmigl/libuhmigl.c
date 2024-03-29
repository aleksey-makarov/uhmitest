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
static struct gbm_surface *gbm_surface;
static struct gbm_device *gbm_device;
static EGLContext context;

#ifdef GL_STATIC_LINKING
typedef EGLDisplay eglGetPlatformDisplayEXT_t(EGLenum platform, void *native_display, const EGLint *attrib_list);
typedef EGLSurface eglCreatePlatformWindowSurfaceEXT_t(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
#endif

int libuhmigl_init(uint16_t *h, uint16_t *v)
{
	EGLint num_config;
	EGLint num_config2;
	int err;

	display = EGL_NO_DISPLAY;
	surface = EGL_NO_SURFACE;

	pr_info("drm_state_init()");
	gbm_device = drm_state_init(h, v);
	if (!gbm_device) {
		pr_err("drm_state_init()");
		goto error;
	}

	pr_info("loader_load_egl(EGL_NO_DISPLAY)");
	err = loader_load_egl(EGL_NO_DISPLAY);
	if (err) {
		pr_err("loader_load_egl()");
		goto error_drm_state_done;
	}

#ifdef GL_STATIC_LINKING
	pr_info("eglQueryString(EGL_EXTENSIONS)");
	char const * const supported_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

	eglGetPlatformDisplayEXT_t *get_platform_display_ext = NULL;
	eglCreatePlatformWindowSurfaceEXT_t *create_platform_window_surface_ext = NULL;

	pr_info("is EGL_EXT_platform_base supported?");
	if (supported_extensions && strstr(supported_extensions, "EGL_EXT_platform_base")) {
		pr_info("eglGetProcAddress(\"eglGetPlatformDisplayEXT\")");
		get_platform_display_ext = (eglGetPlatformDisplayEXT_t *)eglGetProcAddress("eglGetPlatformDisplayEXT");

		pr_info("eglGetProcAddress(\"eglCreatePlatformWindowSurfaceEXT\")");
		create_platform_window_surface_ext = (eglCreatePlatformWindowSurfaceEXT_t *)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");
	}

	if (get_platform_display_ext) {

		pr_info("get_platform_display_ext()");
		display = get_platform_display_ext(EGL_PLATFORM_GBM_KHR, gbm_device, NULL);
		if (display == EGL_NO_DISPLAY) {
			pr_err("eglGetPlatformDisplayEXT(), 0x%x", (unsigned)eglGetError());
		}

	} else {
		pr_err("eglGetProcAddress(\"eglGetPlatformDisplayEXT\")");
	}

	if (display == EGL_NO_DISPLAY) {
		pr_info("eglGetDisplay()");
		display = eglGetDisplay(gbm_device);
		if (display == EGL_NO_DISPLAY)
			EGL_CHECK_ERROR("eglGetDisplay()", error_loader_done);
	}

#else

	pr_info("eglGetPlatformDisplayEXT()");
	display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm_device, NULL);
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
		, EGL_DEPTH_SIZE,      16
		, EGL_CONFORMANT,      EGL_OPENGL_ES2_BIT
		, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT
		, EGL_NONE
		};

	pr_info("eglChooseConfig(0)");
	EGL_RET(eglChooseConfig(display, config_attrs, NULL, 0, &num_config), error_egl_release_thread);
	if (num_config < 1) {
		pr_err("num_config < 1");
		goto error_egl_release_thread;
	}

	// FIXME: Where is it free()ed?
	EGLConfig *configs = calloc(num_config, sizeof(EGLConfig));
	assert(configs);

	pr_info("eglChooseConfig(%u)", num_config);
	EGL_RET(eglChooseConfig(display, config_attrs, configs, num_config, &num_config2), error_egl_release_thread);
	if (num_config != num_config2) {
		pr_err("num_config != num_config2");
		goto error_egl_release_thread;
	}

#define GET_ATTRIBUTE(NAME) \
		EGLint attr_ ## NAME; \
		EGL_RET(eglGetConfigAttrib(display, configs[i], EGL_ ## NAME, & attr_ ## NAME), error_egl_release_thread);

	int i;
	int valid_config_index = -1;

	for (i = 0; i < num_config; i++) {

		GET_ATTRIBUTE(NATIVE_VISUAL_ID);

		if (valid_config_index == -1 && attr_NATIVE_VISUAL_ID == GBM_FORMAT_XRGB8888) {
			valid_config_index = i;

			GET_ATTRIBUTE(CONFIG_ID);
			GET_ATTRIBUTE(BUFFER_SIZE);
			GET_ATTRIBUTE(ALPHA_SIZE);
			GET_ATTRIBUTE(RED_SIZE);
			GET_ATTRIBUTE(GREEN_SIZE);
			GET_ATTRIBUTE(BLUE_SIZE);
			GET_ATTRIBUTE(DEPTH_SIZE);
			GET_ATTRIBUTE(STENCIL_SIZE);
			GET_ATTRIBUTE(SAMPLES);

			char format_string[5];
			format_string[0] = (char)((attr_NATIVE_VISUAL_ID >>  0) & 0xff);
			format_string[1] = (char)((attr_NATIVE_VISUAL_ID >>  8) & 0xff);
			format_string[2] = (char)((attr_NATIVE_VISUAL_ID >> 16) & 0xff);
			format_string[3] = (char)((attr_NATIVE_VISUAL_ID >> 24) & 0xff);
			format_string[4] = 0;

			pr_info("id: %u, format=%u (\"%s\"), buffer_size=%u, alpha=%u, red=%u, green=%u, blue=%u, depth=%u, stencil=%u, samples=%u",
				attr_CONFIG_ID, attr_NATIVE_VISUAL_ID, format_string,
				attr_BUFFER_SIZE, attr_ALPHA_SIZE, attr_RED_SIZE, attr_GREEN_SIZE, attr_BLUE_SIZE, attr_DEPTH_SIZE, attr_STENCIL_SIZE, attr_SAMPLES
			);

			break;
		}
	}

#undef GET_ATTRIBUTE

	if (valid_config_index == -1) {
		pr_err("could not find a good configuration");
		goto error_egl_release_thread;
	}


	EGLint attr_native_visual_id;
	EGL_RET(eglGetConfigAttrib(display, configs[valid_config_index], EGL_NATIVE_VISUAL_ID, & attr_native_visual_id), error_egl_release_thread);

	pr_info("drm_surface_create(%u)", attr_native_visual_id);
	gbm_surface = drm_surface_create((uint32_t)attr_native_visual_id);
	if (!gbm_surface) {
		pr_err("drm_surface_create()");
		goto error_egl_release_thread;
	}

#ifdef GL_STATIC_LINKING

	if (create_platform_window_surface_ext) {

		pr_info("create_platform_window_surface_ext()");
		surface = create_platform_window_surface_ext(display, configs[valid_config_index], gbm_surface, NULL);
		if (surface == EGL_NO_SURFACE) {
			pr_err("eglGetPlatformDisplayEXT(), 0x%x", (unsigned)eglGetError());
		}

	} else {
		pr_err("eglGetProcAddress(\"eglCreatePlatformWindowSurfaceEXT\")");
	}

	if (surface == EGL_NO_SURFACE) {
		pr_info("create_platform_window_surface_ext()");
		surface = create_platform_window_surface_ext(display, configs[valid_config_index], gbm_surface, NULL);
		if (surface == EGL_NO_SURFACE)
			EGL_CHECK_ERROR("create_platform_window_surface_ext()", error_drm_surface_destroy);
	}
#else

	pr_info("eglCreatePlatformWindowSurfaceEXT()");
	surface = eglCreatePlatformWindowSurfaceEXT(display, configs[valid_config_index], gbm_surface, NULL);
	if (surface == EGL_NO_SURFACE)
		EGL_CHECK_ERROR("eglCreateWindowSurface()", error_drm_surface_destroy);

#endif

	static const EGLint context_attrs[] =
		{ EGL_CONTEXT_CLIENT_VERSION, 2
		, EGL_NONE
		};

	pr_info("eglCreateContext()");
	context = eglCreateContext(display, configs[valid_config_index], EGL_NO_CONTEXT, context_attrs);
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

error_drm_surface_destroy:
	drm_surface_destroy();

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
	drm_surface_destroy();
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
