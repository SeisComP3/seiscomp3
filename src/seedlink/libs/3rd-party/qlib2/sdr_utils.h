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

/*	$Id: sdr_utils.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__sdr_utils_h
#define	__sdr_utils_h

#include "timedef.h"
#include "sdr.h"
#include "data_hdr.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern INT_TIME decode_time_sdr
   (SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder);	/* wordorder of time contents.		*/

extern SDR_TIME encode_time_sdr
   (INT_TIME	it,		/* IN_TIME structure to decode.		*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern DATA_HDR *decode_hdr_sdr
   (SDR_HDR	*ihdr,		/* input SDR header.			*/
    int		maxbytes);	/* max # bytes in buffer.		*/

extern int eval_rate 
   (int	sample_rate_factor,	/* Fixed data hdr sample rate factor.	*/
    int	sample_rate_mult);	/* Fixed data hdr sample rate multiplier*/

extern char *asc_sdr_time
   (char	*str,		/* string to encode time into.		*/
    SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern time_t unix_time_from_sdr_time
   (SDR_TIME	st,		/* SDR_TIME structure to convert.	*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern int read_blockettes
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*str);		/* ptr to fixed data header.		*/

extern BS *find_blockette 
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    int		n);		/* blockette type to find.		*/

BS *find_pblockette 
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    BS		*bs,		/* BS* to start with.			*/
    int		n);		/* blockette type to find.		*/

extern int blockettecmp
   (BS		*bs1,		/* BS* of first blockette to compare.	*/
    BS		*bs2);		/* BS* of first blockette to compare.	*/

extern int write_blockettes
   (DATA_HDR	*hdr,		/* ptr to data_hdr			*/
    char	*str);		/* ptr to output SDR.			*/

extern int add_blockette
   (DATA_HDR	*hdr,		/* ptr to data_hdr.			*/
    char	*str,		/* pre-constructed blockette.		*/
    int		type,		/* blockette type.			*/
    int		l,		/* length of blockette.			*/
    int		wordorder,	/* wordorder of blockette contents.	*/
    int		where);		/* i -> i-th blockette from start,	*/
				/* -1 -> append as last blockette.	*/

extern int delete_blockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		n);		/* blockette # to delete.  -1 -> ALL.	*/

extern int delete_pblockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    BS		*dbs);		/* BS* to delete.			*/

extern int add_required_miniseed_blockettes 
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR.			*/

extern void init_data_hdr 
   (DATA_HDR	    *hdr);	/* ptr to DATA_HDR to initialize.	*/

extern DATA_HDR *new_data_hdr (void);	

extern DATA_HDR *copy_data_hdr
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src);	/* ptr to source DATA_HDR to copy.	*/

extern DATA_HDR *dup_data_hdr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR to duplicate.	*/

extern void free_data_hdr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR to free.	*/

extern void dump_hdr 
   (DATA_HDR	*h,		/* ptr to Data_Hdr structure.		*/
    char	*str,		/* write debugging info into string.	*/
    int		date_fmt);	/* format specified for date/time.	*/

extern void dump_sdr_flags
   (DATA_HDR	*ch,		/* DATA_HDR for record.			*/
    int		detail);	/* dump only non-zero field if false.	*/

extern int is_sdr_header 
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max number of bytes for header.	*/

extern int is_sdr_vol_header
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max	number of bytes for header.	*/

extern int decode_seqno 
   (char	*str,		/* string containing ascii seqno.	*/
    char	**pp);		/* ptr to next char after seqno.	*/

extern char *q_clock_model
   (char	clock_model);	/* numeric clock model from blockette.	*/

extern char *q_clock_status
   (char	*status,	/* status string from blockette.	*/
    char	clock_model);	/* numeric clock model from blockette.	*/

extern int swab_blockette
   (int		type,		/* blockette number.			*/
    char	*contents,	/* string containing blockette.		*/
    int		len);		/* length of blockette (incl header).	*/

extern int init_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for sdr data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for sdr record.	*/
    BS		*extra_bs);	/* ptr to block-specific blockettes.	*/

extern int update_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for SDR data hdr.	*/
    DATA_HDR	*hdr);		/* initial DATA_HDR for SDR record.	*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to sdr_utils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern void f_init_data_hdr_
#else
extern void f_init_data_hdr
#endif
   (DATA_HDR	    *hdr);	/* ptr to DATA_HDR to initialize.	*/

#ifdef	fortran_suffix
extern int f_delete_blockette_
#else
extern int f_delete_blockette
#endif
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		*n);		/* blockette # to delete.  -1 -> ALL.	*/

#endif

#ifdef	fortran_suffix
extern void f_copy_data_hdr_
#else
extern void f_copy_data_hdr
#endif
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src);	/* ptr to source DATA_HDR to copy.	*/

#ifdef	__cplusplus
}
#endif

#endif

