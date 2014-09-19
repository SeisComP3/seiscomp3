BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a
SLINKDIR = $(BASEDIR)/libslink
SLINKLIB = $(SLINKDIR)/libslink.a
QDIR   = $(BASEDIR)/qlib2
QLIB   = $(QDIR)/libqlib2.a

XML_INCL := $(shell xml2-config --cflags)
XML_LIBS := $(shell xml2-config --libs)

OBJS = chain_plugin.o schedule.o

CXX = g++
CXXFLAGS = -Wall -O2
CC = gcc
CFLAGS = -Wall -Wstrict-prototypes -O2
LDFLAGS =
LDLIBS =
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0

all: $(BINDIR)/chain_plugin

.PHONY: all depend clean

$(BINDIR)/chain_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) $(SLINKLIB) \
    $(QLIB)
	$(CXX) $(LDFLAGS) $+ $(XML_LIBS) $(LDLIBS) -o $@

chain_plugin.o: chain_plugin.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(SLINKDIR) -I$(QDIR) $(XML_INCL) -c $< -o $@

schedule.o: schedule.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

$(SLINKLIB):
	$(MAKE) -C $(SLINKDIR)

$(QLIB):
	$(MAKE) -C $(QDIR)

depend:
	$(CXX) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(SLINKDIR) \
           -I$(QDIR) $(XML_INCL) -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

