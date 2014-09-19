BASEDIR = ../..
BINDIR = $(BASEDIR)/bin
SLINKDIR = $(BASEDIR)/libslink
SLINKLIB = $(SLINKDIR)/libslink.a

CC = gcc
CFLAGS  = -O2 -Wall

# For using another compiler (e.g. the Sun compiler)
#CC = cc
#CFLAGS = -O

LDFLAGS =
LDLIBS  =

# For SunOS/Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl -lrt

OBJS = dsarchive.o archive.o slarchive.o

all: $(BINDIR)/slarchive

.PHONY: all clean install depend

$(BINDIR)/slarchive: $(OBJS) $(SLINKLIB)
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I$(SLINKDIR) -c $< -o $@

$(SLINKLIB):
	$(MAKE) -C $(SLINKDIR)

depend:
	$(CC) $(CFLAGS) -I$(SLINKDIR) -MM *.c > .depend

clean:
	$(RM) $(OBJS) .depend

install:
	@echo
	@echo "No install method, copy the binary and man page yourself"
	@echo

-include .depend

