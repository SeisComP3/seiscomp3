/************************************************************************/
/*  Routines for processing DRM Quanterra data.				*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2002 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef lint
static char sccsid[] = "$Id: drm_utils.c 2 2005-07-26 19:28:46Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "qdefines.h"
#include "msdatatypes.h"
#include "timedef.h"
#include "qsteim.h"
#include "sdr.h"
#include "data_hdr.h"
#include "qtime.h"
#include "qutils.h"
#include "qda_utils.h"
#include "drm_utils.h"
#include "sdr_utils.h"

#ifdef	QLIB_DEBUG
extern FILE *info;		/*:: required only for debugging	*/
extern int  debug_option;	/*:: required only for debugging	*/
#endif

/************************************************************************/
/*  NOTE:   DRM files are processed ONLY if the are in SEED_BIG_ENDIAN.	*/
/************************************************************************/

/************************************************************************/
/*  decode_time_drm:							*/
/*	Convert from DRM format time to INT_TIME.			*/
/*  return:								*/
/*	INT_TIME containing time from input time structure.		*/
/************************************************************************/
INT_TIME decode_time_drm 
   (DA_TIME	dt,		/* DRM time structure containing time.	*/
    int		wordorder)	/* wordorder of time contents.		*/
{
    DA_TIME	ct = dt;
    EXT_TIME	et;

    if (my_wordorder < 0) get_my_wordorder();
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&ct.millisec);
    }
#ifdef	QLIB_DEBUG
    if (debug_option & 128) 
    fprintf (info, "time = %02d/%02d/%02d %02d:%02d:%02d:%06d\n",
	     ct.time_sample[0], ct.time_sample[1], ct.time_sample[2],
	     ct.time_sample[3], ct.time_sample[4], ct.time_sample[5],
	     ct.millisec*USECS_PER_MSEC);
#endif

    /*	KLUDGE to add in century.					*/
    /*	Assume NOT data before 1970.					*/
    /*	This code will BREAK on 78 years...				*/
    et.year = ct.time_sample[0];
    if (et.year < 70)	et.year +=2000;
    else if (et.year < 100)	et.year +=1900;
	
    et.month = ct.time_sample[1];
    et.day = ct.time_sample[2];
    et.hour = ct.time_sample[3];
    et.minute = ct.time_sample[4];
    et.second = ct.time_sample[5];
    et.usec = ct.millisec*USECS_PER_MSEC;
    et.doy = mdy_to_doy(et.month,et.day,et.year);
    return (normalize_time(ext_to_int(et)));
}

/************************************************************************/
/*  encode_time_drm:							*/
/*	Convert from INT_TIME to DRM format time.			*/
/*  return:								*/
/*	DA_TIME containing time from input time structure.		*/
/************************************************************************/
DA_TIME encode_time_drm 
   (INT_TIME	it,		/* IN_TIME structure to decode.		*/
    int		wordorder)	/* wordorder for encoded time contents.	*/
{
    DA_TIME	dt;
    EXT_TIME	et;

    if (my_wordorder < 0) get_my_wordorder();
    et = int_to_ext (it);
    dt.time_sample[0] = et.year % 100;
    dt.time_sample[1] = et.month;
    dt.time_sample[2] = et.day;
    dt.time_sample[3] = et.hour;
    dt.time_sample[4] = et.minute;
    dt.time_sample[5] = et.second;
    dt.millisec = et.usec / USECS_PER_MSEC;
#ifdef	QLIB_DEBUG
    if (debug_option & 128) 
    fprintf (info, "time = %02d/%02d/%02d %02d:%02d:%02d:%06d\n",
	     dt.time_sample[0], dt.time_sample[1], dt.time_sample[2],
	     dt.time_sample[3], dt.time_sample[4], dt.time_sample[5],
	     dt.millisec*USECS_PER_MSEC);
#endif
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&dt.millisec);
    }
    return (dt);
}

