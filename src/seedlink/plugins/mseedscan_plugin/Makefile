BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a

OBJS = mseedscan.o seedutil.o rbtree.o stack.o

CC = gcc
CFLAGS = -Wall -O2
INC = -I$(SLPLUGINSDIR)
LDFLAGS = 
LDLIBS = 

# For Solaris
# LDFLAGS = -lrt

all: $(BINDIR)/mseedscan_plugin

.PHONY: all depend clean

$(BINDIR)/mseedscan_plugin: $(OBJS) $(SLPLUGINSLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(SLPLUGINSLIB) $(LDLIBS) -o $@

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

depend:
	$(CC) $(INC) -MM *.c > .depend

clean:
	$(RM) $(OBJS) .depend

# Implicit rule for compiling source files
%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $<

-include .depend

