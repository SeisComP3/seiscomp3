/************************************************************************
 *  Routines for unpacking STEIM1, STEIM2, INT_16, and INT_32           
 *  data records.							
 *									
 *	Douglas Neuhauser						
 *	Seismographic Station						
 *	University of California, Berkeley				
 *	doug@seismo.berkeley.edu					
 *									
 *									
 *  Changes:	       							
 *									
 *  2003.10.07
 *  - Some more rearrangement for better logging (using SLlog log params)
 *  with sl_log_rl().
 *
 *  2003.09.19
 *  - Get rid of the check for bad frame count in the Steim 1 and 2
 *  unpack routines, it was meaningless as the frame count was just
 *  calculated with the information being used to check it.
 *
 *  2003.07.26
 *  - Set the decompression error flag in the SLCD.
 *  - Minor renaming updates.
 *
 *  2003.06.05
 *  - Declare the unpack_x routines as static. 
 *
 *  2003.03.02
 *  - Move needed header stuff into the original file.    	
 *  - Add an interface function msh_unpack() to work within the context	
 *    of libslink.
 *  - port to use libslink functions usage [tswapx() and sl_log()].
 *  - change to use uintXX_t type declarations for fixed integer types.
 *  - cleanup for clarity and remove some minor debugging/unused code.
 *
 *
 *  Modified by Chad Trabant, ORFEUS/EC-Project MEREDIAN		
 *
 *  modified: 2006.344
 ************************************************************************/

/*
 * Copyright (c) 1996 The Regents of the University of California.
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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "libslink.h"

/* Data types */
#define STEIM1                  10
#define STEIM2                  11
#define INT_16                  1
#define INT_32                  3

#define	VALS_PER_FRAME	(16-1)		/* # of ints for data per frame.*/

#define STEIM1_SPECIAL_MASK     0
#define STEIM1_BYTE_MASK        1
#define STEIM1_HALFWORD_MASK    2
#define STEIM1_FULLWORD_MASK    3

#define STEIM2_SPECIAL_MASK     0
#define STEIM2_BYTE_MASK        1
#define STEIM2_123_MASK         2
#define STEIM2_567_MASK         3

#define	X0  pf->w[0].fw
#define	XN  pf->w[1].fw

typedef union u_diff {                  /* union for steim 1 objects.   */
    int8_t          byte[4];            /* 4 1-byte differences.        */
    int16_t         hw[2];              /* 2 halfword differences.      */
    int32_t         fw;                 /* 1 fullword difference.       */
} SLP_PACKED U_DIFF;

typedef struct frame {                  /* frame in a seed data record. */
    uint32_t	    ctrl;               /* control word for frame.      */
    U_DIFF          w[15];              /* compressed data.             */
} SLP_PACKED FRAME;


/* Internal unpacking routines */
static int unpack_steim1 (FRAME*, int, int, int, int32_t*, int32_t*,
			  int32_t*, int32_t*, int, SLlog*);
static int unpack_steim2 (FRAME*, int, int, int, int32_t*, int32_t*,
			  int32_t*, int32_t*, int, SLlog*);
static int unpack_int_16 (int16_t*, int, int, int, int32_t*, int);
static int unpack_int_32 (int32_t*, int, int, int, int32_t*, int);


