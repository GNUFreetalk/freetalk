SUBDIRS = $(ARGP_STANDALONE_DIR) src extensions doc examples

PACKAGE = @PACKAGE@
VERSION = @VERSION@

EXTRA_DIST = autogen.sh contrib/argp-standalone

CLEANFILES = *~

gitclean: distclean
	find . -name Makefile.in -exec rm -f {} \;
	find . -name Makefile -exec rm -f {} \;
	rm -fr autom4te.cache m4
	rm -f missing aclocal.m4 config.h.in config.guess
	rm -f config.sub ltmain.sh install-sh configure depcomp
	rm -f config.rpath INSTALL
	-rm -fr $(CONTRIBDIR)/argp-standalone/autom4te.cache
	-rm -f $(CONTRIBDIR)/argp-standalone/aclocal.m4
	-rm -f $(CONTRIBDIR)/argp-standalone/config.h.in
	-rm -f $(CONTRIBDIR)/argp-standalone/configure
	-rm -f $(CONTRIBDIR)/argp-standalone/depcomp
	-rm -f $(CONTRIBDIR)/argp-standalone/install-sh
	-rm -f $(CONTRIBDIR)/argp-standalone/missing
