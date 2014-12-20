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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "freetalk.h"

/*
 * Parse a string containing a JID into its component parts and put them into
 * the jid_t.
 *
 * Ref. Section 3.1, RFC 3920
 */

void
parse_jid_string (char *s, jid_t *jid)
{
        char *str = g_strdup (s);
        char *at = strchr (str, '@');
        char *domain = str;
        char *slash;

        if (at)
        {
                *at = '\0';
                jid->node = g_strdup (str);
                domain = at+1;
        }

        slash = strchr (domain, '/');

        if (slash)
        {
                *slash = '\0';
                jid->domain = g_strdup (domain);
                jid->resource = g_strdup (slash+1);
        }
        else
        {
                jid->domain = g_strdup (domain);
                /* Loudmouth complains if this is NULL */
                jid->resource = g_strdup ("GNU Freetalk");
        }

        g_free (str);
}

char *
first_word (char *full_line)
{
        while (full_line && (*full_line == ' '))
                full_line ++;
        return full_line;
}

char *
second_word (char *full_line)
{
        if (!full_line)
                return NULL;


        while (full_line &&
               ( *full_line == ' ' ||
                 *full_line == '\t' ||
                 *full_line == '\r' ||
                 *full_line == '\n'))
                full_line ++;

        if (!full_line)
                return NULL;

        while (full_line &&
               *full_line != ' ' &&
               *full_line != '\t' &&
               *full_line != '\r' &&
               *full_line != '\n')
                full_line ++;

        if (!full_line)
                return NULL;

        while (full_line &&
               ( *full_line == ' ' ||
                 *full_line == '\t' ||
                 *full_line == '\r' ||
                 *full_line == '\n'))
                full_line ++;

        return *full_line ? full_line : NULL;
}


void
async_printf (const char *fmt, va_list ap)
{
        int tmp_rl_point = rl_point;
        int n = rl_end;
        unsigned int i;

        if (rl_end >= 0 ) {
                rl_kill_text (0, rl_end);
                rl_redisplay ();
        }
        printf ("\r");
        for (i=0 ; i<=strlen (state.prompt) ; i++)
                printf (" ");
        printf ("\r");
        vprintf (fmt, ap);
        printf ("\n");
        fflush(stdout);
        if (n) {
                rl_do_undo ();
                rl_point = tmp_rl_point;
                rl_reset_line_state ();
        }
        rl_forced_update_display ();
}

void
sync_printf (const char *fmt, va_list ap)
{
        vprintf (fmt, ap);
        printf ("\n");
        fflush (stdout);
}

void
check_first_run (void)
{
        if( system ("sh " DATADIR "/" PACKAGE_NAME "/extensions/first-time-run.sh") >> 8 )
                exit (1);
}
