BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLINKDIR = $(BASEDIR)/libslink
SLINKLIB = $(SLINKDIR)/libslink.a

OBJS = mseedfifo_plugin.o plugin.o

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =
LDLIBS =
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0

all: $(BINDIR)/mseedfifo_plugin

.PHONY: all depend clean

$(BINDIR)/mseedfifo_plugin: $(OBJS) $(SLINKLIB)
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

mseedfifo_plugin.o: mseedfifo_plugin.c
	$(CC) $(CFLAGS) $(OPTIONS) -I$(SLINKDIR) -I.. -c $< -o $@

plugin.o: ../plugin.c
	$(CC) $(CFLAGS) -I.. -c $< -o $@

$(SLINKLIB):
	$(MAKE) -C $(SLINKDIR)

depend:
	$(CC) $(OPTIONS) -I$(SLINKDIR) -I.. -MM *.c > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

