/*******************************************************************************
 * Copyright (c) 2022 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <GLES2/gl2.h>

#include <libuhmigl.h>

#include "es2gears.h"
#include "msg.h"

#define UNUSED __attribute__((unused))

/* Subtract the ‘struct timeval’ values X and Y,
 * storing the result in RESULT.
 * Return 1 if the difference is negative, otherwise 0.
 */
static int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	 * tv_usec is certainly positive.
	 */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

static unsigned long int timeval_to_ms (struct timeval *t)
{
	return (t->tv_sec * 1000) + (t->tv_usec / 1000);
}

int main(UNUSED int argc, UNUSED char **argv)
{
	int err;
	uint16_t h;
	uint16_t v;
	const GLubyte *cs;

	struct timeval start, now;

	pr_info("libuhmigl_init()");
	err = libuhmigl_init(&h, &v);
	if (err) {
		pr_err("libuhmigl_init()");
		goto error;
	}

	pr_info("width: %u, height: %u", (unsigned)h, (unsigned)v);

	cs = glGetString(GL_VENDOR);
	pr_info("GL_VENDOR: %s", cs);
	cs = glGetString(GL_RENDERER);
	pr_info("GL_RENDERER: %s", cs);
	cs = glGetString(GL_VERSION);
	pr_info("GL_VERSION: %s", cs);

	es2gears_init();

	es2gears_reshape(h, v);

	err = gettimeofday(&start, NULL);
	if (err < 0) {
		pr_errp("gettimeofday()");
		goto error_libuhmigl_done;
	}

	unsigned long int dt_ms;

	do {
		es2gears_draw();

		err = libuhmigl_update();
		if (err < 0) {
			pr_err("libuhmigl_update()");
			goto error_libuhmigl_done;
		}

		err = gettimeofday(&now, NULL);
		if (err < 0) {
			pr_errp("gettimeofday()");
			goto error_libuhmigl_done;
		}

		struct timeval dt;
		timeval_subtract(&dt, &now, &start);

		dt_ms = timeval_to_ms(&dt);

		es2gears_idle(dt_ms);

	} while (1);

	pr_info("libuhmigl_done()");
	libuhmigl_done();

	exit(EXIT_SUCCESS);

error_libuhmigl_done:
	pr_info("libuhmigl_done()");
	libuhmigl_done();

error:
	exit(EXIT_FAILURE);
}
