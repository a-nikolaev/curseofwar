
SHELL = /bin/sh
CC      = gcc
INSTALL = install
PREFIX ?= $(DESTDIR)/usr
MANPREFIX=$(PREFIX)/man
BINDIR = $(PREFIX)/bin

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXEC   = curseofwar
EXECS = $(EXEC)
CFLAGS  += -Wall -O2
LDFLAGS += -lncurses -lm 

VERSION  = 1.1.2

.PHONY: all clean cleanall

all: $(EXEC) gzipman

clean:
	-rm -f $(OBJS) $(EXECS) $(EXEC).1.gz

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) grid.o state.o king.o network.o output.o client.o server.o main.o $(LDFLAGS)

gzipman: $(EXEC).6
	gzip < $^ > $^.gz

install:
	$(INSTALL) -D $(EXEC) $(BINDIR)/$(EXEC)
	-mkdir -p ${MANPREFIX}/man6
	-sed "s/VERSION/${VERSION}/g" < curseofwar.6 > ${MANPREFIX}/man6/curseofwar.6
	-chmod 644 ${MANPREFIX}/man6/curseofwar.6

install-strip:
	$(INSTALL) -D -s $(EXEC) $(BINDIR)/$(EXEC)

uninstall:
	  -rm $(BINDIR)/$(EXEC)
	  -rm -f ${MANPREFIX}/man6/curseofwar.6

show-path:
	@echo would install to ${BINDIR}
