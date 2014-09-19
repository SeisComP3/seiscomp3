/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2003 The Regents of the University of California.
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
static char sccsid[] = "$Id: ms_utils.c 346 2006-04-11 20:49:47Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#include "qdefines.h"
#include "msdatatypes.h"
#include "timedef.h"
#include "qsteim.h"
#include "sdr.h"
#include "data_hdr.h"
#include "qtime.h"
#include "qutils.h"
#include "sdr_utils.h"
#include "ms_unpack.h"
#include "ms_utils.h"

#define MAXBLKSIZE	32768
#define	FIXED_DATA_HDR_SIZE 48

typedef struct data_format_table {
    int format;
    char *name;
} FORMAT_ENTRY;

static FORMAT_ENTRY data_format_table[] =
{
    INT_16,		"INT_16",
    INT_24,		"INT_24",
    INT_32,		"INT_32",
    STEIM1,		"STEIM1",
    STEIM2,		"STEIM2",
    UNKNOWN_DATATYPE,	"UNKNOWN",
    UNKNOWN_DATATYPE,	NULL,
};

/************************************************************************/
/*  read_ms:								*/
/*	Read a MiniSEED record, unpack the data, and return to the user	*/
/*	a data_hdr and the unpacked data.				*/
/*  returns:								*/
/*	number of data samples on success.				*/
/*	EOF on eof.							*/
/*	MS_ERROR on MiniSEED error.					*/
/*	QLIB2_MALLOC_ERROR on malloc error.				*/
/************************************************************************/
int read_ms 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		max_num_points,	/* max # data points to return.		*/
    FILE	*fp)		/* FILE pointer for input file.		*/
{
    int status;
    char *pbuf = NULL;		/* ptr to ptr to MiniSEED record.	*/
    int blksize;		/* blksize of MiniSEED record.		*/
    int nsamples;		/* number of data samples unpacked.	*/

    if (max_num_points < 0) return (MS_ERROR);
    if (max_num_points == 0) return (0);
    status = blksize = read_ms_record (phdr, &pbuf, fp);
    if (blksize > 0) {
	status = nsamples = ms_unpack (*phdr, max_num_points,
				      pbuf, data_buffer);
    }
    if (pbuf) free (pbuf);
    return (status);
}

/************************************************************************/
/*  read_ms_record:							*/
/*	Read a MiniSEED record, returning to the user a data_hdr and	*/
/*	the raw record.							*/
/*	If *pbuf == NULL, allocated space for the record.		*/
/*	Otherwise, assume that it points to a valid buffer to use.	*/
/*	If we allocate space for the buffer, caller must free space.	*/
/*  returns:								*/
/*	blksize on success.						*/
/*	EOF on eof.							*/
/*	QLIB2 error code on error.					*/
/************************************************************************/
int read_ms_record 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf ptr for MiniSEED record.	*/
    FILE	*fp)		/* FILE pointer for input file.		*/
{
    int status;			/* return status from functions.	*/
    int offset;			/* offset to data in MiniSEED record.	*/
    int blksize;		/* blksize of MiniSEED record.		*/

    status = offset = read_ms_hdr (phdr, pbuf, fp);
    if (offset > 0) {
	status = blksize = read_ms_data (*phdr, *pbuf, status, fp);
    }
    return (status);
}

