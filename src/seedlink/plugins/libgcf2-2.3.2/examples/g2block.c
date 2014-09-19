/*
 * g2block.c:
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

static char rcsid[] = "$Id: g2block.c,v 1.2 2004/05/02 10:12:19 root Exp $";

/*
 * $Log: g2block.c,v $
 * Revision 1.2  2004/05/02 10:12:19  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/05/01 23:46:03  root
 * *** empty log message ***
 *
 */


#include <gcf2.h>


/* A G2Block, is merley 1024 bytes of data and a size */

/*
 *
 * typedef struct { 		
 *  uint8_t data[1024];
 *  int size; 
 * } G2Block;
 *
 */

/* (Note that a pointer to a G2SerBlock can be cast to a pointer to
 * G2Block see g2serial.c) */

/* A note about blocks. There is small ambiguity in the GCF format */
/* regarding 24 and 32 bit blocks, both are marked with a format value */
/* of 1. Blocks on disks should always be 32 bits long, but blocks from */
/* serial ports may be 24 bits. The Block Parser, looks at the size */
/* element of G2Block to disambiguate the two. If the Block is in format */
/* 1 and the size indicates a 24 bit block, the parser will decode a 24 */
/* bit block otherwise, it will decode a 32 bit block */
/* Two utility functions G2transcode24to32(G2Block *in,G2Block *out) */
/* and G2transcode32to24(G2Block *in, G2Block *out) are provided to */
/* transcode if neccessary and or possible, they will copy blocks */
/* (of any format) otherwise, they both return zero on success */

int
main (int argc, char *argv[])
{
  G2Block b;

  printf ("%s\n", rcsid);
  {
/*See g2file.c*/
    G2File *g = G2FileOpen ("example-blocks.gcf");

    if (!g)
      {
        fprintf (stderr, "Failed to open example-blocks\n");
        exit (1);
      }

/*Read a block 0 from the file, the second argument is in blocks */
/*like the second argument of G2File1KRead, it returns the number */
/*of blocks sucessfully read - which is 1 on success */

/*(There is is also G2FileReadBlock(G2File*,G2Block*), which */
/*reads from the current position and returns the number of */
/*bytes read - 1024 on success)*/

    if (G2FileRead1KBlock (g, (G2Offt) 0, &b) != 1)
      {
        fprintf (stderr, "Failed to read a block from example-blocks\n");
        exit (1);
      }

    G2FileClose (g);
  }

/* hex dumps a block to a stdio FILE * */
  printf ("G2DumpBlock(stdout,&b)={\n");
  G2DumpBlock (stdout, &b);
  printf ("}\n");

  {
/* A G2PBlockH contains a broken down form of the header of a GCF */
/* block */
/* 
 * typedef struct {
 *   char sysid[7];        System id
 *   char strid[7];        Stream id
 *   
 *   int sample_rate;      Sample rate
 *   int format;           format (1=32/24bit,2=16bit,4=8bit)
 *   int records;          records (number of 4 byte words)
 *   int samples;          number of samples
 *   int ttl;              filter coeficient information
 * 
 *   G2GTime start;        Time of first sample of block
 *   G2GTime end;          Time of first sample after the end of block
 * 
 *   int32_t cric;         The calculated RIC
 *   int32_t oric;         The RIC contained in the block
 * } G2PBlockH;
 * 
 */

    G2PBlockH pbh;

/* G2ParseBlockHead, parses the header of a block into G2PBlockH */
/* it returns  0 on success. All relevant feilds are filled in  */
/* except cric */
    if (G2ParseBlockHead (&b, &pbh))
      {
        fprintf (stderr, "G2ParseBlockHead(&b,&pbh) failed\n");
        exit (1);
      }

/* G2DumpPBlockH, dumps the parsed block header in a human readable form */
/* to a stdio FILE * */
    printf ("G2DumpPBlockH(stdout,&pbh)={\n");
    G2DumpPBlockH (stdout, &pbh);
    printf ("}\n");

  }

  {
/*A G2PBlock contains a parsed block, note that you can cast a pointer */
/*to a G2PBlock to pointer to G2PBlockH */
/*
 *
 * typedef struct {
 *   char sysid[7];        System id
 *   char strid[7];        Stream id
 *   
 *   int sample_rate;      Sample rate
 *   int format;           format (1=32/24bit,2=16bit,4=8bit)
 * 
 *   int records;          records (number of 4 byte words)
 * 
 *   int samples;          number of samples
 * 
 *   int ttl;              filter coeficient information
 * 
 *   G2GTime start;        Time of first sample of block
 *   G2GTime end;          Time of first sample after the end of block
 * 
 *   int32_t cric;         The calculated RIC
 *   int32_t oric;         The RIC contained in the block
 *   
 *   union {
 *   int32_t data[1024];   Data from the block
 *   char status[1024];    or status information in the block
 *   } d;
 * 
 * } G2PBlock;
 *
 */

    G2PBlock pb;

    if (G2ParseBlock (&b, &pb))
      {
        fprintf (stderr, "G2ParseBlock(&b,&pb) failed\n");
        exit (1);
      }
/* G2DumpPBlock, dumps the parsed block in a human readable form */
/* to a stdio FILE * */
    printf ("G2DumpPBlock(stdout,&pb)={\n");
    G2DumpPBlock (stdout, &pb);
    printf ("}\n");

  }

  return 0;
}
