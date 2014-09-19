/************************************************************************/
/*  Routines for unpacking STEIM1, STEIM2, INT_32, INT_16, and INT_24	*/
/*  data records.							*/
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
static char sccsid[] = "$Id: unpack.c 2 2005-07-26 19:28:46Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#include "qdefines.h"
#include "qutils.h"
#include "msdatatypes.h"
#include "qutils.h"
#include "qsteim.h"
#include "unpack.h"

#define	info	stderr
#define	VALS_PER_FRAME	(16-1)		/* # of ints for data per frame.*/

#define	X0  pf->w[0].fw
#define	XN  pf->w[1].fw

/************************************************************************/
/*  unpack_steim1:							*/
/*	Unpack STEIM1 data frames and place in supplied buffer.		*/
/*	Data is divided into frames.					*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	Fast decompression does not decompress all frames, and does not	*/
/*	verify that the last sample == xN.  Fast decompression ios used	*/
/*	primarily to obtain the first value and difference.		*/
/*  Return:								*/
/*	# of samples returned on success.				*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int unpack_steim1
   (FRAME	*pf,		/* ptr to Steim1 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data.			*/
    char	**p_errmsg)	/* ptr to ptr to error message.		*/
{
    int		*diff = diffbuff;
    int		*data = databuff;
    int		*prev;
    int		num_data_frames = nbytes / sizeof(FRAME);
    int		nd = 0;		/* # of data points in packet.		*/
    int		fn;		/* current frame number.		*/
    int		wn;		/* current work number in the frame.	*/
    int		c;		/* current compression flag.		*/
    int		fast = 0;	/* flag for fast decompression.		*/
    int		nr, last_data, i;
    int		itmp;
    short int	stmp;
    int		swapflag;
    unsigned int ctrl;
    static char	errmsg[256];

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);
    if (num_data_frames * sizeof(FRAME) != nbytes) return (MS_ERROR);
    if (num_samples < 0) return (MS_ERROR);
    if (num_samples == 0) return (0);
    if (req_samples < 0) {
	req_samples = -req_samples;
	fast = 1;
    }

    /* Extract forward and reverse integration constants in first frame.*/
    *px0 = X0;
    *pxn = XN;
    if (swapflag) {
	swab4 (px0);
	swab4 (pxn);
    }

    /*	Decode compressed data in each frame.				*/
    for (fn = 0; fn < num_data_frames; fn++) {
	if (fast && nd >= req_samples) break;
	ctrl = pf->ctrl;
	if (swapflag) swab4 ((int *)&ctrl);
	for (wn = 0; wn < VALS_PER_FRAME; wn++) {
	    if (nd >= num_samples) break;
	    if (fast && nd >= req_samples) break;
	    c = (ctrl >> ((VALS_PER_FRAME-wn-1)*2)) & 0x3;
	    switch (c) {
		case STEIM1_SPECIAL_MASK:
		    /* Headers info -- skip it.				*/
		    break;
		case STEIM1_BYTE_MASK:
		    /* Next 4 bytes are 4 1-byte differences.		*/
		    /* NOTE: THIS CODE ASSUMES THAT CHAR IS SIGNED.	*/
		    for (i=0; i<4 && nd<num_samples; i++,nd++)
#ifdef	BROKEN
		    if (swapflag) 
			*diff++ = pf->w[wn].byte[3-i];
		    else
#endif
			*diff++ = pf->w[wn].byte[i];
		    break;
		case STEIM1_HALFWORD_MASK:
		    /* Next 4 bytes are 2 2-byte differences.		*/
		    for (i=0; i<2 && nd<num_samples; i++,nd++) {
			if (swapflag) {
#ifdef	BROKEN
			    stmp = pf->w[wn].hw[1-i];
#else
			    stmp = pf->w[wn].hw[i];
#endif
			    swab2 (&stmp);
			    *diff++ = stmp;
			}
			else *diff++ = pf->w[wn].hw[i];
		    }
		    break;
		case STEIM1_FULLWORD_MASK:
		    /* Next 4 bytes are 1 4-byte difference.		*/
		    if (swapflag) {
			itmp = pf->w[wn].fw;
			swab4 (&itmp);
			*diff++ = itmp;
		    }
		    else *diff++ = pf->w[wn].fw;
		    nd++;
		    break;
		default:
		    /* Should NEVER get here.				*/
		    fprintf (info, "Error: unpack_steim1 - invalid ck = %d\n", c);
		    fflush (info);
		    if (QLIB2_CLASSIC) exit(1);
		    return (MS_ERROR);
		    break;
	    }
	}
	++pf;
    }

    /*	For now, assume sample count in header to be correct.		*/
    /*	One way of "trimming" data from a block is simply to reduce	*/
    /*	the sample count.  It is not clear from the documentation	*/
    /*	whether this is a valid or not, but it appears to be done	*/
    /*	by other program, so we should not complain about its effect.	*/
    nr = req_samples;

    /* Compute first value based on last_value from previous buffer.	*/
    /* The two should correspond in all cases EXCEPT for the first	*/
    /* record for each component (because we don't have a valid xn from	*/
    /* a previous record).  Although the Steim compression algorithm	*/
    /* defines x(-1) as 0 for the first record, this only works for the	*/
    /* first record created since coldstart of the datalogger, NOT the	*/
    /* first record of an arbitrary starting record for an event.	*/

    /* In all cases, assume x0 is correct, since we don't have x(-1).	*/
    data = databuff;
    diff = diffbuff;
    last_data = *px0;
    if (nr > 0) *data = *px0; 

    /* Compute all but first values based on previous value.		*/
    /* Compute all data values in order to compare last value with xn,	*/
    /* but only return the number of values desired by calling routine.	*/
    prev = data - 1;
    while (--nr > 0 && --nd > 0)
	last_data = *++data = *++diff + *++prev;

    if (! fast) {
	while (--nd > 0)
	    last_data = *++diff + last_data;

	/* Verify that the last value is identical to xn.		*/
	if (last_data != *pxn) {
	    sprintf(errmsg, "%s, last_data=%d, xn=%d\n", 
		    "Data integrity for STEIM1 data frame",
		    last_data, *pxn);
	    if (p_errmsg) *p_errmsg = errmsg;
	    else fprintf (info, errmsg);
	    return (MS_ERROR);
	}
    }

    return ((req_samples<num_samples) ? req_samples : num_samples);
}

