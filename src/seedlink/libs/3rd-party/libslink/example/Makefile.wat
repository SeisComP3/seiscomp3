#
#
# Wmake File for libslink example client - For Watcom's wmake
# Use 'wmake -f Makefile.wat'

.BEFORE
	@set INCLUDE=.;$(%watcom)\H;$(%watcom)\H\NT
	@set LIB=.;$(%watcom)\LIB386

cc     = wcc386
cflags = -zq
lflags = OPT quiet OPT map LIBRARY ..\libslink.lib LIBRARY ws2_32.lib
cvars  = $+$(cvars)$- -DWIN32

BIN = slclient.exe

INCS = -I..

OBJS=	slclient.obj

all: $(BIN)

$(BIN):	$(OBJS)
	wlink $(lflags) name slclient file {$(OBJS)}

# Source dependencies:
slclient.obj:	slclient.c

# How to compile sources:
.c.obj:
	$(cc) $(cflags) $(cvars) $(INCS) $[@ -fo=$@

# Clean-up directives:
clean:	.SYMBOLIC
	del *.obj *.map
	del $(BIN)
