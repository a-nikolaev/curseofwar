
SHELL = /bin/sh
CC      = gcc
INSTALL = install
PREFIX ?= $(DESTDIR)/usr
BINDIR = $(PREFIX)/bin

SRCS    = $(wildcard *.c)
HDRS    = $(wildcard *.h)
OBJS    = $(patsubst %.c,%.o,$(SRCS))
EXEC   = curseofwar
EXECS = $(EXEC)
CFLAGS  += -Wall
LDFLAGS += -lncurses -lm -lrt

.PHONY: all clean cleanall

all: $(EXEC)

clean:
	-rm -f $(OBJS) $(EXECS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $(patsubst %.o,%.c,$@)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) grid.o state.o king.o network.o output.o client.o server.o main.o $(LDFLAGS)


install:
	$(INSTALL) -D $(EXEC) $(BINDIR)/$(EXEC)

install-strip:
	$(INSTALL) -D -s $(EXEC) $(BINDIR)/$(EXEC)

uninstall:
	  -rm $(BINDIR)/$(EXEC)

show-path:
	@echo would install to ${BINDIR}
