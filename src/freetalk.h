/* freetalk.h
   Copyright (C) 2005-2014 Freetalk Core Team
   Copyright (C) 2016 Mathieu Lirzin <mthl@gnu.org>

   This file is part of GNU Freetalk.

   GNU Freetalk is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   GNU Freetalk is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Freetalk.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef __FREETALK_H__
#define __FREETALK_H__

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <loudmouth/loudmouth.h>
#include <time.h>
#include "util.h"
#include "roster.h"

#define FT_GLOBAL_EXT_DIR DATADIR "/" PACKAGE "/extensions"
/* Relative to $HOME.  */
#define FT_LOCAL_EXT_DIR "." PACKAGE "/extensions"
#define FT_CONFIG_SCM "." PACKAGE "/" PACKAGE ".scm"

enum ft_conn_state
{
  FT_DEAD = 0,
  FT_CONN,
  FT_AUTH
};

typedef struct
{
  char *server;
  char *jid_str;
  jid_t jid;
  char *password;
  FtRosterItem *current_buddy;
  LmConnection *conn;           /* = (LmConnection *) conn */
  char *prompt;                 /* "freetalk> " */
  GError *error;
  char need_ssl;
  char need_tls;
  unsigned short port;
  char daemon;
  char async_printf;
  enum ft_conn_state conn_state;
  char *status_msg;
  GSList *roster;
  char *script;
  char *config_dir;
  time_t last;
  char need_proxy;
  char *proxyserver;
  unsigned short proxyport;
  char *proxyuname;
  char *proxypasswd;
  LmProxy *proxy;
} ft_state;

extern ft_state state;

void interface_init (void);
int ft_msg_iq_version_cb (LmMessage *);
int ft_msg_iq_last_cb (LmMessage *);
void ft_bind_key (char key, char *command);
void ft_bind_to_ctrl_key (char key, char *command);

#endif /* __FREETALK_H__ */
