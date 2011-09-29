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

/* Status->
 * -> reciever end is recieving the data... and also a small message ...
 * -> sender side segfaults when run outside debugger */
#include <glib.h>
#include <stdlib.h>
#include <loudmouth/loudmouth.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "freetalk.h"
#include "extensions.h"
#include "file_transfer.h"
#include "compat.h"

/* file-transfer: implementation howto
 *
 * Sender blocks till the file transfer is complete.
 * Sender does following in a single call ft_send_file:
 * -> Send service discovery request to the reciever.
 * -> If reciever cannot recieve a file, then return from here.
 * -> Create an IBB (In-Band Bytestream) with the reciever, because we support only IBB as of now.
 * -> Activate IBB.
 * -> Send file content after encoding it in Base64.
 * -> Close the IBB.
 *
 * Reciever asyncronously recieves the file-transfer messages from the Sender.
 * File transfer for a reciever works based on call-backs.
 * -> callback function is called for each IQ message based on its subtype.
 * -> reciever declines a second file-transfer request when the first is in progress.
 */



/* file-transfer support for freetalk */

/* we need to block in this function, since we use TCP */
static int cookie_count;

static int ft_send_sdisco(char *reciever)
{
  LmMessage *msg, *reply;
  LmMessageNode *child_node;
  FtRosterItem *rcv_item;
  char *reciever_full = NULL;

  rcv_item = ft_roster_lookup (reciever);
  
  if (!rcv_item) {
    return -1;
  }
  
  if (!rcv_item->is_online || !rcv_item->resource) {
    return -2;
  }

  reciever_full = (char *)calloc (strlen (reciever) + 
			  strlen (rcv_item->resource) + 2, 1);
  sprintf(reciever_full, "%s/%s", reciever, rcv_item->resource);
  
  msg = lm_message_new_with_sub_type (reciever_full, 
				      LM_MESSAGE_TYPE_IQ,
				      LM_MESSAGE_SUB_TYPE_GET);

  child_node = lm_message_node_add_child (msg->node, "query", NULL);
  lm_message_node_set_attribute (child_node,
				 "xmlns", "http://jabber.org/protocol/disco#info");
  
  
  reply = lm_connection_send_with_reply_and_block (state.conn, msg, NULL);

  /* check if the far end supports the protocol we expect them to support
   * i.e, IBB */

  if (reciever_full)
    free (reciever_full);
  return 1;

}


/* Callbacks */

/* function called when we recieve a dicovery service message */

/* right now assumed that only type of 'get' request is service discovery */
int ft_msg_sub_type_get_cb(LmMessage *msg)
{
  const char *from = lm_message_node_get_attribute (msg->node, "from");
  const char *id   = lm_message_node_get_attribute (msg->node, "id");
  LmMessage *send_msg;
  LmMessageNode *child, *parent_node = NULL;
  
  send_msg = lm_message_new_with_sub_type (from,
				      LM_MESSAGE_TYPE_IQ,
				      LM_MESSAGE_SUB_TYPE_RESULT);
  
  lm_message_node_set_attribute (send_msg->node,
				 "id", id);
  child = lm_message_node_add_child (send_msg->node, "query", NULL);
  lm_message_node_set_attribute (child, 
				 "xmlns", "http://jabber.org/protocol/disco#info");
  
  parent_node = child;
  child = lm_message_node_add_child (parent_node, "feature", NULL);
  lm_message_node_set_attribute (child,
				 "var", "http://jabber.org/protocol/si");
  
  child = lm_message_node_add_child (parent_node, "feature", NULL);
  lm_message_node_set_attribute (child,
				 "var", "http://jabber.org/protocol/si/profile/file-transfer");

  child = lm_message_node_add_child (parent_node, "feature", NULL);
  lm_message_node_set_attribute (child,
				 "var", "http://jabber.org/protocol/ibb");


  return lm_connection_send (state.conn, send_msg, NULL);

}

/* sid compare function for g_slist */
static int file_sid_compare (gconstpointer p, gconstpointer q)
{
  return g_ascii_strcasecmp (((ft_file_state *)p)->session_id, ((ft_file_state *)q)->session_id);
}

