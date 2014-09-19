#pragma ident "$Id: steim.h 165 2005-12-23 12:34:58Z andres $"
/****************************************************************************
	Program	:	
	Module	:	
	Unit		:	STEIM.H
	Purpose	:	Steim compression structures and constants.
   Host     :	MSVC++ 6.0,  GNUC
   Target   :	Win32 (x86), Linux (x86), Solaris (x86, Sparc)
   Packages :	
	Author	:	Robert Banfill
   Company  :	Refraction Technology, Inc.
					2626 Lombardy Lane, Suite 105
					Dallas, Texas  75220
					(214) 353-0609     Fax (214) 353-9659
	Copyright:	(c) 1993-2005 Refraction Technology, Inc.  All Rights Reserved.

-----------------------------------------------------------------------------
	Notes		:	Modified for RefTek PASSCAL packet storage

-----------------------------------------------------------------------------
   $Source$
   $Revision: 165 $
	$Date: 2005-12-23 13:34:58 +0100 (Fri, 23 Dec 2005) $

-----------------------------------------------------------------------------
	Revised	:
 		29Sep05	(pld) add support for Steim2
 					(pld) add parm to encode functions
		01Nov93	(RLB) initial development
****************************************************************************/

#include "platform.h"

#ifndef _STEIM_TYPES_
#define _STEIM_TYPES_

/* DFA state constants ------------------------------------------------ */
#define _START_STATE   0   /* Ground state                       */
#define _D1            1   /* 1 INT8 difference                  */
#define _D2            2   /* 1 INT16 difference                 */
#define _D1_D1         3   /* 2 INT8 differences                 */
#define _D1_D1_D1      4   /* 3 INT8 differences                 */
#define _D4_f          5   /* 1 INT32 difference  (final state)  */
#define _D2_D2_f       6   /* 2 INT16 differences (final state)  */
#define _D1_D1_D1_D1_f 7   /* 4 INT8 differences (final state)   */

/* Chunk types */
#define CHUNK_NULL     0L  /* No data        */
#define CHUNK_BYTES    1L  /* 4 INT8 values  */
#define CHUNK_WORDS    2L  /* 2 INT16 values */
#define CHUNK_LONG     3L  /* 1 INT32 value  */

/* Steim structures and unions ---------------------------------------- */

/* Chunk union, accessible as bytes, words or long */
typedef union _CHUNK {
   INT8		b[4];				/* 4 INT8  values */
   INT16		w[2];				/* 2 INT16 values */
   INT32		l;					/* 1 INT32 value  */
   UINT32	u;					/* 1 UINT32 value (Steim2)  */
} CHUNK;

/* Frame structure */
typedef struct _FRAME {
   UINT32 flags;           /* Frame control header */
   CHUNK chunk[15];        /* Array of chunks composing 1 frame */
} FRAME;

/* Steim compressed data record */
typedef struct _STEIM {
   FRAME frame[15];        /* Array of frames composing 1 record */
} STEIM;

/* DFA transition table entry */
typedef struct _TRANSITION {
   INT8 new_state ;        /* New state after transition     */
   INT8 unget ;            /* Number of difs to unget        */
   INT8 d_ndx ;            /* Index into buffer to place dif */
} TRANSITION ;

/* Data state */
typedef struct _DATA_STATE {
   INT32 x0;               /* Forward integrating constant (x-0) */
   INT32 xn;               /* Reverse integrating constant (x-n) */
   UINT32 flags;            /* Current frame flags storage        */
   INT16 f_ndx;            /* Current frame number within record */
   INT16 c_ndx;            /* Current chunk number within frame  */
} DATA_STATE;

INT16 encode_steim ( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev );
BOOL  decode_steim ( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev );

/*---------------------------------------------------------------------
	Steim2 support
---------------------------------------------------------------------*/
/* Constants */
#define CMPRSN_FRMS_PER_PCKT	15				/* # data frames in output buffer */
#define WRDS_PER_FRM				16				/* # words (chunks) in a frame */
#define FRAMES_PER_PCKT			CMPRSN_FRMS_PER_PCKT+1	/* number of frame including header frame */
#define WRDS_PER_PCKT			WRDS_PER_FRM*CMPRSN_FRMS_PER_PCKT	/* # words per data block */
#define END_SAMPLE_POS			2
#define WORD_BITS					32

