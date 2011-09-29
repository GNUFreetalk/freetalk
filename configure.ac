dnl Copyright 200, 2006, 2007 Freetalk Core Team
dnl GNU General Publicense v3 or later
    
dnl Process this file with autoconf to produce a configure script

AC_INIT([freetalk], [3.2], [freetalk-dev@gnu.org])
AM_INIT_AUTOMAKE

AC_PREREQ([2.57])
AC_CONFIG_SRCDIR([src/freetalk.cc])

AC_CONFIG_FILES([Makefile src/Makefile extensions/Makefile extensions/init.scm extensions/first-time-run.sh doc/Makefile po/Makefile.in examples/Makefile])

AC_PROG_MAKE_SET
AC_PROG_CXX

AC_SEARCH_LIBS([tgetent],[ncurses termcap curses])
AC_CHECK_LIB([readline], [readline],,AC_MSG_ERROR([ERROR! readline not found...]))
GUILE_FLAGS
AC_CHECK_LIB([guile],[scm_boot_guile],,AC_MSG_ERROR([ERROR! Guile 1.6 or above required.]))

AC_CHECK_PROG([PKGCONFIG], [pkg-config], [yes], [no])
if test "x$PKGCONFIG" = "xno"; then
   AC_MSG_ERROR([pkg-config not found])
fi

dnl AC_CHECK_LIB([glib-2.0],[g_slist_free],,AC_MSG_ERROR([ERROR! glib-2.0 not found...]))
GLIB_CFLAGS=`pkg-config --cflags glib-2.0`
GLIB_LIBS=`pkg-config --libs glib-2.0`
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl AC_CHECK_LIB([loudmouth1],[lm_session_is_open],,AC_MSG_ERROR([ERROR! loudmouth1 not found...]))
LOUDMOUTH_CFLAGS=`pkg-config --cflags loudmouth-1.0`
LOUDMOUTH_LIBS=`pkg-config --libs loudmouth-1.0`
AC_SUBST(LOUDMOUTH_CFLAGS)
AC_SUBST(LOUDMOUTH_LIBS)


AC_CHECK_HEADER([argp.h],AC_DEFINE(HAVE_ARGP,[1]))

AC_CHECK_HEADERS([errno.h])
AC_CHECK_TYPES([error_t],
	[],
	[AC_DEFINE([error_t], [int])],
	[#ifdef HAVE_ERRNO_H
#define __need_error_t
#include <errno.h>
#endif])

dnl Gettext Support
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])

dnl AM_PO_SUBDIRS
AM_ICONV

AC_OUTPUT
