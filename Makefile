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

# Game resources directories
INSTALL_DATA ?= $(DESTDIR)$(PREFIX)/share/$(GAME_TITLE)
IMAGESDIR = images

# Sources
SRCS_INDEP = grid.c state.c king.c network.c client.c server.c output-common.c path.c main-common.c
SRCS_NCURSES = output.c main.c 
SRCS_SDL = output-sdl.c main-sdl.c

HDRS_INDEP = common.h messaging.h $(SRCS_INDEP:.c=.h)
HDRS_NCURSES = output.h
HDRS_SDL = output-sdl.h

OBJS_INDEP = $(SRCS_INDEP:.c=.o)
OBJS_NCURSES = $(SRCS_NCURSES:.c=.o)
OBJS_SDL = $(SRCS_SDL:.c=.o)

EXECS = $(EXEC_NCURSES) $(EXEC_SDL)
CFLAGS ?= -Wall -O2
LDLIBS += -lm

# Common sources and header
OBJS = $(OBJS_INDEP) 
HDRS = $(HDRS_INDEP) 
# SDL or ncurses
ifdef SDL
 OBJS += $(OBJS_SDL)
 HDRS += $(HDRS_SDL)
 CFLAGS += $(shell pkg-config sdl --cflags)
 LDLIBS += $(shell pkg-config sdl --libs)
 EXEC = $(EXEC_SDL)
 # Version-specific
 INSTALL_OPTIONAL = install-sdl-manpage
 UNINSTALL_OPTIONAL = uninstall-sdl-manpage
 ifdef INSTALL_DATA
  INSTALL_OPTIONAL += install-images
  UNINSTALL_OPTIONAL += uninstall-images
 endif
else
 OBJS += $(OBJS_NCURSES)
 HDRS += $(HDRS_NCURSES)
 LDLIBS += -lncurses
 EXEC = $(EXEC_NCURSES)
 # Version-specific
 INSTALL_OPTIONAL = install-manpage install-changelog
 UNINSTALL_OPTIONAL = uninstall-manpage uninstall-changelog
endif

VERSION=$(shell cat VERSION)
CPPFLAGS += -DVERSION="\"$(VERSION)\""

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
install-manpage:
	$(INSTALL) -m 755 -d $(MANDIR)
	-sed "s/%VERSION%/$(VERSION)/g" $(GAME_TITLE).6 | gzip -c > $(MANDIR)/$(GAME_TITLE).6.gz
	-chmod 644 $(MANDIR)/$(GAME_TITLE).6.gz
install-changelog:
	$(INSTALL) -m 755 -d $(DOCDIR)
	-cat CHANGELOG | gzip -c > $(DOCDIR)/changelog.gz
	-chmod 644 $(DOCDIR)/changelog.gz
install-images:
	$(INSTALL) -m 755 -d $(INSTALL_DATA)/$(IMAGESDIR)
	for file in $(IMAGESDIR)/*; do \
		$(INSTALL) -m 0644 $$file $(INSTALL_DATA)/$(IMAGESDIR); \
	done
install-sdl-manpage:
	$(INSTALL) -m 755 -d $(MANDIR)
	-sed "s/%VERSION%/$(VERSION)/g" $(EXEC_SDL).6 | gzip -c > $(MANDIR)/$(EXEC_SDL).6.gz
	-chmod 644 $(MANDIR)/$(GAME_TITLE).6.gz

install: all $(INSTALL_OPTIONAL) 
	$(INSTALL) -m 755 -D $(EXEC) $(BINDIR)/$(EXEC)

# Uninstall
uninstall-manpage:
	-rm -f $(MANDIR)/$(GAME_TITLE).6.gz
uninstall-changelog:
	-rm $(DOCDIR)/changelog.gz
	-rmdir $(DOCDIR)

uninstall-images:
	-rm -f $(INSTALL_DATA)/$(IMAGESDIR)/*
	-rmdir $(INSTALL_DATA)/$(IMAGESDIR)
	-rmdir $(INSTALL_DATA)
uninstall-sdl-manpage:
	-rm -f $(MANDIR)/$(EXEC_SDL).6.gz

uninstall: $(UNINSTALL_OPTIONAL)
	-rm -f $(BINDIR)/$(EXEC)

show-path:
	@echo would install to ${BINDIR}