/************************************************************************/
/*  sl_msr_unpack():							*/
/*  Unpack Mini-SEED data for a given SLMSrecord.  The data is accessed	*/
/*  in the record indicated by SLMSrecord->msrecord and the unpacked	*/
/*  samples are placed in SLMSrecord->datasamples.  The resulting data	*/
/*  samples are 32-bit integers in host order.				*/
/*                                                                      */
/*  Return number of samples unpacked or -1 on error.                   */
/************************************************************************/
int
sl_msr_unpack (SLlog * log, SLMSrecord * msr, int swapflag)
{
  int     i;
  int     blksize;		/* byte size of Mini-SEED record	*/
  int     format;		/* SEED data encoding value		*/
  int     datasize;             /* byte size of data samples in record 	*/
  int     nsamples;		/* number of samples unpacked		*/
  int     unpacksize;		/* byte size of unpacked samples	*/
  const char *dbuf;
  int32_t    *diffbuff;
  int32_t     x0, xn;

  /* Reset the error flag */
  msr->unpackerr = MSD_NOERROR;

  /* Determine data format and blocksize from Blockette 1000 */
  if ( msr->Blkt1000 != NULL )
    {
      format = msr->Blkt1000->encoding;
      for (blksize = 1, i = 1; i <= msr->Blkt1000->rec_len; i++)
	blksize *= 2;
    }
  else
    {
      sl_log_rl (log, 2, 0, "msr_unpack(): No Blockette 1000 found!\n");
      return (-1);
    }

  /* Calculate buffer size needed for unpacked samples */
  unpacksize = msr->fsdh.num_samples * sizeof(int32_t);

  /* Allocate space for the unpacked data */
  if ( msr->datasamples != NULL )
    msr->datasamples = (int32_t *) malloc (unpacksize);
  else
    msr->datasamples = (int32_t *) realloc (msr->datasamples, unpacksize);

  datasize = blksize - msr->fsdh.begin_data;
  dbuf = msr->msrecord + msr->fsdh.begin_data;

  /* Decide if this is a format that we can decode.			*/
  switch (format)
    {
      
    case STEIM1:
      if ((diffbuff = (int32_t *) malloc(unpacksize)) == NULL)
	{
	  sl_log_rl (log, 2, 0, "unable to malloc diff buffer in msr_unpack()\n");
	  return (-1);
	}
      
      sl_log_rl (log, 1, 2, "Unpacking Steim-1 data frames\n");

      nsamples = unpack_steim1 ((FRAME *)dbuf, datasize, msr->fsdh.num_samples,
				msr->fsdh.num_samples, msr->datasamples, diffbuff, 
				&x0, &xn, swapflag, log);
      free (diffbuff);
      break;
      
    case STEIM2:
      if ((diffbuff = (int32_t *) malloc(unpacksize)) == NULL)
	{
	  sl_log_rl (log, 2, 0, "unable to malloc diff buffer in msr_unpack()\n");
	  return (-1);
	}
      
      sl_log_rl (log, 1, 2, "Unpacking Steim-2 data frames\n");

      nsamples = unpack_steim2 ((FRAME *)dbuf, datasize, msr->fsdh.num_samples,
				msr->fsdh.num_samples, msr->datasamples, diffbuff,
				&x0, &xn, swapflag, log);
      free (diffbuff);
      break;
      
    case INT_16:
      sl_log_rl (log, 1, 2, "Unpacking INT-16 data samples\n");

      nsamples = unpack_int_16 ((int16_t *)dbuf, datasize, msr->fsdh.num_samples,
				msr->fsdh.num_samples, msr->datasamples,
				swapflag);
      break;
      
    case INT_32:
      sl_log_rl (log, 1, 2, "Unpacking INT-32 data samples\n");

      nsamples = unpack_int_32 ((int32_t *)dbuf, datasize, msr->fsdh.num_samples,
				msr->fsdh.num_samples, msr->datasamples,
				swapflag);
      break;
      
    default:
      sl_log_rl (log, 2, 0, "Unable to unpack format %d for %.5s.%.2s.%.2s.%.3s\n", format,
	      msr->fsdh.station, msr->fsdh.network,
	      msr->fsdh.location, msr->fsdh.channel);

      msr->unpackerr = MSD_UNKNOWNFORMAT;
      return (-1);
    }
  
  if (nsamples > 0 || msr->fsdh.num_samples == 0)
    {
      return (nsamples);
    }
  if (nsamples < 0)
    {
      msr->unpackerr = nsamples;
    }
  
  return (-1);
}


