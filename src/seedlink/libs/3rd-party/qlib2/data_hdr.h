/************************************************************************/
/*  data_hdr structure used to store parsed Mini-SEED header and	*/
/*  blockettes in an accessible manner.					*/
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

/*	$Id: data_hdr.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__data_hdr_h
#define	__data_hdr_h

#define	DH_STATION_LEN	5
#define	DH_CHANNEL_LEN	3
#define	DH_LOCATION_LEN	2
#define DH_NETWORK_LEN	2

#include "timedef.h"
#include "msdatatypes.h"

/* Linked list structure for storing blockettes.			*/
typedef struct _bs {			/* blockette structure.		*/
    char	*pb;			/* ptr to actual blockette.	*/
    unsigned short int type;		/* blockette number.		*/
    unsigned short int len;		/* length of blockette in bytes.*/
    unsigned short int wordorder;	/* wordorder of blockette.	*/
    struct _bs	*next;			/* ptr to next blockette struct.*/
} BS;

/* Data header structure, containing SEED Fixed Data Header info	*/
/* as well as other useful info.					*/
typedef struct	data_hdr {
    int		seq_no;			/* sequence number		*/
    char	station_id[DH_STATION_LEN+1];	/* station name		*/
    char	location_id[DH_LOCATION_LEN+1];	/* location id		*/
    char	channel_id[DH_CHANNEL_LEN+1];	/* channel name		*/
    char	network_id[DH_NETWORK_LEN+1];	/* network id		*/
    INT_TIME	begtime;		/* begin time with corrections	*/
    INT_TIME	endtime;		/* end time of packet		*/
    INT_TIME	hdrtime;		/* begin time in hdr		*/
    int		num_samples;		/* number of samples		*/
    int		num_data_frames;	/* number of data frames	*/
    int		sample_rate;		/* sample rate			*/
    int		sample_rate_mult;	/* sample rate multiplier.	*/
    int		num_blockettes;		/* # of blockettes (0)		*/
    int		num_ticks_correction;	/* time correction in ticks	*/
    int		first_data;		/* offset to first data		*/
    int		first_blockette;	/* offset of first blockette	*/
    BS		*pblockettes;		/* ptr to blockette structures	*/
    int		data_type;		/* data_type (for logs or data)	*/
    int		blksize;		/* blocksize of record (bytes).	*/
    unsigned char activity_flags;	/* activity flags		*/
    unsigned char io_flags;		/* i/o flags			*/
    unsigned char data_quality_flags;	/* data quality flags		*/
    unsigned char hdr_wordorder;	/* wordorder of header.		*/
    unsigned char data_wordorder;	/* wordorder of data.		*/
    char	record_type;		/* record type (D,R,Q or V)	*/
    char	cextra[2];		/* future expansion.		*/
    int		x0;			/* first value (STEIM compress)	*/
    int		xn;			/* last value (STEIM compress)	*/
    int		xm1;			/* last value in prev record	*/
    int		xm2;			/* next to last val in prev rec	*/
    float	rate_spsec;		/* blockette 100 sample rate	*/
} DATA_HDR;

/* Attribute structure for a specific data_hdr and blksize.		*/
typedef struct _ms_attr{
    int sample_size;			/* # bytes for sample (0=NA)	*/
    int	alignment;			/* alignment requirement (1 min)*/
    int nframes;			/* # of frame in use (0=NA)	*/
    int framelimit;			/* max # of frames  (0=NA)	*/
    int nbytes;				/* # of bytes in use		*/
    int bytelimit;			/* max # of bytes		*/
} MS_ATTR;

#endif

