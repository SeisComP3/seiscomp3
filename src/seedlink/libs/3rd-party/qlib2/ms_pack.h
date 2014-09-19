/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
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

/*	$Id: ms_pack.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__ms_pack_h
#define	__ms_pack_h

#include "qsteim.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int ms_pack_data
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		num_samples,	/* number of data samples.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_update_hdr
   (DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		num_records,	/* number of mseed records just packed.	*/
    int		num_samples,	/* number of samples just packed.	*/
    int		*data);		/* data buffer used for last ms_pack.	*/

extern int init_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for miniSEED record.*/
    BS		*extra_bs);	/* ptr to block-specific blockettes.	*/

extern int update_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr);		/* initial DATA_HDR for miniSEED record.*/

extern int ms_pack_steim 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*diff,		/* ptr to diff buffer (optional)	*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_int 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to ms_pack routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern int f_ms_pack_data_
#else
extern int f_ms_pack_data
#endif
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    int		*num_samples,	/* number of data samples.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	*p_ms,		/* ptr *miniSEED (required).		*/
    int		*ms_len);	/* miniSEED buffer len (required).	*/

#ifdef	fortran_suffix
extern int ms_pack_update_hdr_
#else
extern int ms_pack_update_hdr
#endif
   (DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		*num_records,	/* number of mseed records just packed.	*/
    int		*num_samples,	/* number of samples just packed.	*/
    int		*data);		/* data buffer used for last ms_pack.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

