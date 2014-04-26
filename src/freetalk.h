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

#define FT_GLOBAL_EXT_DIR   DATADIR "/" PACKAGE_NAME "/extensions"
#define FT_LOCAL_EXT_DIR    "." PACKAGE_NAME "/extensions" /* relative to $HOME */
#define FT_CONFIG_SCM        "." PACKAGE_NAME "/" PACKAGE_NAME ".scm"

enum ft_conn_state {
        FT_DEAD = 0,
        FT_CONN,
        FT_AUTH
};

typedef struct {
        int argc;
        char **argv;
        char *server;
        char *jid_str;
        jid_t jid;
        char *password;
        FtRosterItem *current_buddy;
        LmConnection *conn; /* = (LmConnection *) conn */
        char *prompt; /* "freetalk> " */
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
int ft_msg_iq_version_cb (LmMessage*);
int ft_msg_iq_last_cb (LmMessage*);
void ft_bind_key (char key, char *command);
void ft_bind_to_ctrl_key (char key, char *command);

#endif /* __FREETALK_H__ */
