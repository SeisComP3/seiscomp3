
BASEDIR = ../..

CC=gcc
CFLAGS = -Wall
LDFLAGS =

# For Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl

INC = ..
BIN = naqs_plugin
BINDIR = ../../bin

OBJECTS = naqs_utils.o plugin.o naqs_plugin.o

$(BINDIR)/$(BIN): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

clean:
	$(RM) $(OBJECTS)
