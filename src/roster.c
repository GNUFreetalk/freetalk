/*
  Copyright (c) 2005-2014 Freetalk Core Team
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
*/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <sys/stat.h>

#include <glib.h>
#include <loudmouth/loudmouth.h>

#include "extensions.h"
#include "freetalk.h"
#include "util.h"
#include "commands.h"
#include "roster.h"
#include "presence.h"
#include "common.h"

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
        CURLM *curlm;
        CURLMcode status;
        char *json_data = NULL;
        struct json_result json = {NULL, 0};

        /* Thread un-safe but needed for initialization */
        curl_global_init (CURL_GLOBAL_ALL);

        curl = curl_easy_init ();
        curlm = curl_multi_init ();
        if (!curlm || !curl)
                goto out;

        json_data = g_malloc_n (1, JSON_SIZE);
        if(!json_data)
                goto out;

        json.data = json_data;
        json.pos = 0;

        curl_multi_setopt (curlm, CURLMOPT_PIPELINING, 1);
        curl_easy_setopt (curl, CURLOPT_URL, url);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, json_response);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, &json);
        curl_multi_add_handle (curlm, curl);

        int still_running = 0;
        do {
                status = curl_multi_perform (curlm, &still_running);
        } while (still_running);

        if (status != 0) {
                PRINTF ("%s", curl_multi_strerror (status));
                goto out;
        }

        /* zero-terminate the result */
        json.data [json.pos] = '\0';

out:
        curl_multi_remove_handle (curlm, curl);
        curl_easy_cleanup (curl);
        curl_multi_cleanup (curlm);
        curl_global_cleanup ();
        return json.data;

}

static char *
get_username (uint64_t id)
{
        char url [256];
        char *username = NULL;
        char *text = NULL;

        json_error_t error;
        json_t *root = NULL;

        snprintf (url, sizeof(url), "http://graph.facebook.com/%"SCNu64, id);
        text = graph_request (url);
        usleep (50000);
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

#else

static char *
get_username (uint64_t id)
{
        return NULL;
}

#endif /* FACEBOOK */

GSList *
ft_roster_get (void)
{
        return state.roster;
}

/*
  Retrieve the user's roster (called once on login)
*/

void
ft_roster_retrieve (LmConnection *conn)
{
        LmMessage *msg;
        LmMessageNode *query;

        msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ,
                                            LM_MESSAGE_SUB_TYPE_GET);
        query = lm_message_node_add_child (msg->node, "query", NULL);
        lm_message_node_set_attributes (query, "xmlns", "jabber:iq:roster",
                                        NULL);
        lm_connection_send (conn, msg, NULL);

        lm_message_node_unref (query);
        lm_message_unref (msg);
}

/*
  Return the subscription state for a roster
  item
*/

static FtSubscriptionState
subscription_state (LmMessageNode *item)
{
        const char *subscription = lm_message_node_get_attribute (item,
                                                                  "subscription");
        if (subscription)
        {
                const char *ask = lm_message_node_get_attribute (item, "ask");
                if (!g_ascii_strcasecmp (subscription, "none"))
                {
                        if (ask && !g_ascii_strcasecmp (ask, "subscribe"))
                                return FT_SUBSCRIPTION_NONE_PENDING_OUT;
                        return FT_SUBSCRIPTION_NONE;
                }
                else if (!g_ascii_strcasecmp (subscription, "from"))
                {
                        if (ask && !g_ascii_strcasecmp (ask, "subscribe"))
                                return FT_SUBSCRIPTION_FROM_PENDING_OUT;
                        return FT_SUBSCRIPTION_FROM;
                }
                else if (!g_ascii_strcasecmp (subscription, "to"))
                        return FT_SUBSCRIPTION_TO;
                else if (!g_ascii_strcasecmp (subscription, "both"))
                        return FT_SUBSCRIPTION_BOTH;
        }
        return FT_SUBSCRIPTION_NONE;
}

static char *
subscription_type_to_str (FtSubscriptionState type)
{
        switch (type) {
        case FT_SUBSCRIPTION_NONE:
                return _("none");
        case FT_SUBSCRIPTION_NONE_PENDING_OUT:
                return _("pending");
        case FT_SUBSCRIPTION_FROM:
                return _("from");
        case FT_SUBSCRIPTION_FROM_PENDING_OUT:
                return _("from + pending");
        case FT_SUBSCRIPTION_TO:
                return _("to");
        case FT_SUBSCRIPTION_BOTH:
                return _("both");
        default:
                break;
        }
        return _("none");
}

