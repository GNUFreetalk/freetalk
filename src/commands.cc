/*
  Copyright (c) 2005, 2006, 2007 Freetalk Core Team 
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

#include <stdio.h>
#include <regex.h>
#include <loudmouth/loudmouth.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "freetalk.h"
#include "commands.h"
#include "callbacks.h"
#include "util.h"
#include "extensions.h"
#include "roster.h"
#include "presence.h"

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

  //  PRINTF ("Server: %s \nJID: %s", state.server, state.jid_str); fflush(stdout);

  if (!state.password) {
    state.password =  getpass ("Password: ");
  }

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
    if (!state.proxyuname) state.proxyuname = NULL;
    if (!state.proxypasswd) state.proxypasswd = NULL;
    
    lm_proxy_set_username (state.proxy, state.proxyuname);
    lm_proxy_set_password (state.proxy, state.proxypasswd);
    
    lm_connection_set_proxy (state.conn, state.proxy);
    lm_proxy_unref(state.proxy);
  }


  lm_connection_set_jid (state.conn, state.jid_str);

  if (state.need_ssl) {
    LmSSL *ssl;
    
    if (!lm_ssl_is_supported ()) {
      return -3;
    }
    ssl = lm_ssl_new (NULL, ft_ssl_response_cb, NULL, NULL);
    lm_connection_set_ssl (state.conn, ssl);
    lm_connection_set_port (state.conn, do_get_port () ? do_get_port () : LM_CONNECTION_DEFAULT_PORT_SSL);
  } else {
    lm_connection_set_port (state.conn, do_get_port () ? do_get_port () : LM_CONNECTION_DEFAULT_PORT);
  }

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

  if (!lm_connection_open (state.conn, ft_connection_open_cb, (gpointer)&state, NULL, &state.error)) {
    perror (state.error->message);
    return -4;
  }

  return 0;
}

void
do_session_init (gboolean success)
{
  LmMessage *msg;

  scm_run_hook (ex_login_hook, gh_list (gh_bool2scm (success),
					SCM_UNDEFINED));
  if (success) {
    do_set_conn_status (FT_AUTH);
  } else {
    do_set_conn_status (FT_CONN);
    return;
  }

  ft_roster_retrieve (state.conn);

  msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_PRESENCE,
				      LM_MESSAGE_SUB_TYPE_AVAILABLE);
  lm_connection_send (state.conn, msg, NULL);
  lm_message_unref (msg);
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
    return -4;
  }
  
  PRINTF (_("Connected."));
  
  do_set_conn_status (FT_CONN);

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
 scm_run_hook(ex_quit_hook, gh_list (gh_int2scm(err),
			 SCM_UNDEFINED));
 exit (err);
}

int
do_set_jid (const char *jidstr)
{
  if (!jidstr) 
    return -1;

  if (state.jid_str) 
    free (state.jid_str);
  state.jid_str = strdup (jidstr);

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
    free (state.password);
  state.password = strdup (password);

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
    free (state.server);
  state.server = strdup (server);

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
    free (state.prompt);
  state.prompt = strdup (prompt);

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
do_send_message_no_hook (char *jid, char *msg_str)
{
  LmMessage *msg;
  state.last = time(NULL);

  if (!jid || !msg_str)
    return -2;

  if (do_get_conn_status () != FT_AUTH)
    return -1;

  msg = lm_message_new (jid, LM_MESSAGE_TYPE_MESSAGE);

  //  Intermediate Invisible mode messaging fix : TODO
  //if (!strcmp (do_get_status_msg (), "invisible")) {
  //  lm_message_node_set_attribute (msg->node, "type", "chat");
  //  lm_message_node_add_child (msg->node, "body", msg_str);
  //  lm_message_node_add_child (msg->node, "priority", priority);
  //} else {
    lm_message_node_set_attribute (msg->node, "type", "chat");
    lm_message_node_add_child (msg->node, "body", msg_str);
    //}

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
  scm_run_hook (ex_message_send_hook, gh_list (gh_str02scm (jid),
					       gh_str02scm (msg_str),
					       SCM_UNDEFINED));
  if (get_hook_return () == 1)
    return LM_HANDLER_RESULT_REMOVE_MESSAGE;

  return do_send_message_no_hook (jid, msg_str);
}

int
do_set_current_buddy (char *bud)
{
  if (state.current_buddy)
    g_free (state.current_buddy);
  if (bud)
    state.current_buddy = strdup (bud);
  else
    state.current_buddy = NULL;
  return 0;
}

const char *
do_get_current_buddy (void)
{
  return state.current_buddy ? state.current_buddy : "";
}

static void
roster_iterator (gpointer r, gpointer data)
{
  FtRosterItem *r_item = (FtRosterItem *)r;
  GSList **l = (GSList **)data;
  *l = g_slist_append (*l, (gpointer) r_item->jid);
  //  printf ("* %s\n", r_item->nickname ? r_item->nickname : r_item->jid);
}

GSList *
do_get_buddy_list (void)
{
  GSList *list = NULL;
  ft_roster_foreach (roster_iterator, (gpointer )&list);
  return list;
}

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
  const char *valid[] = { "online", "away", "chat", "xa", "dnd", "invisible", NULL }, *text = NULL;
  int show, offset;
  
  if((text = strchr(status, ' '))) {
    offset = text - status;
    text++;
  } else {
    offset = strlen(status);
  }
  for(show = 0; valid[show] != NULL; show++) {
    if( !strncmp(valid[show], status, offset) ) {
      break;
    }
  }
  if( !valid[show] ) {
    PRINTF ("Status must be one of [online|away|chat|xa|dnd|invisible]");
    return -1;
  }
  if (state.status_msg)
    g_free (state.status_msg);
  state.status_msg = strdup (status);

  LmMessage *msg = lm_message_new (NULL, LM_MESSAGE_TYPE_PRESENCE);
  if( show != 0 ) { // online status is implicit
    if (!strcmp (valid[show], "invisible")) {
      lm_message_node_set_attribute (msg->node, "type", "unavailable");
    } else {
      lm_message_node_add_child (msg->node, "show", valid[show]);
    }
  }
  if( text != NULL ) {
    lm_message_node_add_child (msg->node, "status", text);
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
  return -strcmp ((const char *)a, (const char *)b);
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
  if (g_strcasecmp (npass, ""))
    {
      LmMessage *msg = lm_message_new_with_sub_type (state.server,
						     LM_MESSAGE_TYPE_IQ,
						     LM_MESSAGE_SUB_TYPE_SET);
      LmMessageNode *query = lm_message_node_add_child (msg->node, "query", "");
      lm_message_node_set_attribute (query, "xmlns", "jabber:iq:register");
      lm_message_node_add_child (query, "username", state.jid.node);
      lm_message_node_add_child (query, "password", npass);

      LmMessage *reply = lm_connection_send_with_reply_and_block (state.conn, 
								  msg, NULL);

      const char *type = lm_message_node_get_attribute (reply->node, "type");
      if (!g_strcasecmp (type, "error"))
	{
	  do_printf ("Password change failed.\n");
	}
      else
	{
	  do_printf ("Password changed.\n");
	}
      lm_message_unref (msg);
    }
  free (npass);
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
    free (state.proxyserver);
  state.proxyserver = strdup (proxyserver);
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
    free (state.proxyuname);

  state.proxyuname = strdup (proxyuname);

  return 0;
}

int 
do_set_proxypasswd (const char *proxypasswd) 
{
  if (!proxypasswd) 
    return -1;

  if (state.proxypasswd)
    free (state.proxypasswd);

  state.proxypasswd = strdup (proxypasswd);

  return 0;
}

