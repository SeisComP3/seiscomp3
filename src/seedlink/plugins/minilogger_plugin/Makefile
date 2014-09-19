
# Build environment can be configured the following
# environment variables:
#   CC : Specify the C compiler to use
#   CFLAGS : Specify compiler options to use

#  IMPORTANT: change here to set the directory for binary executable files
ifdef MYBIN
export BINDIR=${MYBIN}
else
# with the following, binary executables will be placed in bin subdirectory of your home directory
#BINDIR=~/bin/
# if in doubt, use the following - binary executables will be placed in the current directory
export BINDIR=.
endif

# Options specific for GCC
ifndef CC
export CC = gcc
endif
#
ifndef CCFLAGS
#
export CCFLAGS_BASIC =  -Wall -fPIC -I..
#
# optimized
export CCFLAGS = -O3 $(CCFLAGS_BASIC)
#
# profile
#CCFLAGS=-O3 -pg $(CCFLAGS_BASIC)
#
# debug - gdb, valgrind, ...
#CCFLAGS = $(CCFLAGS_BASIC) -g
# valgrind --leak-check=yes  exe_name <args>
# valgrind --leak-check=full --show-reachable=yes exe_name exe_name <args>
endif

#LDFLAGS = -L..
#LDLIBS = -lm

# variables to support compiling in different configurations (called by higher level Makefile in distribution)
ifndef LIB_MSEED
	export LIB_MSEED = -lmseed
endif
ifndef LIB_RT
	export LIB_RT =
endif

OBJ=minisepdevice.o settings/settings.o settings/strmap.o


all: $(BINDIR)/mini_logger_writer

$(BINDIR)/mini_logger_writer: mini_logger_writer.o modules $(OBJ)
	$(CC) $(CCFLAGS) -o $(BINDIR)/mini_logger_writer mini_logger_writer.o $(OBJ) $(LIB_MSEED) $(LIB_RT) -lm


MODULES = \
	settings

modules:
	@for x in $(MODULES); \
	do \
		(echo ------; cd $$x; echo Making $@ in:; pwd; \
		make -f Makefile); \
	done

clean:
	@for x in $(MODULES); \
	do \
		(cd $$x; echo Cleaning in:; pwd; \
		make -f Makefile clean); \
	done
	rm -f $(BINDIR)/mini_logger_writer
	rm -f *.o
	rm -f *.a


# Implicit rule for building object files
%.o: %.c
	$(CC) $(CCFLAGS) -c $<

install:
	@echo
	@echo "No install target, copy the executable(s) yourself"
	@echo