static int
roster_item_compare (gconstpointer p, gconstpointer q)
{
        return g_ascii_strcasecmp (((FtRosterItem *)p)->jid,
                                   ((FtRosterItem *)q)->jid);
}

/*
  Lookup a roster item based on the JID.
  The 'resource' part of the JID is ignored for comparision purposes.
*/

FtRosterItem *
ft_roster_lookup (const char *jid)
{
        FtRosterItem incoming;
        GSList *elem;
        gchar **pieces;
        pieces = g_strsplit (jid, "/", 2);

        incoming.jid = pieces[0];
        /*  incoming.resource = pieces[1];*/
        elem = g_slist_find_custom (state.roster, &incoming,
                                    roster_item_compare);

        g_strfreev (pieces);
        return elem ? (FtRosterItem *)elem->data : NULL;
}

int
get_username_id_from_jid (const gchar *jid, char **username, int64_t *id)
{
        char    *ptr           = NULL;
        char    *token         = NULL;
        int     ret            = -1;
        char    jid_buf[256]   = {0,};
        const char delimiter[] = "@";

        ptr = g_strdup (jid);
        if (!ptr)
                return ret;

        token = strsep (&ptr, delimiter);
        if (token == NULL)
                return ret;

        *id = strtoll (token, (char **) NULL, 10);

        if (*id == 0)
                return ret;

        char *real_jid = get_username (llabs (*id));

        if (real_jid)
                snprintf (jid_buf, sizeof(jid_buf), "%s@chat.facebook.com",
                          real_jid);
        else
                snprintf (jid_buf, sizeof(jid_buf),
                          "%"PRId64"@chat.facebook.com", *id);

        *username = g_strdup (jid_buf);

        ret = 0;
        return ret;
}

static void
roster_result_rcvd (LmMessage *msg)
{
        LmMessageNode *query = lm_message_node_get_child (msg->node, "query");
        LmMessageNode *item = lm_message_node_get_child (query, "item");
        char *username = NULL;
        int64_t id     = 0;

        ft_presence_send_initial ();

        while (item) {
                FtRosterItem *r_item = NULL;

                r_item = g_try_new0 (FtRosterItem, 1);
                if (!r_item) {
                        PRINTF (_("error\n"));
                        return;
                }
                if (is_facebook ()) {
                        if (get_username_id_from_jid
                            (lm_message_node_get_attribute
                             (item, "jid"), &username, &id)) {
                                PRINTF (_("error\n"));
                                return;
                        }

                        if (id)
                                r_item->id = id;
                        if (username)
                                r_item->jid = username;
                } else
                        r_item->jid = g_strdup (lm_message_node_get_attribute
                                                (item, "jid"));

                r_item->subscription = subscription_state (item);
                r_item->nickname = g_strdup (lm_message_node_get_attribute
                                             (item, "name"));
                /* Assume unavailable until presence is recieved */
                r_item->is_online = FALSE;
                r_item->status_msg = NULL;
                r_item->show_msg = NULL;
                r_item->resource = NULL;

                state.roster = g_slist_append (state.roster, r_item);
                item = item->next;
        }
}

/*
  TODO: support facebook id->username, facebook doesn't send back "set" request
*/

