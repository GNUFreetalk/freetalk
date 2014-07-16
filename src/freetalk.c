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

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <glib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <loudmouth/loudmouth.h>
#include <libguile.h>
#include <getopt.h>

#include <argp.h>

#include "freetalk.h"
#include "callbacks.h"
#include "util.h"
#include "roster.h"
#include "interpreter.h"
#include "commands.h"
#include "extensions.h"
#include "register.h"
#include "common.h"

ft_state state;

GSList *key_bindings = NULL;

typedef struct {
        char key;
        char *command;
} key_binding;

static void
state_init (void)
{
        do_set_prompt ("~\\/~ ");
        state.roster = NULL;
        state.script = NULL;
        state.config_dir = (char *)calloc (1, strlen (getenv ("HOME")) + 1 + strlen (".freetalk") + 1);
        sprintf (state.config_dir, "%s/.freetalk", getenv ("HOME"));
        state.need_ssl = 0; /* turn off ssl by default */
        state.need_tls = 0; /* turn off tls by default */
        state.need_proxy = 0; /* turn off proxy by default */
        state.last = time(NULL);
}

SCM
catcher_handler (void *data SCM_UNUSED, SCM tag, SCM throw_args SCM_UNUSED)
{
        PRINTF("%s", _("No such command or buddy. See /help"));
        return SCM_BOOL_F;
}

static SCM
scm_freetalk_eval_string (void *data)
{
        char *scheme_code = (char *) data;
        return scm_c_eval_string (scheme_code);
}

static SCM
scm_freetalk_catch (const char *str, scm_t_catch_handler handler)
{
        return scm_internal_catch (SCM_BOOL_T,
                                   (scm_t_catch_body) scm_freetalk_eval_string,
                                   (void *) str,
                                   (scm_t_catch_handler) handler,
                                   (void *) str);
}

static void
process_line (char *line)
{
        char *eval_str;

        if (!line)
                return;

        if (*line)
                add_history (line);

        eval_str = g_strdup (line);

        state.async_printf = 0;
        if (interpreter (line) != 0) {
                scm_freetalk_catch (eval_str,
                                    (scm_t_catch_handler) catcher_handler);
                scm_force_output (scm_current_output_port ());
        }
        g_free (eval_str);
        state.async_printf = 1;
}

gboolean
stdin_input_cb (GIOChannel *chan, GIOCondition *cond, gpointer conn)
{
        rl_callback_read_char ();
        return TRUE;
}

char *get_word_break_characters()
{
        int i = 0;

        if  (rl_line_buffer[0] != '/') {
                for (i = rl_point; i >= 0; i--) {
                        if (rl_line_buffer[i] == ':')
                                return (char *)rl_basic_word_break_characters;
                }
                return "\t\n\"\\'`@$><=;|&{(";
        }

        return (char *)rl_basic_word_break_characters;
}

void
interface_init (void)
{
        GIOChannel *chan;

        interpreter_init ();

        rl_completion_word_break_hook = get_word_break_characters;
        rl_callback_handler_install (state.prompt, process_line);
        rl_attempted_completion_function = ft_auto_complete;
        rl_completion_entry_function = NULL;

        state.async_printf = 1;

        chan = g_io_channel_unix_new (0);
        g_io_add_watch (chan, G_IO_IN, (GIOFunc) stdin_input_cb, NULL);
}

const char *argp_program_version = PACKAGE_NAME " " PACKAGE_VERSION;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;

static error_t
parse_opts (int key, char *arg, struct argp_state *_state)
{
        switch (key) {
        case 'j':
                do_set_jid (arg);
                break;
        case 's':
                state.script = arg;
                break;
                //case 'r':
                // TODO - C++ code fix it later
                //ft_register ();
                break; /* not reached */
        default:
                /* hack to allow args to script */
                if (!state.script)
                        return ARGP_ERR_UNKNOWN;
        }

        return 0;
}

static void
mode_init (void)
{
        int i = 1;

        while (i < state.argc) {
                if (!g_strcmp0 (state.argv[i], "-s")) {
                        if (state.argv[i+1])
                                state.script = state.argv[i+1];
                }
                i++;
        }
}

static void
args_init (void)
{
        struct {
                char *f[2];
        } f;
        static char doc[] = "Freetalk is a console based jabber client/bot with a readline interface and guile extensions";
        static char argp_doc[] = " ";
        static struct argp_option options [] = {
                {"jid", 'j', "JABBERID", 0, "user@domain Jabber ID" },
                {"script", 's', "SCRIPTFILE", 0, "Freetalk script" },
                //{"register", 'r', 0, 0, "Register an account with a server"},
                { 0, }
        };
        static struct argp argp = { options, parse_opts, argp_doc, doc };

        argp_parse (&argp, state.argc, state.argv, 0, 0, &f);
}

