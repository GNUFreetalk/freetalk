SUBDIRS = src

PACKAGE = @PACKAGE@
VERSION = @VERSION@

EXTRA_DIST = bootstrap

CLEANFILES = *~

gitclean: distclean
	find . -name Makefile.in -exec rm -f {} \;
	find . -name Makefile -exec rm -f {} \;
	rm -fr autom4te.cache m4
	rm -f missing aclocal.m4 config.h.in config.guess
	rm -f config.sub ltmain.sh install-sh configure depcomp
	rm -f config.rpath INSTALL

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
CLEANFILES += $(nobase_nodist_pkgdata_DATA)

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
