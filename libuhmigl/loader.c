/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

extern char eglGetProcAddress[];

void *loader_init(void)
{
	return &eglGetProcAddress;
}

void loader_done(void)
{
}
