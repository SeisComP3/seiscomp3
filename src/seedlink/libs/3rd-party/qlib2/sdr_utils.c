/************************************************************************/
/*  Routines for processing SEED Data Record (SDR) Quanterra data.	*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2003 The Regents of the University of California.
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
static char sccsid[] = "$Id: sdr_utils.c 346 2006-04-11 20:49:47Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "qdefines.h"
#include "msdatatypes.h"
#include "timedef.h"
#include "qsteim.h"
#include "sdr.h"
#include "data_hdr.h"
#include "qtime.h"
#include "qutils.h"
#include "sdr_utils.h"
#include "ms_utils.h"

#ifdef	QLIB_DEBUG
extern FILE *info;		/*:: required only for debugging	*/
extern int  debug_option;	/*:: required only for debugging	*/
#endif

/************************************************************************/
/*  decode_time_sdr:							*/
/*	Convert from SDR format time to INT_TIME.			*/
/*  return:								*/
/*	INT_TIME structure.						*/
/************************************************************************/
INT_TIME decode_time_sdr
   (SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder)	/* wordorder of time contents.		*/
{
    SDR_TIME ct = st;
    EXT_TIME et;

    if (my_wordorder < 0) get_my_wordorder();
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&ct.year);
	swab2 ((short int *)&ct.day);
	swab2 ((short int *)&ct.ticks);
    }
#ifdef	QLIB_DEBUG
    if (debug_option & 128) 
    fprintf (info, "time = %02d.%02d %02d:%02d:%02d:%04d\n",
	     ct.year,	ct.day,	    ct.hour,
	     ct.minute,	ct.second,  ct.ticks);
#endif
    et.year = ct.year;
    et.doy = ct.day;
    et.hour = ct.hour;
    et.minute = ct.minute;
    et.second = ct.second;
    et.usec = ct.ticks * USECS_PER_TICK;
    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
    return (normalize_time(ext_to_int(et)));
}

/************************************************************************/
/*  encode_time_sdr:							*/
/*	Convert from INT_TIME to SDR format time.			*/
/*  return:								*/
/*	SDR_TIME structure.						*/
/************************************************************************/
SDR_TIME encode_time_sdr
   (INT_TIME	it,		/* IN_TIME structure to decode.		*/
    int		wordorder)	/* wordorder for encoded time contents.	*/
{
    SDR_TIME st;
    EXT_TIME et = int_to_ext(it);

    if (my_wordorder < 0) get_my_wordorder();
    st.year = et.year;
    st.day = et.doy;
    st.hour = et.hour;
    st.minute = et.minute;
    st.second = et.second;
    st.pad = 0;
    st.ticks = et.usec / USECS_PER_TICK;
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&st.year);
	swab2 ((short int *)&st.day);
	swab2 ((short int *)&st.ticks);
    }
    return (st);
}

