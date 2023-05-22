/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __loader_h__
#define __loader_h__

int loader_load_egl(void *display);
int loader_load_gles(void);

#if __ANDROID__
int loader_android_load_egl(void *display);
int loader_android_load_gles(void);
#endif

void loader_done(void);

#endif
