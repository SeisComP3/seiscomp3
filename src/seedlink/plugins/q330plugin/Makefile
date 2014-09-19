BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a
Q330DIR = ../lib330
Q330LIB = $(Q330DIR)/lib330.a

CONFIG_DIR  = /home/sysop/config
CONFIG_FILE = $(CONFIG_DIR)/plugins.ini

OBJS = q330_plugin.o

CC = gcc
CFLAGS = -O2 -D_REENTRANT
CXX = g++
CXXFLAGS = -Wall -O2 -D_REENTRANT
LDFLAGS =
LDLIBS = -lpthread
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\"$(CONFIG_FILE)\"

all: $(BINDIR)/q330_plugin

.PHONY: all depend clean

$(BINDIR)/q330_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) $(QLIB) $(Q330LIB)
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

q330_plugin.o: q330_plugin.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
        -I$(Q330DIR) -c $< -o $@

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

$(Q330LIB):
	$(MAKE) -C $(Q330DIR) CC="$(CC)" CFLAGS="$(CFLAGS)"

depend:
	$(CXX) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(Q330DIR) \
           -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

