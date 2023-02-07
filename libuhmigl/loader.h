/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __loader_h__
#define __loader_h__

/*
 * The loader library returns the address of the eglGetProcAddress() function
 * On Android it loads UHMI mesa library with dlopen().
 * On Linux it uses the mesa library linked to executable.
 */

void *loader_init(void);
void loader_done();

#endif