/* cookie compare */
static int file_cookie_compare (gconstpointer p, gconstpointer q)
{
  return ((((ft_file_state *)p)->cookie == ((ft_file_state *)q)->cookie)?0:1);
}

/* sender compare */
static int file_sender_compare (gconstpointer p, gconstpointer q)
{
  return g_ascii_strcasecmp (((ft_file_state *)p)->sender, ((ft_file_state *)q)->sender);
}

/* reciever compare */
static int file_receiver_compare (gconstpointer p, gconstpointer q)
{
  if (((ft_file_state *)q)->reciever)
    return (g_ascii_strcasecmp (((ft_file_state *)p)->reciever, ((ft_file_state *)q)->reciever) || g_ascii_strcasecmp (((ft_file_state *)p)->message_id, ((ft_file_state *)q)->message_id));

  return 1;
}

/* 'set' request is assumed to be for file-transfer init 
 * we'll setup the ft_file_state list */
int ft_msg_sub_type_set_cb(LmMessage *msg)
{
  LmMessage *send_msg = NULL;
  LmMessageNode *child_node = NULL;
  ft_file_state *file = NULL, incoming;
  GSList *file_slist = NULL;
  const char *msg_id = lm_message_node_get_attribute (msg->node, "id");
  int ret = -1;
  const char *from = lm_message_node_get_attribute (msg->node, "from");

  if (!g_ascii_strcasecmp (msg_id, "offer1")){
    /* Get the name of the file that the reciever is willing to send */
    file = (ft_file_state *)calloc (sizeof (ft_file_state), 1);
    
    /* set cookie, increment the global cookie count */
    file->cookie = cookie_count++;


    child_node = lm_message_node_get_child (msg->node, "si");
    child_node = lm_message_node_get_child (child_node, "file");
    if (child_node){
      const char *file_name = lm_message_node_get_attribute (child_node, "name");
      file->remote_file = g_strdup (file_name);
      file->sender      = g_strdup (from);

      /* now, we'll stop here... we'll continue with file transfer only after
       * user accepts the offer with /allow-file */
      scm_run_hook (ex_notify_file_hook, scm_list_n (scm_from_locale_string (from),
						     scm_from_locale_string (file->remote_file),
						     scm_from_int (FT_FILE_SUCCESS),
						     scm_from_int (file->cookie),
						     SCM_UNDEFINED));
    }
    state.f_state = g_slist_append (state.f_state, file);
  } else {
    /* this is for IBB 'close'/'open' */
    
    if ((child_node = lm_message_node_get_child (msg->node, "open"))){
      const char *session = lm_message_node_get_attribute (child_node, "sid");
      const char *sender = lm_message_node_get_attribute (msg->node, "from");

      incoming.sender = g_strdup (sender);
      file_slist = g_slist_find_custom (state.f_state, &incoming, file_sender_compare);
      file = (ft_file_state *)file_slist->data;
      file->session_id = g_strdup (session);
      if (incoming.sender)
	g_free (incoming.sender);
      
      /* Before sending the 'result' reply, open the file to write to */
      if ((file->fd = open (file->local_file, O_CREAT | O_WRONLY, S_IWUSR)) < 0){
	scm_run_hook (ex_notify_file_hook, scm_list_n (scm_from_locale_string (file->sender),
						       scm_from_locale_string (file->local_file),
						       scm_from_int (FT_FILE_NO_OPEN),
						       scm_from_int (file->cookie),
						       SCM_UNDEFINED));
	/* send an 'error' message to the sender */
	state.f_state = g_slist_remove (state.f_state, file);
	if (file)
	  free (file);

	send_msg = lm_message_new_with_sub_type (from,
						 LM_MESSAGE_TYPE_IQ,
						 LM_MESSAGE_SUB_TYPE_ERROR);
	lm_message_node_set_attribute (send_msg->node, 
				       "id", msg_id);
	ret = lm_connection_send (state.conn, send_msg, NULL);

	return -2;
      }
    } else if((child_node = lm_message_node_find_child (msg->node, "close"))){
      const char *session = lm_message_node_get_attribute (child_node, "sid");
      incoming.session_id = g_strdup (session);
      file_slist = g_slist_find_custom (state.f_state, &incoming, file_sid_compare);
      file = (ft_file_state *)file_slist->data;

      /* free all the resources that we hold for the current transfer */
      if (file->remote_file)
	g_free (file->remote_file);
      if (file->local_file)
	g_free (file->local_file);
      if (file->session_id)
	g_free (file->session_id);
      if (file->sender)
	g_free (file->sender);
      	
      /* close the file we are writing to */
      close (file->fd);
      
      state.f_state = g_slist_remove (state.f_state, file);
      if (file)
	free (file);
      
    } else {
      /* this is not the case of 'open'/'close' */
      ;
    }
    /* since both 'open' and 'close' are one node messages, 'result' for both
     * are created using same sequence of calls */
    send_msg = lm_message_new_with_sub_type (from,
					     LM_MESSAGE_TYPE_IQ,
					     LM_MESSAGE_SUB_TYPE_RESULT);
    lm_message_node_set_attribute (send_msg->node, 
				   "id", msg_id);
    ret = lm_connection_send (state.conn, send_msg, NULL);


  }
  
  return ret;

}