/************************************************************************/
/*  decode_hdr_sdr:							*/
/*	Decode SDR header stored with each SDR data block,		*/
/*	and return ptr to dynamically allocated DATA_HDR structure.	*/
/*	Fill in structure with the information in a easy-to-use format.	*/
/*	Skip over vol_hdr record, which may be on Quanterra Ultra-Shear	*/
/*	tapes.								*/
/*  return:								*/
/*	DATA_HDR pointer on success.					*/
/*	NULL on failure.						*/
/************************************************************************/
DATA_HDR *decode_hdr_sdr
   (SDR_HDR	*ihdr,		/* input SDR header.			*/
    int		maxbytes)	/* max # bytes in buffer.		*/
{
    char tmp[80];
    DATA_HDR *ohdr;
    BS *bs;			/* ptr to blockette structure.		*/
    char *p;
    char *pc;
    int i, next_seq;
    int seconds, usecs;
    int swapflag;
    int		itmp[2];
    short int	stmp[2];
    unsigned short int ustmp[2];
    int tmp_wordorder;

    qlib2_errno = 0;
    if (my_wordorder < 0) get_my_wordorder();

    /* Perform data integrity check, and pick out pertinent header info.*/
    if (! (is_data_hdr_ind (ihdr->data_hdr_ind) || is_vol_hdr_ind (ihdr->data_hdr_ind))) {
	/*  Don't have a data header.  See if the entire header is	*/
	/*  composed of NULLS.  If so, print warning and return NULL.	*/
	/*  Some early Quanterras output a spurious block with null	*/
	/*  header info every 16 blocks.  That block should be ignored.	*/
	if (allnull((char *)ihdr, sizeof(SDR_HDR))) {
	    return ((DATA_HDR *)NULL);
	}
	else {
	    qlib2_errno = 1;
	    return ((DATA_HDR *)NULL);
	}
    }

    if ((ohdr = new_data_hdr()) == NULL) return (NULL);
    ohdr->record_type = ihdr->data_hdr_ind;
    ohdr->seq_no = atoi (charncpy (tmp, ihdr->seq_no, 6) );

    /* Handle volume header.					    */
    /* Return a pointer to a DATA_HDR structure containing blksize. */
    /* Save actual blockette for later use.			    */
    if (is_vol_hdr_ind(ihdr->data_hdr_ind)) {
	/* Get blksize from volume header.			    */
	p = (char *)ihdr+8;
	ohdr->blksize = 4096;	/* default tape blksize.	    */
	/* Put volume blockette number in data_type field.	    */
	ohdr->data_type = atoi (charncpy (tmp, p, 3));
	switch (ohdr->data_type) {
	  int ok;
	  case 5:
	  case 8:
	  case 10:
	    ohdr->blksize = (int)pow(2.0,atoi(charncpy(tmp,p+11,2)));
	    ok = add_blockette (ohdr, p, ohdr->data_type, 
			   atoi(charncpy(tmp,p+3,4)), my_wordorder, 0);
	    if (! ok) {
		qlib2_errno = 2;
		free_data_hdr(ohdr);
		return ((DATA_HDR *)NULL);
	    }
	    break;
	  default:
	    break;
	}
	return (ohdr);
    }

    /* Determine word order of the fixed record header.			*/
    tmp_wordorder = wordorder_from_time((unsigned char *)&(ihdr->time));
    if (tmp_wordorder < 0) {
	qlib2_errno = 3;
	free_data_hdr (ohdr);
	return ((DATA_HDR *)NULL);
    }
    ohdr->hdr_wordorder = tmp_wordorder;
    ohdr->data_wordorder = ohdr->hdr_wordorder;
    swapflag = (ohdr->hdr_wordorder != my_wordorder);
    charncpy (ohdr->station_id, ihdr->station_id, 5);
    charncpy (ohdr->location_id, ihdr->location_id, 2);
    charncpy (ohdr->channel_id, ihdr->channel_id, 3);
    charncpy (ohdr->network_id, ihdr->network_id, 2);
    trim (ohdr->station_id);
    trim (ohdr->location_id);
    trim (ohdr->channel_id);
    trim (ohdr->network_id);
    ohdr->hdrtime = decode_time_sdr(ihdr->time, ohdr->hdr_wordorder);
    if (swapflag) {
	/* num_samples.	*/
	ustmp[0] = ihdr->num_samples;
	swab2 ((short int *)&ustmp[0]);
	ohdr->num_samples = ustmp[0];
	/* data_rate	*/
	stmp[0] = ihdr->sample_rate_factor;
	stmp[1] = ihdr->sample_rate_mult;
	swab2 ((short int *)&stmp[0]);
	swab2 ((short int *)&stmp[1]);
	ohdr->sample_rate = stmp[0];
	ohdr->sample_rate_mult = stmp[1];
	/* num_ticks_correction. */
	itmp[0] = ihdr->num_ticks_correction;
	swab4 (&itmp[0]);
	ohdr->num_ticks_correction = itmp[0];
	/* first_data	*/
	ustmp[0] = ihdr->first_data;
	swab2 ((short int *)&ustmp[0]); 
	ohdr->first_data = ustmp[0];
	/* first_blockette */
	ustmp[1] = ihdr->first_blockette;
	swab2 ((short int *)&ustmp[1]);
	ohdr->first_blockette = ustmp[1];
    }
    else {
	ohdr->num_samples = ihdr->num_samples;
	ohdr->sample_rate = ihdr->sample_rate_factor;
	ohdr->sample_rate_mult = ihdr->sample_rate_mult;
	ohdr->num_ticks_correction = ihdr->num_ticks_correction;
	ohdr->first_data = ihdr->first_data;
	ohdr->first_blockette = ihdr->first_blockette;
    }

    /*	WARNING - may need to convert flags to independent format	*/
    /*	if we ever choose a different flag format for the DATA_HDR.	*/
    ohdr->activity_flags = ihdr->activity_flags;
    ohdr->io_flags = ihdr->io_flags;
    ohdr->data_quality_flags = ihdr->data_quality_flags;

    ohdr->num_blockettes = ihdr->num_blockettes;
    ohdr->data_type = 0;		/* assume unknown datatype.	*/
    ohdr->pblockettes = (BS *)NULL;	/* Do not parse blockettes here.*/

    if (ohdr->num_blockettes == 0) ohdr->pblockettes = (BS *)NULL;
    else {
	if (read_blockettes (ohdr, (char *)ihdr) != 1) {
	    free ((char *)ohdr);
	    return ((DATA_HDR *)NULL);
	}
    }

    /*	Process any blockettes that follow the fixed data header.	*/
    /*	If a blockette 1000 exists, fill in the datatype.		*/
    /*	Otherwise, leave the datatype as unknown.			*/
    ohdr->data_type = UNKNOWN_DATATYPE;
    ohdr->num_data_frames = -1;
    if ((bs=find_blockette(ohdr, 1000))) {
	/* Ensure we have proper output blocksize in the blockette.	*/
	BLOCKETTE_1000 *b1000 = (BLOCKETTE_1000 *) bs->pb;
	ohdr->data_type = b1000->format;
	ohdr->blksize = (int)pow(2.0,b1000->data_rec_len);
	ohdr->data_wordorder = b1000->word_order;
    }
    if ((bs=find_blockette(ohdr, 1001))) {
	/* Add in the usec99 field to the hdrtime.			*/
	BLOCKETTE_1001 *b1001 = (BLOCKETTE_1001 *) bs->pb;
	ohdr->hdrtime = add_time (ohdr->hdrtime, 0, b1001->usec99);
	ohdr->num_data_frames = b1001->frame_count;
    }

    /*	If the time correction has not already been added, we should	*/
    /*	add it to the begtime.  Do NOT change the ACTIVITY flag, since	*/
    /*	it refers to the hdrtime, NOT the begtime/endtime.		*/
    ohdr->begtime = ohdr->hdrtime;
    if ( ohdr->num_ticks_correction != 0 && 
	((ohdr->activity_flags & ACTIVITY_TIME_CORR_APPLIED) == 0) ) {
	ohdr->begtime = add_dtime (ohdr->begtime,
				   (double)ohdr->num_ticks_correction * USECS_PER_TICK);
    }
    /* Compute endtime.  Use precise sample interval in blockette 100.	*/
    /* For client convenience convert it to my_wordorder if not already.*/
    if ((bs=find_blockette(ohdr, 100))) {
	double actual_rate, dusecs;
        BLOCKETTE_100 *b = (BLOCKETTE_100 *) bs->pb;
	if (bs->wordorder != my_wordorder) {
	    swab_blockette (bs->type, bs->pb, bs->len);
	    bs->wordorder = my_wordorder;
	}
	actual_rate = b->actual_rate;
	dusecs = ((double)((ohdr->num_samples-1)/actual_rate))*USECS_PER_SEC;
	ohdr->endtime = add_dtime (ohdr->begtime,  dusecs);
	ohdr->rate_spsec = actual_rate;
    }
    else {
	time_interval2(ohdr->num_samples - 1, ohdr->sample_rate, ohdr->sample_rate_mult,
		       &seconds, &usecs);
	ohdr->endtime = add_time(ohdr->begtime, seconds, usecs);
    }

    /*	Attempt to determine blocksize if current setting is 0.		*/
    /*	We can detect files of either 512 byte or 4K byte blocks.	*/
    if (ohdr->blksize == 0) {
	for (i=1; i< 4; i++) {
	    pc = ((char *)(ihdr)) + (i*512);
	    if (pc - (char *)(ihdr) >= maxbytes) break;
	    if ( allnull ( pc,sizeof(SDR_HDR)) ) continue;
	    next_seq = atoi (charncpy (tmp, ((SDR_HDR *)pc)->seq_no, 6) );
	    if (next_seq == ohdr->seq_no + i) {
		ohdr->blksize = 512;
		break;
	    }
	}
	/* Can't determine the blocksize.  Assume default.		*/
	/* Assume all non-MiniSEED SDR data is in STEIM1 format.	*/
	/* Assume data_wordorder == hdr_wordorder.			*/
	if (ohdr->blksize == 0) ohdr->blksize = (maxbytes >= 1024) ? 4096 : 512;
	if (ohdr->num_samples > 0 && ohdr->sample_rate != 0) {
	    ohdr->data_type = STEIM1;
	    ohdr->num_data_frames = (ohdr->blksize-ohdr->first_data)/sizeof(FRAME);
	    ohdr->data_wordorder = ohdr->hdr_wordorder;
	}
    }

    /* Fill in num_data_frames, since there may not be a blockette 1001.*/
    if (IS_STEIM_COMP(ohdr->data_type) && ohdr->num_samples > 0 && 
	ohdr->sample_rate != 0 && ohdr->num_data_frames < 0) {
	ohdr->num_data_frames = (ohdr->blksize-ohdr->first_data)/sizeof(FRAME);
    }
	
    return (ohdr);
}

