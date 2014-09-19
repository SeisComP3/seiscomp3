
#
#   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
#   CHECKED IT OUT USING THE COMMAND CHECKOUT.
#
#    $Id: makefile.nt 4345 2011-06-29 20:17:29Z stefan $
#
#    Revision history:
#     $Log$
#     Revision 1.1  2011/07/14 18:41:39  saurel
#     Initial revision
#
#
#

B = $(EW_HOME)/$(EW_VERSION)/bin
L = $(EW_HOME)/$(EW_VERSION)/lib


#FC=f77
#FFLAGS=

#
Hi=	main.o hypo1m.o hypo2.o hypo3.o hypo4.o single.o  \
	ytrv.o input1.o timz3.o geo_sp.o
hypo:	$(Hi)
	$(FC)  $(Hi) -o $B/Hypo71PC

.f.o:
	$(FC) -c $*.f

clean:
	rm -f *.o

clean_bin:
	rm -f $B/Hypo71PC

