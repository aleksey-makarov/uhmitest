/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __drm_state_h__
#define __drm_state_h__

#include <stdint.h>

int drm_state_init(void);
void drm_state_done(void);

struct gbm_surface;
struct gbm_surface *drm_surface_create(uint32_t format);
void drm_surface_destroy(struct gbm_surface *surface);

struct gbm_device;
extern struct gbm_device *drm_state_gbm_device;

struct _drmModeModeInfo;
extern struct _drmModeModeInfo *drm_state_mode;

int drm_state_flip(void);

#endif
