/************************************************************************/
/*  Routines for packing STEIM1, STEIM2, INT_32, INT_16, and INT_24	*/
/*  data records.							*/
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
static char sccsid[] = "$Id: pack.c 2 2005-07-26 19:28:46Z andres $ ";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#ifdef	SUNOS4
#include <malloc.h>
#endif

#include "qdefines.h"
#include "msdatatypes.h"
#include "qutils.h"
#include "qsteim.h"
#include "pack.h"

#define	VALS_PER_FRAME	(16-1)		/* # of ints for data per frame.*/
#define	EMPTY_BLOCK(fn,wn) (fn+wn == 0)

/************************************************************************/

#define	X0  p_sdf->f[0].w[0].fw
#define	XN  p_sdf->f[0].w[1].fw

#define	BIT4PACK(i,points_remaining)   \
    (points_remaining >= 7 && \
     (minbits[i] <= 4) && (minbits[i+1] <= 4) && \
     (minbits[i+2] <= 4) && (minbits[i+3] <= 4) && \
     (minbits[i+4] <= 4) && (minbits[i+5] <= 4) && \
     (minbits[i+6] <= 4))

#define	BIT5PACK(i,points_remaining)   \
    (points_remaining >= 6 && \
     (minbits[i] <= 5) && (minbits[i+1] <= 5) && \
     (minbits[i+2] <= 5) && (minbits[i+3] <= 5) && \
     (minbits[i+4] <= 5) && (minbits[i+5] <= 5))

#define	BIT6PACK(i,points_remaining)   \
    (points_remaining >= 5 && \
     (minbits[i] <= 6) && (minbits[i+1] <= 6) && \
     (minbits[i+2] <= 6) && (minbits[i+3] <= 6) && \
     (minbits[i+4] <= 6))

#define	BYTEPACK(i,points_remaining)   \
    (points_remaining >= 4 && \
     (minbits[i] <= 8) && (minbits[i+1] <= 8) && \
     (minbits[i+2] <= 8) && (minbits[i+3] <= 8))

#define	BIT10PACK(i,points_remaining)   \
    (points_remaining >= 3 && \
     (minbits[i] <= 10) && (minbits[i+1] <= 10) && \
     (minbits[i+2] <= 10))

#define	BIT15PACK(i,points_remaining)   \
    (points_remaining >= 2 && \
     (minbits[i] <= 15) && (minbits[i+1] <= 15))

#define	HALFPACK(i,points_remaining)   \
    (points_remaining >= 2 && (minbits[i] <= 16) && (minbits[i+1] <= 16))

#define	BIT30PACK(i,points_remaining)   \
    (points_remaining >= 1 && \
     (minbits[i] <= 30))

#define	MINBITS(diff,minbits)	\
	if (diff >= -8 && diff < 8) minbits= 4; \
	else if (diff >= -16 && diff < 16) minbits = 5; \
	else if (diff >= -32 && diff < 32) minbits = 6; \
	else if (diff >= -128 && diff < 128) minbits = 8; \
	else if (diff >= -512 && diff < 512) minbits = 10; \
	else if (diff >= -16384 && diff < 16384) minbits = 15; \
	else if (diff >= -32768 && diff < 32768) minbits = 16; \
        else if (diff >= -536870912 && diff < 536870912) minbits = 30; \
	else minbits = 32;

#define PACK(bits,n,m1,m2)  {\
    int i = 0; \
    unsigned int val = 0; \
    for (i=0;i<n;i++) { \
	val = (val<<bits) | (diff[ipt++]&m1); \
    } \
    val |= ((unsigned int)m2 << 30); \
    p_sdf->f[fn].w[wn].fw = val; }

