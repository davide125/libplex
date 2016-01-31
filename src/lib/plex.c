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

#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plex.h"

/* Private */

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = realloc(s->ptr, new_len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

/* Public */

int plex_global_init() {
    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret != 0) {
        return 1;
    }
    xmlInitParser();
    return 0;
}

void plex_global_cleanup() {
    curl_global_cleanup();
    xmlCleanupParser();
}

const char *plex_get_auth_token(const char *username, const char *password) {
    CURLcode res;
    struct curl_slist *list = NULL;
    CURL *curl = curl_easy_init();

    if (curl) {
        const char *token;
        struct string s;
        xmlDocPtr doc;
        xmlChar *query = (xmlChar*) "/user/authentication-token/text()";
        xmlXPathContext *context;
        xmlXPathObject *result;
        xmlNode *node;

        init_string(&s);

        curl_easy_setopt(curl, CURLOPT_URL, "https://my.plexapp.com/users/sign_in.xml");
		    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.80 Safari/537.36");
        list = curl_slist_append(list, "X-Plex-Platform: Linux");
        list = curl_slist_append(list, "X-Plex-Platform-Version: 3.19.5-100.fc20.x86_64");
        list = curl_slist_append(list, "X-Plex-Client-Identifier: 0x7c7a91b82d2eL");
        list = curl_slist_append(list, "X-Plex-Device: Linux-3.19.5-100.fc20.x86_64-x86_64-with-fedora-20-Heisenbug");
        list = curl_slist_append(list, "X-Plex-Product: PlexAPI");
        list = curl_slist_append(list, "X-Plex-Provides: player,controller");
        list = curl_slist_append(list, "X-Plex-Version: 1.1.0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
            return NULL;
        }
        doc = xmlParseMemory(s.ptr, s.len);
        if (doc == NULL) {
            fprintf(stderr, "Failed to parse document");
            return NULL;
        }

        context = xmlXPathNewContext(doc);
        result = xmlXPathEvalExpression(query, context);
        node = result->nodesetval->nodeTab[0];
        token = strdup((char *)node->content);
        xmlXPathFreeObject(result);

        xmlFreeDoc(doc);
        free(s.ptr);

        return token;
    }

    return NULL;
}
