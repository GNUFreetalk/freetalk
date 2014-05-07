/*
  Copyright (c) 2014 Freetalk core team
  This file is part of Freetalk.

  Freetalk is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3 of the License,
  or (at your option) any later version.

  Freetalk is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

  NOTE - routines for Facebook graph API
*/


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

#include <glib.h>
#include "commands.h"
#include "facebook.h"

#ifdef FACEBOOK
#include <curl/curl.h>
#include <jansson.h>

/* Practical limit */
#define JSON_SIZE 512

struct json_result {
        char *data;
        int pos;
};

static size_t
json_response (void *ptr, size_t size, size_t nmemb, void *stream)
{
        struct json_result *json = (struct json_result *)stream;
        size_t jsonlen = 0;
        size_t new_pos = 0;

        jsonlen = size * nmemb;
        new_pos = json->pos + jsonlen;

        if (new_pos >= (JSON_SIZE - 1)) {
                PRINTF ("too large buffer");
                return 0;
        }

        memcpy (json->data + json->pos, ptr, jsonlen);
        json->pos += jsonlen;

        return jsonlen;
}

static char *
graph_request (const char *url)
{
        CURL *curl;
        CURLcode status;
        char *json_data = NULL;
        struct json_result json = {NULL, 0};

        curl = curl_easy_init ();
        json_data = g_malloc_n (1, JSON_SIZE);
        if(!curl || !json_data)
                goto out;

        json.data = json_data;
        json.pos = 0;

        curl_easy_setopt (curl, CURLOPT_URL, url);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, json_response);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, &json);

        status = curl_easy_perform (curl);
        if (status != 0) {
                PRINTF ("%s", curl_easy_strerror (status));
                goto out;
        }

        /* zero-terminate the result */
        json.data [json.pos] = '\0';

out:
        curl_easy_cleanup (curl);
        curl_global_cleanup ();
        return json.data;

}

char *
get_username (uint32_t id)
{
        char url [256];
        char *username = NULL;
        char *text = NULL;

        json_error_t error;
        json_t *root = NULL;

        snprintf (url, sizeof(url), "http://graph.facebook.com/%d", id);
        text = graph_request (url);

        if (!text)
                goto out;

        root = json_loads (text, 0, &error);
        if (!root) {
                PRINTF ("error: on line %d: %s",
                        error.line,
                        error.text);
                goto out;
        }

        json_unpack (root, "{s:s}", "username", &username);
out:
        if (text)
                g_free (text);
        return username;
}

#endif /* FACEBOOK */