/************************************************************************/
/*  pack_steim1:							*/
/*	Pack data into STEIM1 data frames.				*/
/*  return:								*/
/*	0 on success.							*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int pack_steim1
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int		points_remaining = ns;
    int		*minbits;	/* min bytes for difference.		*/
    int		i, j;
    int		mask;
    int		ipt = 0;	/* index of initial data to pack.	*/
    int		fn = 0;		/* index of initial frame to pack.	*/
    int		wn = 2;		/* index of initial word to pack.	*/
    int		itmp;
    short int	stmp;
    int		swapflag;
    int		nb;		/* number of minbits to compute.	*/
    int		max_samples_per_frame;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    max_samples_per_frame = 4 * VALS_PER_FRAME;	/* steim1 compression.	*/
    nb = max_samples_per_frame * nf;
    if (nb > points_remaining) nb = points_remaining;

    minbits = NULL;
    minbits = (int *)malloc(nb * sizeof(int));
    if (minbits == NULL) {
	fprintf (stderr, "Error: mallocing minbits in pack_steim1\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (MS_ERROR);
    }
    for (i=0; i<nb; i++) MINBITS(diff[i],minbits[i]);
    
    p_sdf->f[fn].ctrl = 0;

    /*	Set new X0 value in first frame.				*/
    X0 = data[0];
    if (swapflag) swab4((int *)&X0);
    p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM1_SPECIAL_MASK;
    XN = data[ns-1];
    if (swapflag) swab4((int *)&XN);
    p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM1_SPECIAL_MASK;

    while (points_remaining > 0) {
	/*  Pack the next available data into the most compact form.	*/
	if (BYTEPACK(ipt,points_remaining)) {
	    mask = STEIM1_BYTE_MASK;
	    for (j=0; j<4; j++) p_sdf->f[fn].w[wn].byte[j] = diff[ipt++];
	    points_remaining -= 4;
	}
	else if (HALFPACK(ipt,points_remaining)) {
	    mask = STEIM1_HALFWORD_MASK;
	    for (j=0; j<2; j++) {
		stmp = diff[ipt++];
		if (swapflag) swab2 (&stmp);
		p_sdf->f[fn].w[wn].hw[j] = stmp;
	    }
	    points_remaining -= 2;
	}
	else {
	    mask = STEIM1_FULLWORD_MASK;
	    itmp = diff[ipt++];
	    if (swapflag) swab4 (&itmp);
	    p_sdf->f[fn].w[wn].fw = itmp;
	    points_remaining -= 1;
	}

	/* Append mask for this word to current mask.			*/
	p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | mask;

	/* Check for full frame or full block.				*/
	if (++wn >= VALS_PER_FRAME) {
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].ctrl);
	    /* Reset output index to beginning of frame.		*/
	    wn = 0;
	    /* If block is full, output block and reinitialize.		*/
	    if (++fn >= nf) break;
	    p_sdf->f[fn].ctrl = 0;
	}
    }

    /* Set new XN value in first frame.					*/
    XN = data[(ns-1)-points_remaining];
    if (swapflag) swab4((int *)&XN);

    /* End of data.  Pad current frame and optionally rest of block.	*/
    /* Do not pad and output a completely empty block.			*/
    if (! EMPTY_BLOCK(fn,wn)) {
	*pnframes = pad_steim_frame(p_sdf,fn,wn,nf,swapflag,pad);
    }
    else {
	*pnframes = 0;
    }
    *pnsamples = ns - points_remaining;
    free ((char *)minbits);
    return(0);
}

