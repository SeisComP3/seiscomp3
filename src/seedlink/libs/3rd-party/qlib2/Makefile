########################################################################
TARGET1	= libqlib2.a
TARGET2	= libqlib2.debug.a
TARGET3	= libqlib2nl.a
TARGET4	= libqlib2nl.debug.a
TARGET5	= libqlib2-64.a
TARGET6	= libqlib2-64.debug.a
TARGET7	= libqlib2nl-64.a
TARGET8	= libqlib2nl-64.debug.a
TARGET	= $(TARGET1)

DIR	= qlib2

SRCS	= qlib2_version.c drm_utils.c ms_utils.c ms_pack.c ms_unpack.c pack.c \
	qda_utils.c qtime.c sdr_utils.c unpack.c qutils.c 


HDR =	qlib2.h

HDRS =	qdefines.h msdatatypes.h timedef.h \
	qsteim.h sdr.h qda.h seismo.h data_hdr.h \
	qtime.h qutils.h qda_utils.h drm_utils.h sdr_utils.h \
	ms_utils.h ms_pack.h pack.h ms_unpack.h unpack.h
	
FHDR =	qlib2.inc

FHDRS =	qdefines.inc data_hdr.inc qfortran.inc

OBJS	= $(SRCS:%.c=%.o)

INCDIR= /usr/local/include
LIBDIR=	/usr/local/lib
MANDIR=	/usr/local/man
MANEXT=	3

RANLIB	= ranlib

.PRECIOUS:	$(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8)

BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man
MANEXT	= 1

CC	= gcc
COPT	= -O2
CFLAGS  = $(COPT)

# -Dfortran_suffix is required for fortran Compilers that use the BSD 
# convention of appending a "_" suffix to the subroutine or function name.
# -Dqlib2_fortran is required to compile the fortran interface routines.
# -DSUNOS4 is required for SunOS 4.1.x.
#CFLAGS	= -Dfortran_suffix -Dqlib2_fortran
#CFLAGS	= -Dfortran_suffix -Dqlib2_fortran -DSUNOS4

C64	= -xarch=v9 -xchip=ultra3
$(TARGET1)  := CFLAGS += $(COPT)
$(TARGET2) := CFLAGS += -g
$(TARGET3) := CFLAGS += $(COPT) -DNO_LEAPSECONDS
$(TARGET4) := CFLAGS += -g -DNO_LEAPSECONDS
$(TARGET5) := CFLAGS += $(COPT) $(C64)
$(TARGET6) := CFLAGS += -g $(C64)
$(TARGET7) := CFLAGS += $(COPT) -DNO_LEAPSECONDS $(C64)
$(TARGET8) := CFLAGS += -g -DNO_LEAPSECONDS $(C64)

ALLHDRS	= $(HDR) $(FHDR) 
ALLLIBS32 = $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)
ALLLIBS64 = $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8)
ALLLIBS = $(ALLLIBS32) $(ALLLIBS64)
#:: ALLLIBS = $(ALLLIBS32)
#ALL	= $(ALLHDRS) $(ALLLIBS)
ALL	= $(HDR) $(TARGET)

########################################################################

all:	$(ALL)

$(HDR):	$(HDRS)
	cat $(HDRS)  | grep -v '#include "'> $@

$(FHDR): $(FHDRS)
	cat $(FHDRS) | grep -v '#include "'> $@

$(TARGET1):	$(TARGET1)($(OBJS))
	$(RANLIB) $@

$(TARGET2):	$(TARGET2)($(OBJS))
	$(RANLIB) $@

$(TARGET3):	$(TARGET3)($(OBJS))
	$(RANLIB) $@

$(TARGET4):	$(TARGET4)($(OBJS))
	$(RANLIB) $@

$(TARGET5):	$(TARGET5)($(OBJS))
	$(RANLIB) $@

$(TARGET6):	$(TARGET6)($(OBJS))
	$(RANLIB) $@

$(TARGET7):	$(TARGET7)($(OBJS))
	$(RANLIB) $@

$(TARGET8):	$(TARGET8)($(OBJS))
	$(RANLIB) $@

install_all:	install install_leapseconds install_man

install:	$(ALL) leapseconds 
	cp $(HDR) $(FHDR) drm_seismo.h $(INCDIR)/
	for lib in $(ALLLIBS) ; do \
		cp $$lib $(LIBDIR) ; \
		( cd $(LIBDIR) ; $(RANLIB) $$lib ) ; \
	done

install_leapseconds:	leapseconds
	cp leapseconds $(LIBDIR)/leapseconds

install_man:	qlib2.man
	cp qlib2.man $(MANDIR)/man$(MANEXT)/qlib2.$(MANEXT)
	
clean:
	-rm -f *.o $(ALL)

veryclean:	clean

tar:	$(ALL)
	(cd ..; tar cf - $(DIR)/*.c $(DIR)/*.h $(DIR)/Makefile \
		$(DIR)/*.inc $(DIR)/*.man \
		$(DIR)/$(TARGET1) $(DIR)/$(TARGET2) \
		$(DIR)/$(TARGET3) $(DIR)/$(TARGET4) \
		$(DIR)/$(TARGET5) $(DIR)/$(TARGET6) \
		$(DIR)/$(TARGET7) $(DIR)/$(TARGET8) \
		$(DIR)/README $(DIR)/CHANGES $(DIR)/leapseconds ) \
	| gzip > /tmp/$(DIR).tar.gz

$(QLIB2):	FORCE
	(cd ../qlib2; make)

FORCE:

cleandepend:
	makedepend

depend:	$(HDR) $(FHDR)
	makedepend $(CFLAGS) *.c
	sed -e "s/^[^ 	]*\.o/$(TARGET)(&)/" < Makefile > Makefile.new \
		&& mv Makefile.new Makefile

########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
