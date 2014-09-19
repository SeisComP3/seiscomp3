/************************************************************************/
/*  Routines for processing native binary dialup (QDA) Quanterra data.	*/
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
static char sccsid[] = "$Id: qda_utils.c 2 2005-07-26 19:28:46Z andres $ ";
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
#include "sdr_utils.h"

#define	RECORD_HEADER_1	1

#ifdef	QLIB_DEBUG
extern FILE *info;		/*:: required only for debugging	*/
extern int  debug_option;	/*:: required only for debugging	*/
#endif

/************************************************************************/
/*  NOTE:   DRM files are processed ONLY if the are in SEED_BIG_ENDIAN.	*/
/************************************************************************/

/************************************************************************/
/*  Table used to map stream numbers to common stream names.		*/
/*  This mapping is currently hard-coded in the Quanterra software.	*/
/************************************************************************/
static char *stream_name[] = {
    "VBB", "VSP", "LG", "MP", "VP", "VLP", "ULP", NULL };


/************************************************************************/
/*  Table used to map channel number to common component name.		*/
/*  This really needs to be externally table driven, based on station.	*/
/************************************************************************/
static char *component_name[] = {
    "Z", "N", "E", "V4", "V5", "V6", "M1", "M2", "M3",
    "Z", "N", "E", "L4", "L5", "L6", "L7", "L8", "L9", 
    "R1", "R2", NULL };

/************************************************************************/
/*  get_component_name:							*/
/*	Return the component name bases on the station and component	*/
/*	number.								*/
/************************************************************************/
char *get_component_name
   (char	*station,	/* station name.			*/
    int		comp)		/* component number.			*/
{
    return (component_name[comp]);
}

/************************************************************************/
/*  decode_time_qda:							*/
/*	Convert from QDA format time to INT_TIME.			*/
/************************************************************************/
INT_TIME decode_time_qda
   (QDA_TIME	qt,		/* input time in QDA format.		*/
    short int	millisecond,	/* input time (milliseconds).		*/
    int		wordorder)	/* wordorder of time contents.		*/
{
    INT_TIME	it;
    EXT_TIME	et;
    short int	stmp;

    if (my_wordorder < 0) get_my_wordorder();
    /*	KLUDGE to add in century.					*/
    /*	Assume NOT data before 1970.					*/
    /*	This code will BREAK on 78 years...				*/
    et.year = qt.year;
    if (et.year < 70)	et.year +=2000;
    else if (et.year < 100)	et.year +=1900;
	
    et.month = qt.month;
    et.day = qt.day;
    et.hour = qt.hour;
    et.minute = qt.minute;
    et.second = qt.second;
    stmp = millisecond;
    if (my_wordorder != SEED_BIG_ENDIAN) swab2(&stmp);
    et.usec = (int)stmp * USECS_PER_MSEC;
    et.doy = mdy_to_doy(et.month,et.day,et.year);
    it = normalize_time(ext_to_int(et));
#ifdef	QLIB_DEBUG
    if (debug_option & 128) 
    fprintf (info, "time = %02d/%02d/%02d %02d:%02d:%02d:%06d\n",
	     qt.year,	qt.month,   qt.day,
	     qt.hour,	qt.minute,  qt.second,
	     et.usec);
#endif
    return (it);
}

/************************************************************************/
/*  decode_flags_qda:							*/
/*	Create SEED flags from QDA SOH flag.				*/
/************************************************************************/
void decode_flags_qda
   (int		*pclock,	/* ptr to clock correction (output).	*/
    int		soh,		/* state_of_health flag.		*/
    unsigned char *pa,		/* ptr to activity flag (output);	*/
    unsigned char *pi,		/* ptr to i/o flag (output).		*/
    unsigned char *pq,		/* ptr to quality flag (output).	*/
    int		wordorder)	/* wordorder of time contents.		*/
{
    *pa = 0;
    *pi = 0;
    *pq = 0;

    /*	Updated 01/23/91 to be consistent with Quanterra's processing	*/
    /*	of SOH -> SEED flags.						*/

    /*	ACTIVITY flags:							*/
    if (soh & SOH_BEGINNING_OF_EVENT) *pa |= ACTIVITY_BEGINNING_OF_EVENT;
    if (soh & SOH_CAL_IN_PROGRESS) *pa |= ACTIVITY_CALIB_PRESENT;
    if (soh & SOH_EVENT_IN_PROGRESS) *pa |= ACTIVITY_EVENT_IN_PROGRESS;

    /*	IO flags:   They have no mapping.				*/

    /* If there is a clock correction, it is already added in		*/
    /* (according to Joe Steim).					*/
    /* This means that we should be able to IGNORE it.			*/
    /* In fact, we currently MUST zero it, since otherwise we must	*/
    /* set the ACTIVITY_TIME_CORR_APPLIED flag to indicated that it 	*/
    /* has been added in already.					*/
    *pclock = 0;

    /*	QUALITY flags:							*/
    /* Map other information into QUALITY_QUESTIONABLE_TIMETAG flag.	*/
    if (soh & SOH_GAP) *pq |= QUALITY_MISSING;
    if (soh & SOH_INACCURATE) *pq |= QUALITY_QUESTIONABLE_TIMETAG;
    if (((soh ^ SOH_EXTRANEOUS_TIMEMARKS) & 
	(SOH_EXTRANEOUS_TIMEMARKS | SOH_EXTERNAL_TIMEMARK_TAG)) == 0)
	*pq |= QUALITY_QUESTIONABLE_TIMETAG;

    /*	We can't do anything with					*/
    /*		SOH_EXTERNAL_TIMEMARK_TAG				*/
    /*		SOH_RECEPTION_GOOD					*/
    /*	since they don't appear to be turned on in every packet.	*/
    /*	Their absence does NOT appear to indicate inaccurate timing.	*/

    /* PUNT on ACTIVITY_END_OF_EVENT flag for the moment.		*/
    /*	It requires history.						*/

    return;
}

