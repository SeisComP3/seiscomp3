CC = gcc
CFLAGS = -O2 -Wall
LDFLAGS =
LDLIBS = -lm

# For Solaris uncomment the following line
#LDLIBS = -lsocket -lnsl -lm

BIN = ../../bin/scream_plugin

OBJECTS = dispatch.o gcf.o gputil.o main.o scream.o util.o plugin.o

$(BIN): $(OBJECTS) 
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

dispatch.o: dispatch.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

gcf.o: gcf.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

gputil.o: gputil.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

main.o: main.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

scream.o: scream.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

util.o: util.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

plugin.o: ../plugin.c
	$(CC) -c $(CFLAGS) -I.. -c $< -o $@

clean:
	$(RM) $(OBJECTS)

