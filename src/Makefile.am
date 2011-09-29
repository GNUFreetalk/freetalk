AM_CPPFLAGS= -Wall -g $(GUILE_CFLAGS) $(GLIB_CFLAGS) $(LOUDMOUTH_CFLAGS) -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\"

bin_PROGRAMS=freetalk

freetalk_SOURCES=freetalk.cc util.cc commands.cc callbacks.cc roster.cc \
        interpreter.cc extensions.cc primitives.cc presence.cc \
	file_transfer.cc register.cc compat.cc\
	freetalk.h util.h commands.h callbacks.h \
	roster.h interpreter.h extensions.h primitives.h presence.h \
	file_transfer.h register.h compat.h

freetalk_LDFLAGS=$(LOUDMOUTH_LIBS) $(GLIB_LIBS) $(GUILE_LDFLAGS)

CLEANFILES = *~
