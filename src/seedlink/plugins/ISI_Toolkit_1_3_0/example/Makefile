# $Id: Makefile.example,v 1.3 2007/06/28 20:36:15 dechavez Exp $
# Makefile for Ida System Interface library and example client

# uncomment the appropriate
OSNAME = SOLARIS
#OSNAME = LINUX
#OSNAME = DARWIN

# uncomment the following if required (eg, Solaris)
SOCKLIBS = -lsocket -lnsl

CC     = gcc
VPATH  =
INCDIR = ../include
BINDIR = .
OPTMIZ = -g
INCS   = -I$(INCDIR)
DEFS   = -D$(OSNAME)
CFLAGS = $(OPTMIZ) $(INCS) -L$(LIBDIR) $(DEFS)
LIB    = libisi.a
LIBDIR = ../lib
OUTPUT = isi

all: $(OUTPUT)

$(LIB): FORCE
	cd $(LIBDIR); $(MAKE) OSNAME=$(OSNAME)

isi: ReleaseNotes.c main.c $(LIB)
	$(CC) $(CFLAGS) -o $@ ReleaseNotes.c main.c -lisi $(SOCKLIBS) -lm -lrt -lz

clean: FORCE
	rm -f $(LIB) $(OUTPUT)

FORCE:
