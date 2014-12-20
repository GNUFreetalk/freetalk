/*
  Copyright (c) 2005-2014 Freetalk Core Team
  This file is part of GNU Freetalk.

  Freetalk is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

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
#include <regex.h>
#include <loudmouth/loudmouth.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "freetalk.h"
#include "commands.h"
#include "callbacks.h"
#include "util.h"
#include "extensions.h"
#include "roster.h"
#include "presence.h"
#include "common.h"

void
do_ssl ()
{
        LmSSL *ssl;
        ssl = lm_ssl_new (NULL, ft_ssl_response_cb, NULL, NULL);
        lm_ssl_use_starttls(ssl, ! state.need_ssl, state.need_tls);
        lm_connection_set_ssl (state.conn, ssl);
        lm_connection_set_port (state.conn, do_get_port () ? do_get_port () :
                                LM_CONNECTION_DEFAULT_PORT_SSL);
}

int
do_connect_common ()
{
        if (do_get_conn_status ())
                return -6;

        if (!state.server)
                return -1;

        if (!state.jid_str)
                return -2;

        parse_jid_string (state.jid_str, &state.jid);
        if (!state.password)
                state.password =  getpass ("Password: ");

        state.conn = lm_connection_new (state.server);
        lm_connection_ref (state.conn);

        do_set_conn_status (FT_DEAD);

        /* Proxy Support */

        if (state.need_proxy) {

                if (!state.proxyserver)
                        return -5;

                state.proxy = lm_proxy_new_with_server (LM_PROXY_TYPE_HTTP,
                                                        state.proxyserver,
                                                        state.proxyport);
                if (!state.proxyuname)
                        state.proxyuname = NULL;
                if (!state.proxypasswd)
                        state.proxypasswd = NULL;

                lm_proxy_set_username (state.proxy, state.proxyuname);
                lm_proxy_set_password (state.proxy, state.proxypasswd);

                lm_connection_set_proxy (state.conn, state.proxy);
                lm_proxy_unref(state.proxy);
        }


        if (state.need_ssl || state.need_tls) {
                if (!lm_ssl_is_supported ())
                        return -3;
                do_ssl ();
        } else {
                lm_connection_set_port (state.conn,
                                        do_get_port () ? do_get_port () :
                                        LM_CONNECTION_DEFAULT_PORT);
        }

        lm_connection_set_jid (state.conn, state.jid_str);

        ft_register_msg_handlers (state.conn);

        return 0;
}

int
do_connect (void)
{
        int ret;

        ret = do_connect_common ();

        if (ret)
                return ret;

        PRINTF (_("Connecting ..."));

        if (!lm_connection_open (state.conn, ft_connection_open_cb,
                                 (gpointer)&state, NULL, &state.error)) {
                PRINTF (state.error->message);
                g_clear_error(&state.error); // free error and set it to NULL
                return -4;
        }

        return 0;
}

void
do_session_init (gboolean success)
{
        LmMessage *msg;

        if (!success) {
                /* Disconnecting if not authenticated */
                do_set_conn_status (FT_DEAD);
                return;
        }

        do_set_conn_status (FT_AUTH);

        ft_roster_retrieve (state.conn);

        msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_PRESENCE,
                                            LM_MESSAGE_SUB_TYPE_AVAILABLE);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);

        scm_run_hook (ex_login_hook, scm_list_n (scm_from_bool (success),
                                                 SCM_UNDEFINED));

}

int
do_connect_blocking (void)
{
        int ret;

        ret = do_connect_common ();

        if (ret)
                return ret;

        PRINTF (_("Connecting ..."));

        if (!lm_connection_open_and_block (state.conn, &state.error)) {
                PRINTF (_("Could not connect."));
                g_clear_error(&state.error); // free error and set it to NULL
                return -4;
        }

        do_set_conn_status (FT_CONN);
        PRINTF (_("Connected."));

        PRINTF (_("Authenticating ..."));
        ret = lm_connection_authenticate_and_block (state.conn,
                                                    state.jid.node,
                                                    state.password,
                                                    state.jid.resource,
                                                    &state.error);
        do_session_init ((ret!=0));
        return (ret==0);
}

int
do_disconnect ()
{
        /*  if (!do_get_conn_status ())
            return -1; */

        switch (do_get_conn_status ()) {
        case FT_AUTH:
                ft_presence_send_final ();
                lm_connection_close (state.conn, NULL);
                break;
        case FT_CONN:
                lm_connection_close (state.conn, NULL);
                break;
        default:
                if (state.conn)
                        lm_connection_cancel_open (state.conn);
                /*    else
                      PRINTF (_("Not connected")); */
                break;
        }
        //  exit (err);
        return 0; /* :p */
}

int
do_quit (int err)
{
        do_disconnect();
        scm_run_hook(ex_quit_hook, scm_list_n (scm_from_int(err),
                                               SCM_UNDEFINED));
        exit (err);
}

