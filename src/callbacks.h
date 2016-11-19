/* callbacks.h -- callback functions declarations
   Copyright (C) 2005-2014 Freetalk Core Team

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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

void ft_authenticate_cb (LmConnection *conn, gboolean success,
                         gpointer user_data);

void ft_connection_open_cb (LmConnection *conn, gboolean success,
                            gpointer user_data);

void ft_register_msg_handlers (LmConnection *conn);

LmSSLResponse ft_ssl_response_cb (LmSSL *ssl, LmSSLStatus st, gpointer data);

#endif /* __CALLBACKS_H__ */
