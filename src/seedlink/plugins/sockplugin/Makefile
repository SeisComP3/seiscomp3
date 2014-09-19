CC=gcc
CFLAGS = -Wall -O2
LDFLAGS =

# For Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl

INC = ..

BINDIR = ../../bin

all: $(BINDIR)/sock_plugin

$(BINDIR)/sock_plugin: sock_plugin.o seedutils.o plugin.o
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

sock_plugin.o:	sock_plugin.c
	$(CC) -c $(CFLAGS) -I$(INC) -c $< -o $@

seedutils.o:	seedutils.c
	$(CC) -c $(CFLAGS) -I$(INC) -c $< -o $@

plugin.o: ../plugin.c
	$(CC) -c $(CFLAGS) -I$(INC) -c $< -o $@

clean:
	$(RM) sock_plugin.o seedutils.o plugin.o 
