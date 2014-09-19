OBJS    =
OBJS   += accept.o
OBJS   += attr.o
OBJS   += close.o
OBJS   += cmdpkt.o
OBJS   += log.o
OBJS   += misc.o
OBJS   += open.o
OBJS   += pid.o
OBJS   += recv.o
OBJS   += send.o
OBJS   += server.o
OBJS   += soh.o
OBJS   += stat.o
OBJS   += version.o

CC = gcc
CFLAGS = -O2

all: librtp.a

.PHONY: all clean

librtp.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) librtp.a

