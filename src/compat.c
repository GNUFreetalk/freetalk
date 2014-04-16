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

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>

#include "compat.h"

error_t
argp_parse_ (const struct argp * __argp,
             int __argc, char **  __argv,
             unsigned __flags, int * __arg_index,
             void * __input)
{
        const struct argp_option * options = __argp->options;
        int num_opts = 0;
        struct option * getopt_long_options;
        char *getopt_short_options;
        int c;
        int short_idx = 0;
        int long_idx = 0;

        while (options->name) {
                num_opts++;
                options++;
        }

        getopt_long_options = (struct option *) calloc (num_opts+1, sizeof (*getopt_long_options));
        getopt_short_options = (char *) calloc (num_opts+1, 2 * sizeof (char));

        options = __argp->options;

        while (options->name) {
                getopt_short_options[short_idx++] = options->key;
                getopt_long_options[long_idx].name = options->name;
                getopt_long_options[long_idx].val = options->key;

                if (options->arg != NULL) {
                        getopt_short_options[short_idx++] = ':';
                        getopt_long_options[long_idx].has_arg = 1;
                }
                options++;
                long_idx++;
        }

        while (1) {
                int option_index = 0;

                c = getopt_long (__argc, __argv, getopt_short_options,
                                 getopt_long_options, &option_index);

                if (c == -1)
                        break;

                __argp->parser (c, optarg, NULL);
        }
        return 0;
}
