OBJS    = si.o

CC = gcc
CFLAGS = -O2

all: libsi.a

.PHONY: all clean

libsi.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) libsi.a