/************************************************************************/
/*  encode_flags_qda:							*/
/*	Create QDA SOH flag from SEED flags.				*/
/************************************************************************/
void encode_flags_qda 
   (int		old_soh,	/* old state_of_health flag (input).	*/
    unsigned char *soh,		/* state_of_health flag (output).	*/
    unsigned char pa,		/* activity flag (input).		*/
    unsigned char pi,		/* i/o flag (input).			*/
    unsigned char pq,		/* quality flag (input).		*/
    int		wordorder)	/* wordorder of time contents.		*/
{
    /*	Updated 01/23/91 to be consistent with Quanterra's processing	*/
    /*	of SOH -> SEED flags.						*/

    /*	ACTIVITY flags:							*/
    if (pa & ACTIVITY_BEGINNING_OF_EVENT) *soh |=  SOH_BEGINNING_OF_EVENT;
    if (pa & ACTIVITY_CALIB_PRESENT) *soh |= SOH_CAL_IN_PROGRESS;
    if (pa & ACTIVITY_EVENT_IN_PROGRESS) *soh |= SOH_EVENT_IN_PROGRESS;

    /*	IO flags:   They have no mapping.				*/

    /*	QUALITY flags:							*/
    /* Map other information into QUALITY_QUESTIONABLE_TIMETAG flag.	*/
    if (pq & QUALITY_MISSING) *soh |= SOH_GAP;

    /*	I'm not sure how to remap the QUALITY_QUESTIONALBLE_TIMETAG.	*/
    /*	There is not a 1-1 mapping between it and a single SOH bit.	*/
    /*	Just map it to SOH_INACCURATE.					*/
    if (pq & QUALITY_QUESTIONABLE_TIMETAG) 
	*soh |= (old_soh & (SOH_INACCURATE | SOH_EXTRANEOUS_TIMEMARKS));
    *soh |= (old_soh & (SOH_RECEPTION_GOOD | SOH_EXTERNAL_TIMEMARK_TAG));

    /*	We can't do anything with					*/
    /*		SOH_EXTERNAL_TIMEMARK_TAG				*/
    /*		SOH_RECEPTION_GOOD					*/
    /*	since they don't appear to be turned on in every packet.	*/
    /*	Their absence does NOT appear to indicate inaccurate timing.	*/

    /* PUNT on ACTIVITY_END_OF_EVENT flag for the moment.		*/
    /*	It requires history.	*/

    return;
}

