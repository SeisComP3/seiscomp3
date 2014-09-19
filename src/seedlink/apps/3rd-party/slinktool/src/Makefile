BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLINKDIR = $(BASEDIR)/libslink
SLINKLIB = $(SLINKDIR)/libslink.a

XML_INCL := $(shell xml2-config --cflags)
XML_LIBS := $(shell xml2-config --libs)

CC = gcc
CFLAGS = -Wall -O2

# For using another compiler (e.g. the Sun compiler)
#CC = cc
#CFLAGS = -O

LDFLAGS =
LDLIBS  =

# For SunOS/Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl -lrt

OBJS = dsarchive.o archive.o slinkxml.o slinktool.o

all: $(BINDIR)/slinktool

.PHONY: all clean install depend 

$(BINDIR)/slinktool: $(OBJS) $(SLINKLIB)
	$(CC) $(LDFLAGS) $+ $(XML_LIBS) $(LDLIBS) -o $@

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) $(XML_INCL) -I$(SLINKDIR) -c $< -o $@

$(SLINKLIB):
	$(MAKE) -C $(SLINKDIR)

depend:
	$(CC) $(CFLAGS) $(XML_INCL) -I$(SLINKDIR) -MM *.c > .depend

clean:
	$(RM) $(OBJS) .depend

install:
	@echo
	@echo "No install method, copy the binary and man page yourself"
	@echo

-include .depend