/************************************************************************/
/*  eval_rate:								*/
/*	Evaluate sample rate.						*/
/*	Return >0 if samples/second, <0 if seconds/sample, 0 if 0.	*/
/************************************************************************/
int eval_rate 
   (int	sample_rate_factor,	/* Fixed data hdr sample rate factor.	*/
    int	sample_rate_mult)	/* Fixed data hdr sample rate multiplier*/
{
    double drate;
    int rate;

    if (sample_rate_factor > 0 && sample_rate_mult > 0) 
	drate = (double)sample_rate_factor * (double)sample_rate_mult;
    else if (sample_rate_factor > 0 && sample_rate_mult < 0) 
	drate = -1. * (double)sample_rate_factor / (double)sample_rate_mult;
    else if (sample_rate_factor < 0 && sample_rate_mult > 0) 
	drate = -1. * (double)sample_rate_mult / (double)sample_rate_factor;
    else if (sample_rate_factor < 0 && sample_rate_mult < 0) 
	drate = (double)sample_rate_mult / (double)sample_rate_factor;
    else drate = 0.;

    if (drate == 0.) rate = 0;
    else if (drate >= 1.) rate = roundoff(drate);
    else rate = -1 * roundoff(1./drate);
    return (rate);
}

/************************************************************************/
/*  asc_sdr_time:							*/
/*	Convert SDR_TIME to ascii string.				*/
/*	Note that we output string in IRIS-style format with commas.	*/
/************************************************************************/
char *asc_sdr_time
   (char	*str,		/* string to encode time into.		*/
    SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder)	/* wordorder for encoded time contents.	*/

{
    if (my_wordorder < 0) get_my_wordorder();
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&st.year);
	swab2 ((short int *)&st.day);
	swab2 ((short int *)&st.ticks);
    }
    sprintf(str,"%04d,%03d,%02d:%02d:%02d.%04d", st.year,
	    st.day, st.hour, st.minute, st.second, st.ticks);
    return (str);
}

/************************************************************************/
/*  unix_time_from_sdr_time:						*/
/*	Convert SDR_TIME to unix timestamp.				*/
/*  return:								*/
/*	Unix time_t timestamp.						*/
/************************************************************************/
time_t unix_time_from_sdr_time
   (SDR_TIME	st,		/* SDR_TIME structure to convert.	*/
    int		wordorder)	/* wordorder for encoded time contents.	*/
    
{    
    EXT_TIME	et;

    if (my_wordorder < 0) get_my_wordorder();
    if (my_wordorder != wordorder) {
	swab2 ((short int *)&st.year);
	swab2 ((short int *)&st.day);
	swab2 ((short int *)&st.ticks);
    }
    et.year = st.year;
    et.doy = st.day;
    et.hour = st.hour;
    et.minute = st.minute;
    et.second = st.second;
    et.usec = st.ticks * USECS_PER_TICK;
    dy_to_mdy (et.doy, et.year, &et.month, &et.day);
    return (unix_time_from_ext_time(et));
}

/************************************************************************/
/*  SEED Data Blockette routines.					*/
/************************************************************************/