/* handler called when file data is recieved in a 'message' */
int ft_send_file_message_data(LmMessage *msg)
{
  LmMessageNode *child_node;
  ft_file_state *file = NULL, incoming;
  GSList *file_slist = NULL;
  char *file_data = NULL;
  int data_len = -1;
  int ret = 0;

  child_node = lm_message_node_get_child (msg->node, "data");
  if (child_node){
    const char *session = lm_message_node_get_attribute (child_node, "sid");

    incoming.session_id = g_strdup (session);
    
    file_slist = g_slist_find_custom (state.f_state, &incoming, file_sid_compare);
    file = (ft_file_state *)file_slist->data;

    if (incoming.session_id)
      g_free (incoming.session_id);
  } else {
    return -2;
  }

  /* recieve the file data. decode file data. store it to a local file */
  /* right now, assumed that we get the whole file in one stretch, and file is
   * zero sized */
  /* not required here 
  if ( !atoi (lm_message_node_get_attribute (child_node, "seq"))){
    if ((fd = open(file->local_file, O_CREAT | O_WRONLY)) < 0){
      return -2;
    } else {
      file->fd = fd;
    }
    } */
  
  /* get the data and write to file */
  file_data = g_strdup (lm_message_node_get_value (child_node));
  data_len = strlen (file_data);

  ret = write (file->fd, file_data, data_len);

  if (file_data){
    g_free (file_data);
  }

  
  return ret;
}
/* transfer the file using In-Band Bytestream */
static int ft_do_send_file(char *reciever, char *file)
{
  LmMessage *send_msg = NULL;
  LmMessageNode *child_node = NULL, *parent_node = NULL;
  LmMessage *file_msg = NULL, *reply = NULL;
  char *session = NULL, *block_size = NULL, *sequence = NULL;
  int bsize = 4096; /* FIXME: bsize is set to maximum possible value */
  int seqno = 0;
  int read_size = -1;
  char *data_buffer = NULL;
  int fd = -1;



  session = (char *)calloc (strlen (file) + strlen (reciever) + 
		    2 + FT_INT_STR_LEN, 1);
  sprintf (session, "%s_%s_%d", file, reciever, (int)(random()));
  block_size = (char *)calloc (FT_INT_STR_LEN, 1);
  sprintf (block_size, "%d", bsize);
  sequence = (char *)calloc (FT_INT_STR_LEN, 1);

  /* initiate byte-stream setup */
  send_msg = lm_message_new_with_sub_type (reciever,
					   LM_MESSAGE_TYPE_IQ,
					   LM_MESSAGE_SUB_TYPE_SET);
  lm_message_node_set_attribute (send_msg->node, 
				 "id", "inband_1");
  
  child_node = lm_message_node_add_child (send_msg->node, "open", NULL);
  lm_message_node_set_attributes (child_node,
				  "sid", session,
				  "block-size", block_size,
				  "xmlns", "http://jabber.org/protocol/ibb",
				  NULL);
  
  reply = lm_connection_send_with_reply_and_block (state.conn, send_msg, NULL);
  
  if (!reply){
    return -2;
  }
  
  if ((fd = open (file, O_RDONLY)) < 0){
    return -2;
  }
  
  data_buffer = (char *)calloc (FT_FILE_BUFFER_SIZE, sizeof(char));

  if (data_buffer && (LM_MESSAGE_SUB_TYPE_RESULT == lm_message_get_sub_type (reply))) {
    /* we have got the response from the reciever that he is ready for the
     * file transfer. We can now start transferring the file */
    while ((read_size = read (fd, data_buffer, FT_FILE_BUFFER_SIZE))){
      file_msg = lm_message_new (reciever,
				 LM_MESSAGE_TYPE_MESSAGE);
      lm_message_node_set_attribute (file_msg->node,
				     "id", "data_1");
    
      child_node = lm_message_node_add_child (file_msg->node, "data", NULL);
      
      lm_message_node_set_value (child_node, data_buffer);

      sprintf (sequence, "%d", seqno++);
      lm_message_node_set_attributes (child_node,
				      "xmlns", "http://jabber.org/protocol/ibb",
				      "sid", session,
				      "seq", sequence,
				      NULL);
    
      child_node = lm_message_node_add_child (file_msg->node, "amp", NULL);
      lm_message_node_set_attribute (child_node,
				     "xmlns", "http://jabber.org/protocol/amp");
    
      parent_node = child_node;
      child_node = lm_message_node_add_child (child_node, "rule", NULL);
      lm_message_node_set_attributes (child_node,
				      "condition", "deliver-at",
				      "value", "stored",
				      "action", "error",
				      NULL);
      
      child_node = lm_message_node_add_child (parent_node, "rule", NULL);
      lm_message_node_set_attributes (child_node,
				      "condition", "match-resource",
				      "value", "exact",
				      "action", "error",
				      NULL);

      lm_connection_send (state.conn, file_msg, NULL);
      sleep(1);
      memset (data_buffer, 0, FT_FILE_BUFFER_SIZE);
    }
    
  } else {
    /* reciever is not ready to recieve the file */
    return -2;
  }
  if (data_buffer)
    free (data_buffer);
  
  /* we are done with sending the file. Now we have to close the IBB */
  send_msg = lm_message_new_with_sub_type (reciever,
					   LM_MESSAGE_TYPE_IQ,
					   LM_MESSAGE_SUB_TYPE_SET);
  lm_message_node_set_attribute (send_msg->node,
				  "id", "inband_2");
  
  child_node = lm_message_node_add_child (send_msg->node, "close", NULL);
  lm_message_node_set_attributes (child_node, 
				  "xmlns", "http://jabber.org/protocol/ibb",
				  "sid", session,
				  NULL);
  
  reply = lm_connection_send_with_reply_and_block (state.conn, send_msg, NULL);

  if (session)
    free (session);
  if (block_size)
    free (block_size);
  if (sequence)
    free (sequence);

  if (!reply) {
    return -2;
  }
  
  if (LM_MESSAGE_SUB_TYPE_RESULT == lm_message_get_sub_type (reply)){
    return 0;
  }
  
  return -2;
	   	  
}

