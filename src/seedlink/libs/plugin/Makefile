BASEDIR = ..
SLUTILSDIR = $(BASEDIR)/libslutils
QDIR   = $(BASEDIR)/qlib2

CXX_OBJ = plugin_channel.o
CC_OBJ = plugin.o

CXX = g++
CXXFLAGS = -Wall -O2
CC = gcc
CFLAGS = -Wall -Wstrict-prototypes -O2

all: libslplugin.a

.PHONY: all depend clean

libslplugin.a: $(CXX_OBJ) $(CC_OBJ)
	ar -cr $@ $+

$(CXX_OBJ): %.o: %.cc
	$(CXX) $(CXXFLAGS) -I$(SLUTILSDIR) -I$(QDIR) -c $< -o $@

$(CC_OBJ): %.o: %.c
	$(CC) $(CFLAGS) -I$(QDIR) -c $< -o $@

depend:
	$(CXX) -I$(SLUTILSDIR) -I$(QDIR) -MM *.cc *.c > .depend

clean:
	$(RM) libslplugin.a $(CXX_OBJ) $(CC_OBJ) .depend

-include .depend