/************************************************************************/
/*  unpack_steim2:							*/
/*	Unpack STEIM2 data frames and place in supplied buffer.		*/
/*	Data is divided into frames.					*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	Fast decompression does not decompress all frames, and does not	*/
/*	verify that the last sample == xN.  Fast decompression ios used	*/
/*	primarily to obtain the first value and difference.		*/
/*  Return:								*/
/*	# of samples returned on success.				*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int unpack_steim2 
   (FRAME	*pf,		/* ptr to Steim2 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data.			*/
    char	**p_errmsg)	/* ptr to ptr to error message.		*/
{
    int		*diff = diffbuff;
    int		*data = databuff;
    int		*prev;
    int		num_data_frames = nbytes / sizeof(FRAME);
    int		nd = 0;		/* # of data points in packet.		*/
    int		fn;		/* current frame number.		*/
    int		wn;		/* current work number in the frame.	*/
    int		c;		/* current compression flag.		*/
    int		fast = 0;	/* flag for fast decompression.		*/
    int		nr, last_data, i;
    int		n, bits, m1, m2;
    int		val, dnib;
    int		swapflag;
    unsigned int ctrl;
    static char	errmsg[256];

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);
    if (num_data_frames * sizeof(FRAME) != nbytes) return (MS_ERROR);
    if (num_samples < 0) return (MS_ERROR);
    if (num_samples == 0) return (0);
    if (req_samples < 0) {
	req_samples = -req_samples;
	fast = 1;
    }

    /* Extract forward and reverse integration constants in first frame.*/
    *px0 = X0;
    *pxn = XN;
    if (swapflag) {
	swab4 (px0);
	swab4 (pxn);
    }

    /*	Decode compressed data in each frame.				*/
    for (fn = 0; fn < num_data_frames; fn++) {
	if (fast && nd >= req_samples) break;
	ctrl = pf->ctrl;
	if (swapflag) swab4 ((int *)&ctrl);
	for (wn = 0; wn < VALS_PER_FRAME; wn++) {
	    if (nd >= num_samples) break;
	    if (fast && nd >= req_samples) break;
	    c = (ctrl >> ((VALS_PER_FRAME-wn-1)*2)) & 0x3;
	    switch (c) {
	      case STEIM2_SPECIAL_MASK:
		/* Headers info -- skip it.				*/
		break;
	      case STEIM2_BYTE_MASK:
		/* Next 4 bytes are 4 1-byte differences.		*/
		/* NOTE: THIS CODE ASSUMES THAT CHAR IS SIGNED.	*/
		for (i=0; i<4 && nd<num_samples; i++,nd++)
		    *diff++ = pf->w[wn].byte[i];
		break;
	      case STEIM2_123_MASK:
		val = pf->w[wn].fw;
		if (swapflag) swab4((int *)&val);
		dnib =  val >> 30 & 0x3;
		switch (dnib) {
		  case 1:	/*	1 30-bit difference.		*/
		    bits = 30; n = 1; m1 = 0x3fffffff; m2 = 0x20000000; break;
		  case 2:	/*  2 15-bit differences.		*/
		    bits = 15; n = 2; m1 = 0x00007fff; m2 = 0x00004000; break;
		  case 3:	/*  3 10-bit differences.		*/
		    bits = 10; n = 3; m1 = 0x000003ff; m2 = 0x00000200; break;
		  default:	/*	should NEVER get here.		*/
		    sprintf (errmsg, "invalid ck, dnib, fn, wn = %d, %d, %d, %d\n", 
			     c, dnib, fn, wn);
		    if (p_errmsg) *p_errmsg = errmsg;
		    else fprintf (info, errmsg);
		    return(MS_ERROR);
		    break;
		}
		/*  Uncompress the differences.			*/
		for (i=(n-1)*bits; i>=0 && nd<num_samples; i-=bits,nd++) {
		    *diff = (val >> i) & m1;
		    *diff = (*diff & m2) ? *diff | ~m1 : *diff;
		    diff++;
		}
		break;
	      case STEIM2_567_MASK:
		val = pf->w[wn].fw;
		if (swapflag) swab4((int *)&val);
		dnib =  val >> 30 & 0x3;
		switch (dnib) {
		  case 0:	/*  5 6-bit differences.		*/
		    bits = 6; n = 5; m1 = 0x0000003f; m2 = 0x00000020; break;
		  case 1:	/*  6 5-bit differences.		*/
		    bits = 5; n = 6; m1 = 0x0000001f; m2 = 0x00000010; break;
		  case 2:	/*  7 4-bit differences.		*/
		    bits = 4; n = 7; m1 = 0x0000000f; m2 = 0x00000008; break;
		  default:
		    sprintf (errmsg, "invalid ck, dnib, fn, wn = %d, %d, %d, %d\n",
			     c, dnib, fn, wn);
		    if (p_errmsg) *p_errmsg = errmsg;
		    else fprintf (info, errmsg);
		    return(MS_ERROR);
		    break;
		}
		/*  Uncompress the differences.			*/
		for (i=(n-1)*bits; i>=0 && nd < num_samples; i-=bits,nd++) {
		    *diff = (val >> i) & m1;
		    *diff = (*diff & m2) ? *diff | ~m1 : *diff;
		    diff++;
		}
		break;
	      default:
		/* Should NEVER get here.				*/
		fprintf (info, "Error: unpack_steim2 - invalid ck, fn, wn = %d, %d %d\n", c);
		fflush (info);
		if (QLIB2_CLASSIC) exit(1);
		return (MS_ERROR);
		break;
	    }
	}
	++pf;
    }

    /*	For now, assume sample count in header to be correct.		*/
    /*	One way of "trimming" data from a block is simply to reduce	*/
    /*	the sample count.  It is not clear from the documentation	*/
    /*	whether this is a valid or not, but it appears to be done	*/
    /*	by other program, so we should not complain about its effect.	*/
    nr = req_samples;

    /* Compute first value based on last_value from previous buffer.	*/
    /* The two should correspond in all cases EXCEPT for the first	*/
    /* record for each component (because we don't have a valid xn from	*/
    /* a previous record).  Although the Steim compression algorithm	*/
    /* defines x(-1) as 0 for the first record, this only works for the	*/
    /* first record created since coldstart of the datalogger, NOT the	*/
    /* first record of an arbitrary starting record for an event.	*/

    /* In all cases, assume x0 is correct, since we don't have x(-1).	*/
    data = databuff;
    diff = diffbuff;
    last_data = *px0;
    if (nr > 0) *data = *px0; 

    /* Compute all but first values based on previous value.		*/
    /* Compute all data values in order to compare last value with xn,	*/
    /* but only return the number of values desired by calling routine.	*/
    prev = data - 1;
    while (--nr > 0 && --nd > 0)
	last_data = *++data = *++diff + *++prev;

    if (! fast) {
	while (--nd > 0)
	    last_data = *++diff + last_data;

	/* Verify that the last value is identical to xn.		*/
	if (last_data != *pxn) {
	    sprintf(errmsg, "%s, last_data=%d, xn=%d\n", 
		    "Data integrity for STEIM2 data frame",
		    last_data, *pxn);
	    if (p_errmsg) *p_errmsg = errmsg;
	    else fprintf (info, errmsg);
	    return (MS_ERROR);
	    }
    }

    return ((req_samples<num_samples) ? req_samples : num_samples);
}

