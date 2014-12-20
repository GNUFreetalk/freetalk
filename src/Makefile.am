AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = freetalk

freetalk_SOURCES = freetalk.c util.c commands.c callbacks.c roster.c \
	interpreter.c extensions.c primitives.c presence.c more.c \
	# register.c

noinst_HEADERS = freetalk.h util.h commands.h callbacks.h \
	roster.h interpreter.h extensions.h primitives.h presence.h \
	common.h more.h # register.h

freetalk_LDADD = $(ARGP_STANDALONE_LDADD)

LIBS = $(LOUDMOUTH_LIBS) $(GLIB_LIBS) $(GUILE_LIBS) $(RLLIBS) # $(FACEBOOK_LIBS)

AM_CFLAGS = -Wall -g $(GUILE_CFLAGS) $(GLIB_CFLAGS) $(LOUDMOUTH_CFLAGS) \
	 $(ARGP_STANDALONE_CFLAGS)

CLEANFILES = *~
