/*
  Copyright (c) 2005-2014 Freetalk Core Team
  This file is part of GNU Freetalk.

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

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#define PRINTF do_printf
void do_ssl (void);
int do_connect (void);
int do_disconnect (void);
int do_quit (int status);
int do_set_jid (const char *jidstr);
const char * do_get_jid (void);
int do_set_server (const char *server);
const char * do_get_server (void);
int do_set_port (unsigned short int port);
int do_get_port (void);
int do_set_proxyserver (const char *proxyserver);
const char * do_get_proxyserver (void);
int do_set_proxyuname (const char *proxyuname);
int do_set_proxypasswd (const char *proxypasswd);
const char * do_get_proxyuname (void);
const char * do_get_proxypasswd (void);
int do_set_proxyport (unsigned short int proxyport);
int do_get_proxyport (void);
int do_set_password (const char *password);
const char * do_get_password (void);
int do_set_prompt (const char *prompt);
const char * do_get_prompt (void);
int do_set_ssl (char value);
int do_get_ssl (void);
int do_set_tls (char value);
int do_get_tls (void);
int do_set_proxy (char value);
int do_get_proxy (void);
int do_send_message (char *jid, char *msg);
int do_send_message_no_hook (char *jid, char *msg);
int do_set_daemon (void); /* switch to daemon mode */
int do_get_daemon (void); /* in daemon mode ? */

int do_printf (const char *fmt, ...);
int do_get_conn_status (void);
enum ft_conn_state do_set_conn_status (enum ft_conn_state status);
int do_add (char *jid);
int do_set_status_msg (char *status);
const char * do_get_status_msg (void);
int do_set_current_buddy (char *bud);
const char *do_get_current_buddy (void);
int do_reset_fs_state (void);
void do_dict_append (char *word);
void do_dict_prepend (char *word);
void do_dict_insert (char *word);
void do_dict_remove (char *word);
int do_connect_blocking (void);
int do_auth_blocking (void);
void do_session_init (gboolean success);
void do_main_loop (void);
void do_change_password (char *npass);

#endif /* __COMMANDS_H__ */
