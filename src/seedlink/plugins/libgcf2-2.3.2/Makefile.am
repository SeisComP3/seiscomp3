#
# ./Makefile.am:
#
# Copyright (c) 2003 Guralp Systems Limited
# Author James McKenzie, contact <software@guralp.com>
# All rights reserved.
#
# $Id: Makefile.am,v 1.9 2005/07/29 14:00:16 root Exp $
#
# $Log: Makefile.am,v $
# Revision 1.9  2005/07/29 14:00:16  root
# *** empty log message ***
#
# Revision 1.8  2004/05/05 16:17:39  root
# *** empty log message ***
#
# Revision 1.7  2004/05/02 10:31:16  root
# *** empty log message ***
#
# Revision 1.6  2003/12/17 09:54:32  root
# *** empty log message ***
#
# Revision 1.5  2003/06/24 16:33:35  root
# *** empty log message ***
#
# Revision 1.4  2003/06/06 09:45:04  root
# *** empty log message ***
#
# Revision 1.3  2003/05/29 12:18:41  root
# *** empty log message ***
#
# Revision 1.2  2003/05/29 12:16:14  root
# *** empty log message ***
#
# Revision 1.1  2003/05/16 10:36:26  root
# *** empty log message ***
#
# Revision 1.3  2003/04/01 18:17:47  root
# #
#
# Revision 1.2  2003/04/01 17:52:11  root
# #
#
#
SUBDIRS=src  gcftool examples

bin_SCRIPTS = libgcf2-config

libgcf2-config.src:libgcf2-config.src.in
libgcf2-config:libgcf2-config.src version.sed
	@SED@ -f version.sed < libgcf2-config.src > libgcf2-config || rm -f libgcf2-config
	chmod +x $@
	
EXTRA_DIST=version-files version-major version-minor version-md5sums version-micro src/gdeclut.h

VFD=${srcdir}
VF=${shell cat ${VFD}/version-files}
VFS=${VF:%=${VFD}/%}
VCHK=${shell cat ${VFS} | @MD5SUM@ | @AWK@ '{print $$1 }' }
VNUM=${shell @GREP@ ${VCHK} ${VFD}/version-md5sums | @AWK@ '{ print $$2 }'  }
VDEF=${shell echo `cat ${VFD}/version-major`.`cat ${VFD}/version-minor`.`cat ${VFD}/version-micro` }

version.sed: $(VFD)/version-files $(VFD)/version-major \
	$(VFD)/version-minor $(VFD)/version-micro \
	$(VFD)/version-md5sums ${VFS} Makefile
	if [ .${VNUM} = . ]; then \
		echo "s/%VERSION%/${VDEF}-E/g" > version.sed; \
	else \
		echo "s/%VERSION%/${VNUM}/g" > version.sed; \
	fi

