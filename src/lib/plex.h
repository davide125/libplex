/*
 * libplex: an interface to PleX REST API.
 * Copyright (C) 2016 Davide Cavalca <davide@geexbox.org>
 *
 * This file is part of libplex.
 *
 * libplex is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libplex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libplex; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef PLEX_H
#define PLEX_H

#ifdef __cplusplus
extern "C" {
#endif

#define LIBPLEX_VERSION "0.0.1"

int plex_global_init();
void plex_global_cleanup();
const char *plex_get_auth_token(const char *username, const char *password);
const char *plex_get_device_uri(const char *token, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* PLEX_H */
