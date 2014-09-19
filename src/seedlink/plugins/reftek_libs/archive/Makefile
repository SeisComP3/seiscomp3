OBJS    =
OBJS   += archive.o
OBJS   += create.o
OBJS   += errors.o
OBJS   += evnfiles.o
OBJS   += find.o
OBJS   += linklist.o
OBJS   += open.o
OBJS   += purge.o
OBJS   += rate.o
OBJS   += read.o
OBJS   += recfmt.o
OBJS   += serialize.o
OBJS   += state.o
OBJS   += streams.o
OBJS   += strlist.o
OBJS   += unix.o
OBJS   += write.o

CC = gcc
CFLAGS = -O2

all: libarchive.a

.PHONY: all clean

libarchive.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -I../include $(PLATFORM) -c $< -o $@

clean:
	$(RM) $(OBJS) libarchive.a