/************************************************************************/
/*  pack_steim2:							*/
/*	Pack data into STEIM1 data frames.				*/
/*  return:								*/
/*	0 on success.							*/
/*	negative QLIB2 error code on error.				*/
/************************************************************************/
int pack_steim2
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.  	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int		points_remaining = ns;
    int		*minbits;	/* min bits for difference.		*/
    int		i, j;
    int		mask;
    int		ipt = 0;	/* index of initial data to pack.	*/
    int		fn = 0;		/* index of initial frame to pack.	*/
    int		wn = 2;		/* index of initial word to pack.	*/
    int		itmp;
    short int	stmp;
    int		swapflag;
    int		nb;		/* number of minbits to compute.	*/
    int		max_samples_per_frame;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    max_samples_per_frame = 8 * VALS_PER_FRAME;	/* steim2 compression.	*/
    nb = max_samples_per_frame * nf;
    if (nb > points_remaining) nb = points_remaining;

    minbits = NULL;
    minbits = (int *)malloc(nb * sizeof(int));
    if (minbits == NULL) {
	fprintf (stderr, "Error: mallocing minbits in pack_steim1\n");
	fflush (stderr);
	if (QLIB2_CLASSIC) exit(1);
	return (MS_ERROR);
    }
    for (i=0; i<nb; i++) MINBITS(diff[i],minbits[i]);
    
    p_sdf->f[fn].ctrl = 0;

    /*	Set new X0 value in first frame.				*/
    X0 = data[0];
    if (swapflag) swab4((int *)&X0);
    p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM2_SPECIAL_MASK;
    XN = data[ns-1];
    if (swapflag) swab4((int *)&XN);
    p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM2_SPECIAL_MASK;

    while (points_remaining > 0) {
	/*  Pack the next available datapoints into the most compact form.  */
	if (BIT4PACK(ipt,points_remaining)) {
	    PACK(4,7,0x0000000f,02)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_567_MASK;
	    points_remaining -= 7;
	}
	else if (BIT5PACK(ipt,points_remaining)) {
	    PACK(5,6,0x0000001f,01)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_567_MASK;
	    points_remaining -= 6;
	}
	else if (BIT6PACK(ipt,points_remaining)) {
	    PACK(6,5,0x0000003f,00)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_567_MASK;
	    points_remaining -= 5;
	}
	else if (BYTEPACK(ipt,points_remaining)) {
	    mask = STEIM2_BYTE_MASK;
	    for (j=0; j<4; j++) p_sdf->f[fn].w[wn].byte[j] = diff[ipt++];
	    points_remaining -= 4;
	}
	else if (BIT10PACK(ipt,points_remaining)) {
	    PACK(10,3,0x000003ff,03)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_123_MASK;
	    points_remaining -= 3;
	}
	else if (BIT15PACK(ipt,points_remaining)) {
	    PACK(15,2,0x00007fff,02)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_123_MASK;
	    points_remaining -= 2;
	}
	else if (BIT30PACK(ipt,points_remaining)) {
	    PACK(30,1,0x3fffffff,01)
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].w[wn].fw);
	    mask = STEIM2_123_MASK;
	    points_remaining -= 1;
	}
	else {
	    fprintf (stderr, "Error: Unable to represent difference in <= 30 bits\n");
	    fflush (stderr);
	    if (QLIB2_CLASSIC) exit(1);
	    return (MS_ERROR);
	}

	/* Append mask for this word to current mask.			    */
	p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | mask;

	/* Check for full frame or full block.				*/
	if (++wn >= VALS_PER_FRAME) {
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].ctrl);
	    /* Reset output index to beginning of frame.		*/
	    wn = 0;
	    /* If block is full, output block and reinitialize.		*/
	    if (++fn >= nf) break;
	    p_sdf->f[fn].ctrl = 0;
	}
    }

    /* Set new XN value in first frame.					*/
    XN = data[(ns-1)-points_remaining];
    if (swapflag) swab4((int *)&XN);

    /* End of data.  Pad current frame and optionally rest of block.	*/
    /* Do not pad and output a completely empty block.			*/
    if (! EMPTY_BLOCK(fn,wn)) {
	*pnframes = pad_steim_frame(p_sdf,fn,wn,nf,swapflag,pad);
    }
    else {
	*pnframes = 0;
    }
    *pnsamples = ns - points_remaining;
    free ((char *)minbits);
    return(0);
}

/************************************************************************/
/*  pad_steim_frame:							*/
/*	Pad the rest of the data record with null values,		*/
/*	and optionally the rest of the total number of frames.		*/
/*  return:								*/
/*	total number of frames in record.				*/
/************************************************************************/
int pad_steim_frame
   (SDF	    	*p_sdf,
    int		fn,	    	/* current frame number.		*/
    int	    	wn,		/* current work number.			*/
    int	    	nf,		/* total number of data frames.		*/
    int		swapflag,	/* flag to swap byte order of data.	*/
    int	    	pad)		/* flag to pad # frames to nf.		*/
{
    /* Finish off the current frame.					*/
    if (wn < VALS_PER_FRAME && fn < nf) {
	for (; wn < VALS_PER_FRAME; wn++) {
	    p_sdf->f[fn].w[wn].fw = 0;
	    p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM1_SPECIAL_MASK;
	}
	if (swapflag) swab4 ((int *)&p_sdf->f[fn].ctrl);
	fn++;
    }

    /* Fill the remaining frames in the block.				*/
    if (pad) {
	for (; fn<nf; fn++) {
	    p_sdf->f[fn].ctrl = STEIM1_SPECIAL_MASK;	/* mask for ctrl*/
	    for (wn=0; wn<VALS_PER_FRAME; wn++) {
		p_sdf->f[fn].w[wn].fw = 0;
		p_sdf->f[fn].ctrl = (p_sdf->f[fn].ctrl<<2) | STEIM1_SPECIAL_MASK;
	    }
	    if (swapflag) swab4 ((int *)&p_sdf->f[fn].ctrl);
	}
    }
    return (fn);
}