/************************************************************************/
/*  decode_flags_drm:							*/
/*	Create SEED flags from DRM SOH flag.				*/
/************************************************************************/
void decode_flags_drm
   (int		*pclock,	/* ptr to clock correction (output).	*/
    int		soh,		/* state_of_health flag.		*/
    unsigned char *pa,		/* ptr to activity flag (output);	*/
    unsigned char *pi,		/* ptr to i/o flag (output).		*/
    unsigned char *pq,		/* ptr to quality flag (output).	*/
    int		wordorder)	/* wordorder for encoded time contents.	*/
{
    /* The DRM flags should be the same as the QDA flags.		*/
    /* Therefore, just call that routine.				*/
    decode_flags_qda (pclock, soh, pa, pi, pq, wordorder);
}

/************************************************************************/
/*  encode_flags_drm:							*/
/*	Create DRM SOH flag from SEED flags.				*/
/************************************************************************/
void encode_flags_drm
   (int		old_soh,	/* old state_of_health flag (input).	*/
    unsigned char *soh,		/* state_of_health flag (output).	*/
    unsigned char pa,		/* activity flag (input).		*/
    unsigned char pi,		/* i/o flag (input).			*/
    unsigned char pq,		/* quality flag (input).		*/
    int		wordorder)	/* wordorder for encoded time contents.	*/
{
    /* The DRM flags should be the same as the QDA flags.		*/
    /* Therefore, just call that routine.				*/
    encode_flags_qda (old_soh, soh, pa, pi, pq, wordorder);
}

