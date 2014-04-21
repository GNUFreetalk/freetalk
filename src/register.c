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

#include <loudmouth/loudmouth.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <unistd.h>

#include <string>
#include <map>

using namespace std;

static LmConnection *conn;
static char *server;

static LmMessage *send_registration_query ()
{
        LmMessage *msg = lm_message_new_with_sub_type (server, LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_GET);
        LmMessageNode *query = lm_message_node_add_child (msg->node, "query", "");
        lm_message_node_set_attribute (query, "xmlns", "jabber:iq:register");
        return lm_connection_send_with_reply_and_block (conn, msg, NULL);
}

static void handle_registration_reply (LmMessage *reply)
{
        const char *type = lm_message_node_get_attribute (reply->node, "type");
        if (!g_ascii_strcasecmp (type, "error"))
        {
                LmMessageNode *err = lm_message_node_find_child (reply->node, "error");
                printf ("Registration failed: ");
                if (err)
                        printf ("%s", lm_message_node_get_value (err));
                printf ("\n");
                exit (1);
        }

        printf ("Registration successful\n");
        exit (0);
}

map<string, string> fields;

static void send_required_fields (LmMessageNode *query)
{
        if (LmMessageNode *n = lm_message_node_find_child (query, "instructions"))
        {
                printf ("--------------------\n");
                printf ("Server instructions:\n");
                printf ("--------------------\n");
                printf ("%s\n", lm_message_node_get_value (n));
        }
        if (lm_message_node_find_child (query, "username"))
        {
                char *val = readline ("username: ");
                fields[string ("username")] = string (val);
                g_free (val);
        }
        if (lm_message_node_find_child (query, "password"))
        {
                string p1 = getpass ("password: ");
                string p2 = getpass ("password again: ");
                if (p1 != p2)
                {
                        printf ("Passwords don't match.\n");
                        exit (2);
                }
                fields[string ("password")] = p1;
        }

        for (LmMessageNode *n = query->children; n != NULL; n = n->next)
        {
                if (g_ascii_strcasecmp (n->name, "username") &&
                    g_ascii_strcasecmp (n->name, "password") &&
                    g_ascii_strcasecmp (n->name, "instructions"))
                {
                        char *val;
                        string name = string (n->name);
                        string prompt = name + string (": ");
                        val = readline (prompt.c_str ());
                        fields[name] = string (val);
                        g_free (val);
                }
        }


        LmMessage *msg = lm_message_new_with_sub_type (server,
                                                       LM_MESSAGE_TYPE_IQ,
                                                       LM_MESSAGE_SUB_TYPE_SET);
        LmMessageNode *q = lm_message_node_add_child (msg->node,
                                                      "query", "");
        lm_message_node_set_attribute (q, "xmlns", "jabber:iq:register");

        for (map<string,string>::iterator i = fields.begin ();
             i != fields.end (); i++)
        {
                lm_message_node_add_child (q, i->first.c_str (), i->second.c_str ());
        }

        LmMessage *reply = lm_connection_send_with_reply_and_block (conn, msg, NULL);
        handle_registration_reply (reply);
}

static void do_registration ()
{
        LmMessage *reply = send_registration_query ();
        const char *type = lm_message_node_get_attribute (reply->node, "type");

        if (!g_ascii_strcasecmp (type, "error"))
        {
                printf ("In-band registration not available on this server.\n");
                exit (1);
        }
        else
        {
                LmMessageNode *query = lm_message_node_get_child (reply->node,
                                                                  "query");
                if (lm_message_node_find_child (query, "registered"))
                {
                        printf ("You have already registered\n");
                        exit (1);
                }

                send_required_fields (query);
        }
}

void ft_register ()
{
        printf ("--------------------\n\n");
        printf ("Registration\n");
        printf ("--------------------\n\n");

        while (!server)
                server = readline ("Server: ");

        conn = lm_connection_new (server);
        if (!lm_connection_open_and_block (conn, NULL))
        {
                printf ("Could not connect.\n");
                exit (1);
        }

        do_registration ();
}
