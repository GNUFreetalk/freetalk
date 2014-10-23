/*
  Copyright (c) 2005-2014 Freetalk core team
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

  NOTE -
  This is the readline interpreter
  This file is the interface between readline and commands.c (do_*)
*/

#ifndef _CONFIG_H
#define _CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <libguile.h>

#include "interpreter.h"
#include "extensions.h"
#include "freetalk.h"
#include "commands.h"
#include "roster.h"
#include "common.h"

GSList *dict_words = NULL;

int is_buddy (char *input, char** jid) {
        GSList *list = ft_roster_get ();
        int size = g_slist_length (list);
        int i = 0;
        while (i < size) {
                FtRosterItem *item = (FtRosterItem *)g_slist_nth_data (list, i);
                if (!strcasecmp (input, item->jid) || (item->nickname && !strcasecmp(input, item->nickname))) {
                        *jid = item->jid;
                        return 1;
                }
                i++;
        }
        return 0;
}

int
interpreter (char *line)
{
        char *head, *tail, *line_cpy, *jid;
        int ret = 1;

        /* strtok, you are is evil, work on backup instead */
        line_cpy = g_strdup(line);

        head = strtok (line_cpy, line_cpy[0] == '/' ? " " : ":" );

        if (!head)
                /* absurd! */
                goto out;

        tail = strtok (NULL, "\0");

        if ( head[0] == '/' ) {
                /* Avati - dyn-commands.scm should not interpret
                   its args as messages. hence created new
                   hook 'ex_commands_hook' for dyn-commands
                   and moved this below check to do_send_message ()
                */

                set_hook_return (0);
                state.async_printf = 0;

                scm_run_hook (ex_command_hook,
                        scm_list_n (scm_from_locale_string (head),
                            scm_from_locale_string (tail ? tail : ""),
                            SCM_UNDEFINED));

                if (get_hook_return () == 1) {
                    state.async_printf = 1;
                    ret = 0;
                }
        } else { // head[0] != '/'
                if (is_buddy (head, &jid)) {
                        if ( tail && tail[0] == ' ' )
                                ++tail;
                        do_send_message (jid, tail);
                        ret = 0;
                } else if (state.current_buddy) {
                        do_send_message(state.current_buddy->jid, line);
                        ret = 0;
                }
        }

out:
        g_free(line_cpy);
        return ret;
}

void
interpreter_init (void)
{
        /* build regex for autocompletion etc */
}

