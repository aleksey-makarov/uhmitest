/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

/* glad/egl.h uses c preprocessor to redefine `eglGetProcAddress`
 * so this should be before #include's
 */
extern void (* eglGetProcAddress(const char *))(void);

#include <libuhmigl.h>

#define LOG_TAG "libuhmigl"
#include "pr.h"

#ifndef __unused
#define __unused __attribute__((unused))
#endif

#ifdef GL_STATIC_LINKING

int loader_load_egl(__unused void *display)
{
	return 0;
}

int loader_load_gles(void)
{
	return 0;
}

void loader_done(void)
{}

#else

#ifndef __ANDROID__

#include <glad/egl.h>
#undef eglGetProcAddress

int loader_load_egl(void *display)
{
	int version = gladLoadEGL(display, eglGetProcAddress);
	if (version == 0) {
		pr_err("gladLoadEGL(0x%016lx, eglGetProcAddress)", (unsigned long)display);
		return -1;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	return 0;
}

int loader_load_gles(void)
{
	int version = gladLoadGLES2(eglGetProcAddress);
	if (version == 0) {
		pr_err("gladLoadGLES2(eglGetProcAddress)");
		return -1;
        }
	pr_info("Loaded OpenGLES2 %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	return 0;
}

void loader_done(void)
{}

#else

#include <dlfcn.h>
#include <stdlib.h>
#include <glad/egl.h>

static const char * dl_path_default = "/system/lib64/drm/libGLES_mesa.so";
static const char * dl_path_env_var_name = "LIBUHMIGL_DL_PATH";

static void *dl_handle = 0;
static void *eglGetProcAddress_ptr = 0;

static void *get_eglGetProcAddress(void)
{
	if (eglGetProcAddress_ptr)
		return eglGetProcAddress_ptr;

	const char * dl_path = getenv(dl_path_env_var_name);
	if (!dl_path)
		dl_path = dl_path_default;

	pr_info("dlopen(%s)", dl_path);
	dl_handle = dlopen(dl_path, RTLD_NOW);
	if (!dl_handle) {
		pr_err("dlopen(%s): %s", dl_path, dlerror());
		goto error;
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

	return eglGetProcAddress_ptr;

error_dlclose:
	dlclose(dl_handle);
	dl_handle = 0;

error:
	return NULL;
}

int loader_load_egl(void *display)
{
	void *a = get_eglGetProcAddress();
	if (!a) {
		pr_err("get_eglGetProcAddress()");
		return -1;
	}

	int version = gladLoadEGL(display, a);
	if (version == 0) {
		pr_err("gladLoadEGL(0x%016lx, a)", (unsigned long)display);
		return -1;
	}
	pr_info("Loaded EGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	return 0;
}


int loader_load_gles(void)
{
	void *a = get_eglGetProcAddress();
	if (!a) {
		pr_err("get_eglGetProcAddress()");
		return -1;
	}

	int version = gladLoadGLES2(a);
	if (version == 0) {
		pr_err("gladLoadGLES2(eglGetProcAddress)");
		return -1;
        }
	pr_info("Loaded OpenGLES2 %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	return 0;
}

void loader_done(void)
{
	dlclose(dl_handle);
	dl_handle = NULL;
	eglGetProcAddress_ptr = NULL;
}

#endif /* __ANDROID__ */

#endif /* GL_STATIC_LINKING */
