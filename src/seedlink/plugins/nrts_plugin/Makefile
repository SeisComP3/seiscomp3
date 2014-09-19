BINDIR = ../../bin
ISI_DIR = ../ISI_Toolkit_1_3_0
ISI_LIB = $(ISI_DIR)/lib/libisi.a

# uncomment the appropriate
#OSNAME = SOLARIS
OSNAME = LINUX

OBJS = nrts_plugin.o plugin.o

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =
LDLIBS = -lpthread -lrt -lm
DEFS = -D$(OSNAME) -DSYSLOG_FACILITY=LOG_LOCAL0

all: $(BINDIR)/nrts_plugin

.PHONY: all depend clean

$(BINDIR)/nrts_plugin: $(OBJS) $(ISI_LIB)
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

nrts_plugin.o: nrts_plugin.c
	$(CC) $(CFLAGS) $(DEFS) -I$(ISI_DIR)/include -I.. -c $< -o $@

plugin.o: ../plugin.c
	$(CC) $(CFLAGS) -I.. -c $< -o $@

$(ISI_LIB):
	$(MAKE) -C $(ISI_DIR)/lib OSNAME=$(OSNAME)

depend:
	$(CC) $(DEFS) -I$(ISI_DIR)/include -I.. -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

