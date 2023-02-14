/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __pr_h__
#define __pr_h__

#ifdef __ANDROID__
# include <android/log.h>
#else
# define ANDROID_LOG_INFO  4
# define ANDROID_LOG_ERROR 6
# ifndef LOG_TAG
#  define LOG_TAG NULL
# endif
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *pr_basename(const char *name);

#ifdef __ANDROID__
extern int pr_use_stderr;
#endif

extern int pr_disable_info;

int pr_log_print(int prio, const char* tag, const char* fmt, ...)
#if defined(__GNUC__)
    __attribute__((__format__(printf, 3, 4)))
#endif
    ;

#define pr_info( format, ... ) \
        pr_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s:%d (%s) : " format "\n", pr_basename(__FILE__), __LINE__, __func__, ##__VA_ARGS__ )

#define pr_err( format, ... ) \
        pr_log_print(ANDROID_LOG_ERROR, LOG_TAG, "* %s:%d (%s) : " format "\n", pr_basename(__FILE__), __LINE__, __func__, ##__VA_ARGS__ )

#ifdef __cplusplus
}
#endif

#endif
