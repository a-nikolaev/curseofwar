SHELL = /bin/sh
CC      = gcc
INSTALL = install
EXEC_NCURSES = curseofwar
EXEC_SDL = curseofwar-sdl

GAME_TITLE = $(EXEC_NCURSES)

PREFIX ?= /usr
MANPREFIX = $(PREFIX)/share/man

BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(MANPREFIX)/man6
DOCDIR = $(DESTDIR)$(PREFIX)/share/doc/$(GAME_TITLE)

# Game resources directory
DATADIR ?= $(DESTDIR)$(PREFIX)/share/$(GAME_TITLE)
IMAGESDIR = images

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

# Define a preprocessor variable DATADIR if DESTDIR is provided
ifdef DESTDIR
CFLAGS += -DDATADIR=\"$(DATADIR)\"
endif

# Common sources and header
OBJS = $(OBJS_INDEP) 
HDRS = $(HDRS_INDEP) 
# SDL or ncurses
ifdef SDL
 OBJS += $(OBJS_SDL)
 HDRS += $(HDRS_SDL)
 CFLAGS += $(shell sdl-config --cflags)
 LDLIBS += $(shell sdl-config --libs)
 EXEC = $(EXEC_SDL)
 # Install and uninstall resources
 INSTALL_OPTIONAL = install-res-dir install-images install-sdl-manpage
 UNINSTALL_OPTIONAL = uninstall-images uninstall-res-dir uninstall-sdl-manpage
else
 OBJS += $(OBJS_NCURSES)
 HDRS += $(HDRS_NCURSES)
 LDLIBS += -lncurses
 EXEC = $(EXEC_NCURSES)
 #No need to install/uninstall resources here, unless we make campaingns
endif

VERSION=`cat VERSION`

.PHONY: all clean cleanall

# Build
all: $(EXEC)

clean:
	-rm -f $(OBJS_INDEP) $(OBJS_NCURSES) $(OBJS_SDL) $(EXECS)

%.o: %.c $(HDRS)  
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

$(EXEC): $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(EXEC)


# Install
install-res-dir:
	$(INSTALL) -m 755 -d $(DATADIR)
install-images:
	$(INSTALL) -m 755 -d $(DATADIR)/$(IMAGESDIR)
	for file in $(IMAGESDIR)/*; do \
		$(INSTALL) -m 0644 $$file $(DATADIR)/$(IMAGESDIR); \
	done
install-sdl-manpage:
	$(INSTALL) -m 755 -d $(MANDIR)
	-sed "s/%VERSION%/$(VERSION)/g" $(EXEC_SDL).6 | gzip -c > $(MANDIR)/$(EXEC_SDL).6.gz
	-chmod 644 $(MANDIR)/$(GAME_TITLE).6.gz

install: all $(INSTALL_OPTIONAL) 
	$(INSTALL) -m 755 -D $(EXEC) $(BINDIR)/$(EXEC)
	$(INSTALL) -m 755 -d $(MANDIR)
	-sed "s/%VERSION%/$(VERSION)/g" $(GAME_TITLE).6 | gzip -c > $(MANDIR)/$(GAME_TITLE).6.gz
	-chmod 644 $(MANDIR)/$(GAME_TITLE).6.gz
	$(INSTALL) -m 755 -d $(DOCDIR)
	-cat CHANGELOG | gzip -c > $(DOCDIR)/changelog.gz
	-chmod 644 $(DOCDIR)/changelog.gz

# Uninstall
uninstall-res-dir:
	-rmdir $(DATADIR)
uninstall-images:
	-rm $(DATADIR)/$(IMAGESDIR)/*
	-rmdir $(DATADIR)/$(IMAGESDIR)
uninstall-sdl-manpage:
	-rm -f $(MANDIR)/$(EXEC_SDL).6.gz

uninstall: $(UNINSTALL_OPTIONAL)
	-rm $(BINDIR)/$(EXEC)
	-rm -f $(MANDIR)/$(GAME_TITLE).6.gz
	-rm $(DOCDIR)/changelog.gz
	-rmdir $(DOCDIR)

show-path:
	@echo would install to ${BINDIR}
