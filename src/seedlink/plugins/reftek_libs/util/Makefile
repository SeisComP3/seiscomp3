OBJS    =
OBJS   += bcd2long.o
OBJS   += dump.o
OBJS   += getline.o
OBJS   += find.o
OBJS   += mstime.o
OBJS   += parse.o
OBJS   += string.o
OBJS   += swap.o
OBJS   += timer.o
OBJS   += timefunc.o

CC = gcc
CFLAGS = -O2

all: libutil.a

.PHONY: all clean

libutil.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) libutil.a