/************************************************************************/
/*  unpack_steim1:							*/
/*	Unpack STEIM1 data frames and place in supplied buffer.		*/
/*	Data is divided into frames.					*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	Fast decompression does not decompress all frames, and does not	*/
/*	verify that the last sample == xN.  Fast decompression is used	*/
/*	primarily to obtain the first value and difference.		*/
/*  Return: # of samples returned; negative unpack/decomp flag on error.*/
/************************************************************************/
int unpack_steim1
   (FRAME	*pf,		/* ptr to Steim1 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int32_t	*databuff,	/* ptr to unpacked data array.		*/
    int32_t	*diffbuff,	/* ptr to unpacked diff array.		*/
    int32_t	*px0,		/* return X0, first sample in frame.	*/
    int32_t	*pxn,		/* return XN, last sample in frame.	*/
    int		swapflag,	/* if data should be swapped.	        */
    SLlog       *log)
{
    int32_t    	*diff = diffbuff;
    int32_t	*data = databuff;
    int32_t	*prev;
    int		num_data_frames = nbytes / sizeof(FRAME);
    int		nd = 0;		/* # of data points in packet.		*/
    int		fn;		/* current frame number.		*/
    int		wn;		/* current work number in the frame.	*/
    int		compflag;      	/* current compression flag.		*/
    int		fast = 0;	/* flag for fast decompression.		*/
    int		nr, i;
    int32_t	last_data;
    int32_t	itmp;
    int16_t	stmp;
    uint32_t	ctrl;

    if (num_samples < 0) return (MSD_BADSAMPCOUNT);
    if (num_samples == 0) return (0);
    if (req_samples < 0) {
	req_samples = -req_samples;
	fast = 1;
    }

    /* Extract forward and reverse integration constants in first frame */
    *px0 = X0;
    *pxn = XN;

    if ( swapflag )
      {
	sl_gswap4 (px0);
	sl_gswap4 (pxn);
      }
    
    /*	Decode compressed data in each frame.				*/
    for (fn = 0; fn < num_data_frames; fn++)
      {
	if (fast && nd >= req_samples)
	  break;
	
	ctrl = pf->ctrl;
	if ( swapflag ) sl_gswap4 (&ctrl);
	
	for (wn = 0; wn < VALS_PER_FRAME; wn++)
	  {
	    if (nd >= num_samples) break;
	    if (fast && nd >= req_samples) break;

	    compflag = (ctrl >> ((VALS_PER_FRAME-wn-1)*2)) & 0x3;

	    switch (compflag)
	      {
		
	      case STEIM1_SPECIAL_MASK:
		/* Headers info -- skip it.				*/
		break;
		
	      case STEIM1_BYTE_MASK:
		/* Next 4 bytes are 4 1-byte differences.		*/
		for (i=0; i < 4 && nd < num_samples; i++, nd++)
		  *diff++ = pf->w[wn].byte[i];
		break;
		
	      case STEIM1_HALFWORD_MASK:
		/* Next 4 bytes are 2 2-byte differences.		*/
		for (i=0; i < 2 && nd < num_samples; i++, nd++)
		  {
		    if (swapflag)
		      {
			stmp = pf->w[wn].hw[i];
			if ( swapflag ) sl_gswap2 (&stmp);
			*diff++ = stmp;
		      }
		    else *diff++ = pf->w[wn].hw[i];
		  }
		break;
		
	      case STEIM1_FULLWORD_MASK:
		/* Next 4 bytes are 1 4-byte difference.		*/
		if (swapflag)
		  {
		    itmp = pf->w[wn].fw;
		    if ( swapflag ) sl_gswap4 (&itmp);
		    *diff++ = itmp;
		  }
		else *diff++ = pf->w[wn].fw;
		nd++;
		break;
		
	      default:
		/* Should NEVER get here.				*/
		sl_log_rl (log, 2, 0, "unpack_steim1(): invalid cf = %d\n", compflag);
		return (MSD_STBADCOMPFLAG);
		break;
	      }
	  }
	++pf;
      }
    
    /* Test if the number of samples implied by the data frames is the
     * same number indicated in the header
     */
    if ( nd != num_samples )
      sl_log_rl (log, 2, 2,
		 "unpack_steim1(): number of samples indicated in header (%d) does not equal data (%d)\n",
		 num_samples, nd);

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
    if (nr > 0)
      *data = *px0;

    /* Compute all but first values based on previous value.		*/
    /* Compute all data values in order to compare last value with xn,	*/
    /* but only return the number of values desired by calling routine.	*/

    prev = data - 1;
    while (--nr > 0 && --nd > 0)
	last_data = *++data = *++diff + *++prev;

    if (! fast)
      {
	while (--nd > 0)
	  last_data = *++diff + last_data;
	
	/* Verify that the last value is identical to xn.		*/
	if (last_data != *pxn)
	  {
	    sl_log_rl (log, 2, 0,
		       "Data integrity for Steim-1 is bad, last_data=%d, xn=%d\n",
		       last_data, *pxn);

	    return (MSD_STBADLASTMATCH);
	  }
      }

    return ((req_samples<num_samples) ? req_samples : num_samples);
}  /* End of unpack_steim1() */


