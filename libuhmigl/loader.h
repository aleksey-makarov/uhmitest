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

void loader_done(void);

#endif