int ft_msg_sub_type_result_cb(LmMessage *msg)
{
  LmMessageNode *child_node = lm_message_node_get_child (msg->node, "si");
  const char *from = lm_message_node_get_attribute (msg->node, "from");
  char *from_jid = NULL;
  GSList *file_slist = NULL;
  ft_file_state *file_state = NULL, incoming;

  /* only result message we care about right now is for the file transfer
     offer. we will carry on the actual file data transfer once we recieve
     'result' reply for file transfer offer */
  if (child_node){
    incoming.message_id = g_strdup (lm_message_node_get_attribute (msg->node, "id"));
    incoming.reciever = g_strdup (from);

    file_slist = g_slist_find_custom (state.f_state, &incoming, file_receiver_compare);
    if (!file_slist)
      return -2;

    file_state = (ft_file_state *)file_slist->data;
    
    if (incoming.message_id)
      g_free (incoming.message_id);
    if (incoming.reciever)
      g_free (incoming.reciever);
    
    from_jid = g_strdup (from);

    
    if (ft_do_send_file(from_jid, file_state->local_file)){
      if (from_jid)
	g_free (from_jid);
      return -2;
    }

    if (from_jid)
      g_free (from_jid);
    /* file transfer is done, remove the file state from the list */
    state.f_state = g_slist_remove (state.f_state, file_state);
    
    return 0;
        
  }

  /* see, from whom we have recieved the result message.
   * then decide what has to be done on reciept of this result */
  return 0;

}



