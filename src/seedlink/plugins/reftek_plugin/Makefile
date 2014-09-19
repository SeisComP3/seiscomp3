BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a
RTINCL = ../reftek_libs/include
REFTEKDIR = ../reftek_libs/reftek
REFTEKLIB = $(REFTEKDIR)/libreftek.a
RTPDIR = ../reftek_libs/rtp
RTPLIB = $(RTPDIR)/librtp.a
RTUTILDIR = ../reftek_libs/util
RTUTILLIB = $(RTUTILDIR)/libutil.a

CONFIG_DIR  = /home/sysop/config
CONFIG_FILE = $(CONFIG_DIR)/plugins.ini

PLATFORM = -DLINUX -Di386 -D_REENTRANT

OBJS = reftek_plugin.o

CXX = g++
CXXFLAGS = -Wall -O2
LDFLAGS =
LDLIBS = -lpthread
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\"$(CONFIG_FILE)\"

all: $(BINDIR)/reftek_plugin

.PHONY: all depend clean

$(BINDIR)/reftek_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) \
    $(REFTEKLIB) $(RTPLIB) $(RTUTILLIB)
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

reftek_plugin.o: reftek_plugin.cc
	$(CXX) $(CXXFLAGS) $(PLATFORM) $(OPTIONS) -I$(SLPLUGINSDIR) \
           -I$(SLUTILSDIR) -I$(RTINCL) -c $< -o $@

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

$(REFTEKLIB):
	$(MAKE) -C $(REFTEKDIR) PLATFORM="$(PLATFORM)"

$(RTPLIB):
	$(MAKE) -C $(RTPDIR) PLATFORM="$(PLATFORM)"

$(RTUTILLIB):
	$(MAKE) -C $(RTUTILDIR) PLATFORM="$(PLATFORM)"

depend:
	$(CXX) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(RTINCL) \
           -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

