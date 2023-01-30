/*******************************************************************************
 * Copyright (c) 2023 OpenSynergy GmbH.
 *
 * This software may not be used in any way or distributed without
 * permission. All rights reserved.
 ******************************************************************************/

#include "libuhmigl.h"

#include <sys/system_properties.h>

int libuhmigl_set_scanout(const char *scanout)
{
	return __system_property_set("uhmi.rproxy.scanout", scanout);
}

int libuhmigl_set_remote_address(const char *remote_address)
{
	return __system_property_set("uhmi.rproxy.remote_address", remote_address);
}