/* Structures */
/*	Union/structure for compression code word of a Steim frame.  Values loaded
	as two-bit values; the result is written as a four-byte word.  The int
	array "tmp_cmprsn_flgs" (not part of the union/structure) is used to store
	values preparatory to constructing the output word.
*/
typedef union cmprsn_flgs
	{
	struct
		{
#ifdef BIG_ENDIAN_HOST
		unsigned short	word00 : 2;
		unsigned short	word01 : 2;
		unsigned short	word02 : 2;
		unsigned short	word03 : 2;
		unsigned short	word04 : 2;
		unsigned short	word05 : 2;
		unsigned short	word06 : 2;
		unsigned short	word07 : 2;
		unsigned short	word08 : 2;
		unsigned short	word09 : 2; 
		unsigned short	word10 : 2;
		unsigned short	word11 : 2;
		unsigned short	word12 : 2;
		unsigned short	word13 : 2;
		unsigned short	word14 : 2;
		unsigned short	word15 : 2;
#else
		unsigned short	word15 : 2;
		unsigned short	word14 : 2;
		unsigned short	word13 : 2;
		unsigned short	word12 : 2;
		unsigned short	word11 : 2;
		unsigned short	word10 : 2;
		unsigned short	word09 : 2;
		unsigned short	word08 : 2;
		unsigned short	word07 : 2;
		unsigned short	word06 : 2; 
		unsigned short	word05 : 2;
		unsigned short	word04 : 2;
		unsigned short	word03 : 2;
		unsigned short	word02 : 2;
		unsigned short	word01 : 2;
		unsigned short	word00 : 2;
#endif
		}	flag;
	UINT32	word;
	}	CMP_FLAGS_T;

/* this structure is designed to be used in PKT_STOR with the stream 
	dependent info. It stores variables used by the compression
	algorithm that need to be saved between calls to PKT_STOR.
*/
typedef struct strm_cmp_strct
	{
	UINT16	tmp_cmprsn_flgs[18];			/* temp locn for cmprsn flags */
	INT32		tmp_frame_buffer[18];		/* words in a frame */
	UINT16	word_per_frame_cnt;			/* # of words in frame so far */
	INT32		last_sample;					/* previous sample */
	UINT16	data_offset;					/* ptr into buffer */
	UBYTE		new_pckt_flg;					/* cmprsn type so far */
	UBYTE		type;								/* cmprsn type so far */
	UINT16	sample_count;					/* count of compressed samples */
	UBYTE		diffs_n_que;					/* describes data on hand */
	UBYTE		cmprsn_residl;					/* compression residual (WAN/pld) */
	INT32		diff_que[7];					/* differences on hand */
	INT32		sample_que[7];					/* samples on hand */
	}	CMP_INFO_T;

/*  functions */
INT32 steim2_compress(
	INT32			sample,						/* smpl to be compressed */
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			*data_buffer);				/* output buffer */

int steim2_flush(
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			*data_buffer);				/* output buffer */

int steim2_diff (
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			this_sample,				/* value of current sample */
	INT32			*beg_sample_ptr,			/* first sample of each output */
	INT32			*end_sample_ptr);			/* last sample of each output */

int steim2_flush_diff (
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			output[],					/* output of this process */
	INT32			beg_sample[],				/* first sample of each output */
	INT32			end_sample[],				/* last sample of each output */
	UBYTE			*type);						/* Steim type of output */

int steim2_build_word(
	CMP_INFO_T	*ps,							/* Address of compression structure */
	UBYTE			word_type);

void init_packet_flags(
	CMP_INFO_T	*ps,					/* Address of compression structure */
	INT32			*data_buffer);		/* output buffer */

INT16 encode_steim2( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev );
BOOL  decode_steim2( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev );


#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:33  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.3  2004/08/11 14:50:04  rstavely
 * Changes for version 1.19 Arc utils include multi units & streams, & alignment for
 * arcfetch.
 * Add Rtptrig utility based on rtpftp.
 *
 * Revision 1.2  2002/01/18 17:55:58  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
