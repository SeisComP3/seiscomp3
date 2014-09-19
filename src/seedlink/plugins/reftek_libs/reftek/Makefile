OBJS    =
OBJS   += ad.o
OBJS   += cd.o
OBJS   += ds.o
OBJS   += dt.o
OBJS   += eh.o
OBJS   += et.o
OBJS   += om.o
OBJS   += sh.o
OBJS   += sc.o
OBJS   += com.o
OBJS   += type.o
OBJS   += steim.o
OBJS   += string.o

CC = gcc
CFLAGS = -O2

all: libreftek.a

.PHONY: all clean

libreftek.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) libreftek.a

