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
 * Copyright (c) 1996-2004 The Regents of the University of California.
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

#ifndef lint
static char sccsid[] = "$Id: ms_unpack.c 2 2005-07-26 19:28:46Z andres $ ";
#endif

#include <stdlib.h>
#include <math.h>
#ifdef	SUNOS4
#include <malloc.h>
#endif

#include "qdefines.h"
#include "msdatatypes.h"
#include "timedef.h"
#include "qsteim.h"
#include "sdr.h"
#include "data_hdr.h"
#include "qtime.h"
#include "qutils.h"
#include "sdr_utils.h"
#include "ms_utils.h"
#include "ms_unpack.h"
#include "unpack.h"

/************************************************************************/
/*  ms_unpack:								*/
/*	Unpack Mini-SEED data and place in supplied buffer.		*/
/*  Return:	# of samples on success, error code on error.		*/
/************************************************************************/
int ms_unpack 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR for Mini-SEED record.*/
    int		max_num_points,	/* max # of points to return.		*/
    char	*ms,		/* ptr to Mini-SEED record.		*/
    void	*data_buffer)	/* ptr to output data buffer.		*/
{
    int blksize;	       	/* blksize of Mini-SEED record.		*/
    BS *bs;			/* ptr to blockette structure.		*/
    BLOCKETTE_1000 *b1000;	/* ptr to blockette 1000.		*/
    int format;
    int datasize;
    int nsamples;
    char *dbuf;
    int *diffbuff;

    /* Determine blocksize and data format from the blockette 1000.	*/
    /* If we don't have one, it is an error.				*/
    if ((bs = find_blockette (hdr, 1000)) == NULL) {
	return (MS_ERROR);
    }
    b1000 = (BLOCKETTE_1000 *)bs->pb;
    format = b1000->format;
    blksize = (int)pow (2., (double)b1000->data_rec_len);
    hdr->blksize = blksize;
    datasize = blksize - hdr->first_data;
    dbuf = (char *)ms + hdr->first_data;

    /* Decide if this is a format that we can decode.			*/
    switch (format) {
      case STEIM1:
	if ((diffbuff = (int *)malloc(hdr->num_samples * sizeof(int))) == NULL) {
	    fprintf (stderr, "Error: unable to malloc diff buffer in ms_read\n");
	    if (QLIB2_CLASSIC) exit(1);
	    return (QLIB2_MALLOC_ERROR);
	}
	nsamples = unpack_steim1 ((FRAME *)dbuf, datasize, hdr->num_samples,
				  max_num_points, (int *)data_buffer, diffbuff, 
				  &hdr->x0, &hdr->xn, hdr->data_wordorder, NULL);
	free ((char *)diffbuff);
	break;
      case STEIM2:
	if ((diffbuff = (int *)malloc(hdr->num_samples * sizeof(int))) == NULL) {
	    fprintf (stderr, "Error: unable to malloc diff buffer in ms_read\n");
	    if (QLIB2_CLASSIC) exit(1);
	    return (QLIB2_MALLOC_ERROR);
	}
	nsamples = unpack_steim2 ((FRAME *)dbuf, datasize, hdr->num_samples,
				  max_num_points, (int *)data_buffer, diffbuff, 
				  &hdr->x0, &hdr->xn, hdr->data_wordorder, NULL);
	free ((char *)diffbuff);
	break;
      case INT_16:
	nsamples = unpack_int_16 ((short *)dbuf, datasize, hdr->num_samples,
				  max_num_points, (int *)data_buffer, 
				  hdr->data_wordorder, NULL);
	break;
      case INT_32:
	nsamples = unpack_int_32 ((int *)dbuf, datasize, hdr->num_samples,
				  max_num_points, (int *)data_buffer, 
				  hdr->data_wordorder, NULL);
	break;
      case INT_24:
	nsamples = unpack_int_24 ((unsigned char *)dbuf, datasize, hdr->num_samples,
				  max_num_points, (int *)data_buffer, 
				  hdr->data_wordorder, NULL);
	break;
     default:
	fprintf (stderr, "Error: Currently unable to read format %d for %s.%s.%s\n", format,
		 hdr->station_id, hdr->network_id, hdr->channel_id);
	if (QLIB2_CLASSIC) exit(1);
	return (MS_ERROR);
    }
    if (nsamples > 0 || hdr->num_samples == 0) {
	return (nsamples);
    }
    return (MS_ERROR);
}