/************************************************************************/
/*  unpack_int_16:							*/
/*	Unpack int_16 miniSEED data and place in supplied buffer.	*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	(not useful with this data format).				*/
/*  Return:								*/
/*	# of samples returned on success.				*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int unpack_int_16 
   (short int	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data.			*/
    char	**p_errmsg)	/* ptr to ptr to error message.		*/
{
    int		*data = databuff;
    int		nd = 0;		/* # of data points in packet.		*/
    short int	stmp;
    int		swapflag;
    static char	errmsg[256];

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    if (num_samples < 0) return (MS_ERROR);
    if (req_samples < 0) req_samples = -req_samples;

    for (nd=0; nd<req_samples && nd<num_samples; nd++) {
	stmp = ibuf[nd];
	if (swapflag) swab2 (&stmp);
	databuff[nd] = stmp;
    }

    return (nd);
}

/************************************************************************/
/*  unpack_int_32:							*/
/*	Unpack int_32 miniSEED data and place in supplied buffer.	*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	(not useful with this data format).				*/
/*  Return:								*/
/*	# of samples returned on success.				*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int unpack_int_32
   (int		*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data.			*/
    char	**p_errmsg)	/* ptr to ptr to error message.		*/
{
    int		*data = databuff;
    int		nd = 0;		/* # of data points in packet.		*/
    int		itmp;
    int		swapflag;
    static char	errmsg[256];

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    if (num_samples < 0) return (MS_ERROR);
    if (req_samples < 0) req_samples = -req_samples;

    for (nd=0; nd<req_samples && nd<num_samples; nd++) {
	itmp = ibuf[nd];
	if (swapflag) swab4 (&itmp);
	databuff[nd] = itmp;
    }

    return (nd);
}

