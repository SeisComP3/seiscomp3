# $Id: Makefile.lib,v 1.5 2007/06/28 20:36:15 dechavez Exp $

#OSNAME  = SOLARIS
OSNAME  = LINUX
#OSNAME = DARWIN

CC      = gcc
RANLIB  = ranlib
LIBDIR  = .
INCDIR  = ../include
OPTMIZ  = -O2
INCS    = -I$(INCDIR)
DEFS    = -D$(OSNAME)
CFLAGS  = $(OPTMIZ) $(INCS) $(DEFS)
ARCHIVE = libisi.a
OUTPUT  = $(LIBDIR)/$(ARCHIVE)
OBJS    = $(OUTPUT)(iacp_auth.o)
OBJS   += $(OUTPUT)(iacp_close.o)
OBJS   += $(OUTPUT)(iacp_connect.o)
OBJS   += $(OUTPUT)(iacp_free.o)
OBJS   += $(OUTPUT)(iacp_get.o)
OBJS   += $(OUTPUT)(iacp_handshake.o)
OBJS   += $(OUTPUT)(iacp_open.o)
OBJS   += $(OUTPUT)(iacp_recv.o)
OBJS   += $(OUTPUT)(iacp_send.o)
OBJS   += $(OUTPUT)(iacp_set.o)
OBJS   += $(OUTPUT)(iacp_stats.o)
OBJS   += $(OUTPUT)(iacp_string.o)
OBJS   += $(OUTPUT)(iacp_util.o)
OBJS   += $(OUTPUT)(isi_cnf.o)
OBJS   += $(OUTPUT)(isi_datreq.o)
OBJS   += $(OUTPUT)(isi_ezio.o)
OBJS   += $(OUTPUT)(isi_log.o)
OBJS   += $(OUTPUT)(isi_netutil.o)
OBJS   += $(OUTPUT)(isi_open.o)
OBJS   += $(OUTPUT)(isi_pack.o)
OBJS   += $(OUTPUT)(isi_raw.o)
OBJS   += $(OUTPUT)(isi_read.o)
OBJS   += $(OUTPUT)(isi_seqno.o)
OBJS   += $(OUTPUT)(isi_soh.o)
OBJS   += $(OUTPUT)(isi_string.o)
OBJS   += $(OUTPUT)(isi_ts.o)
OBJS   += $(OUTPUT)(isi_utils.o)
OBJS   += $(OUTPUT)(isi_wfdisc.o)
OBJS   += $(OUTPUT)(logio_dump.o)
OBJS   += $(OUTPUT)(logio_init.o)
OBJS   += $(OUTPUT)(logio_msg.o)
OBJS   += $(OUTPUT)(logio_string.o)
OBJS   += $(OUTPUT)(logio_threshold.o)
OBJS   += $(OUTPUT)(util_compress.o)
OBJS   += $(OUTPUT)(util_etoh.o)
OBJS   += $(OUTPUT)(util_hexdump.o)
OBJS   += $(OUTPUT)(util_htoe.o)
OBJS   += $(OUTPUT)(util_list.o)
OBJS   += $(OUTPUT)(util_misc.o)
OBJS   += $(OUTPUT)(util_pack.o)
OBJS   += $(OUTPUT)(util_parse.o)
OBJS   += $(OUTPUT)(util_peer.o)
OBJS   += $(OUTPUT)(util_string.o)
OBJS   += $(OUTPUT)(util_swap.o)
OBJS   += $(OUTPUT)(util_timefunc.o)
OBJS   += $(OUTPUT)(util_timer.o)

.PRECIOUS : $(OUTPUT)

all: $(OBJS)
	$(RANLIB) $(OUTPUT)
	touch makestamp
