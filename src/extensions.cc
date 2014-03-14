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

#include <stdlib.h>
#include <guile/gh.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "freetalk.h"
#include "roster.h"
#include "primitives.h"
#include "compat.h"

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
        char *path;
        struct stat foo;
        int len;

        /* ladies first */
        if (stat (file, &foo) == 0) {
                gh_load (file);
                return;
        }

        len = strlen (getenv ("HOME")) + 1 + strlen (FT_LOCAL_EXT_DIR) + 1 + strlen (file) + 1;
        path = (char *)calloc (len, 1);
        sprintf (path, "%s/%s/%s", getenv ("HOME"), FT_LOCAL_EXT_DIR, file);
        if (stat (path, &foo) == 0) {
                gh_load (path);
                free (path);
                return;
        }
        free (path);

        len = strlen (FT_GLOBAL_EXT_DIR) + 1 + strlen (file) + 1;
        path = (char *)calloc (len, 1);
        sprintf (path, "%s/%s", FT_GLOBAL_EXT_DIR, file);
        if (stat (path, &foo) == 0) {
                gh_load (path);
                free (path);
                return;
        }
        free (path);

        fprintf (stderr, "%s: not found\n", file);
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
        // C++ is pedantic about the types
        gh_new_procedure1_0 ("ft-load", ex_load);

        gh_new_procedure0_0 ("ft-get-jid", ex_get_jid);
        gh_new_procedure1_0 ("ft-set-jid!", ex_set_jid);

        gh_new_procedure0_0 ("ft-get-server", ex_get_server);
        gh_new_procedure1_0 ("ft-set-server!", ex_set_server);

        gh_new_procedure0_0 ("ft-get-proxyserver", ex_get_proxyserver);
        gh_new_procedure1_0 ("ft-set-proxyserver!", ex_set_proxyserver);

        gh_new_procedure0_0 ("ft-get-proxyuname", ex_get_proxyuname);
        gh_new_procedure1_0 ("ft-set-proxyuname!", ex_set_proxyuname);

        gh_new_procedure0_0 ("ft-get-proxypasswd", ex_get_proxypasswd);
        gh_new_procedure1_0 ("ft-set-proxypasswd!", ex_set_proxypasswd);

        gh_new_procedure0_0 ("ft-get-password", ex_get_password);
        gh_new_procedure1_0 ("ft-set-password!", ex_set_password);

        gh_new_procedure0_0 ("ft-get-sslconn?", ex_get_sslconn);
        gh_new_procedure1_0 ("ft-set-sslconn!", ex_set_sslconn);

        gh_new_procedure0_0 ("ft-connect", ex_connect);
        gh_new_procedure0_0 ("ft-disconnect", ex_disconnect);

        gh_new_procedure2_0 ("ft-send-message", ex_send_message);
        gh_new_procedure2_0 ("ft-send-message-no-hook", ex_send_message_no_hook);

        gh_new_procedure0_0 ("ft-set-daemon", ex_set_daemon);
        gh_new_procedure0_0 ("ft-get-daemon", ex_get_daemon);

        gh_new_procedure0_0 ("ft-get-port", ex_get_port);
        gh_new_procedure1_0 ("ft-set-port!", ex_set_port);

        gh_new_procedure0_0 ("ft-get-proxyport", ex_get_proxyport);
        gh_new_procedure1_0 ("ft-set-proxyport!", ex_set_proxyport);
  
        gh_new_procedure0_0 ("ft-get-proxy?", ex_get_proxy);
        gh_new_procedure1_0 ("ft-set-proxy!", ex_set_proxy);

        gh_new_procedure0_0 ("ft-get-prompt", ex_get_prompt);
        gh_new_procedure1_0 ("ft-set-prompt!", ex_set_prompt);

        gh_new_procedure1_0 ("ft-display", ex_display);

        gh_new_procedure0_0 ("ft-get-conn-status", ex_get_conn_status);

        gh_new_procedure1_0 ("ft-add-buddy!", ex_add_buddy);
        gh_new_procedure1_0 ("ft-remove-buddy!", ex_remove_buddy);
        gh_new_procedure0_0 ("ft-get-roster-list", ex_get_roster_list);
        gh_new_procedure1_0 ("ft-get-roster-status-msg", ex_get_roster_status_msg);
        gh_new_procedure1_0 ("ft-get-roster-is-online", ex_get_roster_is_online);
        gh_new_procedure1_0 ("ft-roster-lookup", ex_roster_lookup);
        gh_new_procedure2_0 ("ft-roster-set-nickname", ex_roster_set_nickname);
        gh_new_procedure1_0 ("ft-subscription-allow", ex_subscription_allow);
        gh_new_procedure1_0 ("ft-subscription-deny", ex_subscription_deny);

        gh_new_procedure1_0 ("ft-quit", ex_quit);
  
        /* Return immediately after the hook procedure returns */
        gh_new_procedure0_0 ("ft-hook-return", ex_hook_return);
        /* syntax: (ft-hook-return)
         */

        gh_new_procedure0_0 ("ft-get-current-buddy", ex_get_current_buddy);
        gh_new_procedure1_0 ("ft-set-current-buddy!", ex_set_current_buddy);

        gh_new_procedure0_0 ("ft-get-status-msg", ex_get_status_msg);
        gh_new_procedure1_0 ("ft-set-status-msg!", ex_set_status_msg);

        gh_new_procedure0_0 ("ft-reset-fs-state!", ex_reset_fs_state);

        gh_new_procedure1_0 ("ft-dict-append!", ex_dict_append);
        gh_new_procedure1_0 ("ft-dict-prepend!", ex_dict_prepend);
        gh_new_procedure1_0 ("ft-dict-insert!", ex_dict_prepend);
        gh_new_procedure1_0 ("ft-dict-remove!", ex_dict_remove);

        gh_new_procedure0_0 ("ft-version", ex_version);

        gh_new_procedure0_0 ("ft-login-blocking", ex_login_blocking);

        gh_new_procedure0_0 ("ft-cli-on", ex_cli_on);

        gh_new_procedure0_0 ("ft-main-loop", ex_main_loop);

        gh_new_procedure0_0 ("ft-load-default-config", ex_load_default_config);

        gh_new_procedure0_0 ("ft-get-config-dir", ex_get_config_dir);

        gh_new_procedure2_0 ("ft-beep", ex_beep);

        gh_new_procedure0_0 ("ft-give-repl", ex_give_repl);

        /* file transfer procedure */
        gh_new_procedure2_0 ("ft-send-file", ex_send_file);
        gh_new_procedure2_0 ("ft-set-allow-file", ex_set_allow_file);

        gh_new_procedure1_0 ("ft-change-password", ex_change_password);
        gh_new_procedure2_0 ("ft-bind-to-ctrl-key", ex_bind_to_ctrl_key);  
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
        char *file;
        struct stat foo;

        len = strlen (getenv ("HOME")) + 1 + strlen (FT_CONFIG_SCM) + 1;
        file = (char *)calloc (len, 1);

        sprintf (file, "%s/%s", getenv ("HOME"), FT_CONFIG_SCM);

        if (stat (file, &foo) == 0) {
                gh_load (file);
        }

        free (file);
}
