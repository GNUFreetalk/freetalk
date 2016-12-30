## Process this file with automake to produce Makefile.in
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

bin_PROGRAMS = freetalk

freetalk_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\"
freetalk_CFLAGS = $(GLIB_CFLAGS) $(GUILE_CFLAGS) $(LOUDMOUTH_CFLAGS)
freetalk_LDADD = $(GLIB_LIBS) $(GUILE_LIBS) $(LOUDMOUTH_LIBS) $(RLLIBS)
freetalk_SOURCES = \
  src/callbacks.c \
  src/callbacks.h \
  src/commands.c \
  src/commands.h \
  src/common.h \
  src/extensions.c \
  src/extensions.h \
  src/freetalk.c \
  src/freetalk.h \
  src/interpreter.c \
  src/interpreter.h \
  src/more.c \
  src/more.h \
  src/presence.c \
  src/presence.h \
  src/primitives.c \
  src/primitives.h \
  src/roster.c \
  src/roster.h \
  src/util.c \
  src/util.h
# src/register.c
# src/register.h

# Guile Extensions.
nobase_dist_pkgdata_DATA = \
  extensions/loudscream.scm \
  extensions/login.scm \
  extensions/dyn-commands.scm \
  extensions/beep.scm \
  extensions/color.scm \
  extensions/dict-buddy.scm \
  extensions/shell.scm \
  extensions/pipe.scm \
  extensions/state.scm \
  extensions/connection.scm \
  extensions/roster.scm \
  extensions/mr-oxford.scm \
  extensions/utils.scm \
  extensions/history.scm \
  extensions/url.scm \
  extensions/proud-of-freetalk.scm \
  extensions/broadcast.scm \
  extensions/hacker-romance.scm \
  extensions/smart-prompt.scm \
  extensions/first-time-run.sh

nobase_nodist_pkgdata_DATA = extensions/init.scm

CLEANFILES = $(nobase_nodist_pkgdata_DATA)

EXTRA_DIST = \
  bootstrap \
  src/register.c \
  src/register.h

# '.tarball-version' is used by 'git-version-gen' when not building from a git
# checkout.
dist-hook: gen-ChangeLog
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version

gen_start_date = 2016-11-15
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -d $(srcdir)/.git; then			\
	  log_fix="$(srcdir)/build-aux/git-log-fix";			\
	  test -e "$$log_fix"						\
	    && amend_git_log="--amend=$$log_fix"			\
	    || amend_git_log=;						\
	  $(srcdir)/build-aux/gitlog-to-changelog			\
	    $$amend_git_log --since=$(gen_start_date) > $(distdir)/cl-t && \
	    { rm -f $(distdir)/ChangeLog &&				\
	      mv $(distdir)/cl-t $(distdir)/ChangeLog; }		\
	fi

## --------------- ##
## Documentation.  ##
## --------------- ##

info_TEXINFOS = doc/freetalk.texi
doc_freetalk_TEXINFOS = \
  doc/authors.texi \
  doc/gpl-3.0.texi \
  doc/fdl-1.3.texi \
  doc/permissions.texi

dist_man_MANS = doc/freetalk.1

# Examples.
nobase_dist_doc_DATA = \
  examples/freetalk.scm \
  examples/daemon.ft \
  examples/hello-world.ft \
  examples/mimic.ft
