
SHELL = /bin/sh
CC      = gcc
INSTALL = install
PREFIX ?= $(DESTDIR)/usr
MANPREFIX=$(PREFIX)/man
BINDIR = $(PREFIX)/bin
MANDIR = $(MANPREFIX)/man6

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXEC   = curseofwar
EXECS = $(EXEC)
CFLAGS  += -Wall -O2
LDFLAGS += -lncurses -lm 

.PHONY: all clean cleanall

all: $(EXEC) gzipman

clean:
	-rm -f $(OBJS) $(EXECS) $(EXEC).6.gz

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) grid.o state.o king.o network.o output.o client.o server.o main.o $(LDFLAGS)

gzipman: $(EXEC).6
	gzip < $^ > $^.gz

install:
	$(INSTALL) -D $(EXEC) $(BINDIR)/$(EXEC)
	$(INSTALL) -D -m 644 $(EXEC).6.gz $(MANDIR)/$(EXEC).6.gz

install-strip:
	$(INSTALL) -D -s $(EXEC) $(BINDIR)/$(EXEC)

uninstall:
	  -rm $(BINDIR)/$(EXEC)
	  -rm -f $(MANDIR)/$(EXEC).6.gz

show-path:
	@echo would install to ${BINDIR}
