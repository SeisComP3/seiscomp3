CC = gcc
CFLAGS  = -D_REENTRANT -O2 -Wall

# For using another compiler (e.g. the Sun compiler)
#CC = cc
#CFLAGS = -O -I../libslink

LDFLAGS = 
LDLIBS  = -lpthread

# For SunOS/Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl -lrt -lpthread

BIN = ../../bin/ewexport_plugin

OBJS = ewexport_plugin.o network.o util.o swap.o plugin.o

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

ewexport_plugin.o: ewexport_plugin.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

network.o: network.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

swap.o: swap.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

util.o: util.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

plugin.o: ../plugin.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)

