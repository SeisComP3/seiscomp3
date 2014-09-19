BINDIR = ../../bin

OBJS = win_plugin.o plugin.o

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS =
LDLIBS =
DEFS = -DSYSLOG_FACILITY=LOG_LOCAL0

all: $(BINDIR)/win_plugin

.PHONY: all depend clean

$(BINDIR)/win_plugin: $(OBJS) $(ISI_LIB)
	$(CC) $(LDFLAGS) $+ $(LDLIBS) -o $@

win_plugin.o: win_plugin.c
	$(CC) $(CFLAGS) $(DEFS) -I$(ISI_DIR)/include -I.. -c $< -o $@

plugin.o: ../plugin.c
	$(CC) $(CFLAGS) -I.. -c $< -o $@

depend:
	$(CC) $(DEFS) -I$(ISI_DIR)/include -I.. -MM *.cc > .depend

clean:
	$(RM) $(OBJS) .depend
	
-include .depend

