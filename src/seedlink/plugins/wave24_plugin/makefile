INCDIR=$(DEVELDIR)

BINNAME=../../bin/wave24_plugin

PLUGINDIR=..

COMPILER=g++
CFLAGS=-O2 -fno-strict-aliasing
.SUFFIXES: .o .cpp

LINKER=g++
LDFLAGS=
LDLIBS=

RM=rm -v

HEADERS= Wave24Logger.h Wave24Messages.h Wave24Seed.h Wave24Exception.h

OBJS= Wave24Drv.o Wave24Logger.o Wave24Messages.o Wave24Seed.o plugin.o

all: $(BINNAME)

$(BINNAME): $(OBJS)
	$(LINKER) $(LDFLAGS) $+ $(LDLIBS) -o $@

.cpp.o:
	$(COMPILER) $(CFLAGS) -I$(PLUGINDIR) -c $< -o $@

#lets make plugin.o
plugin.o:$(PLUGINDIR)/plugin.c $(PLUGINDIR)/plugin.h
	$(COMPILER) $(CFLAGS) -I$(PLUGINDIR) -c $< -o $@

clean:
	$(RM) $(OBJS)

