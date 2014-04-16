/*
  Copyright (c) 2005-2014 Freetalk Core Team
  This file is part of GNU Freetalk.

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

#ifndef __COMPAT_H__
#define __COMPAT_H__

#ifndef HAVE_ARGP
#include <stdio.h>
#include <errno.h>

#define ARGP_ERR_UNKNOWN E2BIG

struct argp;
struct argp_state;
struct argp_child;

struct argp_option {
        const char *name;
        int key;
        const char *arg;
        int flags;
        const char *doc;
        int group;
};

typedef error_t (*argp_parser_t) (int key, char *arg,
                                  struct argp_state *state);


struct argp_child
{
        /* The child parser.  */
        __const struct argp *argp;

        /* Flags for this child.  */
        int flags;

        /* If non-zero, an optional header to be printed in help output before the
           child options.  As a side-effect, a non-zero value forces the child
           options to be grouped together; to achieve this effect without actually
           printing a header string, use a value of "".  */
        __const char *header;

        /* Where to group the child options relative to the other (`consolidated')
           options in the parent argp; the values are the same as the GROUP field
           in argp_option structs, but all child-groupings follow parent options at
           a particular group level.  If both this field and HEADER are zero, then
           they aren't grouped at all, but rather merged with the parent options
           (merging the child's grouping levels with the parents).  */
        int group;
};

struct argp_state
{
        const struct argp *root_argp;

        int argc;
        char **argv;
        int next;
        unsigned flags;
        unsigned arg_num;
        int quoted;

        void *input;
        void **child_inputs;
        void *hook;
        char *name;

        FILE *err_stream;
        FILE *out_stream;

        void *pstate;
};

struct argp
{
        const struct argp_option *options;
        argp_parser_t parser;
        const char *args_doc;
        const char *doc;
        const struct argp_child *children;
        char *(*help_filter) (int __key, __const char *__text, void *__input);
        const char *argp_domain;
};

#define argp_parse argp_parse_
error_t argp_parse_ (const struct argp * __argp,
                     int __argc, char **  __argv,
                     unsigned __flags, int * __arg_index,
                     void * __input);
#else
#include <argp.h>
#endif /* HAVE_ARGP */


#include <libguile.h>
#if SCM_MAJOR_VERSION == 1 && SCM_MINOR_VERSION == 6

#define scm_is_string(x)          SCM_ROSTRINGP(x)
#define scm_is_integer(x)         scm_is_true(scm_integer_p (x))
#define scm_to_int(x)             scm_num2int(x, SCM_ARGn, "scm_to_int")
#define scm_from_int(x)           scm_int2num(x)
#define scm_is_true(x)            SCM_NFALSEP(x)
#define scm_is_false(x)           SCM_FALSEP(x)
#define scm_from_locale_string(x) scm_makfrom0str(x)
#define scm_to_locale_string(x)   strdup(SCM_STRING_CHARS(x))
#define scm_to_size_t(x)          scm_num2uint(x, SCM_ARGn, "scm_to_size_t")
#define scm_from_ulong(x)         scm_ulong2num (x)
#define scm_from_bool(x)          SCM_BOOL(x)
#define scm_to_bool(x)            (!SCM_FALSEP (x))

#endif

#endif /* __COMPAT_H__ */
