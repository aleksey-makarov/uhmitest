/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __libuhmigl_h__
#define __libuhmigl_h__

#include <glad/gles2.h>

#ifdef __cplusplus
extern "C" {
#endif

int  libuhmigl_init(uint16_t *h, uint16_t *v);
void libuhmigl_done(void);
int  libuhmigl_update(void);
int  libuhmigl_load(void);

int libuhmigl_set_scanout(const char *scanout);
int libuhmigl_set_remote_address(const char *remote_address);

struct ANativeWindow;

int  libuhmigl_android_init(struct ANativeWindow *window, uint16_t *h, uint16_t *v);
void libuhmigl_android_done(void);
int  libuhmigl_android_update(void);
int  libuhmigl_android_load(void);

#ifdef __cplusplus
}
#endif

#endif