int
do_set_jid (const char *jidstr)
{
        if (!jidstr)
                return -1;

        if (state.jid_str)
                g_free (state.jid_str);
        state.jid_str = g_strdup (jidstr);

        return 0;
}

const char *
do_get_jid (void)
{
        return state.jid_str ? state.jid_str : "";
}

int
do_set_password (const char *password)
{
        if (!password)
                return -1;

        if (state.password)
                g_free (state.password);
        state.password = g_strdup (password);

        return 0;
}

const char *
do_get_password (void)
{
        return state.password ? state.password : "";
}

int
do_set_server (const char *server)
{
        if (!server)
                return -1;

        if (state.server)
                g_free (state.server);
        state.server = g_strdup (server);

        return 0;
}

const char *
do_get_server (void)
{
        return state.server ? state.server : "";
}

int
do_set_port (unsigned short int port)
{
        state.port = port;
        return 0;
}

int
do_get_port ()
{
        return state.port;
}

int
do_set_prompt (const char *prompt)
{
        if (!prompt)
                return -1;

        if (state.prompt)
                g_free (state.prompt);

        state.prompt = g_strdup (prompt);
        rl_set_prompt (state.prompt);

        return 0;
}

const char *
do_get_prompt (void)
{
        return state.prompt ? state.prompt : "";
}

int
do_set_ssl (char value)
{
        state.need_ssl = value;
        return 0;
}

int
do_get_ssl (void)
{
        return state.need_ssl;
}

int
do_set_tls (char value)
{
        state.need_tls = value;
        return 0;
}

int
do_get_tls (void)
{
        return state.need_tls;
}

int
do_send_message_no_hook (char *jid, char *msg_str)
{
        LmMessage *msg;
        state.last = time(NULL);

        if (!jid || !msg_str)
                return -2;

        if (do_get_conn_status () != FT_AUTH)
                return -1;

        msg = lm_message_new (jid, LM_MESSAGE_TYPE_MESSAGE);

        lm_message_node_set_attribute (msg->node, "type", "chat");
        lm_message_node_add_child (msg->node, "body", msg_str);

        return lm_connection_send (state.conn, msg, NULL);
}

int
do_send_message (char *jid, char *msg_str)
{
        /* set before hook, allowing hook functions to override
           default buddy
        */

        do_set_current_buddy (jid);

        if (!jid || !msg_str)
                return -2;

        if (do_get_conn_status () != FT_AUTH)
                return -1;

        set_hook_return (0);
        scm_run_hook (ex_message_send_hook,
                      scm_list_n (scm_from_locale_string (jid),
                                  scm_from_locale_string (msg_str),
                                  SCM_UNDEFINED));
        if (get_hook_return () == 1)
                return LM_HANDLER_RESULT_REMOVE_MESSAGE;

        return do_send_message_no_hook (jid, msg_str);
}

int
do_set_current_buddy (char *bud)
{
        if (bud)
                state.current_buddy = ft_roster_lookup (bud);
        else
                state.current_buddy = NULL;
        return 0;
}

const char *
do_get_current_buddy (void)
{
        return state.current_buddy ? state.current_buddy->jid : "";
}

/*
static void
roster_iterator (gpointer r, gpointer data)
{
        FtRosterItem *r_item = (FtRosterItem *)r;
        GSList **l = (GSList **)data;
        *l = g_slist_append (*l, (gpointer) r_item->jid);
}
*/


int
do_set_daemon (void)
{
        int ret=0;
        state.daemon = 1; /* no way out */
        ret = daemon (0, 0);
        return ret;
}

int
do_get_daemon (void)
{
        return state.daemon;
}

int
do_printf (const char *fmt, ...)
{
        va_list ap;

        if (! do_get_daemon ()) {
                va_start (ap, fmt);

                if (state.async_printf)
                        async_printf (fmt, ap);
                else
                        sync_printf (fmt, ap);
        }

        return 0;
}

int
do_get_conn_status (void)
{
        return (int) state.conn_state;
}

enum ft_conn_state
do_set_conn_status (enum ft_conn_state status)
{
        state.conn_state = status;
        return state.conn_state;
}

/* Roster management */
int
do_add (char *jid)
{
        if (do_get_conn_status () != FT_AUTH)
        {
                PRINTF ("Not connected (use /connect)");
                return -1;
        }
        ft_roster_add (jid, NULL);
        return 0;
}