static void
roster_set_rcvd (LmMessage *msg)
{
        FtRosterItem *old, *newi;

        LmMessageNode *query = lm_message_node_get_child (msg->node, "query");
        LmMessageNode *item = lm_message_node_get_child (query, "item");

        old = ft_roster_lookup (lm_message_node_get_attribute (item, "jid"));

        if (old) {
                char *nickname;
                char *jid;
                FtSubscriptionState subscription;

                jid = g_strdup (lm_message_node_get_attribute (item, "jid"));
                nickname = g_strdup (lm_message_node_get_attribute (item,
                                                                    "name"));
                subscription = subscription_state (item);

                if (g_strcmp0 (old->nickname ? old->nickname : "",
                               nickname ? nickname : "")) {
                        PRINTF (_("[%s nickname: %s -> %s]"), old->jid,
                                old->nickname, nickname);
                        g_free (old->nickname);
                        old->nickname = nickname;
                }

                if (old->subscription != subscription) {
                        PRINTF (_("[%s subscription: %s -> %s]"),
                                old->jid,
                                subscription_type_to_str (old->subscription),
                                subscription_type_to_str (subscription));
                        old->subscription = subscription;
                }

                if (!g_ascii_strcasecmp
                    (lm_message_node_get_attribute (item,
                                                    "subscription"), "remove")) {
                        PRINTF (_("[%s removed from buddy list]"), jid);
                        state.roster = g_slist_remove (state.roster, old);
                }
        }
        else {
                newi = g_new (FtRosterItem, 1);
                state.roster = g_slist_append (state.roster, newi);
                newi->is_online = FALSE;
                newi->status_msg = NULL;
                newi->show_msg = NULL;
                newi->resource = NULL;
                newi->jid = g_strdup (lm_message_node_get_attribute (item,
                                                                     "jid"));
                newi->nickname = g_strdup (lm_message_node_get_attribute (item,
                                                                          "name"));
                newi->subscription = subscription_state (item);
        }
}

/*
  Called by IQ handler for roster related messages
*/

void
ft_roster_cb (LmMessage *msg)
{
        const char *type = lm_message_node_get_attribute (msg->node, "type");

        if (!g_ascii_strcasecmp (type, "result")) // Initial roster retrieval
                roster_result_rcvd (msg);
        else if (!g_ascii_strcasecmp (type, "set"))
                roster_set_rcvd (msg);
}


/*
  Call a function with each roster item
*/

void
ft_roster_foreach (GFunc func, gpointer userdata)
{
        g_slist_foreach (state.roster, func, userdata);
}

/*
  Adding a buddy
*/

static void
roster_add_send_iq (char *jid, char *nickname)
{
        LmMessage *msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_SET);
        LmMessageNode *query = lm_message_node_add_child (msg->node, "query",
                                                          NULL);

        LmMessageNode *item = lm_message_node_add_child (query, "item", NULL);

        lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
        lm_message_node_set_attribute (item, "jid", jid);
        if (nickname)
                lm_message_node_set_attribute (item, "name", nickname);

        lm_connection_send (state.conn, msg, NULL);

        lm_message_node_unref (query);
        lm_message_node_unref (item);
        lm_message_unref (msg);
}

static void
roster_add_send_subscribe (char *jid)
{
        LmMessage *msg =
                lm_message_new_with_sub_type (jid, LM_MESSAGE_TYPE_PRESENCE,
                                              LM_MESSAGE_SUB_TYPE_SUBSCRIBE);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
}

/*
  Add a buddy to the roster and subscribe to his presence
*/
void
ft_roster_add (char *jid, char *nickname)
{
        roster_add_send_iq (jid, nickname);
        roster_add_send_subscribe (jid);
}

/*
  Remove a buddy from the roster and cancel all
  subscriptions (from and to)
*/

void
ft_roster_remove (char *jid)
{
        LmMessage *msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_SET);
        LmMessageNode *query = lm_message_node_add_child (msg->node, "query",
                                                          NULL);

        LmMessageNode *item = lm_message_node_add_child (query, "item", NULL);

        lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
        lm_message_node_set_attribute (item, "jid", jid);
        lm_message_node_set_attribute (item, "subscription", "remove");

        lm_connection_send (state.conn, msg, NULL);

        lm_message_node_unref (query);
        lm_message_node_unref (item);
        lm_message_unref (msg);
}

/*
 * Set the nickname of a buddy
 */
void
ft_roster_set_nickname (char *jid, char *nickname)
{
        LmMessage *msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_SET);
        LmMessageNode *query = lm_message_node_add_child (msg->node, "query",
                                                          NULL);

        LmMessageNode *item = lm_message_node_add_child (query, "item", NULL);

        lm_message_node_set_attribute (query, "xmlns", "jabber:iq:roster");
        lm_message_node_set_attribute (item, "jid", jid);
        lm_message_node_set_attribute (item, "name", nickname);

        lm_connection_send (state.conn, msg, NULL);

        lm_message_node_unref (query);
        lm_message_node_unref (item);
        lm_message_unref (msg);
}

void
ft_roster_flush ()
{
        g_slist_free (state.roster);
        state.roster = NULL;
}