/************************************************************************/
/*  decode_hdr_qda:							*/
/*	Decode QDA header stored with each DRM data block,		*/
/*	and return ptr to dynamically allocated DATA_HDR structure.	*/
/*	Fill in structure with the information in a easy-to-use format.	*/
/************************************************************************/
DATA_HDR *decode_hdr_qda
   (QDA_HDR	*ihdr,		/* ptr to input header in QDA format.	*/
    int		maxbytes)	/* max # bytes in buffer.		*/
{
    DATA_HDR	*ohdr;
    char	*s, *c, *sc, *pc;
    int		i, next_seq;
    int		seconds, usecs;
    int		blksize = 0;
    int		itmp[2];
    short int	stmp[2];
    unsigned short int ustmp[2];

    /* Perform data integrity check, and pick out pertinent header info.*/
    if (my_wordorder < 0) get_my_wordorder();
    qlib2_errno = 0;

    if ( (ihdr->frame_type != RECORD_HEADER_1) ||
	(ihdr->header_flag != 0) ) {
	/*  Don't have a RECORD_HEADER_1.  See if the entire header is	*/
	/*  composed of NULLS.  If so, print warning and return NULL.	*/
	/*  Some early Quanterras output a spurious block with null	*/
	/*  header info every 16 blocks.  That block should be ignored.	*/
	if (allnull((char *)ihdr, sizeof(QDA_HDR))) {
	    return((DATA_HDR *)NULL);
	}
	else {
	    qlib2_errno = 1;
	    return((DATA_HDR *)NULL);
	}
    }

    if ((ohdr = new_data_hdr()) == NULL) return (NULL);
    ohdr->record_type = default_data_hdr_ind;    
    ohdr->hdr_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/
    ohdr->data_wordorder = SEED_BIG_ENDIAN;	/* WARNING - HARDCODED.	*/

    itmp[0] = ihdr->seq_no;
    if (my_wordorder != ohdr->hdr_wordorder) swab4(&itmp[0]);
    ohdr->seq_no = itmp[0];

    /*	Attempt to determine blocksize if current setting is 0.		*/
    /*	QDA files can be either 512 byte or 4K byte blocks.		*/
    if (blksize == 0) {
	for (i=1; i< 4; i++) {
	    pc = ((char *)(ihdr)) + (i*512);
	    if (pc - (char *)(ihdr) >= maxbytes) break;
	    if ( allnull ( pc,sizeof(QDA_HDR)) )
		continue;
	    next_seq = ((QDA_HDR *)pc)->seq_no;
	    if (next_seq == ohdr->seq_no + i) {
		blksize = 512;
		break;
	    }
	}
	/* Can't determine the blocksize.  Assume default.		*/
	if (blksize == 0) blksize = 4096;
    }
	
    charncpy (ohdr->station_id, ihdr->station_id, 4);
    charncpy (ohdr->location_id, "  ", 2);
    charncpy (ohdr->network_id, "  ", 2);
    trim (ohdr->station_id);
    trim (ohdr->location_id);
    trim (ohdr->network_id);

    ohdr->begtime = decode_time_qda(ihdr->time, ihdr->millisecond, 
				    ohdr->hdr_wordorder);
    ohdr->hdrtime = decode_time_qda(ihdr->time, ihdr->millisecond, 
				    ohdr->hdr_wordorder);

    stmp[0] = ihdr->num_samples;
    if (my_wordorder != ohdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->num_samples = stmp[0];
    ohdr->sample_rate = ihdr->sample_rate;
    ohdr->sample_rate_mult = 1;

    s = stream_name[ihdr->stream];
    c = get_component_name(ohdr->station_id, ihdr->component);
    comp_to_seed(s, c, &sc);
    if (sc != NULL) {
	charncpy (ohdr->channel_id, sc, 3);
	trim (ohdr->channel_id);
    }
    else {
/*::
	fprintf (stderr, "unable to determine seed stream from %s %s, sample rate = %d\n",
	    s, c, ohdr->sample_rate);
::*/
    }

    ohdr->num_blockettes = 0;
    stmp[0] = ihdr->clock_corr;
    if (my_wordorder != ohdr->hdr_wordorder) swab2(&stmp[0]);
    ohdr->num_ticks_correction = (int)stmp[0] * TICKS_PER_MSEC;
    ohdr->first_data = 64;
    ohdr->first_blockette = 0;
    ohdr->pblockettes = NULL;

    decode_flags_qda (&ohdr->num_ticks_correction, ihdr->soh, 
		&ohdr->activity_flags, &ohdr->io_flags, 
		&ohdr->data_quality_flags, ohdr->hdr_wordorder);

    /*	If the time correction has not already been added, we should	*/
    /*	add it to the begtime.  Do NOT change the ACTIVITY flag, since	*/
    /*	it refers to the hdrtime, NOT the begtime/endtime.		*/

    if ( ohdr->num_ticks_correction != 0 && 
	((ohdr->activity_flags & ACTIVITY_TIME_CORR_APPLIED) == 0) ) {
	ohdr->begtime = add_dtime (ohdr->begtime, 
				  (double)ohdr->num_ticks_correction * USECS_PER_TICK);
    }

    time_interval2(ohdr->num_samples - 1, ohdr->sample_rate, ohdr->sample_rate_mult,
		  &seconds, &usecs);
    ohdr->endtime = add_time (ohdr->begtime, seconds, usecs);
    ohdr->data_type = UNKNOWN_DATATYPE;
    ohdr->blksize = blksize;
    /* Assume all QDA data is STEIM1 data.				*/
    ohdr->data_type = STEIM1;
    ohdr->num_data_frames = (ohdr->blksize-ohdr->first_data)/sizeof(FRAME);
    ohdr->data_wordorder = SEED_BIG_ENDIAN;
    return(ohdr);
}

/************************************************************************/
/*  is_qda_header:							*/
/*	Determine whether the buffer contains a QDA record header.	*/
/*  return:								*/
/*	1 if true, 0 otherwise.						*/
/************************************************************************/
int is_qda_header 
   (QDA_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr)		/* max number of bytes for header.	*/
{
    return (nr >= QDA_HDR_SIZE && (p->frame_type == RECORD_HEADER_1) && 
	    (p->header_flag == 0));
}