static char *
auto_complete (const char *text, int _state)
{
        static char need_nickname_completion = 0;
        static char need_roster_completion = 0;
        static char need_roster_domain_completion_hack = 0;
        static char *possible_jid;
        static char need_command_completion = 0;
        static char need_dict_completion = 0;
        static char need_file_completion = 0;
        static char need_separator = 0;
        static char need_command_slash = 0;

        static SCM ft_commands;
        static unsigned long cmd_len, cmd_idx;
        int len = strlen (text);

        static unsigned int roster_idx;
        static unsigned int nickname_idx;
        static GSList *word;

        const char *command_completion_regex [] = {
                "^ */[^ ]*$",
                "^ */help +[^ ]*$",
                NULL
        };

        const char *nickname_completion_regex [] = {
                "^[^/] *[^:]*$", NULL };

        const char *roster_completion_regex [] = {
                "^ *[^ ]*$",
                "^ */pipe +[^ ]*$",
                "^ */history +[^ ]*$",
                "^ */allow +[^ ]*$",
                "^ */remove +[^ ]*$",
                "^ */urlview +[^ ]*$",
                "^ */alias +[^ ]*$",
                "^ */greet +[^ ]*$",
                "^ */deny +[^ ]*$",
                "^ */burst +[^ ]*$",
                "^ */burst-of-romance +[^ ]*$",
                NULL
        };

        const char *file_completion_regex [] = {
                "^ */load +[^ ]*$",
                NULL
        };

        if (!_state) {
                char save;
                regex_t preg;
                char *regex_str;
                int i;

                need_roster_completion = 0;
                need_nickname_completion = 0;
                need_roster_domain_completion_hack = 0;
                need_command_completion = 0;
                need_dict_completion = 0;
                need_file_completion = 0;
                need_separator = 0;
                need_command_slash = 0;

                /* for making the autocompletion context sensitive*/
                save = rl_line_buffer [rl_point];
                rl_line_buffer[rl_point] = '\0';

                i = 0;
                while ((regex_str = (char *)command_completion_regex[i++]) != NULL) {
                        regcomp(&preg, regex_str, REG_EXTENDED|REG_ICASE);
                        if (!regexec (&preg, rl_line_buffer, 0, NULL, 0))
                                need_command_completion = 1;
                        regfree (&preg);
                        if (need_command_completion) {
                                if (i - 1 == 0)
                                        need_command_slash = 1;
                                ft_commands = scm_variable_ref (scm_c_lookup ("dynamic-command-registry"));
                                cmd_len = scm_to_size_t (scm_length (ft_commands));
                                cmd_idx = 0;
                                break;
                        }
                }

                i = 0;
                while ((regex_str = (char *)nickname_completion_regex[i++]) != NULL) {
                        regcomp(&preg, regex_str, REG_EXTENDED|REG_ICASE);
                        if (!regexec (&preg, rl_line_buffer, 0, NULL, 0))
                                need_nickname_completion = 1;
                        regfree (&preg);
                        if (need_nickname_completion) {
                                if (i - 1 == 0)
                                        need_separator = 1;
                                nickname_idx = 0;
                                break;
                        }
                }

                i = 0;
                while ((regex_str = (char *)roster_completion_regex[i++]) != NULL) {
                        regcomp(&preg, regex_str, REG_EXTENDED|REG_ICASE);
                        if (!regexec (&preg, rl_line_buffer, 0, NULL, 0))
                                need_roster_completion = 1;
                        regfree (&preg);
                        if (need_roster_completion) {
                                roster_idx = 0;
                                if (i - 1 == 0) /* 1st roster regexp is selected */
                                    need_separator = 1;
                                if (!ft_roster_get ())
                                        need_roster_completion = 0;
                                /* check if we are autocompleting the 'domain' part */
                                possible_jid = &rl_line_buffer[rl_point];
                                while (possible_jid > (char *)rl_line_buffer) {
                                        possible_jid--;
                                        if (*possible_jid == ' ')
                                                break;
                                        if (*possible_jid == '@') {
                                                need_roster_domain_completion_hack = 1;
                                                need_roster_completion = 0;
                                                while (possible_jid > (char *)rl_line_buffer) {
                                                        possible_jid--;
                                                        if (*possible_jid == ' ') {
                                                                possible_jid++;
                                                                break;
                                                        }
                                                }
                                                break;
                                        }
                                }
                                break;
                        }
                }

                i = 0;
                while ((regex_str = (char *) file_completion_regex[i++]) != NULL) {
                        regcomp(&preg, regex_str, REG_EXTENDED|REG_ICASE);
                        if (!regexec (&preg, rl_line_buffer, 0, NULL, 0))
                                need_file_completion = 1;
                        regfree (&preg);
                        if (need_file_completion)
                                break;
                }

                if (! need_command_completion &&
                    ! need_nickname_completion &&
                    ! need_roster_completion &&
                    ! need_roster_domain_completion_hack &&
                    ! need_file_completion) {
                        need_dict_completion = 1;
                        word = dict_words;
                        while (word && strncasecmp ((const char *)word->data,
                                                    text, len))
                                word = g_slist_next (word);
                }

                /* the 'context sensitive' completion is made transparent */
                rl_line_buffer[rl_point] = save;
        }

        if (need_command_completion) {
                while (cmd_idx < cmd_len) {
                        char *cmd =
                                scm_to_locale_string (scm_list_ref
                                                      (scm_list_ref
                                                       (ft_commands,
                                                        scm_from_ulong
                                                        (cmd_idx++)),
                                                       scm_from_ulong (0)));

                        if (cmd) {
                            char * slash_cmd = g_strdup_printf("%s%s",
                                    need_command_slash ? "/" : "", cmd);
                            g_free(cmd);

                            if (slash_cmd && !strncasecmp (slash_cmd, text, len))
                                return slash_cmd;
                            else
                                g_free (slash_cmd);
                        }
                }
        }

        if (need_file_completion) {
                char *f;
                f = rl_filename_completion_function (text, _state);
                if (f)
                        return f;
                else
                        need_file_completion = 0;
        }

        if (need_nickname_completion) {
                while (nickname_idx < g_slist_length (ft_roster_get ())) {
                        FtRosterItem *roster =
                                (FtRosterItem *) g_slist_nth_data (ft_roster_get (),
                                                                   nickname_idx++);
                        if (roster && roster->nickname && !strncasecmp(roster->nickname, text, len))
                                return g_strdup_printf ("%s%s", roster->nickname,
                                    need_separator ? ":" : "");
                }
        }


        if (need_roster_completion) {
                while (roster_idx < g_slist_length (ft_roster_get ())) {
                        FtRosterItem *roster =
                                (FtRosterItem *) g_slist_nth_data (ft_roster_get (),
                                                                   roster_idx++);
                        if (roster && !strncasecmp (roster->jid, text, len))
                                return g_strdup_printf ("%s%s", roster->jid,
                                    need_separator ? ":" : "");
                }
        }

        if (need_roster_domain_completion_hack) {
                while (roster_idx < g_slist_length (ft_roster_get ())) {
                        FtRosterItem *roster = (FtRosterItem *)g_slist_nth_data (ft_roster_get (),
                                                                                 roster_idx++);
                        if (roster && !strncasecmp (roster->jid, possible_jid,
                                                    strlen (possible_jid)))
                                return g_strdup_printf ("%s%s", strchr (roster->jid, '@') + 1,
                                    need_separator ? ":" : "");
                }
        }
        if (need_dict_completion) {
                while (word && !strncasecmp ((const char *)word->data, text, len)) {
                        char *retword = (char *)word->data;
                        word = word->next;
                        return g_strdup (retword);
                }
        }
        return NULL;
}

char **ft_auto_complete (const char *text, int start, int end)
{
        char **matches  = (char **)NULL;

        matches = rl_completion_matches (text, auto_complete);

        return matches;
}
