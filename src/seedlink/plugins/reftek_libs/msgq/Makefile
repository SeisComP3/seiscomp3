OBJS    = msgq.o

CC = gcc
CFLAGS = -O2

all: libmsgq.a

.PHONY: all clean

libmsgq.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) libmsgq.a