/* Macro to return i-th bit of byte c (bit 0 is least signficant bit).	*/
#define	getbit(c,i) (c >> i & ~(~0 << 1))

/************************************************************************/
/*  unpack_int_24:							*/
/*	Unpack int_24 miniSEED data and place in supplied buffer.	*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	(not useful with this data format).				*/
/*  Return:								*/
/*	# of samples returned on success.				*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int unpack_int_24
   (unsigned char *ibuf,	/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data.			*/
    char	**p_errmsg)	/* ptr to ptr to error message.		*/
{
    U_DIFF	tmp;
    int		*data = databuff;
    int		nd = 0;		/* # of data points in packet.		*/
    int		swapflag;
    static char	errmsg[256];
    int		sbc;		/* starting byte index for 24-bit copy.	*/
    int		sb24;		/* byte index with 24-bit sign.		*/
    int		sb32;		/* byte index for 32-bit sign.		*/

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    if (num_samples < 0) return (MS_ERROR);
    if (req_samples < 0) req_samples = -req_samples;

    if (my_wordorder == SEED_BIG_ENDIAN) {
	sbc = 1;
	sb24 = 1;
	sb32 = 0;
    }
    else {
	sbc = 0;
	sb24 = 2;
	sb32 = 3;
    }

    /* Copy data from input to output buffer.				*/
    /* Ensure sign bit of input value is properly extended.		*/
    for (nd=0; nd<req_samples && nd<num_samples; nd++) {
	memcpy (&tmp.byte[sbc], ibuf, 3);
	if (swapflag) swab3 ((unsigned char *)&tmp.byte[sbc]);
	/* Propogate sign bit.						*/
	tmp.byte[sb32] = (getbit(tmp.byte[sb24],7)) ? 0xff : 0x00;
	databuff[nd] = tmp.fw;
	ibuf += 3;
    }

    return (nd);
}
