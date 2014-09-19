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

/*	$Id: qda_utils.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__qda_utils_h
#define	__qda_utils_h

#include "timedef.h"
#include "qda.h"
#include "seismo.h"

#ifdef	__cplusplus
extern	"C" {
#endif

extern char *get_component_name
   (char	*station,	/* station name.			*/
    int		comp);		/* component number.			*/

extern INT_TIME decode_time_qda
   (QDA_TIME	qt,		/* input time in QDA format.		*/
    short int	millisecond,	/* millisecond offset.			*/
    int		wordorder);	/* wordorder of time contents.		*/

extern void decode_flags_qda
   (int		*pclock,	/* ptr to clock correction (output).	*/
    int		soh,		/* state_of_health flag.		*/
    unsigned char *pa,		/* ptr to activity flag (output);	*/
    unsigned char *pi,		/* ptr to i/o flag (output).		*/
    unsigned char *pq,		/* ptr to quality flag (output).	*/
    int		wordorder);	/* wordorder of time contents.		*/

extern void encode_flags_qda 
   (int		old_soh,	/* old state_of_health flag (input).	*/
    unsigned char *soh,		/* state_of_health flag (output).	*/
    unsigned char pa,		/* activity flag (input).		*/
    unsigned char pi,		/* i/o flag (input).			*/
    unsigned char pq,		/* quality flag (input).		*/
    int		wordorder);	/* wordorder of time contents.		*/

extern DATA_HDR *decode_hdr_qda
   (QDA_HDR	*ihdr,		/* ptr to input header in QDA format.	*/
    int		maxbytes);	/* max # bytes in buffer.		*/

extern int is_qda_header 
   (QDA_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max number of bytes for header.	*/

#ifdef	__cplusplus
}
#endif

#endif

