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

/*	$Id: ms_utils.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__ms_utils_h
#define	__ms_utils_h

#include "stdio.h"
#include "data_hdr.h"
#include "sdr.h"

#ifdef	__cplusplus
extern "C" {
#endif

extern int read_ms 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		max_num_points,	/* max # data points to return.		*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_record 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf for MiniSEED record.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_hdr 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf for MiniSEED record.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_bkt
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*buf,		/* ptr to fixed data header.		*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_data 
   (DATA_HDR	*hdr,		/* pointer to pointer to DATA_HDR.	*/
    char	*buf,		/* pointer to output data buffer.	*/
    int		offset,		/* offset in buffer to write data.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern DATA_HDR *decode_fixed_data_hdr
    (SDR_HDR	*ihdr);		/* MiniSEED header.			*/

extern MS_ATTR get_ms_attr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR structure.		*/

extern int decode_data_format 
   (char	*str);		/* string containing data format.	*/

extern char *encode_data_format 
   (int		format);	/* data format number.			*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to ms_utils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern int f_read_ms_
#else
extern int f_read_ms
#endif
   (DATA_HDR	*fhdr,		/* pointer to FORTRAN DATA_HDR.		*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		*maxpts,	/* max # data points to return.		*/
    FILE	**pfp);		/* FILE pointer for input file.		*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

