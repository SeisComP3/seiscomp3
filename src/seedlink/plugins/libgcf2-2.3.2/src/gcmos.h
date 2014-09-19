/*
 * gcmos.h:
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

/*
 * $Id: gcmos.h,v 1.6 2004/04/20 22:31:47 root Exp $
 */

/*
 * $Log: gcmos.h,v $
 * Revision 1.6  2004/04/20 22:31:47  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/20 20:43:30  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/04/20 20:09:12  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/20 19:04:44  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/20 16:26:48  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/04/20 15:39:08  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/20 13:06:21  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/04/20 13:05:41  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/14 15:39:52  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/13 15:42:03  root
 * *** empty log message ***
 *
 */

#ifndef __GCMOS_H__
#define __GCMOS_H__

#include <gtime.h>

/*Canto III:9 Lasciate ogne speranza, voi ch'intrate*/

/* There is currently no generic or standardized way to get or set the state of
 * a DM - (we are fixing this) 
 */

/* To set a vast array of forth commands are used, getting is more tricky,
 * currently the solution involves fetching the non-volatile memory of the DM
 * and parsing it. 
 */

/* Of course this requires we know a lot more about the DM than we ought
 * G2CmosDecode attempts a best effort and sticks the result in the structure
 * below
 */

#define G2_DM24_IT_V	0
#define G2_DM24_IT_N	0
#define G2_DM24_IT_E	0

typedef struct
{
  uint8_t status;             /* ? */
  uint8_t hundredths;
  G2UTC time;
  uint8_t timer;              /* ? */
} G2DM24Time; 
 
typedef struct
{
  G2DM24Time rtc;		/* RTC */
  uint32_t sysid;               /* base 36 system id */
  uint32_t serial;              /* base 36 serial num */
  uint32_t pwm2;                /* ? */
  uint32_t channels[8];         /* channel bitmap indexd by taps.
                                   *channel 0 is bit 0 etc
                                   *usually channel 0 is V
                                 */
  int tx_bauds[8];              /* tx baud rates per port */
  int rx_bauds[8];              /* rx baud rates per port */
  uint16_t aux_channels;        /* channel bitmap for auxillary 
                                   *channels bit 8 is usually
                                   *V mass pos */
  uint8_t masses[4];            /* Last mass recorded mass possitions */
  uint32_t orientation;         /* ? */
  uint16_t reboots;             /* Number of system reboots */
  G2DM24Time lastboot;		/* Last boot time */

  int decimations[4];           /* Decimations between taps */
				/* 1=>2, 2=>3, 4, 5, 8,, 10, 7=>16 */
  int samplerates[4];           /* Sample rates of taps */
#define G2_DM24_IT_40T	1
#define G2_DM24_IT_ESP	2
#define G2_DM24_IT_3T	3
#define G2_DM24_IT_3TD	4
#define G2_DM24_IT_6TD	5
#define G2_DM24_IT_5TD	6
#define G2_DM24_IT_MAX  7
  int instrument_type;          /* type of connected sensor see above */

  uint8_t stc[6];               /* Short term constants use G2_DM25_IT_V etc. */
  uint8_t ltc[6];               /* Long term constants */
  uint8_t ratio[6];             /* Ratio */
#define G2_DM24_FT_NONE		0
#define G2_DM24_FT_0__1		0
#define G2_DM24_FT_0__0_9	1
#define G2_DM24_FT_0_2__0_9	2
#define G2_DM24_FT_0_5__0_9	3
  uint8_t trigger_filter;       /* Which triggering filter to use see above */
  uint8_t filter_tap;           /* Which tap to feed triggering filters from */
  uint32_t triggered_channels[8]; /* channel bitmap to use when triggered */
  uint8_t triggers[4];          /* ? */
  uint8_t pretrig;              /* amount of data to record before 
                                   *trigger in seconds */
  uint8_t posttrig;             /* amount of data to record after 
                                   *trigger in seconds */
  uint16_t checksum2;
  uint16_t flash_file_size;     /* flash in which units? */
  uint16_t flash_mode;          /* ? */
#define G2_DM24_SS_NONE		0
#define G2_DM24_SS_TRIMBLE	1
#define G2_DM24_SS_GARMIN	2
#define G2_DM24_SS_STREAM	3
#define G2_DM24_SS_MAX		4
  uint8_t sync_src;             /* Timing source see above */
  uint16_t heartbeat;           /* heartbeat interval in units of 30ms */
  uint16_t acknak_wait;         /* time to wait for ack/nak in units of 30ms */
  uint8_t stopbits;             /* number of stop bits, 4 means use CTS? */
  uint8_t once;                 /* non zero -> write once, otherwise use */
  				/* storage as ring buffer */
  uint8_t split;                /* ? */
  uint16_t gps_duty;            /* in minuites % 60 = 0 */
  uint8_t auto_center;          /* 0-250, 255==off */
} G2CmosDM24;

typedef struct {
  int len;
  uint8_t cmos[0x180];
} G2CmosRaw;

int G2CmosLen(char *response);
int G2CmosParse(char *response, G2CmosRaw *out);
int G2CmosDecodeDM24(G2CmosRaw *in, G2CmosDM24 *out);

#endif
