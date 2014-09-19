/*
 * gblockp.h:
 *
 * Copyright (c) 2003 James McKenzie <james@fishsoup.dhs.org>,
 * All rights reserved.
 *
 */

/*
 * $Id: gblockp.h,v 1.6 2006/09/14 14:28:11 lwithers Exp $
 */

/*
 * $Log: gblockp.h,v $
 * Revision 1.6  2006/09/14 14:28:11  lwithers
 * Add G2GetDecLut() and the machinery for the new G2PBlock.dig_type field.
 *
 * Revision 1.5  2004/06/29 14:27:57  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/06/29 13:44:04  root
 * Major incompatability gpblockh size changes
 *
 * Revision 1.3  2004/06/29 12:27:11  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/16 16:19:49  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:23  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/14 16:31:49  root
 * #
 *
 * Revision 1.1  2003/05/13 15:10:08  root
 * #
 *
 */

#ifndef __GBLOCKP_H__
#define __GBLOCKP_H__

#include <stdio.h>

#define G2BLOCKTYPEUNKNOWN  -1
#define G2BLOCKTYPESTATUS   0
#define G2BLOCKTYPEDATA     1
#define G2BLOCKTYPECDSTATUS 2

#define G2DIGTYPEUNKNOWN        0x00
#define G2DIGTYPEMK2            0x10
#define G2DIGTYPEMK3            0x20

typedef struct
{
  int type;

  char sysid[7];
  char strid[7];

  int sample_rate;
  int format;
  int records;
  int samples;
  int dig_type;
  int ttl;

  G2GTime start;
  G2GTime end;

}
G2PBlockH;

typedef struct 
{
  int gps_fix;	 /* gps state: 0 = no coms, '1' = no fix, '2' = 2-d, '3' = 3-d */
  int gps_mode;  /* gps mode: 'A' = auto , 'M' = manual (from NMEA-0183 message)*/
  int gps_control; /* ??? FIXME XXX: 0=off, 0xff=controlling?*/
  int gps_power; /* gps power: 0 = off, 0xff=on */
  int32_t gps_offset; /*Measured offset between GPS and pwm clock in units of 500ns */
  int32_t busy_counter; /*A ~1kHz counter that counts down to zero, as calibration, */
			/*or locking is in progress */
  int	locking;
  int 	unlocking;
  int   centering;
  int   cal[3]; /*Calibration in progress V, N, E */
} G2CDStatus;


typedef struct
{
  int type;

  char sysid[7];
  char strid[7];

  int sample_rate;
  int format;
  int records;
  int samples;
  int dig_type;
  int ttl;

  G2GTime start;
  G2GTime end;

  int32_t cric;
  int32_t tric;

  union
  {
    int32_t data[1024];
    char status[1024];
    G2CDStatus cdstatus;
  }
  d;

}
G2PBlock;

extern int G2ParseBlockHead (G2Block * in, G2PBlockH * out);
extern int G2ParseBlock (G2Block * in, G2PBlock * out);
extern void G2DumpPBlock (FILE * s, G2PBlock * p);
extern void G2DumpPBlockH (FILE * s, G2PBlockH * ph);
extern char *G2BlockId(G2PBlockH * ph);


#endif /* __GBLOCKP_H__ */
