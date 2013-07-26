SHELL = /bin/sh
CC      = gcc
INSTALL = install
EXEC   = curseofwar
EXEC_SDL = curseofwar-sdl

PREFIX ?= /usr
MANPREFIX = $(PREFIX)/share/man

BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(MANPREFIX)/man6
DOCDIR = $(DESTDIR)$(PREFIX)/share/doc/$(EXEC)

SRCS_INDEP = grid.c state.c king.c network.c client.c server.c output-common.c main-common.c
SRCS_NCURSES = output.c main.c 
SRCS_SDL = output-sdl.c main-sdl.c

HDRS_INDEP = common.h messaging.h $(patsubst %.c,%.h,$(SRCS_INDEP))
HDRS_NCURSES = output.h
HDRS_SDL = output-sdl.h

OBJS_INDEP = $(patsubst %.c,%.o,$(SRCS_INDEP))
OBJS_NCURSES = $(patsubst %.c,%.o,$(SRCS_NCURSES))
OBJS_SDL = $(patsubst %.c,%.o,$(SRCS_SDL))

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXECS = $(EXEC) $(EXEC_SDL)
CFLAGS += -Wall -O2
LDLIBS += -lncurses -lm 

CFLAGS_SDL = $(shell sdl-config --cflags)
LDLIBS_SDL = $(shell sdl-config --libs)

VERSION=`cat VERSION`

.PHONY: all clean cleanall

all: $(EXEC)

sdl: $(EXEC_SDL)

clean:
	-rm -f $(OBJS_INDEP) $(OBJS_NCURSES) $(OBJS_SDL) $(EXECS)

$(OBJS_INDEP): $(patsubst %.o,%.c,$@) $(HDRS_INDEP)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(OBJS_NCURSES): $(patsubst %.o,%.c,$@) $(HDRS_INDEP) $(HDRS_NCURSES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(OBJS_SDL): $(patsubst %.o,%.c,$@) $(HDRS_INDEP) $(HDRS_SDL)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_SDL) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS_INDEP) $(OBJS_NCURSES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS_INDEP) $(OBJS_NCURSES) $(LDLIBS) -o $(EXEC)

$(EXEC_SDL): $(OBJS_INDEP) $(OBJS_SDL)
	$(CC) $(CFLAGS) $(CFLAGS_SDL) $(LDFLAGS) $(OBJS_INDEP) $(OBJS_SDL) $(LDLIBS) $(LDLIBS_SDL) -o $(EXEC_SDL)

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
