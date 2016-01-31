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

#include <plex.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
    char *user = getenv("PLEX_USERNAME");
    char *password = getenv("PLEX_PASSWORD");
    const char *token;

    if (user == NULL || password == NULL) {
        printf("No user or password\n");
        return 1;
    }

    if (plex_global_init() != 0) {
        printf("Failed to init libplex!\n");
        return 1;
    }

    token = plex_get_auth_token(user, password);
    if (!token) {
        printf("Failed to get auth token\n");
        return 1;
    }
    printf("PleX auth token: %s\n", token);

    if (argc > 1) {
        char *device = argv[1];
        const char *uri = plex_get_device_uri(token, device);
        printf("Device URI for %s: %s\n", device, uri);
    }

    plex_global_cleanup();
    return 0;
}
