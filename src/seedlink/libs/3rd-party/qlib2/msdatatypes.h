/************************************************************************/
/*  Data types for SEED data records.					*/
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

/*	$Id: msdatatypes.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	SEED_BIG_ENDIAN

/*  Define UNKNOWN datatype.		*/
#define	UNKNOWN_DATATYPE		0

/*  General datatype codes.		*/
#define	INT_16				1
#define	INT_24				2
#define	INT_32				3
#define	IEEE_FP_SP			4
#define IEEE_FP_DP			5

/*  FDSN Network codes.			*/
#define	STEIM1				10
#define	STEIM2				11
#define	GEOSCOPE_MULTIPLEX_24		12
#define	GEOSCOPE_MULTIPLEX_16_GR_3	13
#define	GEOSCOPE_MULTIPLEX_16_GR_4	14
#define	USNN				15
#define	CDSN				16
#define	GRAEFENBERG_16			17
#define	IPG_STRASBOURG_16		18

/*  Older Network codes.		*/
#define	SRO				30
#define	HGLP				31
#define	DWWSSN_GR			32
#define	RSTN_16_GR			33

/*  Definitions for blockette 1000	*/
#define SEED_LITTLE_ENDIAN		0
#define	SEED_BIG_ENDIAN			1

#define	IS_STEIM_COMP(n)    ((n==STEIM1 || n==STEIM2) ? 1 : 0)

#endif

