
SHELL = /bin/sh
CC      = gcc
INSTALL = install
PREFIX ?= $(DESTDIR)/usr
MANPREFIX=$(PREFIX)/share/man
BINDIR = $(PREFIX)/bin
MANDIR = $(MANPREFIX)/man6

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXEC   = curseofwar
EXECS = $(EXEC)
CFLAGS  += -Wall -O2
LDFLAGS += -lncurses -lm 

VERSION=`cat VERSION`

.PHONY: all clean cleanall

all: $(EXEC)

clean:
	-rm -f $(OBJS) $(EXECS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) grid.o state.o king.o network.o output.o client.o server.o main.o $(LDFLAGS)

install: all
	$(INSTALL) -m 755 -D $(EXEC) $(BINDIR)/$(EXEC)
	-mkdir -p $(MANDIR)
	-sed "s/VERSION/$(VERSION)/g" $(EXEC).6 | gzip -c > $(MANDIR)/$(EXEC).6.gz
	-chmod 644 $(MANDIR)/$(EXEC).6.gz

install-strip:
	$(INSTALL) -D -s $(EXEC) $(BINDIR)/$(EXEC)

uninstall:
	  -rm $(BINDIR)/$(EXEC)
	  -rm -f $(MANDIR)/$(EXEC).6.gz

show-path:
	@echo would install to ${BINDIR}