/************************************************************************/
/*  read_blockettes:							*/
/*	Read binary blockettes that follow the SEED fixed data header.	*/
/*	Assume blockettes have the same byteorder as fixed data header.	*/
/*  return:								*/
/*	QLIB2 CLASSIC mode:						*/
/*	    1 on success, 0 on error. (QLIB2 CLASSIC mode)		*/
/*	QLIB2 NOEXIT mode:						*/
/*	    1 on success, negative QLIB2 error code on error.		*/
/************************************************************************/
int read_blockettes
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*str)		/* ptr to fixed data header.		*/
{
    BS *bs, *pbs;
    int offset, i;
    SEED_UWORD bl_len, bl_next, bl_type;

    if (my_wordorder < 0) get_my_wordorder();
    bs = pbs = (BS *)NULL;
    offset = hdr->first_blockette;
    hdr->pblockettes = (BS *)NULL;
    bl_next = 0;

    /*	Run through each blockette, allocate a linked list structure	*/
    /*	for it, and verify that the blockette structures are OK.	*/
    /*	There is a LOT of checking to ensure proper structure.		*/
    for (i=0; i<hdr->num_blockettes; i++) {

	if (i > 0 && bl_next == 0) {
	    fprintf (stderr, "Error: zero offset to next blockette\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (MS_ERROR);
	}

	if ( (bs=(BS *)malloc(sizeof(BS))) == NULL ) {
	    fprintf (stderr, "Error: unable to malloc BS\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (QLIB2_MALLOC_ERROR);
	}
	bs->next = (BS *)NULL;

	/*  Decide how much space the blockette takes up.		*/
	/*  In order to allow for variable blockette size for either	*/
	/*  newer SEED version or vendor-specific additions,		*/
	/*  attempt to determine the required space by the offset to	*/
	/*  the next blockette.  If this is the last blockette, 	*/
	/*  then just use the length of the blockette as it is defined.	*/
	bl_type = ((BLOCKETTE_HDR *)(str+offset))->type;
	bl_next = ((BLOCKETTE_HDR *)(str+offset))->next;
	if (hdr->hdr_wordorder != my_wordorder) {
	    swab2 ((short int *)&bl_type);
	    swab2 ((short int *)&bl_next);
	}
	if (bl_next > 0) {
	    bl_len = (bl_next-offset);
	}
	else {
	    /* No further blockettes.  Assume length of blockette structure.*/
	    switch (bl_type) {
	      /* Fixed length blockettes.	*/
	      case 100: bl_len = sizeof (BLOCKETTE_100); break;
	      case 200: bl_len = sizeof (BLOCKETTE_200); break;
	      case 201: bl_len = sizeof (BLOCKETTE_201); break;
	      case 300: bl_len = sizeof (BLOCKETTE_300); break;
	      case 310: bl_len = sizeof (BLOCKETTE_310); break;
	      case 320: bl_len = sizeof (BLOCKETTE_320); break;
	      case 390: bl_len = sizeof (BLOCKETTE_390); break;
	      case 395: bl_len = sizeof (BLOCKETTE_395); break;
	      case 400: bl_len = sizeof (BLOCKETTE_400); break;
	      case 405: bl_len = sizeof (BLOCKETTE_405); break;
	      case 500: bl_len = sizeof (BLOCKETTE_500); break;
	      case 1000: bl_len = sizeof (BLOCKETTE_1000); break;
	      case 1001: bl_len = sizeof (BLOCKETTE_1001); break;
	      /* Variable length blockettes.  Preserve original length,	*/
	      /* even though it may not is divisible by 4.		*/
	      /* It is up to the user to ensure that that blockettes	*/
	      /* have 4 byte alignment in a SEED data record.		*/
	      case 2000: 
		bl_len = ((BLOCKETTE_2000 *)(str+offset))->blockette_len; 
		if (hdr->hdr_wordorder != my_wordorder) {
		    swab2 ((short int *)&bl_len);
		}
		break;
	      default: bl_type = 0; bl_len = 0; break;
	    }
	    /* Ensure that the blockette length does not exceed space	*/
	    /* available for it after the header and before first_data.	*/
	    if (hdr->first_data > 0 && hdr->first_data - offset > 0 && 
		(int)bl_len > hdr->first_data - offset)
		bl_len = hdr->first_data - offset;
	}

	if (bl_next != 0 && bl_len != 0) {
	    /* Verify length for known blockettes when possible. */
/*::
	    if (bl_len != bl_next-offset) {
		fprintf (stderr, "Error: blockette %d apparent size %d does not match known length %d\n",
			 bl_type, bl_next-offset, bl_len);
		fflush (stderr);
		if (QLIB2_CLASSIC) exit(1);
		return (MS_ERROR);
	    }
::*/
	}
	else if (bl_len == 0 && bl_type == 0) {
	    /* Assume the blockette reaches to first data.  */
	    /* If first data == 0, then abort -- we don't know this blockette.	*/
	    if (hdr->first_data <= offset) {
		fprintf (stderr, "Unknown blockette type %d - unable to determine size\n",
			 ((BLOCKETTE_HDR *)(str+offset))->type);
		fflush (stderr);
		free ((char *)bs);
		continue;
	    }
	    else bl_len = hdr->first_data - offset;
	}
	if ((bs->pb = (char *)malloc(bl_len))==NULL) {
	    fprintf (stderr, "Error: unable to malloc blockette\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (MS_ERROR);
	}
	memcpy (bs->pb,str+offset,bl_len);
	bs->len = bl_len;
	bs->type = bl_type;
	bs->wordorder = hdr->hdr_wordorder;
	offset += bl_len;
	if (i == 0) hdr->pblockettes = bs;
	else pbs->next = bs;
	pbs = bs;
    }

    /* Ensure there are no more blockettes. */
    if (bl_next != 0) {
	fprintf (stderr, "extra blockette found\n");
	fflush (stderr);
	return (QLIB2_CLASSIC ? 0 : MS_ERROR);
    }
    return (1);
}

/************************************************************************/
/*  find_blockette:							*/
/*	Find a specified blockette in our linked list of blockettes.	*/
/*  return:								*/
/*	Pointer to BS linked list element for structure on success.	*/
/*	NULL on failure.						*/
/************************************************************************/
BS *find_blockette 
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    int		n)		/* blockette type to find.		*/
{
    BS		*bs = hdr->pblockettes;

    while (bs != (BS *)NULL) {
	if (bs->type == n) return (bs);
	bs = bs->next;
    }
    return (bs);
}

/************************************************************************/
/*  find_pblockette:							*/
/*	Find the next specified blockette starting with the BS* from	*/
/*	the linked list of blockettes.					*/
/*	This function is required because there can be more than 1	*/
/*	occurance of a numbered blockette.				*/
/*  return:								*/
/*	Pointer to BS linked list element for structure on success.	*/
/*	NULL on failure.						*/
/************************************************************************/
BS *find_pblockette
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    BS		*bs,		/* BS* to start with.			*/
    int		n)		/* blockette type to find.		*/
{
    while (bs != (BS *)NULL) {
	if (bs->type == n) return (bs);
	bs = bs->next;
    }
    return (bs);
}

/************************************************************************/
/*  blockettecmp:							*/
/*	Compare the contents of 2 blockettes, and return result.	*/
/*	Ignore the ptr to next blockette at the beginning blockette.	*/
/*	WARNING: comparison works ONLY if both blockettes have the	*/
/*	same byteorder.							*/
/*  return:								*/
/*	Result of memcmp comparison of blockette contents when 		*/
/*	converted to the same byteorder.				*/
/************************************************************************/
int blockettecmp
   (BS		*bs1,		/* BS* of first blockette to compare.	*/
    BS		*bs2)		/* BS* of first blockette to compare.	*/
{
    int swapflag;
    SEED_UWORD l1, l2, type1, type2;
    int status;
    char *pbc1, *pbc2;
    char *p = NULL;

    if (my_wordorder < 0) get_my_wordorder();
    if (bs1 == NULL && bs2 == NULL) return (0);
    if (bs1 == NULL) return (-1);
    if (bs2 == NULL) return (1);
    swapflag = (bs1->wordorder != bs2->wordorder);
    type1 = bs1->type;
    type2 = bs2->type;
    if (swapflag && bs1->wordorder != my_wordorder) swab2 ((short int *)&type1);
    if (swapflag && bs2->wordorder != my_wordorder) swab2 ((short int *)&type2);
    if (type1-type2) return (type1-type2);
    l1 = bs1->len;
    l2 = bs2->len;
    if (l1-l2) return (l1-l2);
    pbc1 = (char *)bs1->pb;
    pbc2 = (char *)bs2->pb;
    if (swapflag) {
	/* Reorder the wordorder of one of the blockettes for compare.	*/
	p = (char *)malloc(l1-4);
	if (bs1->wordorder != my_wordorder) {
	    memcpy (p, pbc1, l1);
	    swab_blockette (type1, pbc1, l1);
	    pbc1 = p;
	}
	else {
	    memcpy (p, pbc2, l2);
	    swab_blockette (type2, pbc2, l2);
	    pbc2 = p;
	}
    }
    status = memcmp(pbc1+4, pbc2+4, l1-4);
    if (swapflag && p) free (p);
    return (status);
}

/************************************************************************/
/*  write_blockettes:							*/
/*	Write the blockettes contained in the DATA_HDR linked list of	*/
/*	blockettes to the output SEED data records.			*/
/*	Output blockettes in the wordorder specified in the DATA_HDR.	*/
/*	Ensure that all blockettes are written on a 4-byte boundary.	*/
/*  return:								*/
/*	0 on success.							*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int write_blockettes
   (DATA_HDR	*hdr,		/* ptr to data_hdr			*/
    char	*str)		/* ptr to output SDR.			*/
{
    SDR_HDR *ohdr =	(SDR_HDR *)str;
    BS *bs = hdr->pblockettes;
    int offset = hdr->first_blockette;
    SEED_UWORD next;
    int alen;
    int swapflag;

    if (my_wordorder < 0) get_my_wordorder();
    /* Ensure initial offset is a multiple of 4.			*/
    if (offset%4) {
	memset (str+offset, 0, 4-(offset%4));
	offset += 4-(offset%4);
	((SDR_HDR *)str)->first_blockette = offset;
    }
    while (bs != (BS *)NULL) {
	/* Ensure offset to next blockette is correct.			*/
	alen = bs->len;
	if (bs->len%4) alen += 4-(bs->len%4);
	next = (bs->next == NULL) ? 0 : offset + alen;
	if (my_wordorder != bs->wordorder) swab2((short int *)&next);
	((BLOCKETTE_HDR *)(bs->pb))->next = next;
	memcpy (str+offset,bs->pb,bs->len);
	if (alen != bs->len) memset(str+offset+bs->len, 0, alen-bs->len);
	/* Ensure blockette wordorder is the same as hdr wordorder.	*/
	swapflag = (hdr->hdr_wordorder != bs->wordorder);
	if (swapflag) {
	    swab_blockette (bs->type, str+offset, bs->len);
	}
	offset += alen;
	bs = bs->next;
    }
    if (hdr->first_data > 0 && offset > hdr->first_data) {
	fprintf (stderr, "Error: blockettes won't fit between hdr and data.\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (MS_ERROR);
    }
    return (0);
}

/************************************************************************/
/*  add_blockette:							*/
/*	Add the specified blockette to the linked list of blockettes.	*/
/*  return:								*/
/*	1 on success.							*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int add_blockette
   (DATA_HDR	*hdr,		/* ptr to data_hdr.			*/
    char	*str,		/* pre-constructed blockette.		*/
    int		type,		/* blockette type.			*/
    int		l,		/* length of blockette.			*/
    int		wordorder,	/* wordorder of blockette contents.	*/
    int		where)		/* i -> i-th blockette from start,	*/
				/* -1 -> append as last blockette.	*/
{
    BLOCKETTE_HDR *bh = (BLOCKETTE_HDR *)str;
    BS *bs;
    BS *prev;
    int status;

    /*	BEWARE:								*/
    /*  Allow the user to specify where the blockette should be placed	*/
    /*	within the blockette linked list.				*/
    /*	If we a blockette of unknown length, we should always add as	*/
    /*	first blockette in order to keep unknown blockette at end.	*/
    /*	Don't worry about updating the offset within the blockette	*/
    /*	headers, since we will do that on output.			*/

    if ((bs=(BS *)malloc(sizeof(BS)))==NULL) {
	fprintf (stderr, "Error: unable to malloc BS\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (QLIB2_MALLOC_ERROR);
    }
    if ((bs->pb=(char *)malloc(l))==NULL) {
	fprintf (stderr, "Error: unable to malloc blockette\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (QLIB2_MALLOC_ERROR);
    }
    memcpy (bs->pb, str, l);
    bs->type = type;
    bs->len = l;
    bs->wordorder = wordorder;

    prev = hdr->pblockettes;
    if (prev == NULL || where == 0) {
	/* Insert at beginning of the blockette list.	*/
	bs->next = hdr->pblockettes;
	hdr->pblockettes = bs;
    }
    else {
	while (--where != 0 && prev->next != NULL) {
	    prev = prev->next;
	}
	/* Insert blockette after prev.. */
	bs->next = prev->next;
	prev->next = bs;
    }
    if (hdr->num_blockettes == 0) hdr->first_blockette = 48;
    ++(hdr->num_blockettes);
    return (1);
}

/************************************************************************/
/*  delete_blockette:							*/
/*	Delete the specified blockette from the linked list of		*/
/*	blockettes.							*/
/*  return:								*/
/*	The number of blockettes that were deleted.			*/
/************************************************************************/
int delete_blockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		n)		/* blockette # to delete.  -1 -> ALL.	*/
{
    BS *bs = hdr->pblockettes;
    BS *pbs = (BS *)NULL;
    BS *dbs;
    int num_deleted = 0;
    SEED_UWORD type;

    /*	Don't worry about updating the offset within the blockette	*/
    /*	headers, since we will do that on output.			*/
    if (my_wordorder < 0) get_my_wordorder();
    while (bs != (BS *)NULL) {
	type = bs->type;
	if ( n < 0 || n == type) {
	    if (pbs == NULL)
		hdr->pblockettes = bs->next;
	    else 
		pbs->next = bs->next;
	    --(hdr->num_blockettes);
	    if (hdr->num_blockettes <= 0) 
		hdr->first_blockette = 0;
	    dbs = bs;
	    bs = bs->next;
	    free (dbs->pb);
	    free ((char *)dbs);
	    ++num_deleted;
	}
	else {
	    pbs = bs;
	    bs = bs->next;
	}
    }
    return (num_deleted);
}

/************************************************************************/
/*  delete_pblockette:							*/
/*	Delete the blockette specified by the BS* from the linked	*/
/*	list of blockettes.						*/
/*  return:								*/
/*	The number of blockettes that were deleted.			*/
/************************************************************************/
int delete_pblockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    BS		*dbs)		/* BS* to delete.			*/
{
    BS *bs = hdr->pblockettes;
    BS *pbs = (BS *)NULL;
    int num_deleted = 0;

    /*	Don't worry about updating the offset within the blockette	*/
    /*	headers, since we will do that on output.			*/
    pbs = NULL;
    while (bs != (BS *)NULL) {
	if (bs == dbs) {
	    if (pbs == NULL)
		hdr->pblockettes = bs->next;
	    else
		pbs->next = bs->next;
	    free (dbs->pb);
	    free ((char *)dbs);
	    --(hdr->num_blockettes);
	    if (hdr->num_blockettes <= 0) 
		hdr->first_blockette = 0;
	    ++num_deleted;
	    break;
	}
	else {
	    pbs = bs;
	    bs = bs -> next;
	}
    }
    return (num_deleted);
}

