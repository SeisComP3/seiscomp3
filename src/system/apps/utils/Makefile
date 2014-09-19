BASEDIR = ..
BINDIR = $(BASEDIR)/bin
QDIR   = $(BASEDIR)/qlib2
QLIB   = $(QDIR)/libqlib2.a
SLINKDIR = $(BASEDIR)/libslink
SLINKLIB = $(SLINKDIR)/libslink.a

XML_INCL := $(shell xml2-config --cflags)
XML_LIBS := $(shell xml2-config --libs)

CXX = g++
CXXFLAGS = -Wall -O2
CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =
LDLIBS =

OBJ = timeout.o mseed_last_time.o trylock.o waitlock.o run_with_lock.o lockutil.o load_timetable.o

all: $(BINDIR)/timeout $(BINDIR)/trylock $(BINDIR)/waitlock $(BINDIR)/run_with_lock $(BINDIR)/load_timetable

.PHONY: all clean .depend

$(BINDIR)/timeout: timeout.o
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(BINDIR)/mseed_last_time: mseed_last_time.o
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(BINDIR)/trylock: trylock.o
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(BINDIR)/waitlock: waitlock.o
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(BINDIR)/run_with_lock: run_with_lock.o lockutil.o
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

$(BINDIR)/load_timetable: load_timetable.o $(SLINKLIB) $(QLIB) 
	$(CC) $(LDFLAGS) $+ $(XML_LIBS) $(LDLIBS) -o $@

run_with_lock.o lockutil.o timeout.o mseed_last_time.o trylock.o waitlock.o: %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

load_timetable.o: load_timetable.c
	$(CC) $(CFLAGS) -I$(SLINKDIR) -I$(QDIR) $(XML_INCL) -c $< -o $@

$(QLIB):
	$(MAKE) -C $(QDIR)

$(TOOLSLIB):
	$(MAKE) -C $(TOOLSDIR)

$(SLINKLIB):
	$(MAKE) -C $(SLINKDIR)

depend:
	$(CC) -I$(QDIR) -I$(SLINKDIR) $(XML_INCL) -MM *.c > .depend

clean:
	$(RM) $(OBJ) .depend
	
-include .depend

