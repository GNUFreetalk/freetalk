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

#ifndef __FILE_TRANSFER_H__
#define __FILE_TRANSFER_H__

#include <loudmouth/loudmouth.h>
#include "roster.h"

#define FT_FILE_SUCCESS 1
#define FT_FILE_NO_FEATURE 2
#define FT_FILE_NO_OPEN 3

#define FT_INT_STR_LEN 10
#define FT_FILE_BUFFER_SIZE 1024
/* state of a file-transfer instance */
typedef struct ft_file_state_s{
  char *sender;
  char *reciever;
  char *remote_file;
  char *local_file;
  char *session_id;
  char *message_id;
  int fd;
  int cookie;
  struct ft_file_state_s *next;
} ft_file_state;


int ft_send_file (char *jid, char *reciever, char *file_path);
int ft_send_file_cb (LmMessage *msg);

int ft_msg_sub_type_get_cb (LmMessage *msg);
int ft_msg_sub_type_set_cb (LmMessage *msg);
int ft_msg_sub_type_result_cb (LmMessage *msg);
int ft_send_file_message_data (LmMessage *msg);

int ft_set_allow_file (int cookie_id, char *file_name);

void ft_file_flush (void);


#endif /* __FILE_TRANSFER_H__ */