/************************************************************************/
/*  add_required_miniseed_blockettes:					*/
/*	Add any required blockettes to this hdr.			*/
/*  return:								*/
/*	0 on success.							*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int add_required_miniseed_blockettes 
   (DATA_HDR	*hdr)		/* ptr to DATA_HDR.			*/
{
    int status = 0;
    /* Currently only blockette 1000 is required for miniSEED.		*/
    if (my_wordorder < 0) get_my_wordorder();
    if (find_blockette(hdr, 1000) == NULL) {
	BLOCKETTE_1000 b1000;
	int ok;
	b1000.hdr.type = 1000;
	b1000.hdr.next = 0;
	b1000.format = hdr->data_type;
	b1000.word_order = SEED_BIG_ENDIAN;
	b1000.data_rec_len = roundoff(log2((double)hdr->blksize));
	b1000.reserved = 0;
	ok = add_blockette (hdr, (char *)&b1000, 1000, sizeof(BLOCKETTE_1000),
			    my_wordorder, 0);
	if (! ok) status = MS_ERROR;
    }
    return (status);
}

/************************************************************************/
/*  init_data_hdr:							*/
/*	Initialize a DATA_HDR structure.				*/
/************************************************************************/
void init_data_hdr 
   (DATA_HDR	    *hdr)	/* ptr to DATA_HDR to initialize.	*/
{
    memset ((void *)hdr, 0, sizeof(DATA_HDR));
    hdr->hdr_wordorder = hdr_wordorder;
    hdr->data_wordorder = data_wordorder;
    hdr->record_type = default_data_hdr_ind;
    hdr->sample_rate_mult = 1;
}

