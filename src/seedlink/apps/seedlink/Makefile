BASEDIR = ..
BINDIR = $(BASEDIR)/bin
SLINKDIR = $(BASEDIR)/libslink
SLUTILSDIR = $(BASEDIR)/libslutils
SLUTILSLIB = $(SLUTILSDIR)/libslutils.a
SLPLUGINSDIR = $(BASEDIR)/libslplugin
QDIR   = $(BASEDIR)/qlib2
QLIB   = $(QDIR)/libqlib2.a

XML_INCL := $(shell xml2-config --cflags)
XML_LIBS := $(shell xml2-config --libs)

CONFIG_FILE  = /home/sysop/config/seedlink.ini

OBJS = seedlink.o iosystem.o monitor.o sproc.o mseed.o steim1.o steim2.o

CXX = g++
CXXFLAGS = -Wall -O2 -fno-strict-aliasing
LDFLAGS =
LDLIBS =
OPTIONS = -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\"$(CONFIG_FILE)\" \
    -DMSEED_ROUND_TENTH_MILLISEC

# Uncomment the following to enable file descriptor reallocation.
# This is needed for large servers (number of stations multiplied by
# the number of simultaneous connections is larger than 1000).
OPTIONS += -DFD_REALLOC

# Uncomment next two lines if you want to link with libwrap (this adds
# some security, but may cause problems).
#LDLIBS += -lwrap
#OPTIONS += -DTCPWRAP

all: $(BINDIR)/seedlink

.PHONY: all clean .depend

$(BINDIR)/seedlink: $(OBJS) $(SLUTILSLIB) $(QLIB)
	$(CXX) $(LDFLAGS) $+ $(XML_LIBS) $(LDLIBS) -o $@

$(OBJS): %.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) \
           -I$(SLINKDIR) -I$(QDIR) $(XML_INCL) -c $< -o $@

$(SLUTILSLIB):
	$(MAKE) -C $(SLUTILSDIR)

$(QLIB):
	$(MAKE) -C $(QDIR)

depend:
	$(CC) $(OPTIONS) -I$(SLPLUGINSDIR) -I$(SLUTILSDIR) -I$(SLINKDIR) \
          -I$(QDIR) $(XML_INCL) -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend

-include .depend

