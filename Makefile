SHELL = /bin/sh
CC      = gcc
INSTALL = install
EXEC_NCURSES = curseofwar
EXEC_SDL = curseofwar-sdl

NAME_GENERIC = curseofwar

PREFIX ?= /usr
MANPREFIX = $(PREFIX)/share/man

BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(MANPREFIX)/man6
DOCDIR = $(DESTDIR)$(PREFIX)/share/doc/$(EXEC)

SRCS_INDEP = grid.c state.c king.c network.c client.c server.c output-common.c main-common.c
SRCS_NCURSES = output.c main.c 
SRCS_SDL = output-sdl.c main-sdl.c

HDRS_INDEP = common.h messaging.h $(SRCS_INDEP:.c=.h)
HDRS_NCURSES = output.h
HDRS_SDL = output-sdl.h

OBJS_INDEP = $(SRCS_INDEP:.c=.o)
OBJS_NCURSES = $(SRCS_NCURSES:.c=.o)
OBJS_SDL = $(SRCS_SDL:.c=.o)

EXECS = $(EXEC_NCURSES) $(EXEC_SDL)
CFLAGS += -Wall -O2
LDLIBS += -lm

# SDL or ncurses
OBJS = $(OBJS_INDEP) 
HDRS = $(HDRS_INDEP) 
ifdef SDL
 OBJS += $(OBJS_SDL)
 HDRS += $(HDRS_SDL)
 CFLAGS += $(shell sdl-config --cflags)
 LDLIBS += $(shell sdl-config --libs)
 EXEC = $(EXEC_SDL)
else
 OBJS += $(OBJS_NCURSES)
 HDRS += $(HDRS_NCURSES)
 LDLIBS += -lncurses
 EXEC = $(EXEC_NCURSES)
endif

VERSION=`cat VERSION`

.PHONY: all clean cleanall


all: $(EXEC)

clean:
	-rm -f $(OBJS_INDEP) $(OBJS_NCURSES) $(OBJS_SDL) $(EXECS)

.c.o: $(CC) $(CPPFLAGS) $(CFLAGS) -c $<

$(EXEC): $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(EXEC)

install: all
	$(INSTALL) -m 755 -D $(EXEC) $(BINDIR)/$(EXEC)
	$(INSTALL) -m 755 -d $(MANDIR)
	-sed "s/%VERSION%/$(VERSION)/g" $(EXEC).6 | gzip -c > $(MANDIR)/$(EXEC).6.gz
	-chmod 644 $(MANDIR)/$(EXEC).6.gz
	$(INSTALL) -m 755 -d $(DOCDIR)
	-cat CHANGELOG | gzip -c > $(DOCDIR)/changelog.gz
	-chmod 644 $(DOCDIR)/changelog.gz

install-strip:
	$(INSTALL) -D -s $(EXEC) $(BINDIR)/$(EXEC)

uninstall:
	-rm $(BINDIR)/$(EXEC)
	-rm -f $(MANDIR)/$(EXEC).6.gz
	-rm $(DOCDIR)/changelog.gz

show-path:
	@echo would install to ${BINDIR}