/************************************************************************/
/*  new_data_hdr:							*/
/*	Allocate and initialize a DATA_HDR structure.			*/
/*  Return:								*/
/*	Pointer to DATA_HDR structure on success.			*/
/*	NULL on error.							*/
/************************************************************************/
DATA_HDR *new_data_hdr ()
{
    DATA_HDR	    *hdr;

    if (my_wordorder < 0) get_my_wordorder();
    hdr = (DATA_HDR *) malloc (sizeof(DATA_HDR));
    if (hdr == NULL) {
	fprintf (stderr, "Error: unable to allocate data_hdr for output\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit (1);
	return (NULL);
    }
    init_data_hdr (hdr);
    return (hdr);
}

/************************************************************************/
/*  copy_data_hdr:							*/
/*	Copy one DATA_HDR to another DATA_HDR, including all blockettes.*/
/*  return:								*/
/*	pointer to destination DATA_HDR structure on success.		*/
/*	NULL pointer on error.						*/
/************************************************************************/
DATA_HDR *copy_data_hdr
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src)	/* ptr to source DATA_HDR to copy.	*/
{
    BS		    *bs;
    if (hdr_dst == NULL) return (NULL);
    memcpy ((void *)hdr_dst, (void *)hdr_src, sizeof(DATA_HDR));
    hdr_dst->pblockettes = NULL;
    hdr_dst->num_blockettes = 0;
    for (bs = hdr_src->pblockettes; bs != NULL; bs=bs->next) {
	int ok;
	ok = add_blockette (hdr_dst, bs->pb, bs->type, bs->len, 
			    bs->wordorder, -1);
	if (! ok) return ((DATA_HDR *)NULL);
    }
    return (hdr_dst);
}

/************************************************************************/
/*  dup_data_hdr:							*/
/*	Allocate a new header structure and return copy of current one.	*/
/*  return:								*/
/*	pointer to new duplicate DATA_HDR structure on success.		*/
/*	NULL pointer on failure.					*/
/************************************************************************/
DATA_HDR *dup_data_hdr
   (DATA_HDR	*hdr)		/* ptr to DATA_HDR to duplicate.	*/
{
    DATA_HDR	    *new_hdr, *pcopy;
    new_hdr = new_data_hdr();
    pcopy = copy_data_hdr (new_hdr, hdr);
    if (pcopy == NULL && new_hdr != NULL) free_data_hdr (new_hdr);
    return (pcopy);
}

/************************************************************************/
/*  free_data_hdr:							*/
/*	Free all malloced space associated with a DATA_HDR		*/
/************************************************************************/
void free_data_hdr
   (DATA_HDR	*hdr)		/* ptr to DATA_HDR to free.		*/
{
    if (hdr == NULL) return;
    if (hdr->pblockettes != NULL) delete_blockette (hdr, -1);
    free ((char *)hdr);
    return;
}

/************************************************************************/
/*  dump_hdr:								*/
/*	Write header summary info info a string, for debugging.		*/
/************************************************************************/
void dump_hdr 
   (DATA_HDR	*h,		/* ptr to Data_Hdr structure.		*/
    char	*str,		/* write debugging info into string.	*/
    int		date_fmt)	/* format specified for date/time.	*/
{
    int		seconds, usecs;
    double	usecs_per_point;
    char	begtime_str[40], exptime_str[40];
    INT_TIME	exptime;

    strcpy (begtime_str, time_to_str(h->begtime, date_fmt));

    /*	compute expected end time.  */
    time_interval2 (1,h->sample_rate,h->sample_rate_mult,&seconds,&usecs);
    usecs_per_point = seconds * (double)USECS_PER_SEC + usecs;
    exptime = add_dtime (h->endtime, usecs_per_point); 
    strcpy (exptime_str, time_to_str(exptime, date_fmt));

    sprintf (str,
	     "seq=%d stream=%s.%s.%s.%s time=%s to %s corr=%d nsamp=%d aflag=%02x ioflag=%02x qflag=%02x ",
	     h->seq_no, h->station_id, h->network_id, h->channel_id, h->location_id,
	     begtime_str, exptime_str, 
	     h->num_ticks_correction, h->num_samples, 
	     (int) h->activity_flags, (int) h->io_flags, 
	     (int) h->data_quality_flags);
    if (h->sample_rate_mult == 1)
	sprintf (str+strlen(str), "rate=%d\n", h->sample_rate);
    else
	sprintf (str+strlen(str), "rate=%.4lf\n", 
		 sps_rate(h->sample_rate, h->sample_rate_mult));
}

#define	PRINT_FLAG(a,b,c,flag) \
	if (flag || c) fprintf (info, "%s: %s = %d\n", a, b, c)
/************************************************************************/
/*  dump_sdr_flags:							*/
/*	Dump flags for debugging.					*/
/************************************************************************/
void dump_sdr_flags
   (DATA_HDR	*ch,		/* DATA_HDR for record.			*/
    int		detail)		/* dump only non-zero field if false.	*/
{
#ifdef	QLIB_DEBUG
    PRINT_FLAG("Activity flag","calibration", BIT(ch->activity_flags,0),detail);
    PRINT_FLAG("Activity flag","clock correction", BIT(ch->activity_flags,1),detail);
    PRINT_FLAG("Activity flag","begin event", BIT(ch->activity_flags,2),detail);
    PRINT_FLAG("Activity flag","end event", BIT(ch->activity_flags,3),detail);
    PRINT_FLAG("Activity flag","+ leap second", BIT(ch->activity_flags,4),detail);
    PRINT_FLAG("Activity flag","- leap second", BIT(ch->activity_flags,5),detail);
    PRINT_FLAG("Activity flag","event in progress", BIT(ch->activity_flags,6),detail);

    PRINT_FLAG("I/O flag","parity error", BIT(ch->io_flags,0),detail);
    PRINT_FLAG("I/O flag","long record", BIT(ch->io_flags,1),detail);
    PRINT_FLAG("I/O flag","short record", BIT(ch->io_flags,2),detail);

    PRINT_FLAG("Quality flag","saturation", BIT(ch->data_quality_flags,0),detail);
    PRINT_FLAG("Quality flag","clipping", BIT(ch->data_quality_flags,1),detail);
    PRINT_FLAG("Quality flag","spikes", BIT(ch->data_quality_flags,2),detail);
    PRINT_FLAG("Quality flag","glitches", BIT(ch->data_quality_flags,3),detail);
    PRINT_FLAG("Quality flag","missing/padded data", BIT(ch->data_quality_flags,4),detail);
    PRINT_FLAG("Quality flag","sync error", BIT(ch->data_quality_flags,5),detail);
    PRINT_FLAG("Quality flag","charging digital filter", BIT(ch->data_quality_flags,6),detail);
    PRINT_FLAG("Quality flag","questionable time tag", BIT(ch->data_quality_flags,7),detail);
#endif
}

