/************************************************************************/
/*  Steim compression information and datatypes.			*/
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

/*	$Id: qsteim.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__qsteim_h
#define	__qsteim_h

#define	STEIM1_SPECIAL_MASK	0
#define	STEIM1_BYTE_MASK	1
#define	STEIM1_HALFWORD_MASK	2
#define	STEIM1_FULLWORD_MASK	3

#define	STEIM2_SPECIAL_MASK	0
#define	STEIM2_BYTE_MASK	1
#define	STEIM2_123_MASK		2
#define	STEIM2_567_MASK		3

#define PACKED __attribute__ ((packed))

typedef union u_diff {			/* union for steim 1 objects.	*/
    signed char	byte[4];		/* 4 1-byte differences.	*/
    short	    hw[2];		/* 2 halfword differences.	*/
    int		    fw;			/* 1 fullword difference.	*/
} PACKED U_DIFF;

typedef struct frame {			/* frame in a seed data record.	*/
    unsigned int    ctrl;		/* control word for frame.	*/
    U_DIFF	    w[15];		/* compressed data.		*/
} PACKED FRAME;

typedef struct seed_data_frames {	/* seed data frames.		*/
    FRAME f[1];				/* data record header frames.	*/
} PACKED SDF;

#endif