/************************************************************************/
/*  decode_hdr_drm:							*/
/*	Decode DRM header stored with each DRM data block,		*/
/*	and return ptr to dynamically allocated DATA_HDR structure.	*/
/*	Fill in structure with the information in a easy-to-use format.	*/
/*	WARNING:  The station_id, location_id, and channel_id are	*/
/*	NOT AVAILABLE in the block header, and are therefore are not	*/
/*	filled in at this point.  They MUST be filled in by the caller.	*/
/************************************************************************/
DATA_HDR *decode_hdr_drm 
   (STORE_DATA	*ihdr,		/* ptr to raw DRM header.		*/
    int		maxbytes)	/* max # bytes in buffer.		*/
{
    DATA_HDR	*ohdr;
    int		seconds, usecs;
    int		blksize = 0;
    int		itmp[2];
    short int	stmp[2];
    unsigned short int ustmp[2];
    

    /* Perform data integrity check, and pick out pertinent header info.*/
    if (my_wordorder < 0) get_my_wordorder();
    qlib2_errno = 0;

    blksize = 512;		/* drm data always had blksize of 512.	*/

    if ((ohdr = new_data_hdr())==NULL) return(NULL);
    ohdr->record_type = default_data_hdr_ind;    
    ohdr->hdr_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/
    ohdr->data_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/

    itmp[0] = ihdr->packet_seq;
    if (my_wordorder != ohdr->hdr_wordorder) swab4(&itmp[0]);
    ohdr->seq_no = itmp[0];

    ohdr->begtime = decode_time_drm (ihdr->da_begtime, ohdr->hdr_wordorder);
    ohdr->hdrtime = decode_time_drm (ihdr->da_begtime, ohdr->hdr_wordorder);
    stmp[0] = ihdr->num_samples;
    if (my_wordorder != ohdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->num_samples = stmp[0];
    ohdr->sample_rate = ihdr->rate;
    ohdr->sample_rate_mult = 1;

    /* Stream,  channel, location and network are NOT in the block	*/
    /* header but only in the file header.  They will be left empty.	*/
    /* The caller should fill them in.					*/

    ohdr->num_blockettes = 0;
    stmp[0] = ihdr->clock_corr;
    if (my_wordorder != ohdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->num_ticks_correction = stmp[0] * TICKS_PER_MSEC;
    ohdr->first_data = (char *)&ihdr->da_d[0][0].bdiff[0] - (char *)ihdr;
    ohdr->first_blockette = 0;
    ohdr->pblockettes = NULL;

    /*	NOTE: store original clock_corr and SOH in extra header storage	*/
    /*	for possible future format-specific use.			*/
    ohdr->xm1 = ihdr->clock_corr;
    ohdr->xm2 = ihdr->soh;

    decode_flags_drm (&ohdr->num_ticks_correction, ihdr->soh, 
		&ohdr->activity_flags, &ohdr->io_flags, 
		&ohdr->data_quality_flags, ohdr->hdr_wordorder);

    /*	There should never be any time correction since any correction 	*/
    /*	is already included in the beginning and end time.		*/
    ohdr->num_ticks_correction = 0;

    /*	Calculate the end time, since the value stored in the field has	*/
    /*	only millisecond resolution.  This prevents us from using the 	*/
    /*	end time stored in the DRM header.				*/
    time_interval2(ohdr->num_samples - 1, ohdr->sample_rate, ohdr->sample_rate_mult, 
		   &seconds, &usecs);
    ohdr->endtime = add_time (ohdr->begtime, seconds, usecs);
    /* Assume all QDA data is STEIM1 data.				*/
    ohdr->data_type = STEIM1;
    ohdr->num_data_frames = (ohdr->blksize-ohdr->first_data)/sizeof(FRAME);
    ohdr->data_wordorder = SEED_BIG_ENDIAN;
    ohdr->blksize = blksize;
    return(ohdr);
}

/************************************************************************/
/*  encode_hdr_drm:							*/
/*	Convert DATA_HDR back to DRM block header.			*/
/************************************************************************/
STORE_DATA *encode_hdr_drm
   (DATA_HDR	*ihdr)		/* ptr to input DATA_HDR.		*/
{
    STORE_DATA	*ohdr;
    int		itmp[2];
    short int	stmp[2];
    unsigned short int ustmp[2];

    /* Perform data integrity check, and pick out pertinent header info.*/
    if (my_wordorder < 0) get_my_wordorder();
    qlib2_errno = 0;

    ihdr->hdr_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/
    ihdr->data_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/
    if ((ohdr = (STORE_DATA *)malloc(sizeof(STORE_DATA)))==NULL) return(NULL);
    memset ((void *)ohdr, 0,  sizeof(STORE_DATA));
    itmp[0] = ihdr->seq_no;
    if (my_wordorder != ihdr->hdr_wordorder) swab4(&itmp[0]);
    ohdr->packet_seq = itmp[0];

    /*	Since clock correction is assumed to already added in,		*/
    /*	use begtime instead of hdrtime for the beginning time.		*/
    ohdr->da_begtime = encode_time_drm(ihdr->begtime, ihdr->hdr_wordorder);
    ohdr->da_endtime = encode_time_drm(ihdr->endtime, ihdr->hdr_wordorder);
    stmp[0] = ihdr->num_samples;;
    if (my_wordorder != ihdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->num_samples = stmp[0];
    ohdr->rate = ihdr->sample_rate;

    /*	See comment in decode_hdr_drm concerning extra info.		*/
    stmp[0] = ihdr->xm1;
    if (my_wordorder != ihdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->clock_corr = stmp[0];
    encode_flags_drm (ihdr->xm2, &ohdr->soh, 
		ihdr->activity_flags, ihdr->io_flags, 
		ihdr->data_quality_flags, ihdr->hdr_wordorder);
    return(ohdr);
}

/************************************************************************/
/*  is_drm_header:							*/
/*	Determine whether the buffer contains a DRM record header.	*/
/*  return:								*/
/*	1 if true, 0 otherwise.						*/
/************************************************************************/
int is_drm_header 
   (STORE_FILE_HEAD *p,		/* ptr to buffer containing header.	*/
    int		nr)		/* max number of bytes for header.	*/
{
    int itmp;
    int status = 0;
    if (nr >= DRM_FILE_HDR_SIZE) {
	itmp = p->store_magic;
	if (my_wordorder < 0) get_my_wordorder();
	if (my_wordorder != SEED_BIG_ENDIAN) swab4(&itmp);
	status = (itmp == STORE_MAGIC );
    }
    return (status);
}