/************************************************************************/
/*  is_sdr_header:							*/
/*	Determine whether the buffer contains a SDR record header.	*/
/*  return:								*/
/*	1 if true, 0 otherwise.						*/
/************************************************************************/
int is_sdr_header 
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr)		/* max number of bytes for header.	*/
{
    return (nr >= SDR_HDR_SIZE && is_data_hdr_ind(p->data_hdr_ind));
}

/************************************************************************/
/*  is_sdr_vol_header:							*/
/*	Determine whether the buffer contains a SDR volume header.	*/
/*  return:								*/
/*	1 if true, 0 otherwise.						*/
/************************************************************************/
int is_sdr_vol_header
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr)		/* max number of bytes for header.	*/
{
    char *vol = (char *)p;
    return (nr >= SDR_HDR_SIZE && is_vol_hdr_ind(p->data_hdr_ind)
	    /* && strncmp(vol+8,"008",3)==0 */ 
	    );
}

/************************************************************************/
/*  decode_seqno:							*/
/*	Decode seqno (apparent bug in strtol that decodes non-digits),	*/
/*	and get ptr to next character.					*/
/*  return:								*/
/*	sequence number.						*/
/************************************************************************/
int decode_seqno 
   (char	*str,		/* string containing ascii seqno.	*/
    char	**pp)		/* ptr to next char after seqno.	*/
{
    int i;
    char tmpstr[7];
    for (i=0; i<6; i++) {
	if (str[i] < '0' || str[i] > '9') break;
    }
    *pp = str+i;
    if (i<6) return (-1);
    strncpy(tmpstr,str,6);
    tmpstr[6] = '\0';
    i = strtol(tmpstr, pp, 10);
    return (i);
}

/************************************************************************/
/*  q_clock_model:							*/
/*	Get string for the numeric clock model specified in blockette.	*/
/*  return:								*/
/*	String containing clock model.					*/
/************************************************************************/
char *q_clock_model
   (char	clock_model)	/* numeric clock model from blockette.	*/
{
    switch (clock_model) {
      case 1:	return("OS9_internal");
      case 2:	return("Kin_GOES");
      case 3:	return("Kin_OMEGA");
      case 4:	return("Kin_DCF");
      case 5:	return("Mein_UA31S");
      case 6:	return("GPS1_QTS");
      case 7:	return("GPS1_GOES");
      case 8:	return("GPS1_UA31S");
      case 0:
      default:	return("Unknown");
    }
}

/************************************************************************/
/*  q_clock_status:							*/
/*	Get string for the clock status specified in blockette.		*/
/*  return:								*/
/*	String containing clock status.					*/
/************************************************************************/
char *q_clock_status
   (char	*status,	/* status string from blockette.	*/
    char	clock_model)	/* numeric clock model from blockette.	*/
{
    static char snr[40];
    switch (clock_model) {
      case 3:	
	switch(status[0]) {
	  case 'A': return("Norway");
	  case 'B': return("Liberia");
	  case 'C': return("Hawaii");
	  case 'D': return("North Dakota");
	  case 'E': return("La Reunion");
	  case 'F': return("Argentina");
	  case 'G': return("Austrailia");
	  case 'H': return("Japan");
	  default:  return ("Unknown");
	}
	break;
      case 6:	
	sprintf (snr,"(%d,%d,%d,%d,%d,%d)",
		 status[0],status[1],status[2],
		 status[3],status[4],status[5]);
	return (snr);
	break;
      default:	return("none");
    }
}

/************************************************************************/
/*  swab_blockette:							*/
/*	Swap wordorder of all fields in the specified blockette.	*/
/*  return:								*/
/*	0 on success; non-zero on failure.				*/
/************************************************************************/
int swab_blockette
   (int		type,		/* blockette number.			*/
    char	*contents,	/* string containing blockette.		*/
    int		len)		/* length of blockette (incl header).	*/
{
    int status = 0;
    char *p = contents;
    swab2 ((short int *)(p));
    swab2 ((short int *)(p+2));
    p += 4;
    len -= 4;
    switch (type) {
      case 100:
	swabf (p+0);
	break;
      case 200:
      case 201:
	swabf ((float *)(p+0));
	swabf ((float *)(p+4));
	swabf ((float *)(p+8));
	swabt ((SDR_TIME *)(p+14));
	break;
      case 300:
	swabt ((SDR_TIME *)(p+0));
	swab4 ((int *)(p+12));
	swab4 ((int *)(p+16));
	swabf ((float *)(p+20));
	if (len > 28) swabf ((float *)(p+28));
	break;
      case 310:
	swabt ((SDR_TIME *)(p+0));
	swab4 ((int *)(p+12));
	swabf ((float *)(p+16));
	swabf ((float *)(p+20));
	if (len > 28) swabf ((float *)(p+28));
	break;
      case 320:
	swabt ((SDR_TIME *)(p+0));
	swabf ((float *)(p+12));
	swabf ((float *)(p+16));
	if (len > 24) swabf ((float *)(p+24));
	break;
      case 390:
	swabt ((SDR_TIME *)(p+0));
	swabf ((float *)(p+12));
	swabf ((float *)(p+16));
	break;
      case 395:
	swabt ((SDR_TIME *)(p+0));
	swab2 ((short int *)(p+10));
	break;
      case 400:
	swabf ((float *)(p+0));
	swabf ((float *)(p+4));
	swab2 ((short int *)(p+8));
	swab2 ((short int *)(p+10));
	break;
      case 405:
	swab2 ((short int *)(p+0));
	break;
      case 500:
	swabf ((float *)(p+0));
	swabt ((SDR_TIME *)(p+4));
	swab4 ((int *)(p+16));
	break;
      case 1000:
      case 1001:
	break;
      case 2000:
	/* Swap only numeric fields in opaque blockette header.		*/
	swab2 ((short int *)(p));
	swab2 ((short int *)(p+2));
	swab4 ((int *)(p+4));
	break;
      default:
	p -= 4;
	len += 4;
	swab2 ((short int *)(p));
	swab2 ((short int *)(p+2));
	status = -1;
    }
    return (status);
}

