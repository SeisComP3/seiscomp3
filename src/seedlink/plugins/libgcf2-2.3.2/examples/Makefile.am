#
# ./examples/Makefile.am:
#
# Copyright (c) 2003 Guralp Systems Limited
# Author James McKenzie, contact <software@guralp.com>
# All rights reserved.
#
# : Makefile.am,v 1.1 2003/06/24 16:34:31 root Exp $
#
# : Makefile.am,v $
#
#

INCLUDES = -I$(srcdir)/../src

AM_CFLAGS=-g

EXTRA_DIST=example-blocks.gcf

bin_PROGRAMS =  application g2block g2cmos g2file g2ints g2misc g2net g2serial g2serialcli g2serialp g2time 

application_SOURCES = application.c
application_LDADD = ../src/libgcf2.la

g2block_SOURCES = g2block.c
g2block_LDADD = ../src/libgcf2.la

g2cmos_SOURCES = g2cmos.c
g2cmos_LDADD = ../src/libgcf2.la

g2file_SOURCES = g2file.c
g2file_LDADD = ../src/libgcf2.la

g2ints_SOURCES = g2ints.c
g2ints_LDADD = ../src/libgcf2.la

g2misc_SOURCES = g2misc.c
g2misc_LDADD = ../src/libgcf2.la

g2net_SOURCES = g2net.c
g2net_LDADD = ../src/libgcf2.la

g2serial_SOURCES = g2serial.c
g2serial_LDADD = ../src/libgcf2.la

g2serialcli_SOURCES = g2serialcli.c
g2serialcli_LDADD = ../src/libgcf2.la

g2serialp_SOURCES = g2serialp.c
g2serialp_LDADD = ../src/libgcf2.la

g2time_SOURCES = g2time.c
g2time_LDADD = ../src/libgcf2.la
