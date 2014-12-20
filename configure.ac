dnl Copyright 2005-2014 Freetalk Core Team
dnl GNU General Public license v3 or later

dnl Process this file with autoconf to produce a configure script

AC_INIT([freetalk], [4.1], [https://github.com/GNUFreetalk/freetalk/issues])
AM_INIT_AUTOMAKE

AC_PREREQ([2.57])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])

if make --help 2>&1 | grep -q no-print-directory; then
   AM_MAKEFLAGS="$AM_MAKEFLAGS --no-print-directory";
fi

if make --help 2>&1 | grep -q quiet; then
   AM_MAKEFLAGS="$AM_MAKEFLAGS --quiet"
fi

if libtool --help 2>&1 | grep -q quiet; then
   AM_LIBTOOLFLAGS="--quiet";
fi

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile
                src/Makefile
                extensions/Makefile
                extensions/init.scm
                extensions/first-time-run.sh
                doc/Makefile
                examples/Makefile])

AC_PROG_MAKE_SET
AC_PROG_CXX

CONTRIBDIR='$(top_srcdir)/contrib'

dnl Check for argp
AC_CHECK_HEADER([argp.h], AC_DEFINE(HAVE_ARGP, 1, [have argp]))
AC_CONFIG_SUBDIRS(contrib/argp-standalone)

BUILD_ARGP_STANDALONE=no
if test "x${ac_cv_header_argp_h}" = "xno"; then
   BUILD_ARGP_STANDALONE=yes
   ARGP_STANDALONE_CFLAGS='-I${top_srcdir}/contrib/argp-standalone'
   ARGP_STANDALONE_LDADD='${top_builddir}/contrib/argp-standalone/libargp.a'
   ARGP_STANDALONE_DIR='${top_builddir}/contrib/argp-standalone'
fi

AC_SUBST(ARGP_STANDALONE_CFLAGS)
AC_SUBST(ARGP_STANDALONE_LDADD)
AC_SUBST(ARGP_STANDALONE_DIR)

AC_SUBST(CONTRIBDIR)

AC_CHECK_LIB([readline -lcurses],[readline],[RLLIBS="-lreadline -lcurses"])
AC_CHECK_LIB([readline -ltermcap],[readline],[RLLIBS="-lreadline -ltermcap"])
AC_CHECK_LIB([readline -lncurses],[readline],[RLLIBS="-lreadline -lncurses"])

if test "x$RLLIBS" == "x"; then
   AC_MSG_ERROR([ERROR! readline not found..])
fi
AC_SUBST(RLLIBS)

AC_SEARCH_LIBS([tgetent],[ncurses termcap curses])
AC_CHECK_LIB([readline], [readline],,AC_MSG_ERROR([ERROR! readline not found...]))
AC_CHECK_PROG([PKGCONFIG], [pkg-config], [yes], [no])
if test "x$PKGCONFIG" = "xno"; then
   AC_MSG_ERROR([pkg-config not found])
fi

PKG_CHECK_MODULES([GUILE], [guile-2.0], [HAVE_GUILE_2="yes"], [HAVE_GUILE_2="no"])
if test "x$HAVE_GUILE_2" == "xno"; then
   AC_MSG_ERROR([ERROR! guile-2.0 not found...])
fi
PKG_CHECK_MODULES([GLIB], [glib-2.0], [HAVE_GLIB_2="yes"], [HAVE_GLIB_2="no"])
if test "x$HAVE_GLIB_2" == "xno"; then
   AC_MSG_ERROR([ERROR! glib-2.0 not found...])
fi
PKG_CHECK_MODULES([LOUDMOUTH], [loudmouth-1.0], [HAVE_LOUDMOUTH="yes"], [HAVE_LOUDMOUTH="no"])
if test "x$HAVE_LOUDMOUTH" == "xno"; then
   AC_MSG_ERROR([ERROR! loudmouth-1.0 not found...])
fi

AC_CHECK_HEADERS([libintl.h])

AC_OUTPUT