/* send file transfer init negotiation message 
 * this function blocks till a result/error response is recieved
 * target */  
static int ft_send_file_init(char *reciever, char *file)
{
  LmMessage *msg = NULL;
  LmMessageNode *parent_node = NULL, *child_node = NULL;
  FtRosterItem *rcv_item = NULL;
  char *reciever_full = NULL;
  int ret = 1;
  struct stat file_stat;
  char *file_size = (char *)calloc (FT_INT_STR_LEN, 1);
  ft_file_state *file_state = NULL;

  stat (file, &file_stat);
  sprintf (file_size, "%d", (int)file_stat.st_size);
  
  /* Here we check if reciever is online and we append /resource to the 
   * jabber ID of the reciever */
  rcv_item = ft_roster_lookup (reciever);
  
  if (!rcv_item){
    return -1;
  }

  if (!rcv_item->is_online || !rcv_item->resource){
    return -2;
  }

  reciever_full = (char *)calloc (strlen (reciever) +
			  strlen (rcv_item->resource) + 2, 1);
  sprintf (reciever_full, "%s/%s", reciever, rcv_item->resource);

  /* Create a message of type IQ, this message is gives an offer to the reciver
   * about the intended file transfer 
   * http://jabber.org/jeps/jep-0096.html */
  msg = lm_message_new_with_sub_type (reciever_full,
				      LM_MESSAGE_TYPE_IQ,
				      LM_MESSAGE_SUB_TYPE_SET);
  lm_message_node_set_attribute (msg->node,
			     "id", "offer1");
  
  child_node = lm_message_node_add_child (msg->node, "si", NULL);
  lm_message_node_set_attributes (child_node,
                  "xmlns", "http://jabber.org/protocol/si",
				  "id", "a0",
				  "mime-type", "text/plain",
				  "profile", "http://jabber.org/protocol/si/profile/file-transfer",
				  NULL);
  
  parent_node = child_node;
  child_node = lm_message_node_add_child (child_node, "file", NULL);
  lm_message_node_set_attributes (child_node, 
				  "xmlns", "http://jabber.org/protocol/si/profile/file-transfer",
				  "name", file,
				  "size", file_size,
				  NULL);
  
  child_node = lm_message_node_add_child (parent_node, "feature", NULL);
  lm_message_node_set_attribute (child_node, 
				 "xmlns", "http://jabber.org/protocol/feature-neg");
  
  parent_node = child_node;
  child_node = lm_message_node_add_child (parent_node, "x", NULL);
  lm_message_node_set_attributes (child_node,
				  "xmlns", "jabber:x:data",
				  "type", "form", NULL);

  parent_node = child_node;
  child_node = lm_message_node_add_child (parent_node, "field", NULL);
  lm_message_node_set_attributes (child_node,
				  "var", "stream-method",
				  "type", "list-single",
				  NULL);
  
  parent_node = child_node;
  child_node = lm_message_node_add_child (parent_node, "option", NULL);
  child_node = lm_message_node_add_child (child_node, "value", "http://jabber.org/protocol/ibb");
  
  /* we send a file transfer offer to the reciever and store the state in the
     linked list of f_state. file data will be sent in ft_sub_type_result_cb */
  ret = lm_connection_send (state.conn, msg, NULL);
  
  file_state = (ft_file_state *)calloc (sizeof (ft_file_state), 1);
  
  if (!file_state)
    return -2;

  file_state->message_id = g_strdup ("offer1");
  file_state->reciever = g_strdup (reciever_full);
  file_state->local_file = g_strdup (file);
  state.f_state = g_slist_append (state.f_state, file_state);

  if (reciever_full)
    free(reciever_full);

  return ret;

}
  
  

