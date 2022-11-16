/*******************************************************************************
 * Copyright (c) 2022 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#ifndef __msg_h__
#define __msg_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MSG_STRINGIFY(x) MSG_STRINGIFY_(x)
#define MSG_STRINGIFY_(x) #x

#define pr_info( format, ... ) \
        do { fprintf(stderr, ": " __FILE__ ":" MSG_STRINGIFY(__LINE__) " (%s) : " format "\n", __func__, ##__VA_ARGS__ ); fflush(stdout); } while(0)

#define pr_warn( format, ... ) \
        do { fprintf(stderr, "! " __FILE__ ":" MSG_STRINGIFY(__LINE__) " (%s) : " format "\n", __func__, ##__VA_ARGS__ ); fflush(stdout); } while(0)

#define pr_err( format, ... ) \
        do { fprintf(stderr, "* " __FILE__ ":" MSG_STRINGIFY(__LINE__) " (%s) : " format "\n", __func__, ##__VA_ARGS__ ); fflush(stdout); } while(0)

#define pr_errp( format, ... ) \
        do { fprintf(stderr, "* " __FILE__ ":" MSG_STRINGIFY(__LINE__) " (%s) : (%s) " format "\n", __func__, strerror(errno), ##__VA_ARGS__ ); fflush(stdout); } while(0)

#endif
