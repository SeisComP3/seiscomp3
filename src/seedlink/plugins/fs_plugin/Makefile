BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
SLPLUGINSLIB = $(SLPLUGINSDIR)/libslplugin.a
SLINKDIR = $(BASEDIR)/libslink
QDIR   = $(BASEDIR)/qlib2
QLIB   = $(QDIR)/libqlib2.a
TITANDIR = ../titan7.1
SEISANDIR = ../seisan/LIB
SEISANLIB = $(SEISANDIR)/seisan.a

CONFIG_DIR  = /home/sysop/config
CONFIG_FILE = $(CONFIG_DIR)/plugins.ini

# OBJS = fs_plugin.o fs_input_irae.o fs_input_edr.o fs_input_ddb.o \
#        fs_decode_titan.o fs_decode_seisan.o fs_decode_mseed.o read_titan.o \
#        my_printf.o

OBJS = fs_plugin.o fs_input_irae.o fs_input_edr.o fs_input_ddb.o \
       fs_decode_mseed.o

TITAN_OBJ = $(TITANDIR)/segtitan.o $(TITANDIR)/synchro.o \
            $(TITANDIR)/libtitan.o $(TITANDIR)/libutil.o \
            $(TITANDIR)/libinfo.o $(TITANDIR)/libtime.o \
            $(TITANDIR)/libtitio/libio.o

CC = gcc
CFLAGS = -Wall -O2
CXX = g++
CXXFLAGS = -Wall -O2
LDFLAGS =
LDLIBS =
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\"$(CONFIG_FILE)\"

all: $(BINDIR)/fs_plugin

.PHONY: all depend clean

# $(BINDIR)/fs_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) $(QLIB) \
#   $(TITAN_OBJ) $(SEISANLIB)
#	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -lg2c -lm -o $@

$(BINDIR)/fs_plugin: $(OBJS) $(SLPLUGINSLIB) $(SLUTILSLIB) $(QLIB)
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

fs_plugin.o plugin_channel.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

my_printf.o: my_printf.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

fs_input_irae.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

fs_input_edr.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

fs_input_ddb.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

fs_decode_titan.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -I$(TITANDIR) -c $< -o $@

fs_decode_seisan.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -c $< -o $@

fs_decode_mseed.o: %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(QDIR) -I$(SLINKDIR) -c $< -o $@

read_titan.o: read_titan.c
	$(CC) $(CFLAGS) -Dprintf=my_printf -Dfprintf=my_fprintf -I$(TITANDIR) \
          -c $< -o $@

$(TITAN_OBJ):
	$(MAKE) CFLAG='-Dprintf=my_printf -Dfprintf=my_fprintf -Dtrim=cvtit_trim -Dgetline=cvtit_getline' \
            -C $(TITANDIR) $(TITAN_OBJ:$(TITANDIR)/%=%)

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(SLPLUGINSLIB):
	$(MAKE) -C $(SLPLUGINSDIR)

$(QLIB):
	$(MAKE) -C $(QDIR)

$(SEISANLIB):
	$(MAKE) -C $(SEISANDIR)

depend:
	$(CXX) $(OPTIONS) -I. -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(SLINKDIR) \
           -I$(QDIR) -I$(TITANDIR) -MM *.cc > .depend

clean:
	$(RM) $(OBJS) $(TITAN_OBJ) .depend
	
-include .depend

