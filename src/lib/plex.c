/*
 * libplex: an interface to PleX REST API.
 * Copyright (C) 2016 Davide Cavalca <davide@geexbox.org>
 * Copyright (C) 2016 Patrick Shuff <patrick.shuff@gmail.com>
 *
 * This file is part of libplex.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plex.h"

/* Private */

CURL *curl;

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

const char *make_header(const char *key, const char *value) {
    const int BUFSIZE = 1000;
    char *buf = malloc(BUFSIZE);
    const char *header;

    snprintf(buf, BUFSIZE, "%s: %s", key, value);
    header = strdup(buf);
    free(buf);

    return header;
}

/* remember to dispose result with xmlXPathFreeObject() when done */
xmlXPathObject *curl_and_xpath(const char *token, const char *uri, const char *queryfmt, ...) {
    const int QUERY_SIZE = 1024;
    char *query = malloc(QUERY_SIZE);
    CURLcode res;
    struct curl_slist *list = NULL;
    struct string s;
    xmlDoc *doc;
    xmlXPathContext *context;
    xmlXPathObject *result;
    va_list argptr;
    va_start(argptr, queryfmt);
    vsnprintf(query, QUERY_SIZE, queryfmt, argptr);
    va_end(argptr);

    init_string(&s);
    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    list = curl_slist_append(list, make_header("X-Plex-Token", token));
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
    result = xmlXPathEvalExpression((xmlChar*)query, context);
    free(query);

    return result;
}

/* Public */

int plex_global_init() {
    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret != 0) {
        return 1;
    }
    curl = curl_easy_init();
    if (curl == NULL) {
        return 1;
    }
    xmlInitParser();
    return 0;
}

void plex_global_cleanup() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    xmlCleanupParser();
}

const char *plex_get_auth_token(const char *username, const char *password) {
    CURLcode res;
    struct curl_slist *list = NULL;
    curl_easy_reset(curl);

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
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);
        list = curl_slist_append(list, "X-Plex-Client-Identifier: 0x7c7a91b82d2eL");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        // We must send a blank post field otherwise libcurl sends Content-Length
        // of -1 which confuses plex and it sends a 400.
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

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

        curl_slist_free_all(list);
        xmlFreeDoc(doc);
        free(s.ptr);

        return token;
    }

    return NULL;
}

const char *plex_get_device_uri(const char *token, const char *name) {
    xmlXPathObject *result = curl_and_xpath(token,
        "https://plex.tv/api/resources?includeHttps=1",
        "/MediaContainer/Device[@name=\"%s\"]/Connection[@local=1]", name);
    xmlNode *node;
    const char *uri;

    node = result->nodesetval->nodeTab[0];
    uri = strdup((char *)xmlGetProp(node, (xmlChar*)"uri"));
    xmlXPathFreeObject(result);

    return uri;
}
