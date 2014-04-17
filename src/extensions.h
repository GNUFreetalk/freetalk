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

#ifndef __EXTENSIONS_H__
#define __EXTENSIONS_H__

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <libguile.h>

extern SCM ex_message_receive_hook;
extern SCM ex_message_send_hook;
extern SCM ex_presence_receive_hook;
extern SCM ex_subscribe_receive_hook;
extern SCM ex_disconnect_hook;
extern SCM ex_command_hook;
extern SCM ex_login_hook;
extern SCM ex_quit_hook;
extern SCM ex_notify_file_hook;

int get_hook_return (void);
void set_hook_return (int hook_return_value);

void ft_load (const char *file);
void extensions_init (void);
void load_default_config (void);

#endif /* __EXTENSIONS_H__ */
