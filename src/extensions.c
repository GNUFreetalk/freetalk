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

#include <stdlib.h>
#include <libguile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "freetalk.h"
#include "roster.h"
#include "primitives.h"
#include "common.h"

SCM ex_message_receive_hook;
SCM ex_message_send_hook;
SCM ex_presence_receive_hook;
SCM ex_subscribe_receive_hook;
SCM ex_disconnect_hook;
SCM ex_command_hook;
SCM ex_login_hook;
SCM ex_quit_hook;
SCM ex_notify_file_hook;

int hook_return = 0;

int
get_hook_return (void)
{
        return hook_return;
}

void
set_hook_return (int hook_return_value)
{
        hook_return = hook_return_value;
}

void ft_load (const char *file)
{
        char *path_local = NULL;
        char *path_global = NULL;

        struct stat foo;
        int len;

        /* ladies first */
        if (stat (file, &foo) == 0) {
                scm_c_primitive_load (file);
                goto out;
        }

        len = strlen (getenv ("HOME")) +
                1 + strlen (FT_LOCAL_EXT_DIR) + 1 + strlen (file) + 1;
        path_local = g_malloc_n (1, len);
        sprintf (path_local, "%s/%s/%s", getenv ("HOME"), FT_LOCAL_EXT_DIR,
                 file);
        if (stat (path_local, &foo) == 0) {
                scm_c_primitive_load (path_local);
                goto out;
        }

        len = strlen (FT_GLOBAL_EXT_DIR) + 1 + strlen (file) + 1;
        path_global = g_malloc_n (1, len);
        sprintf (path_global, "%s/%s", FT_GLOBAL_EXT_DIR, file);
        if (stat (path_global, &foo) == 0) {
                scm_c_primitive_load (path_global);
                goto out;
        }

        fprintf (stderr, "%s: not found\n", file);
out:
        if (path_local)
                g_free (path_local);
        if (path_global)
                g_free (path_global);
        return;
}

static void
register_hooks ()
{
        ex_message_receive_hook = scm_make_hook (scm_from_int (4));
        scm_c_define ("ft-message-receive-hook", ex_message_receive_hook);

        ex_message_send_hook = scm_make_hook (scm_from_int (2));
        scm_c_define ("ft-message-send-hook", ex_message_send_hook);

        ex_presence_receive_hook = scm_make_hook (scm_from_int (5));
        scm_c_define ("ft-presence-receive-hook", ex_presence_receive_hook);

        ex_subscribe_receive_hook = scm_make_hook (scm_from_int (1));
        scm_c_define ("ft-subscribe-receive-hook", ex_subscribe_receive_hook);

        ex_disconnect_hook = scm_make_hook (scm_from_int (1));
        scm_c_define ("ft-disconnect-hook", ex_disconnect_hook);

        ex_command_hook = scm_make_hook (scm_from_int (2));
        scm_c_define ("ft-command-hook", ex_command_hook);

        ex_login_hook = scm_make_hook (scm_from_int (1));
        scm_c_define ("ft-login-hook", ex_login_hook);

        ex_quit_hook = scm_make_hook (scm_from_int (1));
        scm_c_define ("ft-quit-hook", ex_quit_hook);

        ex_notify_file_hook = scm_make_hook (scm_from_int (4));
        scm_c_define ("ft-notify-file-hook", ex_notify_file_hook);
}

