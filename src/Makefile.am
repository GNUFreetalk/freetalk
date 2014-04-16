AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS = freetalk

freetalk_SOURCES=freetalk.c util.c commands.c callbacks.c roster.c \
	interpreter.c extensions.c primitives.c presence.c \
	compat.c # file_transfer.c register.c
noinst_HEADERS = freetalk.h util.h commands.h callbacks.h \
	roster.h interpreter.h extensions.h primitives.h presence.h \
	file_transfer.h register.h compat.h

LIBS = $(LOUDMOUTH_LIBS) $(GLIB_LIBS) $(GUILE_LIBS) $(RLLIBS)
AM_CFLAGS = -Wall -g $(GUILE_CFLAGS) $(GLIB_CFLAGS) $(LOUDMOUTH_CFLAGS)

CLEANFILES = *~