/************************************************************************/
/*  init_sdr_hdr:							*/
/*	Initialize a sdr header.					*/
/*	Return:								*/
/*	    0 on success.						*/
/*	    negative QLIB2 error code on error.				*/
/************************************************************************/
int init_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for sdr data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for sdr record.	*/
    BS		*extra_bs)	/* ptr to block-specific blockettes.	*/
{
    int status = 0;
    int blockette_space;	/* # of bytes required for blockettes.	*/
    int n_extra_bs;		/* # of extra blockettes.		*/
    BS *bs;			/* ptr to blockette structure.		*/
    BS *last_bs;		/* ptr to last permanent blockette.	*/
    int align;			/* alignment in bytes required for data.*/
    int swapflag;		/* flag to indicate byteswapping.	*/
    short int stmp[2];
    MS_ATTR attr;
    int blksize = hdr->blksize;


    if (my_wordorder < 0) 
	get_my_wordorder();
    swapflag = (my_wordorder != hdr->hdr_wordorder);

    /* Determine the space required for all of the blockettes.		*/
    for (bs=hdr->pblockettes, blockette_space=0, last_bs=NULL; 
	 bs!=NULL; 
	 last_bs=bs, bs=bs->next) {
	blockette_space += bs->len + ((bs->len%4) ? 4-(bs->len%4) : 0);
    }
    for (bs=extra_bs, n_extra_bs=0; bs!=NULL; bs=bs->next, n_extra_bs++) {
	blockette_space += bs->len + ((bs->len%4) ? 4-(bs->len%4) : 0);
    }

    /* Temporarily add the list of extra blockettes to the list of	*/
    /* permanent blockettes.						*/
    if (extra_bs) {
	if (last_bs) last_bs->next = extra_bs;
	else hdr->pblockettes = extra_bs;
	hdr->num_blockettes += n_extra_bs;
    }

    /* Ensure that first_data points to appropriate offset for data.	*/
    /* Some data formats (eg the STEIM compressed formats) require	*/
    /* first_data to be on a frame boundary.				*/
    attr = get_ms_attr(hdr);
    if (attr.alignment == 0) return (MS_ERROR);
    align = attr.alignment;
    hdr->first_data = ((sizeof(SDR_HDR)+blockette_space+align-1)/align)*align;

    /* Update any blockettes that have block-specific info.		*/
    if ((bs=find_blockette(hdr, 1000))) {
	/* Ensure we have proper data in the blockette.			*/
	BLOCKETTE_1000 *b1000 = (BLOCKETTE_1000 *) bs->pb;
	/* These are all byte values, so I can ignore wordorder.	*/
	b1000->data_rec_len = roundoff(log2((double)blksize));
	b1000->format = hdr->data_type;
	b1000->word_order = hdr->data_wordorder;
    }
    if ((bs=find_blockette(hdr, 1001))) {
	/* Ensure we have proper data in the blockette.			*/
	/* Mark all frames as being in use.				*/
	BLOCKETTE_1001 *b1001 = (BLOCKETTE_1001 *) bs->pb;
	/* These are all byte values, so I can ignore wordorder.	*/
	b1001->frame_count = hdr->num_data_frames;
	b1001->usec99 = hdr->hdrtime.usec % 100;
    }
    
    /* Create the SDR fixed data header and data blockettes.		*/

    /* Parts of the header that do not change from block to block.	*/
    sh->space_1 = ' ';
    capnstr(sh->station_id,hdr->station_id,5);
    capnstr(sh->channel_id,hdr->channel_id,3);
    capnstr(sh->network_id,hdr->network_id,2);
    capnstr(sh->location_id,hdr->location_id,2);
    sh->sample_rate_factor = hdr->sample_rate;
    sh->sample_rate_mult = (hdr->sample_rate) ? hdr->sample_rate_mult : 0;

    /*  Parts of the header that change with each block.		*/
    sh->data_hdr_ind = hdr->record_type;
    capnint(sh->seq_no,hdr->seq_no,6);
    sh->time = encode_time_sdr(hdr->hdrtime, hdr->hdr_wordorder);
    sh->activity_flags = hdr->activity_flags;
    sh->io_flags = hdr->io_flags;
    sh->data_quality_flags = hdr->data_quality_flags;
    sh->num_samples = 0;
    sh->num_ticks_correction = hdr->num_ticks_correction;

    /* Parts of the header that depend on the blockettes.		*/
    sh->first_data = hdr->first_data;
    sh->num_blockettes = hdr->num_blockettes;
    sh->first_blockette = hdr->first_blockette;

    /*	Output any data blockettes.					*/
    if (hdr->num_blockettes > 0) {
	write_blockettes(hdr, (char *)sh);
    }

    /* Unlink the extra blockettes from the data_hdr.			*/
    if (extra_bs) {
	if (last_bs) last_bs->next = NULL;
	else hdr->pblockettes = NULL;
	hdr->num_blockettes -= n_extra_bs;
    }

    if (swapflag) {
	swab2 ((short int *)&sh->num_samples);
	swab2 ((short int *)&sh->sample_rate_factor);
	swab2 ((short int *)&sh->sample_rate_mult);
	swab2 ((short int *)&sh->first_data);
	swab2 ((short int *)&sh->first_blockette);
	swab4 ((int *)&sh->num_ticks_correction);
    }

    /* Zero any space between the end of the blockettes and first_data.	*/
    memset ((char*)sh + (sizeof(SDR_HDR)+blockette_space), 0,
	    hdr->first_data - (sizeof(SDR_HDR)+blockette_space));

    /* Return status our SDR header creation.				*/
    return (status);
}

/************************************************************************/
/*  update_sdr_hdr:							*/
/*	Update a previously constructed sdr header.			*/
/*	Return 0 on success, -1 on error.				*/
/************************************************************************/
int update_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for SDR data hdr.	*/
    DATA_HDR	*hdr)		/* initial DATA_HDR for SDR record.	*/
{
    int swapflag;		/* flag to indicate byteswapping.	*/
    short int stmp;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != hdr->hdr_wordorder);
    if (swapflag) {
	stmp = hdr->num_samples;
	swab2 (&stmp);
	sh->num_samples = stmp;
    }
    else {
	sh->num_samples = hdr->num_samples;
    }
    if (hdr->num_samples == 0) sh->first_data = 0;
    return (0);
}

/************************************************************************/
/* Fortran interludes to sdr_utils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
void f_init_data_hdr_
#else
void f_init_data_hdr
#endif
   (DATA_HDR	    *hdr)	/* ptr to DATA_HDR to initialize.	*/
{
    init_data_hdr (hdr);
}

#ifdef	fortran_suffix
int f_delete_blockette_
#else
int f_delete_blockette
#endif
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		*n)		/* blockette # to delete.  -1 -> ALL.	*/
{
    return (delete_blockette(hdr, *n));
}

#ifdef	fortran_suffix
extern void f_copy_data_hdr_
#else
extern void f_copy_data_hdr
#endif
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src)	/* ptr to source DATA_HDR to copy.	*/
{
    copy_data_hdr (hdr_dst, hdr_src);
}