/************************************************************************/
/*  pack_int_32:							*/
/*	Pack integer data into INT_32 format.				*/
/*	Return: 0 on success, -1 on failure.				*/
/************************************************************************/
int pack_int_32 
   (int		p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int bytes_per_sample = 4;	/* number of bytes per packed sample.	*/
    int points_remaining = ns;	/* number of samples remaining to pack.	*/
    int		i = 0;
    int		swapflag;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    while (points_remaining > 0 && max_bytes >= bytes_per_sample) {
	/* Pack the next available data into INT_32 format.		*/
	*p_packed = data[i];
	if (swapflag) swab4 (p_packed);
	p_packed++;
	max_bytes -= bytes_per_sample;
	points_remaining--;
	i++;
    }
    *pnbytes = (ns - points_remaining) * bytes_per_sample;

    /* Pad miniSEED block if necessary.				*/
    if (pad) {
	memset ((void *)p_packed, 0, max_bytes);
	*pnbytes += max_bytes;
    }
    *pnsamples = ns - points_remaining;
    return (0);
}

/************************************************************************/
/*  pack_int_16:							*/
/*	Pack integer data into INT_16 format.				*/
/*	Return: 0 on success, -1 on failure.				*/
/************************************************************************/
int pack_int_16
   (short int p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int bytes_per_sample = 2;	/* number of bytes per packed sample.	*/
    int points_remaining = ns;	/* number of samples remaining to pack.	*/
    int		i = 0;
    int		swapflag;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    while (points_remaining > 0 && max_bytes >= bytes_per_sample) {
	/* Pack the next available data into INT_16 format.		*/
	*p_packed = data[i];
	if (swapflag) swab2 (p_packed);
	p_packed++;
	max_bytes -= bytes_per_sample;
	points_remaining--;
	i++;
    }
    *pnbytes = (ns - points_remaining) * bytes_per_sample;

    /* Pad miniSEED block if necessary.				*/
    if (pad) {
	memset ((void *)p_packed, 0, max_bytes);
	*pnbytes += max_bytes;
    }
    *pnsamples = ns - points_remaining;
    return (0);
}

/************************************************************************/
/*  pack_int_24:							*/
/*	Pack integer data into INT_24 format.				*/
/*	Return: 0 on success, -1 on failure.				*/
/************************************************************************/
int pack_int_24
   (unsigned char p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int bytes_per_sample = 3;	/* number of bytes per packed sample.	*/
    int points_remaining = ns;	/* number of samples remaining to pack.	*/
    int		i = 0;
    int		sbc;
    int		swapflag;

    if (my_wordorder < 0) get_my_wordorder();
    swapflag = (my_wordorder != data_wordorder);

    if (my_wordorder == SEED_BIG_ENDIAN) {
	sbc = 1;
    }
    else {
	sbc = 0;
    }

    while (points_remaining > 0 && max_bytes >= bytes_per_sample) {
	/* Pack the next available data into INT_24 format.		*/
	/* Ignore possible sign loss - nothing we can do about it.	*/
	memcpy (p_packed, ((char *)&data[i])+sbc, bytes_per_sample);
	if (swapflag) swab3 (p_packed);
	p_packed += bytes_per_sample;
	max_bytes -= bytes_per_sample;
	points_remaining--;
	i++;
    }
    *pnbytes = (ns - points_remaining) * bytes_per_sample;

    /* Pad miniSEED block if necessary.				*/
    if (pad) {
	memset ((void *)p_packed, 0, max_bytes);
	*pnbytes += max_bytes;
    }
    *pnsamples = ns - points_remaining;
    return (0);
}

/************************************************************************/
/*  pack_text:								*/
/*	Pack text data into text format.				*/
/*	Return: 0 on success, -1 on failure.				*/
/************************************************************************/
int pack_text 
   (char 	p_packed[],	/* output data array - packed.		*/
    char	data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples)	/* number of samples actually packed.	*/
{
    int points_remaining = ns;	/* number of samples remaining to pack.	*/
    int		last = -1;
    int		nbytes;
    int		i;

    if (my_wordorder < 0) get_my_wordorder();

    /* Split lines only if a single line will not fit in 1 record.	*/
    if (points_remaining > max_bytes) {
	/* Look for the last newline that will fit in output buffer.	*/
	for (i=points_remaining-1; i>=0; i--) {
	    if (data[i] == '\n') {
		last = i;
		break;
	    }
	}
	if (last < 0) last = max_bytes - 1;
    }
    if (last < 0) last = points_remaining - 1;
    nbytes = last + 1;
    memcpy (p_packed, data, nbytes);
    p_packed += nbytes;
    max_bytes -= nbytes;
    *pnbytes = nbytes;
    *pnsamples = nbytes;
    points_remaining -= nbytes;
    /* Pad miniSEED block if necessary.				*/
    if (pad) {
	memset ((void *)p_packed, 0, max_bytes);
	*pnbytes += max_bytes;
    }
    *pnsamples = ns - points_remaining;
    return (0);
}