static void
inner_main (void *closure, int argc, char **argv)
{
        check_first_run ();

        state_init ();
        mode_init ();

        extensions_init ();

        if (!state.script) {
                args_init ();
                ft_load ("init.scm");
                load_default_config (); /* ~/.freetalk/freetalk.scm */
                ft_load ("login.scm");

                interface_init ();

                do_main_loop ();
        } else {
                ft_load (state.script);
        }
}

/*
 * Debugging hint: use LM_DEBUG=NET ./freetalk to see all the traffic
 * on the wire
 */

int
main (int argc, char **argv)
{
        /* Trigerring Gettext */
        setlocale(LC_ALL,"");
        bindtextdomain (PACKAGE, LOCALEDIR);
        textdomain (PACKAGE);

        state.argc = argc;
        state.argv = argv;
        scm_boot_guile (argc, argv, inner_main, 0);

        return 0;
}

/* function called when we receive a jabber:iq:version message */
int ft_msg_iq_version_cb(LmMessage *msg)
{
        const char *from = lm_message_node_get_attribute (msg->node, "from");
        const char *id   = lm_message_node_get_attribute (msg->node, "id");
        LmMessage *send_msg;
        LmMessageNode *query, *name, *version;

        send_msg = lm_message_new_with_sub_type (from,
                                                 LM_MESSAGE_TYPE_IQ,
                                                 LM_MESSAGE_SUB_TYPE_RESULT);

        lm_message_node_set_attribute (send_msg->node,
                                       "id", id);
        query = lm_message_node_add_child (send_msg->node, "query", NULL);
        lm_message_node_set_attribute (query,
                                       "xmlns", "jabber:iq:version");

        name = lm_message_node_add_child (query, "name", PACKAGE_NAME);
        version = lm_message_node_add_child (query, "version", PACKAGE_VERSION);

        int result = lm_connection_send (state.conn, send_msg, NULL);
        lm_message_node_unref (version);
        lm_message_node_unref (name);
        lm_message_node_unref (query);
        lm_message_unref (send_msg);
        return result;
}

/* function called when we receive a jabber:iq:last message */
int ft_msg_iq_last_cb(LmMessage *msg)
{
        const char *from = lm_message_node_get_attribute (msg->node, "from");
        const char *id   = lm_message_node_get_attribute (msg->node, "id");
        LmMessage *send_msg;
        LmMessageNode *query;

        char seconds[256];
        snprintf( seconds, sizeof(seconds)-1, "%ld", time(NULL) - state.last );

        send_msg = lm_message_new_with_sub_type (from,
                                                 LM_MESSAGE_TYPE_IQ,
                                                 LM_MESSAGE_SUB_TYPE_RESULT);

        lm_message_node_set_attribute (send_msg->node,
                                       "id", id);
        query = lm_message_node_add_child (send_msg->node, "query", NULL);

        lm_message_node_set_attribute (query,
                                       "seconds", seconds );
        lm_message_node_set_attribute (query,
                                       "xmlns", "jabber:iq:last" );

        int result = lm_connection_send (state.conn, send_msg, NULL);
        lm_message_node_unref (query);
        lm_message_unref (send_msg);
        return result;
}

int ft_key_bound (int count, int key)
{
        GSList *list;
        for (list = key_bindings; list; list = list->next) {
                key_binding *b = (key_binding*) list->data;
                if (b->key == key) {
                        scm_c_eval_string (b->command);
                        scm_force_output (scm_current_output_port ());
                        return 0;
                }
        }
        return 1;
}

void ft_bind_key (char key, char *command)
{
        GSList *list;
        for (list = key_bindings; list; list = list->next) {
                key_binding *b = (key_binding*) list->data;
                if (b->key == key) {
                        char *old = b->command;
                        b->command = g_strdup (command);
                        g_free (old);
                        return;
                }
        }
        key_binding *b = (key_binding*) g_malloc (sizeof (key_binding));
        b->key = key;
        b->command = g_strdup (command);
        rl_bind_key (key, &ft_key_bound);
        key_bindings = g_slist_append (key_bindings, b);
}

void ft_bind_to_ctrl_key (char key, char *command)
{
        ft_bind_key (CTRL (key), command);
}