static void
register_primitives ()
{
        scm_c_define_gsubr ("ft-load", 1, 0, 0, ex_load);

        scm_c_define_gsubr ("ft-get-jid", 0, 0, 0, ex_get_jid);
        scm_c_define_gsubr ("ft-set-jid!", 1, 0, 0, ex_set_jid);

        scm_c_define_gsubr ("ft-get-server", 0, 0, 0, ex_get_server);
        scm_c_define_gsubr ("ft-set-server!", 1, 0, 0, ex_set_server);

        scm_c_define_gsubr ("ft-get-proxyserver", 0, 0, 0, ex_get_proxyserver);
        scm_c_define_gsubr ("ft-set-proxyserver!", 1, 0, 0, ex_set_proxyserver);

        scm_c_define_gsubr ("ft-get-proxyuname", 0, 0, 0, ex_get_proxyuname);
        scm_c_define_gsubr ("ft-set-proxyuname!", 1, 0, 0, ex_set_proxyuname);

        scm_c_define_gsubr ("ft-get-proxypasswd", 0, 0, 0, ex_get_proxypasswd);
        scm_c_define_gsubr ("ft-set-proxypasswd!", 1, 0, 0, ex_set_proxypasswd);

        scm_c_define_gsubr ("ft-get-password", 0, 0, 0, ex_get_password);
        scm_c_define_gsubr ("ft-set-password!", 1, 0, 0, ex_set_password);

        scm_c_define_gsubr ("ft-get-sslconn?", 0, 0, 0, ex_get_sslconn);
        scm_c_define_gsubr ("ft-set-sslconn!", 1, 0, 0, ex_set_sslconn);

        scm_c_define_gsubr ("ft-get-tlsconn?", 0, 0, 0, ex_get_tlsconn);
        scm_c_define_gsubr ("ft-set-tlsconn!", 1, 0, 0, ex_set_tlsconn);

        scm_c_define_gsubr ("ft-connect", 0, 0, 0, ex_connect);
        scm_c_define_gsubr ("ft-connect-blocking", 0, 0, 0,
                            ex_connect_blocking);
        scm_c_define_gsubr ("ft-disconnect", 0, 0, 0, ex_disconnect);

        scm_c_define_gsubr ("ft-send-message", 2, 0, 0, ex_send_message);
        scm_c_define_gsubr ("ft-send-message-no-hook", 2, 0, 0,
                            ex_send_message_no_hook);

        scm_c_define_gsubr ("ft-set-daemon", 0, 0, 0, ex_set_daemon);
        scm_c_define_gsubr ("ft-get-daemon", 0, 0, 0, ex_get_daemon);

        scm_c_define_gsubr ("ft-get-port", 0, 0, 0, ex_get_port);
        scm_c_define_gsubr ("ft-set-port!", 1, 0, 0, ex_set_port);

        scm_c_define_gsubr ("ft-get-proxyport", 0, 0, 0, ex_get_proxyport);
        scm_c_define_gsubr ("ft-set-proxyport!", 1, 0, 0, ex_set_proxyport);

        scm_c_define_gsubr ("ft-get-proxy?", 0, 0, 0, ex_get_proxy);
        scm_c_define_gsubr ("ft-set-proxy!", 1, 0, 0, ex_set_proxy);

        scm_c_define_gsubr ("ft-get-prompt", 0, 0, 0, ex_get_prompt);
        scm_c_define_gsubr ("ft-set-prompt!", 1, 0, 0, ex_set_prompt);

        scm_c_define_gsubr ("ft-display", 1, 0, 0, ex_display);

        scm_c_define_gsubr ("ft-get-conn-status", 0, 0, 0, ex_get_conn_status);

        scm_c_define_gsubr ("ft-add-buddy!", 1, 0, 0, ex_add_buddy);
        scm_c_define_gsubr ("ft-remove-buddy!", 1, 0, 0, ex_remove_buddy);
        scm_c_define_gsubr ("ft-get-roster-list", 0, 0, 0, ex_get_roster_list);
        scm_c_define_gsubr ("ft-get-roster-status-msg", 1, 0, 0,
                            ex_get_roster_status_msg);
        scm_c_define_gsubr ("ft-get-roster-is-online", 1, 0, 0,
                            ex_get_roster_is_online);
        scm_c_define_gsubr ("ft-roster-lookup", 1, 0, 0, ex_roster_lookup);
        scm_c_define_gsubr ("ft-roster-set-nickname", 2, 0, 0,
                            ex_roster_set_nickname);
        scm_c_define_gsubr ("ft-subscription-allow", 1, 0, 0,
                            ex_subscription_allow);
        scm_c_define_gsubr ("ft-subscription-deny", 1, 0, 0,
                            ex_subscription_deny);

        scm_c_define_gsubr ("ft-quit", 1, 0, 0, ex_quit);

        /* Return immediately after the hook procedure returns */
        scm_c_define_gsubr ("ft-hook-return", 0, 0, 0, ex_hook_return);
        /* syntax: (ft-hook-return)
         */

        scm_c_define_gsubr ("ft-get-current-buddy", 0, 0, 0,
                            ex_get_current_buddy);
        scm_c_define_gsubr ("ft-set-current-buddy!", 1, 0, 0,
                            ex_set_current_buddy);

        scm_c_define_gsubr ("ft-get-status-msg", 0, 0, 0, ex_get_status_msg);
        scm_c_define_gsubr ("ft-set-status-msg!", 1, 0, 0, ex_set_status_msg);

        scm_c_define_gsubr ("ft-reset-fs-state!", 0, 0, 0, ex_reset_fs_state);

        scm_c_define_gsubr ("ft-dict-append!", 1, 0, 0, ex_dict_append);
        scm_c_define_gsubr ("ft-dict-prepend!", 1, 0, 0, ex_dict_prepend);
        scm_c_define_gsubr ("ft-dict-insert!", 1, 0, 0, ex_dict_prepend);
        scm_c_define_gsubr ("ft-dict-remove!", 1, 0, 0, ex_dict_remove);

        scm_c_define_gsubr ("ft-version", 0, 0, 0, ex_version);

        scm_c_define_gsubr ("ft-cli-on", 0, 0, 0, ex_cli_on);

        scm_c_define_gsubr ("ft-main-loop", 0, 0, 0, ex_main_loop);

        scm_c_define_gsubr ("ft-load-default-config", 0, 0, 0,
                            ex_load_default_config);

        scm_c_define_gsubr ("ft-get-config-dir", 0, 0, 0, ex_get_config_dir);

        scm_c_define_gsubr ("ft-beep", 2, 0, 0, ex_beep);

        scm_c_define_gsubr ("ft-give-repl", 0, 0, 0, ex_give_repl);

        scm_c_define_gsubr ("ft-change-password", 1, 0, 0, ex_change_password);
        scm_c_define_gsubr ("ft-bind-to-ctrl-key", 2, 0, 0,
                            ex_bind_to_ctrl_key);

        scm_c_define_gsubr ("ft-run-command", 1, 0, 0, ex_run_command);
        scm_c_define_gsubr ("ft-rl-redisplay", 0, 0, 0, ex_rl_redisplay);
        scm_c_define_gsubr ("ft-pager-display", 1, 0, 0, ex_pager_display);
}

void
extensions_init (void)
{
        register_hooks ();
        register_primitives ();
}

void
load_default_config (void)
{
        int len;
        char *file = NULL;
        struct stat foo;

        len = strlen (getenv ("HOME")) + 1 + strlen (FT_CONFIG_SCM) + 1;

        file = g_malloc_n (1, len);

        sprintf (file, "%s/%s", getenv ("HOME"), FT_CONFIG_SCM);

        if (stat (file, &foo) == 0)
                scm_c_primitive_load (file);

        if (file)
                g_free (file);
}
