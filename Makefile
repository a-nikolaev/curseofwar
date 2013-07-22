SHELL = /bin/sh
CC      = gcc
INSTALL = install
EXEC   = curseofwar

PREFIX ?= /usr
MANPREFIX = $(PREFIX)/share/man

BINDIR = $(DESTDIR)$(PREFIX)/bin
MANDIR = $(DESTDIR)$(MANPREFIX)/man6
DOCDIR = $(DESTDIR)$(PREFIX)/share/doc/$(EXEC)

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXECS = $(EXEC)
CFLAGS  += -Wall -O2
LDLIBS += -lncurses -lm 

VERSION=`cat VERSION`

.PHONY: all clean cleanall

all: $(EXEC)

clean:
	-rm -f $(OBJS) $(EXECS)

%.o: %.c $(HDRS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) grid.o state.o king.o network.o output.o client.o server.o main.o $(LDLIBS) -o $(EXEC)

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
