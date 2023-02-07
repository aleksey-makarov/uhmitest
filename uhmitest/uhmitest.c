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
#include <string.h>
#include <errno.h>

#include <libuhmigl.h>
#define LOG_TAG "uhmitest"
#include <pr.h>
#include <es2gears.h>

#include "timeval.h"

int pr_use_stderr = 1;

#ifndef __unused
#define __unused __attribute__((unused))
#endif

int main(int argc, __unused char **argv)
{
	int err;
	uint16_t h;
	uint16_t v;
	const GLubyte *cs;
	struct es2gears_state *state;

	struct timeval start, now;

#ifdef __ANDROID__
	if (argc == 2) {
		pr_info("libuhmigl_set_remote_address(\"%s\")", argv[1]);
		err = libuhmigl_set_remote_address(argv[1]);
		if (err) {
			pr_err("libuhmigl_set_remote_address()");
			goto error;
		}

		pr_info("wait for 5s for rproxy to restart");
		sleep(5);
	} else
#endif
	if (argc != 1) {
		pr_err("Usage: uhmitest [rvdds_ip:rvdds_port]");
		goto error;
	}

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

	state = es2gears_init();
	if (!state) {
		pr_err("es2gears_init()");
		goto error_libuhmigl_done;
	}

	es2gears_reshape(state, h, v);

	err = gettimeofday(&start, NULL);
	if (err < 0) {
		pr_err("gettimeofday(): %s", strerror(errno));
		goto error_libuhmigl_done;
	}

	unsigned long int dt_ms;

	do {
		es2gears_draw(state);

		err = libuhmigl_update();
		if (err < 0) {
			pr_err("libuhmigl_update()");
			goto error_libuhmigl_done;
		}

		err = gettimeofday(&now, NULL);
		if (err < 0) {
			pr_err("gettimeofday(): %s", strerror(errno));
			goto error_libuhmigl_done;
		}

		struct timeval dt;
		timeval_subtract(&dt, &now, &start);

		dt_ms = timeval_to_ms(&dt);

		es2gears_idle(state, dt_ms);

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