/************************************************************************/
/*  unpack_steim2:							*/
/*	Unpack STEIM2 data frames and place in supplied buffer.		*/
/*	Data is divided into frames.					*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	Fast decompression does not decompress all frames, and does not	*/
/*	verify that the last sample == xN.  Fast decompression is used	*/
/*	primarily to obtain the first value and difference.		*/
/*  Return: # of samples returned; negative unpack/decomp flag on error.*/
/************************************************************************/
int unpack_steim2 
   (FRAME	*pf,		/* ptr to Steim2 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int32_t	*databuff,	/* ptr to unpacked data array.		*/
    int32_t	*diffbuff,	/* ptr to unpacked diff array.		*/
    int32_t	*px0,		/* return X0, first sample in frame.	*/
    int32_t	*pxn,		/* return XN, last sample in frame.	*/
    int		swapflag,	/* if data should be swapped.	        */
    SLlog       *log)
{
    int32_t    	*diff = diffbuff;
    int32_t	*data = databuff;
    int32_t	*prev;
    int		num_data_frames = nbytes / sizeof(FRAME);
    int		nd = 0;		/* # of data points in packet.		*/
    int		fn;		/* current frame number.		*/
    int		wn;		/* current work number in the frame.	*/
    int		compflag;     	/* current compression flag.		*/
    int		fast = 0;	/* flag for fast decompression.		*/
    int		nr, i;
    int		n, bits, m1, m2;
    int32_t	last_data;
    int32_t    	val;
    int8_t	dnib;
    uint32_t	ctrl;

    if (num_samples < 0) return (MSD_BADSAMPCOUNT);
    if (num_samples == 0) return (0);
    if (req_samples < 0) {
	req_samples = -req_samples;
	fast = 1;
    }

    /* Extract forward and reverse integration constants in first frame.*/
    *px0 = X0;
    *pxn = XN;

    if ( swapflag )
      {
	sl_gswap4 (px0);
	sl_gswap4 (pxn);
      }
    
    /*	Decode compressed data in each frame.				*/
    for (fn = 0; fn < num_data_frames; fn++)
      {
	if (fast && nd >= req_samples) break;
	ctrl = pf->ctrl;
	if ( swapflag ) sl_gswap4 (&ctrl);

	for (wn = 0; wn < VALS_PER_FRAME; wn++)
	  {
	    if (nd >= num_samples) break;
	    if (fast && nd >= req_samples) break;

	    compflag = (ctrl >> ((VALS_PER_FRAME-wn-1)*2)) & 0x3;

	    switch (compflag)
	      {
	      case STEIM2_SPECIAL_MASK:
		/* Headers info -- skip it.				*/
		break;

	      case STEIM2_BYTE_MASK:
		/* Next 4 bytes are 4 1-byte differences.		*/
		for (i=0; i < 4 && nd < num_samples; i++, nd++)
		    *diff++ = pf->w[wn].byte[i];
		break;

	      case STEIM2_123_MASK:
		val = pf->w[wn].fw;
		if ( swapflag ) sl_gswap4 (&val);
		dnib =  val >> 30 & 0x3;
		switch (dnib)
		  {
		  case 1:	/*	1 30-bit difference.		*/
		    bits = 30; n = 1; m1 = 0x3fffffff; m2 = 0x20000000; break;
		  case 2:	/*  2 15-bit differences.		*/
		    bits = 15; n = 2; m1 = 0x00007fff; m2 = 0x00004000; break;
		  case 3:	/*  3 10-bit differences.		*/
		    bits = 10; n = 3; m1 = 0x000003ff; m2 = 0x00000200; break;
		  default:	/*	should NEVER get here.		*/
		    sl_log_rl (log, 2, 0,
			       "unpack_steim2(): invalid cf, dnib, fn, wn = %d, %d, %d, %d\n", 
			       compflag, dnib, fn, wn);
		    return (MSD_STBADCOMPFLAG);
		    break;
		}
		/*  Uncompress the differences.			*/
		for (i=(n-1)*bits; i >= 0 && nd < num_samples; i-=bits, nd++)
		  {
		    *diff = (val >> i) & m1;
		    *diff = (*diff & m2) ? *diff | ~m1 : *diff;
		    diff++;
		  }
		break;

	      case STEIM2_567_MASK:
		val = pf->w[wn].fw;
		if ( swapflag ) sl_gswap4 (&val);
		dnib =  val >> 30 & 0x3;
		switch (dnib)
		  {
		  case 0:	/*  5 6-bit differences.		*/
		    bits = 6; n = 5; m1 = 0x0000003f; m2 = 0x00000020; break;
		  case 1:	/*  6 5-bit differences.		*/
		    bits = 5; n = 6; m1 = 0x0000001f; m2 = 0x00000010; break;
		  case 2:	/*  7 4-bit differences.		*/
		    bits = 4; n = 7; m1 = 0x0000000f; m2 = 0x00000008; break;
		  default:
		    sl_log_rl (log, 2, 0,
			       "unpack_steim2(): invalid cf, dnib, fn, wn = %d, %d, %d, %d\n", 
			       compflag, dnib, fn, wn);
		    return (MSD_STBADCOMPFLAG);
		    break;
		}
		/*  Uncompress the differences.			*/
		for (i=(n-1)*bits; i >= 0 && nd < num_samples; i-=bits, nd++)
		  {
		    *diff = (val >> i) & m1;
		    *diff = (*diff & m2) ? *diff | ~m1 : *diff;
		    diff++;
		  }
		break;

	      default:
		/* Should NEVER get here.				*/
		sl_log_rl (log, 2, 0,
			   "unpack_steim2(): invalid cf, fn, wn = %d, %d, %d\n",
			   compflag, fn, wn);
		return (MSD_STBADCOMPFLAG);
		break;
	      }
	  }
	++pf;
      }
    
    /* Test if the number of samples implied by the data frames is the
     * same number indicated in the header
     */
    if ( nd != num_samples )
      sl_log_rl (log, 2, 2,
		 "unpack_steim2(): number of samples indicated in header (%d) does not equal data (%d)\n",
		 num_samples, nd);

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
    if (nr > 0)
      *data = *px0;

    /* Compute all but first values based on previous value.		*/
    /* Compute all data values in order to compare last value with xn,	*/
    /* but only return the number of values desired by calling routine.	*/

    prev = data - 1;
    while (--nr > 0 && --nd > 0)
	last_data = *++data = *++diff + *++prev;

    if (! fast)
      {
	while (--nd > 0)
	  last_data = *++diff + last_data;
	
	/* Verify that the last value is identical to xn.		*/
	if (last_data != *pxn)
	  {
	    sl_log_rl (log, 2, 0,
		       "Data integrity for Steim-2 is bad, last_data=%d, xn=%d\n",
		       last_data, *pxn);
	    return (MSD_STBADLASTMATCH);
	  }
      }
    
    return ((req_samples<num_samples) ? req_samples : num_samples);
}  /* End of unpack_steim2() */


