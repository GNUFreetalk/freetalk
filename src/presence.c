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

#include <loudmouth/loudmouth.h>

#include "freetalk.h"
#include "extensions.h"
#include "roster.h"
#include "primitives.h"
#include "commands.h"

/*
  Extract data from a presence message and populate a FtRosterItem
*/
static FtRosterItem *
roster_item_extract (LmMessage *msg)
{
        char *resource = NULL;
        FtRosterItem *item = g_new (FtRosterItem, 1);
        LmMessageNode *show = lm_message_node_find_child (msg->node, "show");
        LmMessageNode *status = lm_message_node_find_child (msg->node, "status");
        LmMessageNode *name = lm_message_node_find_child (msg->node, "name");

        const char *type = lm_message_node_get_attribute (msg->node, "type");

        item->jid = (char *) lm_message_node_get_attribute (msg->node, "from");

        resource = strchr (item->jid, '/');
        if (resource)
                item->resource = g_strdup (resource + 1);
        else
                item->resource = NULL;

        if (!type || !g_ascii_strcasecmp (type, "available"))
                item->is_online = TRUE;
        else
                item->is_online = FALSE;

        if (show)
                item->show_msg = g_strdup (lm_message_node_get_value (show));
        else
                item->show_msg = NULL;

        if (status)
                item->status_msg = g_strdup (lm_message_node_get_value (status));
        else
                item->status_msg = NULL;

        if (name)
                item->nickname = g_strdup (lm_message_node_get_value (name));
        else
                item->nickname = NULL;

        return item;
}

/*
  Called when presence of type 'available' or 'unavailable' is recieved
*/

static void
presence_availability_rcvd (const char *from, LmMessage *msg)
{
        FtRosterItem *newi;
        FtRosterItem *old;

        if (!from || !msg)
                return;

        newi = roster_item_extract (msg);

        scm_run_hook (ex_presence_receive_hook,
                      roster_item_to_list (newi));

        old = ft_roster_lookup (newi->jid);

        if (old) {
                old->is_online = newi->is_online;

                if (newi->nickname) {
                        if (old->nickname)
                                g_free (old->nickname);
                        old->nickname = newi->nickname;
                }

                if (old->show_msg)
                        g_free (old->show_msg);
                old->show_msg = newi->show_msg;

                if (old->status_msg)
                        g_free (old->status_msg);
                old->status_msg = newi->status_msg;

                if (old->resource)
                        g_free (old->resource);
                old->resource = newi->resource;
        }

        g_free (newi);
}

static void
presence_subscribe_rcvd (const char *from, LmMessage *msg)
{
        scm_run_hook (ex_subscribe_receive_hook,
                      scm_list_n (scm_from_locale_string (from),
                                  SCM_UNDEFINED));
}

/* Callback for all presence messages */

void
ft_presence_cb (LmMessage *msg)
{
        if (!msg)
                return;

        const char *type = lm_message_node_get_attribute (msg->node, "type");
        const char *from = lm_message_node_get_attribute (msg->node, "from");

        if (!from)
                return;

        if (!type) {
                presence_availability_rcvd (from, msg);
                return;
        }

        if (!g_ascii_strcasecmp (type, "unavailable"))
                presence_availability_rcvd (from, msg);

        else if (!g_ascii_strcasecmp (type, "available"))
                presence_availability_rcvd (from, msg);

        else if (!g_ascii_strcasecmp (type, "subscribe"))
                presence_subscribe_rcvd (from, msg);

}

/*
  Send final presence when logging off
*/

void
ft_presence_send_final (void)
{
        LmMessage *msg = lm_message_new_with_sub_type (NULL,
                                                       LM_MESSAGE_TYPE_PRESENCE,
                                                       LM_MESSAGE_SUB_TYPE_UNAVAILABLE);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
}

/*
  Send initial presence on login
*/

void
ft_presence_send_initial (void)
{
        LmMessage *msg;
        msg = lm_message_new_with_sub_type (NULL, LM_MESSAGE_TYPE_PRESENCE,
                                            LM_MESSAGE_SUB_TYPE_AVAILABLE);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
}

/*
  Allow buddy to subscribe to our presence
*/

void ft_presence_subscription_allow (char *jid)
{
        LmMessage *msg = lm_message_new_with_sub_type (jid,
                                                       LM_MESSAGE_TYPE_PRESENCE,
                                                       LM_MESSAGE_SUB_TYPE_SUBSCRIBED);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
}

/*
  Deny buddy permission to subscribe to our presence
*/

void ft_presence_subscription_deny (char *jid)
{
        LmMessage *msg = lm_message_new_with_sub_type (jid,
                                                       LM_MESSAGE_TYPE_PRESENCE,
                                                       LM_MESSAGE_SUB_TYPE_UNSUBSCRIBED);
        lm_connection_send (state.conn, msg, NULL);
        lm_message_unref (msg);
}
