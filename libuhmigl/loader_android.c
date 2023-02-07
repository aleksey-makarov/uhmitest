/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "libuhmigl"
#include "pr.h"

static const char * dl_path_default = "/system/lib64/drm/libGLES_mesa.so";
static const char * dl_path_env_var_name = "LIBUHMIGL_DL_PATH";

static void *dl_handle;

void *loader_init(void)
{
	const char * dl_path = getenv(dl_path_env_var_name);
	void *ret = NULL;

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
	ret = dlsym(dl_handle, "eglGetProcAddress");
	if (!ret) {
		char *dlerror_string = dlerror();
		pr_err("dlsym(\"eglGetProcAddress\"): %s", dlerror_string ? dlerror_string : "???");
		goto error_dlclose;
	}

	return ret;

error_dlclose:
	dlclose(dl_handle);

error:
	return NULL;
}

void loader_done(void)
{
	dlclose(dl_handle);
}
