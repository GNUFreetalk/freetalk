/*
 * Mini more implementation for busybox
 *
 * Copyright (C) 1998 by Erik Andersen <andersee@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Dec 2014 by Anis Elleuch
 *      Properly behave with ANSI colored texts and small term windows
 */


#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>

#include "more.h"

void
gotsig(int sig)
{
        tcsetattr(fileno(cin), TCSANOW, &initial_settings);
}

void
more(char* buffer)
{
        int c, lines = 0, input = 0;
        int next_page = 0, escape_seq = 0;
        int rows = 24, cols = 79, col_pos = 0;
        long int buf_len;
        struct winsize win;
        char *current_pos = NULL;
        struct termios new_settings;

        if (!buffer)
                goto end;

        buf_len = strlen(buffer);
        current_pos = buffer;
        c = *current_pos;

        cin = fopen("/dev/tty", "r");
        tcgetattr(fileno(cin), &initial_settings);
        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        tcsetattr(fileno(cin), TCSANOW, &new_settings);

        (void) signal(SIGINT, gotsig);

        ioctl(0, TIOCGWINSZ, &win);

        if (win.ws_row > 2)
                rows = win.ws_row - 2;
        if (win.ws_col > 0)
                cols = win.ws_col;

        while (c  != '\0') {
                if (next_page) {
                        int len = 0;
                        next_page = 0;
                        lines = 0;
                        len = fprintf(stdout,
                            "--More-- (%d%% of %ld bytes)%s",
                            (int) (100 * ((double) (current_pos - buffer) / (double) buf_len)),
                            buf_len,
                            "");
                        fflush(stdout);
                        input = getc(stdin);
                        while(len-- > 0)
                            putc('\b', stdout);
                        while(len++ < cols)
                            putc(' ', stdout);
                        while(len-- > 0)
                            putc('\b', stdout);
                        fflush(stdout);
                }

            if (input == 'q')
                    goto end;
            if (input == ' ' && c == '\n')
                    next_page = 1;
            if (c == '\n') {
                col_pos = 0;
                if (++lines == (rows + 1))
                    next_page = 1;
            }

            if (c == '\x1b')
                escape_seq = 1;

            if (!escape_seq)
                col_pos++;

            if (escape_seq && c == 'm')
                    escape_seq = 0;

            putc(c, stdout);

            if ( c != '\n' && col_pos == cols ) {
                col_pos = 0;
                if (++lines == (rows + 1))
                    next_page = 1;
            }

            current_pos++;
            c = *current_pos;
        }
        fflush(stdout);

end:
        gotsig(0);
        return;
}