/* Set an away message */
int
do_set_status_msg (char *status)
{
        const char *valid[] = { "online", "away", "chat", "xa", "dnd", "invisible", NULL };
        char *text, *priority;
        int show, offset;

        if((text = strchr(status, ' '))) {
                offset = text - status;
                text++;
        } else {
                offset = strlen(status);
        }

        if( (priority = strchr(status, '/')) && ( !text || priority < text ) ) {
                offset = priority - status;
                ++priority;
        } else {
                priority = NULL;
        }

        for(show = 0; valid[show] != NULL; show++) {
                if( !strncmp(valid[show], status, offset) ) {
                        break;
                }
        }
        if( !valid[show] ) {
                PRINTF ("Status must be one of"
                        "[online|away|chat|xa|dnd|invisible]");
                return -1;
        }
        if (state.status_msg)
                g_free (state.status_msg);
        state.status_msg = g_strdup (status);

        if (text) {
                *(text - 1) = '\0';
        }

        LmMessage *msg = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
        if( show != 0 ) { // online status is implicit
                if (!g_strcmp0 (valid[show], "invisible")) {
                        lm_message_node_set_attribute (msg->node, "type",
                                                       "unavailable");
                } else {
                        lm_message_node_add_child (msg->node, "show",
                                                   valid[show]);
                }
        }
        if( text != NULL ) {
                lm_message_node_add_child (msg->node, "status", text);
        }
        if ( priority ) {
                lm_message_node_add_child( msg->node, "priority", priority );
        }
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
        return 0;
}

const char *
do_get_status_msg (void)
{
        return state.status_msg ? state.status_msg : "";
}

int
do_reset_fs_state (void)
{
        return ( system ("sh " DATADIR "/" PACKAGE_NAME "/extensions/first-time-run.sh reset") >> 8 );
}

extern GSList *dict_words;
static int strcmp_rev (gpointer a, gpointer b)
{
        return -g_strcmp0 ((const char *)a, (const char *)b);
}

void
do_dict_append (char *word)
{
        dict_words = g_slist_append (dict_words, word);
}

void
do_dict_prepend (char *word)
{
        dict_words = g_slist_prepend (dict_words, word);
}

void
do_dict_insert (char *word)
{
        if (! g_slist_find_custom (dict_words, word, (GCompareFunc) strcmp_rev))
                dict_words = g_slist_insert_sorted (dict_words, word, (GCompareFunc) strcmp_rev);
}

void
do_dict_remove (char *word)
{
        GSList *tmp = g_slist_find_custom (dict_words, word, (GCompareFunc) strcmp_rev);

        if (tmp) {
                g_free (tmp->data);
                dict_words = g_slist_remove_link (dict_words, tmp);
        }
}

void
do_main_loop (void)
{
        GMainLoop *main_loop;

        main_loop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (main_loop);
}

void
do_change_password (char *npass)
{
        if (!g_ascii_strcasecmp (npass, "")) {
                PRINTF ("Invalid password\n");
                goto out;
        }

        LmMessage *msg = lm_message_new_with_sub_type (state.server,
                                                       LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_SET);
        LmMessageNode *query = lm_message_node_add_child (msg->node,
                                                          "query", "");
        lm_message_node_set_attribute (query, "xmlns",
                                       "jabber:iq:register");
        lm_message_node_add_child (query, "username", state.jid.node);
        lm_message_node_add_child (query, "password", npass);

        LmMessage *reply = lm_connection_send_with_reply_and_block (state.conn,
                                                                    msg, NULL);

        const char *type = lm_message_node_get_attribute (reply->node,
                                                          "type");
        if (!g_ascii_strcasecmp (type, "error"))
                PRINTF ("Password change failed.\n");
        else
                PRINTF ("Password changed.\n");

        lm_message_unref (msg);
out:
        g_free (npass);
}

int
do_set_proxy (char value)
{
        state.need_proxy = value;
        return 0;
}

int
do_get_proxy (void)
{
        return state.need_proxy;
}

const char *
do_get_proxyserver (void)
{
        return state.proxyserver ? state.proxyserver : "";
}

int
do_get_proxyport (void)
{
        return state.proxyport;
}

int
do_set_proxyserver (const char *proxyserver)
{
        if (!proxyserver)
                return -1;

        if (state.proxyserver)
                g_free (state.proxyserver);
        state.proxyserver = g_strdup (proxyserver);
        return 0;
}

int
do_set_proxyport (unsigned short int proxyport)
{
        state.proxyport = proxyport;
        return 0;
}


const char *
do_get_proxyuname (void)
{
        return state.proxyuname ? state.proxyuname : "";
}

const char *
do_get_proxypasswd (void)
{
        return state.proxypasswd ? state.proxypasswd : "";
}

int
do_set_proxyuname (const char *proxyuname)
{
        if (!proxyuname)
                return -1;

        if (state.proxyuname)
                g_free (state.proxyuname);

        state.proxyuname = g_strdup (proxyuname);

        return 0;
}

int
do_set_proxypasswd (const char *proxypasswd)
{
        if (!proxypasswd)
                return -1;

        if (state.proxypasswd)
                g_free (state.proxypasswd);

        state.proxypasswd = g_strdup (proxypasswd);

        return 0;
}