/************************************************************************/
/*  unpack_int_16:							*/
/*	Unpack int_16 miniSEED data and place in supplied buffer.	*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	(not useful with this data format).				*/
/*  Return: # of samples returned; negative unpack/decomp flag on error.*/
/************************************************************************/
int unpack_int_16 
   (int16_t	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int32_t	*databuff,	/* ptr to unpacked data array.		*/
    int		swapflag)       /* if data should be swapped.	        */
{
    int		nd = 0;		/* # of data points in packet.		*/
    uint16_t	stmp;

    if (num_samples < 0) return (MSD_BADSAMPCOUNT);
    if (req_samples < 0) req_samples = -req_samples;

    for (nd=0; nd<req_samples && nd<num_samples; nd++) {
	stmp = ibuf[nd];
	if ( swapflag ) sl_gswap2 (&stmp);
	databuff[nd] = stmp;
    }

    return (nd);
}  /* End of unpack_int_16() */


/************************************************************************/
/*  unpack_int_32:							*/
/*	Unpack int_32 miniSEED data and place in supplied buffer.	*/
/*	If req_samples < 0, perform fast decompression of |req_samples|.*/
/*	(not useful with this data format).				*/
/*  Return: # of samples returned; negative unpack/decomp flag on error.*/
/************************************************************************/
int unpack_int_32
   (int32_t	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int32_t	*databuff,	/* ptr to unpacked data array.		*/
    int		swapflag)	/* if data should be swapped.	        */
{
    int		nd = 0;		/* # of data points in packet.		*/
    int32_t    	itmp;

    if (num_samples < 0) return (MSD_BADSAMPCOUNT);
    if (req_samples < 0) req_samples = -req_samples;

    for (nd=0; nd<req_samples && nd<num_samples; nd++) {
        itmp = ibuf[nd];
	if ( swapflag ) sl_gswap4 (&itmp);
	databuff[nd] = itmp;
    }

    return (nd);
}  /* End of unpack_int_32() */

