#
# ./src/Makefile.am:
#
# Copyright (c) 2003 Guralp Systems Limited
# Author James McKenzie, contact <software@guralp.com>
# All rights reserved.
#
# $Id: Makefile.am,v 1.13 2006/09/14 14:45:21 root Exp $
#
# $Log: Makefile.am,v $
# Revision 1.13  2006/09/14 14:45:21  root
# *** empty log message ***
#
# Revision 1.12  2006/09/14 14:28:11  lwithers
# Add G2GetDecLut() and the machinery for the new G2PBlock.dig_type field.
#
# Revision 1.11  2006/01/09 12:36:44  root
# *** empty log message ***
#
# Revision 1.10  2005/12/28 10:20:06  root
# *** empty log message ***
#
# Revision 1.9  2004/11/18 12:43:12  root
# *** empty log message ***
#
# Revision 1.8  2004/04/20 15:39:08  root
# *** empty log message ***
#
# Revision 1.7  2004/02/13 15:42:03  root
# *** empty log message ***
#
# Revision 1.6  2003/06/06 16:03:36  root
# *** empty log message ***
#
# Revision 1.5  2003/05/28 22:06:44  root
# *** empty log message ***
#
# Revision 1.4  2003/05/28 21:55:59  root
# *** empty log message ***
#
# Revision 1.3  2003/05/28 21:52:42  root
# *** empty log message ***
#
# Revision 1.2  2003/05/28 21:40:56  root
# *** empty log message ***
#
# Revision 1.1  2003/05/16 10:46:25  root
# *** empty log message ***
#
# Revision 1.6  2003/05/13 15:09:58  root
# #
#
# Revision 1.5  2003/05/13 09:16:14  root
# #
#
# Revision 1.4  2003/04/16 14:28:00  root
# #
#
# Revision 1.3  2003/04/15 11:04:44  root
# #
#
# Revision 1.2  2003/04/01 18:17:47  root
# #
#
# Revision 1.1  2003/04/01 17:52:12  root
# #
#
#

GCFSRCS=gscsi.c scsilow.c gtime.c util.c gfile.c \
	gdir.c gsfile.c libgcf2.c gserialp.c gserial.c gcmos.c \
	gblock.c gint.c gblockp.c version.c gnet.c gserialcli.c \
	gdm24mk2declut.c gdm24mk3declut.c gdeclut.c

noinst_HEADERS= scsilow.h scsireg.h util.h \
	gblock.h gblockp.h gdir.h gfile.h gint.h gscsi.h \
	gserial.h gserialp.h gsfile.h gtime.h includes.h \
	gnet.h gserialcli.h gcmos.h


BUILT_SOURCES=version.h

libgcf2_a_SOURCES =  ${GCFSRCS}
libgcf2_la_SOURCES = ${GCFSRCS}
libgcf2_a_CFLAGS = ${AM_CFLAGS}

lib_LIBRARIES=libgcf2.a
lib_LTLIBRARIES=libgcf2.la

include_HEADERS=gcf2.h

AM_CFLAGS=-g

libgcf2_la_LDFLAGS = \
        -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
        -release $(LT_RELEASE) \
        -export-dynamic 

VFD=${srcdir}/..
VF=${shell cat ${VFD}/version-files}
VFS=${VF:%=${VFD}/%}
VCHK=${shell cat ${VFS} | @MD5SUM@ | @AWK@ '{print $$1 }' }
VNUM=${shell @GREP@ ${VCHK} ${VFD}/version-md5sums | @AWK@ '{ print $$2 }'  }
VDEF=${shell echo `cat ${VFD}/version-major`.`cat ${VFD}/version-minor`.`cat ${VFD}/version-micro` }

version.o:version.h

version.h: $(VFD)/version-files $(VFD)/version-major \
	$(VFD)/version-minor $(VFD)/version-micro \
	$(VFD)/version-md5sums ${VFS} Makefile
	if [ .${VNUM} = . ]; then \
		echo "#define VERSION \"libgcf2 Version ${VDEF} + Edits\"" > version.h; \
		echo ${VDEF}-E > version-num; \
	else \
		echo "#define VERSION \"libgcf2 Version ${VNUM}\"" > version.h; \
		echo ${VNUM} > version-num; \
	fi

