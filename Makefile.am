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

## --------------- ##
## Documentation.  ##
## --------------- ##

info_TEXINFOS = doc/freetalk.texi
doc_freetalk_TEXINFOS = \
  doc/authors.texi \
  doc/gpl.texi \
  doc/fdl.texi \
  doc/permissions.texi

dist_man_MANS = doc/freetalk.1

# Examples.
nobase_dist_doc_DATA = \
  examples/freetalk.scm \
  examples/daemon.ft \
  examples/hello-world.ft \
  examples/mimic.ft