int ft_send_file(char *jid_str, char *reciever, char *file_name)
{
  
  if (ft_send_sdisco (reciever) < 0){
    scm_run_hook (ex_notify_file_hook, scm_list_n (scm_from_locale_string (reciever),
						   scm_from_locale_string (file_name),
						   scm_from_int (FT_FILE_NO_FEATURE),
						   scm_from_int (-1),
						   SCM_UNDEFINED));
    return -2;
  }


  if (ft_send_file_init (reciever, file_name) < 0) {
    /* elaborate on error.... ;) */
  }

  return 1;
}

/* when our buddy wants to transfer a file towards us */
int ft_set_allow_file(int cookie_id, char *file_name)
{
  LmMessage *send_msg = NULL;
  LmMessageNode *child_node = NULL;
  GSList *file_slist = NULL;
  ft_file_state *file = NULL, incoming;
  int ret = -1;

  /* find file state based on cookie_id */
  incoming.cookie = cookie_id;
  file_slist = g_slist_find_custom (state.f_state, &incoming, file_cookie_compare);
  if (!file_slist){
    return -2;
  }
  file = (ft_file_state *)file_slist->data;

  /* there is no active file-transfer with cookie_id */
  if (!file){
    return -2;
  }

  /* now, set the file name, if provided */
  if (strlen (file_name)){
    file->local_file = (char *)calloc (strlen (state.download_dirname) + 1 +
			       strlen (file_name) + 1, 1);
    sprintf (file->local_file, "%s/%s", state.download_dirname, file_name);
  } else {
    file->local_file = (char *)calloc (strlen (state.download_dirname) + 1 +
			       strlen (file->remote_file) +1, 1);
    sprintf (file->local_file, "%s/%s", state.download_dirname, file->remote_file);
  }

  /* we are called as soon as a file-transfer init is called. 
   * user does /allow-file. this is continuation from the point 
   * that we have recieved a file-transfer offer */

  /* prepare a reply for file transfer offer */
  send_msg = lm_message_new_with_sub_type (file->sender,
					   LM_MESSAGE_TYPE_IQ,
					   LM_MESSAGE_SUB_TYPE_RESULT);
  
  lm_message_node_set_attribute (send_msg->node,
				 "id", "offer1");
  
  child_node = lm_message_node_add_child (send_msg->node, "si", NULL);
  lm_message_node_set_attribute (child_node,
				 "xmlns", "http://jabber.org/protocol/si");
  
  child_node = lm_message_node_add_child (child_node, "feature", NULL);
  lm_message_node_set_attribute (child_node,
				 "xmlns", "http://jabber.org/protocol/feature-neg");
  
  child_node = lm_message_node_add_child (child_node, "x", NULL);
  lm_message_node_set_attributes (child_node,
				  "x", "jabber:x:data",
				  "type", "submit", NULL);
  
  child_node = lm_message_node_add_child (child_node, "field", NULL);
  lm_message_node_set_attribute (child_node,
				 "var", "stream-method");
  
  child_node = lm_message_node_add_child (child_node, "value", "http://jabber.org/protocol/ibb");

  /* upon delivery of this message we'll get a 'set' message to open an
   * IBB, which will be handled asyncronously */
  ret = lm_connection_send (state.conn, send_msg, NULL);
    
  return ret;
}

/* flush the list of file transfer states */
void ft_file_flush (void)
{
  g_slist_free (state.f_state);
  state.f_state = NULL;
}
