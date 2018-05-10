dnl Process this file with autoconf to produce a configure script.
# Copyright (C) 2005-2014 Freetalk Core Team.
# Copyright (C) 2016 Mathieu Lirzin <mthl@gnu.org>
#
# This file is part of GNU Freetalk.
#
# GNU Freetalk is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GNU Freetalk is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Freetalk.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([GNU Freetalk],
        m4_esyscmd([build-aux/git-version-gen .tarball-version]),
        [freetalk-dev@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AC_REQUIRE_AUX_FILE([gitlog-to-changelog])
AC_REQUIRE_AUX_FILE([git-version-gen])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([subdir-objects])

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

AC_PROG_MAKE_SET
AC_PROG_CXX

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

PKG_CHECK_MODULES([GUILE], [guile-2.2], [HAVE_GUILE_2="yes"], [HAVE_GUILE_2="no"])
if test "x$HAVE_GUILE_2" == "xno"; then
   AC_MSG_ERROR([ERROR! guile-2.2 not found please install guile-2.2-dev...])
fi
PKG_CHECK_MODULES([GLIB], [glib-2.0], [HAVE_GLIB_2="yes"], [HAVE_GLIB_2="no"])
if test "x$HAVE_GLIB_2" == "xno"; then
   AC_MSG_ERROR([ERROR! glib-2.0 not found please install libglib2.0-dev...])
fi
PKG_CHECK_MODULES([LOUDMOUTH], [loudmouth-1.0], [HAVE_LOUDMOUTH="yes"], [HAVE_LOUDMOUTH="no"])
if test "x$HAVE_LOUDMOUTH" == "xno"; then
   AC_MSG_ERROR([ERROR! loudmouth-1.0 not found please install libloudmouth1-dev...])
fi

AC_CHECK_HEADERS([libintl.h])

AC_CONFIG_FILES([Makefile
                 extensions/init.scm])

AC_OUTPUT
