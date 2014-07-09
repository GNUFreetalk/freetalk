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

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <libguile.h>

#include "freetalk.h"
#include "commands.h"
#include "interpreter.h"
#include "extensions.h"
#include "roster.h"
#include "presence.h"
#include "more.h"
#include "common.h"

SCM ex_load (SCM scm_file)
{
        char *file = scm_to_locale_string (scm_file);

        ft_load (file);
        g_free (file);

        return SCM_UNSPECIFIED;
}

SCM ex_get_jid (void)
{
        return scm_from_locale_string (do_get_jid ());
}

SCM ex_set_jid (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);

        do_set_jid (jid);
        g_free (jid);

        return SCM_UNSPECIFIED;
}

SCM ex_get_server (void)
{
        return scm_from_locale_string (do_get_server ());
}

SCM ex_set_server (SCM scm_server)
{
        char *server = scm_to_locale_string (scm_server);

        do_set_server (server);
        g_free (server);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_proxyserver (void)
{
        return scm_from_locale_string (do_get_proxyserver ());
}

SCM
ex_set_proxyserver (SCM scm_proxyserver)
{
        char *proxyserver = scm_to_locale_string (scm_proxyserver);

        do_set_proxyserver (proxyserver);
        g_free (proxyserver);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_proxyport (void)
{
        return scm_from_int (do_get_proxyport ());
}

SCM
ex_set_proxyport (SCM scm_proxyport)
{
        do_set_proxyport (scm_to_int (scm_proxyport));
        return SCM_UNSPECIFIED;
}

SCM
ex_set_proxyuname (SCM scm_proxyuname)
{
        char *proxyuname = scm_to_locale_string (scm_proxyuname);

        do_set_proxyuname (proxyuname);
        g_free (proxyuname);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_proxyuname (void)
{
        return scm_from_locale_string (do_get_proxyuname ());
}

SCM
ex_set_proxypasswd (SCM scm_proxypasswd)
{
        char *proxypasswd = scm_to_locale_string (scm_proxypasswd);

        do_set_proxypasswd (proxypasswd);
        g_free (proxypasswd);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_proxypasswd (void)
{
        return scm_from_locale_string (do_get_proxypasswd ());
}

SCM
ex_get_password (void)
{
        return scm_from_locale_string (do_get_password ());
}

SCM
ex_set_password (SCM scm_password)
{
        char *password = scm_to_locale_string (scm_password);

        do_set_password (password);
        g_free (password);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_proxy (void)
{
        return scm_from_bool (state.need_proxy);
}

SCM
ex_set_proxy (SCM scm_proxy)
{
        do_set_proxy (scm_to_bool (scm_proxy));

        return SCM_UNSPECIFIED;
}

SCM
ex_get_sslconn (void)
{
        return scm_from_bool (state.need_ssl);
}

SCM
ex_set_sslconn (SCM scm_ssl)
{
        do_set_ssl (scm_to_bool (scm_ssl));

        return SCM_UNSPECIFIED;
}

SCM
ex_get_tlsconn (void)
{
        return scm_from_bool (state.need_tls);
}

SCM
ex_set_tlsconn (SCM scm_tls)
{
        do_set_tls (scm_to_bool (scm_tls));

        return SCM_UNSPECIFIED;
}

SCM
ex_connect (void)
{
        return scm_from_int (do_connect());
}

SCM
ex_connect_blocking (void)
{
        return scm_from_int (do_connect_blocking ());
}

SCM
ex_disconnect (void)
{
        do_disconnect ();

        return SCM_UNSPECIFIED;
}

SCM
ex_send_message (SCM scm_to, SCM scm_msg)
{
        char *to = scm_to_locale_string (scm_to);
        char *msg = scm_to_locale_string (scm_msg);

        do_send_message (to, msg);

        g_free (to);
        g_free (msg);

        return SCM_UNSPECIFIED;
}

SCM
ex_send_message_no_hook (SCM scm_to, SCM scm_msg)
{
        char *to = scm_to_locale_string (scm_to);
        char *msg = scm_to_locale_string (scm_msg);

        do_send_message_no_hook (to, msg);

        g_free (to);
        g_free (msg);

        return SCM_UNSPECIFIED;
}

SCM
ex_set_daemon (void)
{
        do_set_daemon ();
        return SCM_UNSPECIFIED;
}

SCM
ex_get_daemon (void)
{
        return scm_from_int (do_get_daemon ());
}

SCM
ex_set_port (SCM scm_port)
{
        do_set_port (scm_to_int (scm_port));
        return SCM_UNSPECIFIED;
}

SCM
ex_get_port (void)
{
        return scm_from_int (do_get_port ());
}

SCM
ex_get_prompt (void)
{
        return scm_from_locale_string (do_get_prompt ());
}

SCM
ex_set_prompt (SCM scm_prompt)
{
        char *prompt;

        prompt = scm_to_locale_string (scm_prompt);

        do_set_prompt (prompt);
        g_free (prompt);

        return SCM_UNSPECIFIED;
}

SCM
ex_display (SCM scm_line)
{
        char *line = scm_to_locale_string (scm_line);

        PRINTF ("%s",_(line));
        g_free (line);

        return SCM_UNSPECIFIED;
}

SCM
ex_get_conn_status (void)
{
        return scm_from_int (do_get_conn_status ());
}

SCM
ex_add_buddy (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        do_add (jid);
        return SCM_UNSPECIFIED;
}

SCM
ex_remove_buddy (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        if (do_get_conn_status () != FT_AUTH)
        {
                PRINTF ("%s",_("Not connected, (use /connect)"));
                return scm_from_bool (FALSE);
        }
        ft_roster_remove (jid);
        g_free (jid);

        return scm_from_bool (TRUE);
}

SCM
ex_hook_return (void)
{
        set_hook_return (1);
        return SCM_UNSPECIFIED;
}

SCM
ex_get_current_buddy (void)
{
        return scm_from_locale_string (do_get_current_buddy ());
}

SCM
ex_set_current_buddy (SCM scm_bud)
{
        char *buddy = scm_to_locale_string (scm_bud);
        do_set_current_buddy (buddy);
        g_free (buddy);
        return SCM_UNSPECIFIED;
}

SCM
ex_set_status_msg (SCM scm_status)
{
        char *status = scm_to_locale_string (scm_status);
        do_set_status_msg (status);
        g_free (status);
        return SCM_UNSPECIFIED;
}

SCM
ex_get_status_msg (void)
{
        return scm_from_locale_string (do_get_status_msg ());
}


/* Freetalk internal use only */
SCM
roster_item_to_list (FtRosterItem *item)
{
        return scm_list_n (scm_from_locale_string (item->jid),
                           scm_from_bool (item->is_online),
                           scm_from_locale_string (item->nickname ?
                                                   item->nickname : ""),
                           scm_from_locale_string (item->show_msg ?
                                                   item->show_msg : ""),
                           scm_from_locale_string (item->status_msg ?
                                                   item->status_msg : ""),
                           SCM_UNDEFINED);
}

static void
roster_iterator (gpointer r_item, gpointer retval)
{
        FtRosterItem *item = (FtRosterItem *)r_item;
        SCM *ret = (SCM *)retval;
        *ret = scm_cons (roster_item_to_list (item), *ret);
}
/* ___END___*/

SCM
ex_get_roster_list (void)
{
        SCM ret = SCM_EOL;
        ft_roster_foreach (roster_iterator, (gpointer )&ret);
        return ret;
}

SCM
ex_get_roster_status_msg (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        return scm_from_locale_string (ft_roster_lookup (jid)->status_msg);
}

SCM
ex_get_roster_is_online (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        return ft_roster_lookup (jid)->is_online ? SCM_BOOL_T : SCM_BOOL_F;
}

SCM
ex_roster_lookup (SCM scm_jid)
{
        char *jid = NULL;
        FtRosterItem *item = NULL;

        jid = scm_to_locale_string (scm_jid);
        item = ft_roster_lookup (jid);

        if (jid)
                g_free (jid);

        return item ? roster_item_to_list (item) : SCM_EOL;
}

SCM
ex_roster_set_nickname (SCM scm_jid, SCM scm_nickname)
{
        char *jid = scm_to_locale_string (scm_jid);
        char *nickname = scm_to_locale_string (scm_nickname);

        ft_roster_set_nickname (jid, nickname);

        return SCM_UNSPECIFIED;
}

SCM
ex_reset_fs_state (void)
{
        return scm_from_int (do_reset_fs_state ());
}

SCM
ex_dict_append (SCM scm_word)
{
        char *word = scm_to_locale_string (scm_word);

        do_dict_append (word);
        g_free (word);

        return SCM_UNSPECIFIED;
}

SCM
ex_dict_prepend (SCM scm_word)
{
        char *word = scm_to_locale_string (scm_word);

        do_dict_prepend (word);
        return SCM_UNSPECIFIED;
}

SCM
ex_dict_insert (SCM scm_word)
{
        char *word = scm_to_locale_string (scm_word);

        do_dict_insert (word);
        return SCM_UNSPECIFIED;
}

SCM
ex_dict_remove (SCM scm_word)
{
        char *word = scm_to_locale_string (scm_word);

        do_dict_remove (word);
        g_free (word);

        return SCM_UNSPECIFIED;
}

SCM
ex_version (void)
{
        return scm_from_locale_string (PACKAGE_VERSION);
}


SCM
ex_cli_on (void)
{
        if (state.daemon)
                return scm_from_bool (FALSE);

        interface_init ();
        return scm_from_bool (TRUE);
}

SCM
ex_main_loop (void)
{
        do_main_loop ();

        return SCM_UNSPECIFIED;
}

SCM
ex_load_default_config (void)
{
        load_default_config ();

        return SCM_UNSPECIFIED;
}

SCM
ex_get_config_dir (void)
{
        return scm_from_locale_string (state.config_dir);
}

SCM
ex_subscription_allow (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        ft_presence_subscription_allow (jid);
        g_free (jid);
        return SCM_UNSPECIFIED;
}

SCM
ex_subscription_deny (SCM scm_jid)
{
        char *jid = scm_to_locale_string (scm_jid);
        ft_presence_subscription_deny (jid);
        g_free (jid);
        return SCM_UNSPECIFIED;
}

SCM
ex_beep (SCM freq, SCM msec)
{
        printf ("\a");
        fflush (stdout);
        return SCM_UNSPECIFIED;
}

/*
  For debugging purposes
*/
SCM
ex_give_repl (void)
{
        scm_shell (0, NULL);
        return SCM_UNSPECIFIED;
}

SCM
ex_quit (SCM scm_exit_code)
{
        do_quit (scm_to_int (scm_exit_code));
        return SCM_UNSPECIFIED;
}

SCM
ex_change_password (SCM newpass)
{
        char *npass = scm_to_locale_string (newpass);
        do_change_password (npass);
        return SCM_UNSPECIFIED;
}

SCM
ex_bind_to_ctrl_key (SCM key, SCM command)
{
        ft_bind_to_ctrl_key (SCM_CHAR (key), scm_to_locale_string (command));
        return SCM_UNSPECIFIED;
}

SCM
ex_run_command(SCM command)
{
        interpreter (scm_to_locale_string (command));
        return SCM_UNSPECIFIED;
}

SCM
ex_rl_redisplay()
{
        rl_redisplay();
        return SCM_UNSPECIFIED;
}

SCM
ex_pager_display(SCM msgs)
{
        char *buffer = scm_to_locale_string(msgs);
        printf("\r\n");
        more(buffer);
        g_free(buffer);
        rl_forced_update_display ();

        return SCM_UNSPECIFIED;
}
