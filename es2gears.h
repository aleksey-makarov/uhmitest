/*******************************************************************************
 * Copyright (c) 2022 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __es2gears_h__
#define __es2gears_h__

void es2gears_idle(unsigned long int elapsed_time_ms);
void es2gears_reshape(int width, int height);
void es2gears_draw(void);
// void es2gears_special(int special);
void es2gears_init(void);

#endif
