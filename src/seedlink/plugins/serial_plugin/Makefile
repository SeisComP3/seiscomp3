BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a
QDIR   = $(BASEDIR)/qlib2
QLIB   = $(QDIR)/libqlib2.a
GCFDIR = ../libgcf2-2.3.2
GCFINC = $(GCFDIR)/src
GCFLIB = $(GCFDIR)/src/libgcf2.a

CONFIG_DIR  = /home/sysop/config
CONFIG_FILE = $(CONFIG_DIR)/plugins.ini

OBJS = serial_plugin.o rw2300.o linux2300.o proto_edata.o proto_edata2.o \
    proto_guralp2.o proto_dr24.o proto_prema.o proto_hrd24.o proto_sadc.o \
    proto_ws2300.o proto_mws.o proto_solar.o proto_modbus.o

CXX = g++
CXXFLAGS = -Wall -O2 -fno-strict-aliasing
CC = gcc
CFLAGS = -Wall -O2 -fno-strict-aliasing
LDFLAGS =
LDLIBS =
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\"$(CONFIG_FILE)\"
WSOPTIONS = -DVERSION=\"1.10\"

all: $(BINDIR)/serial_plugin

.PHONY: all depend clean

$(BINDIR)/serial_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) $(QLIB) \
    $(GCFLIB)
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

serial_plugin.o: serial_plugin.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_edata.o: proto_edata.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_edata2.o: proto_edata2.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_guralp2.o: proto_guralp2.cc $(GCFINC)/gcf2.h
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -I$(GCFINC) -c $< -o $@

proto_dr24.o: proto_dr24.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_prema.o: proto_prema.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_hrd24.o: proto_hrd24.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_sadc.o: proto_sadc.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_ws2300.o: proto_ws2300.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_mws.o: proto_mws.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_solar.o: proto_solar.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

proto_modbus.o: proto_modbus.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

rw2300.o linux2300.o: %.o: %.c
	$(CC) $(CFLAGS) $(WSOPTIONS) -c $< -o $@

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

$(QLIB):
	$(MAKE) -C $(QDIR)

$(GCFLIB): $(GCFDIR)/Makefile
	$(MAKE) -C $(GCFDIR)

$(GCFDIR)/Makefile: $(GCFDIR)/Makefile.in
	cd $(GCFDIR); ./configure

$(GCFINC)/gcf2.h: $(GCFINC)/gcf2.h.in
	cd $(GCFDIR); ./configure

depend:
	$(CXX) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(QDIR) \
           -I$(GCFINC) -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

