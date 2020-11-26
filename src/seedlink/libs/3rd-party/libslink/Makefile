
# Build environment can be configured the following
# environment variables:
#   CC : Specify the C compiler to use
#   CFLAGS : Specify compiler options to use

MAJOR_VER = 2
MINOR_VER = 6
CURRENT_VER = $(MAJOR_VER).$(MINOR_VER)
COMPAT_VER = $(MAJOR_VER).$(MINOR_VER)

LIB_SRCS = gswap.c unpack.c msrecord.c genutils.c strutils.c \
           logging.c network.c statefile.c config.c \
           globmatch.c slplatform.c slutils.c

LIB_OBJS = $(LIB_SRCS:.c=.o)
LIB_DOBJS = $(LIB_SRCS:.c=.lo)

LIB_A = libslink.a
LIB_SO_FILENAME = libslink.so
LIB_SO_ALIAS = $(LIB_SO_FILENAME).$(MAJOR_VER)
LIB_SO = $(LIB_SO_FILENAME).$(CURRENT_VER)
LIB_DYN_ALIAS = libslink.dylib
LIB_DYN = libslink.$(CURRENT_VER).dylib

all: static

static: $(LIB_A)

shared: $(LIB_SO)

dynamic: $(LIB_DYN)

# Build static library
$(LIB_A): $(LIB_OBJS)
	rm -f $(LIB_A)
	ar -crs $(LIB_A) $(LIB_OBJS)

# Build shared library using GCC-style options
$(LIB_SO): $(LIB_DOBJS)
	rm -f $(LIB_SO) $(LIB_SO_ALIAS) $(LIB_SO_FILENAME)
	$(CC) $(CFLAGS) -shared -Wl,-soname -Wl,$(LIB_SO_ALIAS) -o $(LIB_SO) $(LIB_DOBJS)
	ln -s $(LIB_SO) $(LIB_SO_ALIAS)
	ln -s $(LIB_SO) $(LIB_SO_FILENAME)

# Build dynamic library (usually for Mac OSX)
$(LIB_DYN): $(LIB_DOBJS)
	rm -f $(LIB_DYN) $(LIB_DYN_ALIAS)
	$(CC) $(CFLAGS) -dynamiclib -compatibility_version $(COMPAT_VER) -current_version $(CURRENT_VER) -install_name $(LIB_DYN_ALIAS) -o $(LIB_DYN) $(LIB_DOBJS)
	ln -sf $(LIB_DYN) $(LIB_DYN_ALIAS)

clean:
	rm -f $(LIB_OBJS) $(LIB_DOBJS) $(LIB_A) $(LIB_SO) $(LIB_SO_ALIAS) \
	      $(LIB_SO_FILENAME) $(LIB_DYN) $(LIB_DYN_ALIAS)

install:
	@echo
	@echo "No install method, copy the library, header files, and"
	@echo "documentation to the preferred install location"
	@echo

.SUFFIXES: .c .o .lo

# Standard object building
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

# Standard object building for dynamic library components using -fPIC
.c.lo:
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

