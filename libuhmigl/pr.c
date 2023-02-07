/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include "pr.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include <stdio.h>
#include <stdarg.h>

#ifndef __unused
#define __unused __attribute__((unused))
#endif

#ifdef __ANDROID__
int pr_use_stderr __attribute__((weak)) = 0;
#endif

int pr_disable_info __attribute__((weak)) = 0;

const char *pr_basename(const char *name)
{
	const char *p = name;

	while (*p)
		p++;

	while (1) {

		if (*p == '/') {
			p++;
			break;
		}

		if (p == name)
			break;

		p--;
	}

	return p;
}

int pr_log_print(int prio, __unused const char* tag, const char* fmt, ...)
{
	va_list ap;
	int err;

	if (prio == ANDROID_LOG_INFO && pr_disable_info)
		return 0;

	va_start(ap, fmt);
#ifdef __ANDROID__
	if (!pr_use_stderr) {
		err = __android_log_vprint(prio, tag, fmt, ap);
	} else
#endif
	{
		err = vfprintf(stderr, fmt, ap);
		fflush(stderr);
	}
	va_end(ap);

	return err;
}