/************************************************************************/
/*  read_ms_hdr:							*/
/*	Routine to read Mini-SEED Fixed Data Header and blockettes.	*/
/*	Parses header into data_hdr structure, and writes raw header	*/
/*	and blockettes into user-supplied buffer.			*/
/*  returns:								*/
/*	# of bytes in header and blockettes (up to first_data).		*/
/*	EOF on eof.							*/
/*	QLIB2 error code on error.					*/
/************************************************************************/
int read_ms_hdr 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf for MiniSEED record.	*/
    FILE	*fp)		/* FILE pointer for input file.		*/
{
    char *buf;			/* buffer for hdr and blockettes.	*/
    DATA_HDR *hdr;		/* pointer to DATA_HDR.			*/
    BS *bs;			/* ptr to blockette structure.		*/
    BLOCKETTE_1000 *b1000;	/* prt to blockette 1000.		*/
    int nskip = 0;
    int offset = 0;
    int alloc_buf = 0;
    int nread;
    int bl_limit;		/* offset of data (blksize if no data).	*/

    /* If user supplies a buffer for the raw MiniSEED, use it.		*/
    /* Otherwise, allocate a buffer.					*/
    if (*pbuf == NULL) {
	if ((buf = malloc (MAXBLKSIZE * sizeof(char))) == NULL) {
	    fprintf (stderr, "Error: Unable to allocate buffer in read_ms_hdr\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (QLIB2_MALLOC_ERROR);
	}
	++alloc_buf;
    }
    else {
	buf = *pbuf;
    }

    /* Read and decode SEED Fixed Data Header.				*/
    *phdr = (DATA_HDR *)NULL;
    if ((nread = fread(buf, FIXED_DATA_HDR_SIZE, 1, fp)) != 1) {
	if (alloc_buf) free(buf);
	return ((nread == 0) ? EOF : MS_ERROR);
    }

    offset = FIXED_DATA_HDR_SIZE;
    if ((hdr = decode_fixed_data_hdr((SDR_HDR *)buf)) == NULL) {
	if (alloc_buf) free(buf);
	return (MS_ERROR);
    }

    /* Read blockettes.  Mini-SEED should have at least blockette 1000.	*/
    if (hdr->num_blockettes > 0) {
	if (hdr->first_blockette < offset) {
	    if (alloc_buf) free(buf);
	    free_data_hdr(hdr);
	    return (MS_ERROR);
	}
	if (hdr->first_blockette > offset) {
	    nskip = hdr->first_blockette - offset;
	    if (fread (buf+offset, nskip, 1, fp) != 1) {
		if (alloc_buf) free(buf);
		free_data_hdr(hdr);
		return (MS_ERROR);
	    }
	    offset += nskip;
	}
	if ((offset = read_ms_bkt (hdr, buf, fp)) < 0) {
	    if (alloc_buf) free(buf);
	    free_data_hdr(hdr);
	    return (MS_ERROR);
	}
    }

    /* Determine blocksize and data format from the blockette 1000.	*/
    /* If we don't have one, it is an error.				*/
    if ((bs = find_blockette (hdr, 1000)) == NULL) {
	if (alloc_buf) free(buf);
	free_data_hdr(hdr);
	return (MS_ERROR);
    }

    /* Now reparse the header with all of the blockettes.		*/
    /* This fills in various misc items such as:			*/
    /*  a.  blocksize and data_type from blockette 1000.		*/
    /*  b.  extended time info and frame count from blockette 1001.	*/
    free_data_hdr (hdr);
    if ((hdr = decode_hdr_sdr((SDR_HDR *)buf,offset)) == NULL) {
	if (alloc_buf) free(buf);
	return (MS_ERROR);
    }

    /* If we allocated the buffer, ensure that it is large enough to	*/
    /* hold the full record.						*/
    if (alloc_buf && hdr->blksize > MAXBLKSIZE) {
	if ((buf = realloc(buf, hdr->blksize * sizeof(char))) == NULL) {
	    fprintf (stderr, "Error: Unable to allocate buffer in read_ms_hdr\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    if (alloc_buf) free(buf);
	    free_data_hdr(hdr);
	    return (QLIB2_MALLOC_ERROR);
	}
    }

    /* Skip over space between blockettes (if any) and data.		*/
    bl_limit = (hdr->first_data) ? hdr->first_data : hdr->blksize;
    if (bl_limit < offset) {
	if (alloc_buf) free(buf);
	free_data_hdr(hdr);
	return(MS_ERROR);
    }
    if (bl_limit > offset) {
	nskip = bl_limit - offset;
	if (fread (buf+offset, nskip, 1, fp) != 1) {
	    if (alloc_buf) free(buf);
	    free_data_hdr(hdr);
	    return (MS_ERROR);
	}
	offset += nskip;
    }

    if (alloc_buf) *pbuf = buf;
    *phdr = hdr;
    return (offset);		/* Header successfully read.		*/
}

/************************************************************************/
/*  read_ms_bkt:							*/
/*	Read binary blockettes that follow the SEED fixed data header.	*/
/*  returns:								*/
/*	offset of next byte to be read.					*/
/*	EOF on premature end-of-file.					*/
/*	QLIB2 error code on error.					*/
/************************************************************************/
int read_ms_bkt
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*buf,		/* ptr to fixed data header.		*/
    FILE	*fp)		/* FILE pointer for input file.		*/
{
    BS		*bs, *pbs;
    int		offset, i, bl_limit;
    SEED_UWORD	bl_len, bl_next, bl_type;
    int		bh_len = sizeof(BLOCKETTE_HDR);
    int		blksize = 0;

    if (my_wordorder < 0) get_my_wordorder();
    bs = pbs = (BS *)NULL;
    offset = hdr->first_blockette;
    hdr->pblockettes = (BS *)NULL;
    bl_next = 0;

    /*	Run through each blockette, allocate a linked list structure	*/
    /*	for it, and verify that the blockette structures are OK.	*/
    /*	There is a LOT of checking to ensure proper structure.		*/
    for (i=0; i<hdr->num_blockettes; i++) {

	if (i > 0 && bl_next == 0) {
	    fprintf (stderr, "Error: zero offset to next blockette\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (MS_ERROR);
	}

	if ( (bs=(BS *)malloc(sizeof(BS))) == NULL ) {
	    fprintf (stderr, "Error: unable to malloc BS\n");
	    if (QLIB2_CLASSIC) exit(1);
	    return (QLIB2_MALLOC_ERROR);
	}
	bs->next = (BS *)NULL;
	if (i == 0) hdr->pblockettes = bs;
	else pbs->next = bs;
	pbs = bs;

	/*  Read blockette header.					*/
	if (fread (buf+offset, bh_len, 1, fp) != 1) 
	    return (EOF);

	/*  Decide how much space the blockette takes up.  If we know 	*/
	/*  blockette type, then allocate the appropriate space.	*/
	/*  Otherwise, determine the required space by the offset to	*/
	/*  the next blockette, or by the offset to the first data if	*/
	/*  this is the last blockette.					*/
	/*  If there is not data, then ensure that we know the length	*/
	/*  of the blockette.  If not, consider it to be a fatal error,	*/
	/*  since we have no idea how long it should be.		*/
	/*								*/
	/*  We cannot allow it to extend to the blksize, since we use	*/
	/*  this routine to process blockettes from packed miniSEED	*/
	/*  files.  Packed miniSEED files contain records that are a	*/
	/*  multiple of the packsize (currently 128 bytes) with a block	*/
	/*  whose size is specified in the b1000 blksize field.		*/
	bl_type = ((BLOCKETTE_HDR *)(buf+offset))->type;
	bl_next = ((BLOCKETTE_HDR *)(buf+offset))->next;
	if (hdr->hdr_wordorder != my_wordorder) {
	    swab2 ((short int *)&bl_type);
	    swab2 ((short int *)&bl_next);
	}
	bl_limit = (bl_next) ? bl_next : 
		   (hdr->first_data) ? hdr->first_data :
		   0;
	switch (bl_type) {
	  case 100: bl_len = sizeof (BLOCKETTE_100); break;
	  case 200: bl_len = sizeof (BLOCKETTE_200); break;
	  case 201: bl_len = sizeof (BLOCKETTE_201); break;
	  case 300: bl_len = sizeof (BLOCKETTE_300); break;
	  case 310: bl_len = sizeof (BLOCKETTE_310); break;
	  case 320: bl_len = sizeof (BLOCKETTE_320); break;
	  case 390: bl_len = sizeof (BLOCKETTE_390); break;
	  case 395: bl_len = sizeof (BLOCKETTE_395); break;
	  case 400: bl_len = sizeof (BLOCKETTE_400); break;
	  case 405: bl_len = sizeof (BLOCKETTE_405); break;
	  case 500: bl_len = sizeof (BLOCKETTE_500); break;
	  case 1000: bl_len = sizeof (BLOCKETTE_1000); break;
	  case 1001: bl_len = sizeof (BLOCKETTE_1001); break;
	  default:
	    fprintf (stderr, "Warning: unknown blockette %d\n",bl_type);
	    bl_len = 0;
	    break;
	}

	/* Perform integrity checks on blockette.			*/
	if (bl_len != 0) {
	    /* Known blockettes:					*/
	    /* Check that the presumed blockette length is correct.	*/
	    if (bl_limit > 0 && (int)bl_len > bl_limit-offset) {
		/* Warning only if blockette is too short.		*/
		/* Allow padding between blockettes.			*/
		fprintf (stderr, "Warning: short blockette %d len=%d, expected len=%d\n",
			 bl_type, bl_limit-offset, bl_len);
	    }
	    /* Be safe and extend the effective length of the blockette	*/
	    /* to the limit (next blockette or first data) if there is	*/
	    /* a limit.							*/
	    bl_len = (bl_limit) ? bl_limit - offset : bl_len;
	    /* Check that we do not run into the data portion of record.*/
	    if (hdr->first_data != 0 && (int)bl_len+offset > hdr->first_data) {
		fprintf (stderr, "Warning: blockette %d	at offset=%d len=%d first_data=%d\n",
			 bl_type, bl_limit-offset, bl_len);
		bl_len = bl_limit - offset;
	    }
	}
	else {
	    /* Unknown blockettes:					*/
	    if (bl_limit == 0) {
		fprintf (stderr, "Warning: unknown blockette and no length limit\n");
		return (-1);
	    }
	    /* For unknown blockettes ensure that we have a max len.	*/
	    bl_len = bl_limit - offset;
	}

	if ((bs->pb = (char *)malloc(bl_len))==NULL) {
	    fprintf (stderr, "unable to malloc blockettd\n");
	    return (-1);
	}
	/* Read the body of the blockette, and copy entire blockette.	*/
	if (fread(buf+offset+bh_len, bl_len-bh_len, 1, fp) != 1)
	    return(-1);
	memcpy (bs->pb,buf+offset,bl_len);
	bs->len = bl_len;
	bs->type = bl_type;
	bs->wordorder = hdr->hdr_wordorder;
	if (bl_type == 1000) {
	    blksize = (int)pow(2., (double)((BLOCKETTE_1000 *)(buf+offset))->data_rec_len);
	}
	offset += bl_len;
    }

    /* Ensure there are no more blockettes. */
    if (bl_next != 0) {
	fprintf (stderr, "extra blockette found\n");
	return(-1);
    }
    return (offset);
}

/************************************************************************/
/*  read_ms_data:							*/
/*	Routine to data portion of MiniSEED record.			*/
/*  return:								*/
/*	blksize on success.						*/
/*	MS_ERROR on error.						*/
/************************************************************************/
int read_ms_data 
   (DATA_HDR	*hdr,		/* pointer to pointer to DATA_HDR.	*/
    char	*buf,		/* pointer to output data buffer.	*/
    int		offset,		/* offset in buffer to write data.	*/
    FILE	*fp)		/* FILE pointer for input file.		*/
{
    BS *bs;			/* ptr to blockette structure.		*/
    BLOCKETTE_1000 *b1000;	/* ptr to blockette 1000.		*/
    int format;
    int blksize;
    int datasize;

    /* Determine blocksize and data format from the blockette 1000.	*/
    /* If we don't have one, it is an error.				*/
    if ((bs = find_blockette (hdr, 1000)) == NULL) {
	return (MS_ERROR);
    }
    b1000 = (BLOCKETTE_1000 *)bs->pb;
    format = b1000->format;
    blksize = (int)pow (2., (double)b1000->data_rec_len);
    datasize = (hdr->first_data > 0) ? blksize - hdr->first_data : 0;

    /* If datasize == 0, there is no data, and no action to be taken,	*/
    /* since read_ms_hdr has already read to the end of record.		*/
    if (datasize == 0) {
	return (blksize);
    }

    /* If offset == 0, it implies that the buffer points directly to	*/
    /* where we should put the data portion of the record.		*/
    /* Otherwise, start writing the data portion at the specified	*/
    /* offset in the buffer (presumable to skip header and blockettes).	*/
    if (fread (buf+offset, datasize, 1, fp) != 1) {
	return (MS_ERROR);
    }

    return (blksize);
}

/************************************************************************/
/*  decode_fixed_data_hdr:						*/
/*	Decode SEED Fixed Data Header in the specified buffer,		*/
/*	and return ptr to dynamically allocated DATA_HDR structure.	*/
/*	Fill in structure with the information in a easy-to-use format.	*/
/*	Do not try to parse blockettes -- that will be done later.	*/
/*  return:								*/
/*	DATA_HDR pointer on success.					*/
/*	NULL on failure.						*/
/************************************************************************/
DATA_HDR *decode_fixed_data_hdr
    (SDR_HDR	*ihdr)		/* MiniSEED header.			*/
{
    char	tmp[80];
    DATA_HDR	*ohdr;
    int		seconds, usecs;
    char	*p;
    int		swapflag;	
    int		itmp[2];
    short int	stmp[2];
    unsigned short int ustmp[2];
    int tmp_wordorder;

    if (my_wordorder < 0) get_my_wordorder();

    /* Perform data integrity check, and pick out pertinent header info.*/
    if (! (is_data_hdr_ind (ihdr->data_hdr_ind) || is_vol_hdr_ind (ihdr->data_hdr_ind))) {
	return ((DATA_HDR *)NULL);
    }

    if ((ohdr = new_data_hdr()) == NULL) return (NULL);
    ohdr->record_type = ihdr->data_hdr_ind;
    ohdr->seq_no = atoi (charncpy (tmp, ihdr->seq_no, 6) );

    /* Handle volume header.					    */
    /* Return a pointer to a DATA_HDR structure containing blksize. */
    /* Save actual blockette for later use.			    */
    if (is_vol_hdr_ind(ihdr->data_hdr_ind)) {
	if ((ohdr = new_data_hdr()) == NULL) return (NULL);
	ohdr->record_type = ihdr->data_hdr_ind;
	ohdr->seq_no = atoi (charncpy (tmp, ihdr->seq_no, 6) );
	ohdr->blksize = 4096;	/* default tape blksize.	    */
	p = (char *)ihdr+8;	/* point to start of blockette.	    */
	ohdr->data_type = atoi(charncpy(tmp,p,3));
	switch (ohdr->data_type) {
	  int ok;
	  case 5:
	  case 8:
	  case 10:
	    ohdr->blksize = (int)pow(2.0,atoi(charncpy(tmp,p+11,2)));
	    ok = add_blockette (ohdr, p, ohdr->data_type,
				atoi(charncpy(tmp,p+3,4)), my_wordorder, 0);
	    if (! ok) {
		free_data_hdr(ohdr);
		return ((DATA_HDR *)NULL);
	    }
	    break;
	  default:
	    break;
	}
	return (ohdr);
    }

    /* Determine word order of the fixed record header.			*/
    tmp_wordorder = wordorder_from_time((unsigned char *)&(ihdr->time));
    if (tmp_wordorder < 0) {
	free_data_hdr (ohdr);
	return ((DATA_HDR *)NULL);
    }
    ohdr->hdr_wordorder = tmp_wordorder;
    ohdr->data_wordorder = ohdr->hdr_wordorder;
    swapflag = (ohdr->hdr_wordorder != my_wordorder);
    charncpy (ohdr->station_id, ihdr->station_id, 5);
    charncpy (ohdr->location_id, ihdr->location_id, 2);
    charncpy (ohdr->channel_id, ihdr->channel_id, 3);
    charncpy (ohdr->network_id, ihdr->network_id, 2);
    trim (ohdr->station_id);
    trim (ohdr->location_id);
    trim (ohdr->channel_id);
    trim (ohdr->network_id);
    ohdr->hdrtime = ohdr->begtime = decode_time_sdr(ihdr->time, ohdr->hdr_wordorder);
    if (swapflag) {
	/* num_samples.	*/
	ustmp[0] = ihdr->num_samples;
	swab2 ((short int *)&ustmp[0]);
	ohdr->num_samples = ustmp[0];
	/* data_rate	*/
	stmp[0] = ihdr->sample_rate_factor;
	stmp[1] = ihdr->sample_rate_mult;
	swab2 (&stmp[0]);
	swab2 (&stmp[1]);
	ohdr->sample_rate = stmp[0];
	ohdr->sample_rate_mult = stmp[1];
	/* num_ticks_correction. */
	itmp[0] = ihdr->num_ticks_correction;
	swab4 (&itmp[0]);
	ohdr->num_ticks_correction = itmp[0];
	/* first_data	*/
	ustmp[0] = ihdr->first_data;
	swab2 ((short int *)&ustmp[0]); 
	ohdr->first_data = ustmp[0];
	/* first_blockette */
	ustmp[1] = ihdr->first_blockette;
	swab2 ((short int *)&ustmp[1]);
	ohdr->first_blockette = ustmp[1];
    }
    else {
	ohdr->num_samples = ihdr->num_samples;
	ohdr->sample_rate = ihdr->sample_rate_factor;
	ohdr->sample_rate_mult = ihdr->sample_rate_mult;
	ohdr->num_ticks_correction = ihdr->num_ticks_correction;
	ohdr->first_data = ihdr->first_data;
	ohdr->first_blockette = ihdr->first_blockette;
    }

    /*	WARNING - may need to convert flags to independent format	*/
    /*	if we ever choose a different flag format for the DATA_HDR.	*/
    ohdr->activity_flags = ihdr->activity_flags;
    ohdr->io_flags = ihdr->io_flags;
    ohdr->data_quality_flags = ihdr->data_quality_flags;

    ohdr->num_blockettes = ihdr->num_blockettes;
    ohdr->data_type = 0;		/* assume unknown datatype.	*/
    ohdr->pblockettes = (BS *)NULL;	/* Do not parse blockettes here.*/

    /*	If the time correction has not already been added, we should	*/
    /*	add it to the begtime.  Do NOT change the ACTIVITY flag, since	*/
    /*	it refers to the hdrtime, NOT the begtime/endtime.		*/
    if ( ohdr->num_ticks_correction != 0 && 
	((ohdr->activity_flags & ACTIVITY_TIME_CORR_APPLIED) == 0) ) {
	ohdr->begtime = add_dtime (ohdr->begtime, 
				  (double)ohdr->num_ticks_correction * USECS_PER_TICK);
    }
    time_interval2(ohdr->num_samples - 1, ohdr->sample_rate, ohdr->sample_rate_mult,
		  &seconds, &usecs);
    ohdr->endtime = add_time(ohdr->begtime, seconds, usecs);

    return(ohdr);
}

/************************************************************************/
/*  get_ms_attr:							*/
/*	Compute various attributes of a block, given the data_hdr	*/
/*	and the blocksize.						*/
/*  Return: MS_ATTR							*/
/*	MS_ATTR on success.						*/
/*	MS_ATTR with attr.alignment = 0 on error.			*/
/************************************************************************/
MS_ATTR get_ms_attr
   (DATA_HDR	*hdr)		/* ptr to DATA_HDR structure.		*/
{
    MS_ATTR attr;
    /* Values of 0 may imply NA (Not Applicable).			*/
    switch (hdr->data_type) {
      case UNKNOWN_DATATYPE:
	attr.sample_size = 0;
	attr.alignment = 1;
	attr.nframes = 0;
	attr.framelimit = 0;
	attr.nbytes = 0;
	attr.bytelimit = (hdr->blksize - hdr->first_data);
	break;
      case STEIM1:
      case STEIM2:
	/* Try to accomodate SDR, which may have 0 in hdr->num_frames.	*/
	attr.sample_size = 0;
	attr.alignment = sizeof(FRAME);
	attr.nframes = (hdr->num_data_frames > 0) ? hdr->num_data_frames : 
	    (hdr->num_samples == 0) ? 0 :
	    (hdr->blksize - hdr->first_data) / sizeof(FRAME);
	attr.framelimit = (hdr->blksize - hdr->first_data) / sizeof (FRAME);
	attr.nbytes = attr.nframes * sizeof(FRAME);
	attr.bytelimit = attr.framelimit * sizeof(FRAME);
	break;
      case INT_16:
	attr.sample_size = 2;
	attr.alignment = 1;
	attr.nframes = 0;
	attr.framelimit = 0;
	attr.nbytes = hdr->num_samples * attr.sample_size;
	attr.bytelimit = (hdr->blksize - hdr->first_data);
	break;
      case INT_24:
	attr.sample_size = 3;
	attr.alignment = 1;
	attr.nframes = 0;
	attr.framelimit = 0;
	attr.nbytes = hdr->num_samples * attr.sample_size;
	attr.bytelimit = (hdr->blksize - hdr->first_data);
	break;
      case INT_32:
	attr.sample_size = 4;
	attr.alignment = 1;
	attr.nframes = 0;
	attr.framelimit = 0;
	attr.nbytes = hdr->num_samples * attr.sample_size;
	attr.bytelimit = (hdr->blksize - hdr->first_data);
	break;
      default:
	fprintf (stderr, "Error: Unsupported data type %d in ms_attr\n", hdr->data_type);
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	attr.sample_size = 0;
	attr.alignment = 0;	/* ** ERROR INDICATION ** */
	attr.nframes = 0;
	attr.framelimit = 0;
	attr.nbytes = 0;
	attr.bytelimit = 0;
	break;
    }
    return (attr);
}

/************************************************************************/
/*  decode_data_format:							*/
/*	Determine the data format value from the specified string.	*/
/*  return:								*/
/*	numeric value for data format; UNKNOWN_DATATYPE on error.	*/
/************************************************************************/
int decode_data_format 
   (char	*str)		/* string containing data format.	*/
{
    int i;
    for (i=0; data_format_table[i].format != UNKNOWN_DATATYPE; i++) {
	if (strcasecmp(str, data_format_table[i].name) == 0) {
	    return (data_format_table[i].format);
	}
    }
    return (data_format_table[i].format);
}

/************************************************************************/
/*  encode_data_format:							*/
/*	Determine the data format string from the specified format.	*/
/*  Return: ascii string for data format; "UNKNOWN_DATATYPE" on error.	*/
/************************************************************************/
char *encode_data_format 
   (int		format)		/* data format number.			*/
{
    int i;
    for (i=0; data_format_table[i].format != UNKNOWN_DATATYPE; i++) {
	if (data_format_table[i].format == format) {
	    return (data_format_table[i].name);
	}
    }
    return (data_format_table[i].name);
}

/************************************************************************/
/* Fortran interludes to ms_utils routines.				*/
/************************************************************************/

/************************************************************************/
/*  read_ms_:							      	*/
/*	Fortran interlude to read_ms.					*/
/************************************************************************/
#ifdef	fortran_suffix
int f_read_ms_
#else
int f_read_ms
#endif
   (DATA_HDR	*fhdr,		/* pointer to FORTRAN DATA_HDR.		*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		*maxpts,	/* max # data points to return.		*/
    FILE	**pfp)		/* FILE pointer for input file.		*/
{
    DATA_HDR	*hdr;		/* pointer to DATA_HDR.			*/
    int nread;
    
    nread = 0;
    nread = read_ms (&hdr, data_buffer, *maxpts, *pfp);
    /* Copy hdr to fortran structure, and convert char strings.		*/
    /* For FORTRAN use, I will not return the blockettes, since they	*/
    /* can't reference them directly or free them.			*/
    if (nread > 0 && hdr == NULL) return (MS_ERROR);
    if (hdr != NULL) {
	*fhdr = *hdr;
	cstr_to_fstr(fhdr->station_id, DH_STATION_LEN+1);
	cstr_to_fstr(fhdr->location_id, DH_LOCATION_LEN+1);
	cstr_to_fstr(fhdr->channel_id, DH_CHANNEL_LEN+1);
	cstr_to_fstr(fhdr->network_id, DH_NETWORK_LEN+1);
	free_data_hdr (hdr);
	fhdr->pblockettes = NULL;
    }
    return (nread);
}

