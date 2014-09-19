/*
 * g2file.c:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char rcsid[] = "$Id: g2file.c,v 1.2 2004/05/02 10:12:20 root Exp $";

/*
 * $Log: g2file.c,v $
 * Revision 1.2  2004/05/02 10:12:20  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/01 23:46:03  root
 * *** empty log message ***
 *
 */

#include <gcf2.h>
#include <stdio.h>
#include <unistd.h>

/*
 * examples of how to use G2Files in libgcf2 
 *
 * the interface is designed for reading files, disks or directories
 * filled with GCF blocks, GCF blocks are 1024 bytes long
 *
 */

/* A G2File is always read only */
/* It can physically be: */
/*  * a disk file libgcf2 will do it's darnest to figure out how to get */
/*    LFS working on your system so you can deal with files larger than 2G*/
/*  * A directory of disk files which are treated as if all the files */
/*    other than core files which have an initial lower case letter are */
/*    concatented in alaphabetic order  - useful for filesystems which */
/*    have trouble with big files, these files must be a multiple of */
/*    1024 bytes long */
/*  * a raw device (eg /dev/sda under linux) */
/*  * a SCSI device, under SunOS and Solaris, raw devices can't */
/*    access unlabeled or unpartitioned disks, libgcf2 sends */
/*    SCSI commands directly to the disk to read it (eg /dev/sga under */
/*    linux or FIXME under Solaris) */

/* G2Files are malloced pointers (like stdio FILEs) */
/* G2Offts are 64 bit integers representing offsets in G2Files */

uint8_t buf[1048576];

int
main (int argc, char *argv[])
{
  FILE *f;
  G2File *g;
  G2Offt o;
  int i;

  printf ("%s\n", rcsid);

/*Create a directory and some files to play with*/
  (void) mkdir ("files");
/*Create a 2M file*/
  f = fopen ("files/segment-1", "w");
  fseek (f, 2097151, SEEK_SET);
  fwrite ("", 1, 1, f);
  fclose (f);

/*Create a 1M file*/
  f = fopen ("files/segment-2", "w");
  fseek (f, 1048575, SEEK_SET);
  fwrite ("", 1, 1, f);
  fclose (f);

/*Create a 1M file which will be ignored*/
  f = fopen ("files/Segment-3", "w");
  fseek (f, 1048575, SEEK_SET);
  fwrite ("", 1, 1, f);
  fclose (f);

  /*G2FileOpen returns NULL if it fails */
  g = G2FileOpen ("files/doesntexist");
  printf ("G2FileOpen(\"files/doesntexist\")=%p\n", g);


  /*Open file */
  g = G2FileOpen ("files/segment-1");
  printf ("G2FileOpen(\"files/segement-1\")=%p\n", g);

  /*How long is it */
  o = G2FileLength (g);
  printf ("G2FileLength(g)=%d\n", (int) o);

  /*Read some bytes */
  i = G2FileRead (g, buf, 1048576);
  printf ("G2FileRead(g,buf,1048576)=%d\n", i);

  /*Where are we */
  o = G2FileTell (g);
  printf ("G2FileTell(g)=%d\n", (int) o);

  /*Seek */
  o = G2FileSeek (g, (G2Offt) - 1024, SEEK_CUR);
  printf ("G2FileSeek(g,(G2Offt) -1024,SEEK_CUR)=%d\n", (int) o);

  /*Seek again */
  o = G2FileSeek (g, (G2Offt) 1024, SEEK_SET);
  printf ("G2FileSeek(g,(G2Offt) 1024,SEEK_SET)=%d\n", (int) o);

  /*Since GCF blocks are all 1k long a utility read function */
  /*is provided which seeks to block n, and reads m blocks */

  /*So this reads 4096 bytes starting at 1048576 from the file */
  i = G2File1KRead (g, 1024, buf, 4);
  printf ("G2File1KRead(g,1024,buf,4)=%d\n", i);

  /*Two more utility functions are provided which read a G2Block */
  /*which is the data type representing an unparsed raw GCFBlock */
  /*from a G2File they are covered in g2block.c */

  /*Close the file */
  G2FileClose (g);
  printf ("G2FileClose(g)\n");

  /*Open dir */
  g = G2FileOpen ("files");
  printf ("G2FileOpen(\"files\")=%p\n", g);

  /*How long is it note it ignore Segment-3 because */
  /*It starts with a capital letter, it also ignores */
  /*files which begin with "core" */

  o = G2FileLength (g);
  printf ("G2FileLength(g)=%d\n", (int) o);

  /*Close the file */
  G2FileClose (g);
  printf ("G2FileClose(g)\n");


  return 0;
}


#if 0
extern G2File *G2FileOpen (const char *); /*Opens a "file", which can be 
                                           *a file a directory of files
                                           *a RAW disk device, or a SCSI
                                           *random access device*/
extern G2Offt G2FileLength (G2File * s); /*Returns the number of bytes 
                                          *in the G2File*/
extern G2Offt G2FileBlocks (G2File * s); /*Returns the number of blocks
                                          *in the G2File*/
extern G2Offt G2FileTell (G2File * s); /*Returns the current position
                                        *in the G2File*/
extern G2Offt G2FileSeek (G2File *, G2Offt, int);
                                          /*Seeks to arg2 in the G2File,
                                           *arg3 has the same semantics
                                           *as whence in lseek(2)*/
extern int G2FileRead (G2File *, void *, int);
                                          /*Read some bytes from a G2File,
                                           *has the same semantics as read(2)
                                           */
extern int G2File1KRead (G2File *, G2Offt, void *, int);
                                          /*Read arg4 blocks starting at block
                                           *number arg2 into arg3, returns the
                                           *number of blocks read*/
extern int G2FileReadBlock (G2File * s, G2Block * out);
                                          /*Read a Block from a file */
extern int G2FileRead1KBlock (G2File * s, G2Offt block, G2Block * out);
                                          /*Read block number block from file s */
extern void G2FileClose (G2File *); /*Close a G2File */

#endif
