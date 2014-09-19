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

/*	$Id: unpack.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__unpack_h
#define	__unpack_h

#include "qsteim.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int unpack_steim1
   (FRAME	*pf,		/* ptr to Steim1 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_steim2 
   (FRAME	*pf,		/* ptr to Steim2 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_16 
   (short int	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_32
   (int		*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_24
   (unsigned char *ibuf,	/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

#ifdef	__cplusplus
}
#endif

#endif

