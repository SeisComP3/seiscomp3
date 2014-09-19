/************************************************************************/
/*  Routines for packing STEIM1, STEIM2, INT_32, INT_16, and INT_24	*/
/*  data records.							*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
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

/*	$Id: pack.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__pack_h
#define	__pack_h

#include "qsteim.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int pack_steim1
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_steim2
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

int pad_steim_frame
   (SDF	    	*p_sdf,
    int		fn,	    	/* current frame number.		*/
    int	    	wn,		/* current work number.			*/
    int	    	nf,		/* total number of data frames.		*/
    int		swapflag,	/* flag to swap byte order of data.	*/
    int	    	pad);		/* flag to pad # frames to nf.		*/

extern int pack_int_32 
   (int		p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_int_16
   (short int p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_int_24 
   (unsigned char p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

int pack_text 
   (char 	p_packed[],	/* output data array - packed.		*/
    char	data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

#ifdef	__cplusplus
}
#endif

#endif

