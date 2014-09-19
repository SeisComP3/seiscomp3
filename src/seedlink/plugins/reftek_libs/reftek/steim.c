#pragma ident "$Id: steim.c 165 2005-12-23 12:34:58Z andres $"
/* Steim 1 compression and decompression routines for RefTek PASSCAL packets.
 *
 * These routines are based on the work of many others, the compression
 * algorithm implementation is based on the work of Guy Stewart at IRIS.
 *
 * Developed using Microsoft C/C++ 7.0 in DOS by:
 *
 *    Robert Banfill (banfill@gldvxa.cr.usgs.gov)
 *    Small Systems Support
 *    2 Boston Harbor Place
 *    Big Water,  UT  84741-0205
 *    (801) 675-5827 Voice, (801) 675-3780 Fax
 *
 * Revision history:
 *
 *		29Sep2005	(pld) complete Steim2 support
 *		03Sep2005	(pld) decode_steim2() - just a shell
 *    18Jul1998	(DEC) Modified for for use in RTP library
 *    01Nov1993	(RLB) Modified for RefTek PASSCAL packets
 *    13May1993	(RLB) First effort
 */

/* If REVERSE_BYTE_ORDER is defined, the encode_steim( ) and decode_steim( )
 * functions will swap the byte order of all INT16 and INT32 integers
 * during processing.  This constant should be defined if this code
 * will run on an 80x86 or VAX.  If this code will run on 68000
 * hardware, this constant should not be defined as no swapping is
 * required.
 * 
 * THIS CONSTANT IS DEFINED AT COMPILE TIME IN "private.h", THERE IS NO
 * NEED TO DEFINE OR UNDEF IT HERE.
 * 
 */

/* Debug messages */
#undef DEBUG

/* Includes ----------------------------------------------------------- */
#include <stdio.h>         /* Standard C runtime */
#include <stdlib.h>        /* . */

#include "private.h"

/* Byte order swaps for 16 and 32 bit integers */
INT16 swap_w( INT16 inword );
INT32 swap_l( INT32 inlong );

/* Type-independant maximum value for C standard types */
#ifndef max
#define max(x,y)		((x)>(y)?(x):(y))
#endif

/* Local helper functions, these should not be called externally */
static BOOL _save_chunk( STEIM *rec, DATA_STATE *ds, CHUNK chunk, INT32 c_type );
static VOID _finish_rec( VOID *ptr, DATA_STATE *ds );
static INT8 _chunk_type( UINT32 flags, INT8 c_ndx );

/* Globals ------------------------------------------------------------ */

/* DFA transition table */
TRANSITION transition[] = {
   { _D1,            0,  0 },
   { _D2,            0,  0 },
   { _D4_f,          0,  0 },
   { _D1_D1,         0,  1 },
   { _D2_D2_f,       0,  1 },
   { _D4_f,          1, -1 },
   { _D2_D2_f,       0,  1 },
   { _D2_D2_f,       0,  1 },
   { _D4_f,          1, -1 },
   { _D1_D1_D1,      0,  2 },
   { _D2_D2_f,       1, -1 },
   { _D2_D2_f,       1, -1 },
   { _D1_D1_D1_D1_f, 0,  3 },
   { _D2_D2_f,       2, -1 },
   { _D2_D2_f,       2, -1 }
};

/* Final state indicators */
BOOL final[] = {
   FALSE,
   FALSE,
   FALSE,
   FALSE,
   FALSE,
   TRUE,
   TRUE,
   TRUE
};
const UINT32 st2_ck_flgs[] = {0x00,0x03,0x03,0x03,0x01,0x02,0x02,0x02};
const UINT32 st2_dnbl_flgs[] = {	0x00000000,0x80000000,0x40000000,0x00000000,
											0x00000000,0xc0000000,0x80000000,0x40000000};
const UINT32 st2_max_per_word[] = {0,7,6,5,4,3,2,1};
const UINT32 st2_diff_size[] = {0,4,5,6,8,10,15,30};
const UINT32 st2_diff_offset[] = {0,4,2,2,0,2,2,2};
const INT32 st2_pos_dif[] = {	0x00000000,0x00000007,0x0000000f,0x0000001f,
										0x0000007f,0x000001ff,0x00003fff,0x2fffffff};
const INT32 st2_neg_dif[] = {	0x00000000,-0x00000008,-0x00000010,-0x00000020,
										-0x00000080,-0x00000200,-0x00004000,-0x30000000};

/*---------------------------------------------------------------------
 * Encode INT32 samples using Steim 1 compression.
 *
 *    INT16 encode_steim( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev );
 *
 * Arguments:
 *    samples   = INT32 array of uncompressed samples
 *    n_rawsamp = Number of samples in samples array
 *    ptr       = VOID pointer to a SEED 2.0 data record
 *    prev      = INT32 value to use to calculate first difference
 *
 * Returns the actual number of samples Steim 1 encoded in the PASSCAL
 * data record.
 *
 * ptr should point to a 960 byte block of memory that will contain the
 * PASSCAL output record.  samples should be a INT32 array with at least 892
 * elements and n_rawsamp should specify the actual number of samples
 * contained in samples starting at samples[0].
 *
 * The algorithm:
 *
 * Each Steim compressed PASSCAL record is composed of a 64 byte fixed data
 * header followed by 15 frames of 64 bytes, each beginning with a 4 byte
 * control header and followed by 15 4 bytes "chunks".  Each of these chunks
 * can contain 4 byte values, 2 word values, or 1 long value, depending on
 * the 2 bit flags contained in the control header.  Generally all binary
 * data in PASSCAL records are in big endian (68000) byte order.
 *
 * The compressor is implemented as a Deterministic Finite Automaton (DFA).
 * The transition table for the DFA is:
 *
 * note: _f signifies a final state.
 * ----------------------------------------------------------
 *                | # of    |                | # of  |
 *                | bytes   |                | DIFS  |
 *                | DIF     |                | to    | DIF
 * Current state  | fits in | New State      | unget | index
 * ---------------+---------+----------------+-------+-------
 * _START_STATE   |  1      | _D1            | 0     |  0
 * _START_STATE   |  2      | _D2            | 0     |  0
 * _START_STATE   |  4      | _D4_f          | 0     |  0
 *          _D1   |  1      | _D1_D1         | 0     |  1
 *          _D1   |  2      | _D2_D2_f       | 0     |  1
 *          _D1   |  4      | _D4_f          | 1     | -1
 *          _D2   |  1      | _D2_D2_f       | 0     |  1
 *          _D2   |  2      | _D2_D2_f       | 0     |  1
 *          _D2   |  4      | _D4_f          | 1     | -1
 *       _D1_D1   |  1      | _D1_D1_D1      | 0     |  2
 *       _D1_D1   |  2      | _D2_D2_f       | 1     | -1
 *       _D1_D1   |  4      | _D2_D2_f       | 1     | -1
 *    _D1_D1_D1   |  1      | _D1_D1_D1_D1_f | 0     |  3
 *    _D1_D1_D1   |  2      | _D2_D2_f       | 2     | -1
 *    _D1_D1_D1   |  4      | _D2_D2_f       | 2     | -1
 * ----------------------------------------------------------
 */

INT16 encode_steim( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev ) {
   INT8  state, token, t_ndx;
   UINT16 n_samp;
   INT32  i, dif, difs[4], c_type;
   CHUNK chunk;
   DATA_STATE  ds;
   STEIM *rec;

   /* Initialize the data state */
   ds.x0    = samples[0];     /* Save the forward integrating constant */
   ds.xn    = prev;           /* First dif will = x-prev               */
   ds.flags = 0x00000000;     /* Clear the frame flags                 */
   ds.f_ndx = 0;              /* Start at first frame,                 */
   ds.c_ndx = 2;              /* 3rd chunk, the 1st and 2nd will store */
                              /* the integrating constants             */

//   printf( "\nSTM1_EN: %p:%u:%p:%u", samples, n_rawsamp, ptr, prev );

   /* Initialize the sample counter and record pointer */
   n_samp = 0;
   rec = (STEIM *)((INT8 *)ptr+64);

   /* Initialize the DFA state */
   state = _START_STATE;

   /* Start of compression loop, compress until out of samples or the */
   /* record is full (_save_chunk() returns FALSE). */
   for( i=0; i<n_rawsamp; i++ ) {

      /* Compute the difference */
      dif = samples[i] - ds.xn;
      ds.xn = samples[i];

      /* Examine the difference */
      if( dif <= 127 && dif >= -128 )
         token = 0;
      else if( dif <= 32767 && dif >= -32768 )
         token = 1;
      else
         token = 2;

#ifdef DEBUG
      printf( "state:     %d, i: %ld, sam: %ld, dif: %ld, token: %d\n",
         state, i, samples[i], dif, token );
#endif

      /* Make the transition */
      t_ndx = (state * 3) + token;

      /* Unget samples as needed */
      if( transition[t_ndx].unget ) {
         i -= transition[t_ndx].unget;
         ds.xn = samples[i];
      }

      /* Save dif (if needed) in the proper slot in the buffer */
      if( transition[t_ndx].d_ndx >= 0 )
         difs[transition[t_ndx].d_ndx] = dif;

      /* Get new state */
      state = transition[t_ndx].new_state;

#ifdef DEBUG
      printf( "new state: %d, i: %ld, buf: %ld %ld %ld %ld\n",
         state, i, difs[0], difs[1], difs[2], difs[3] );
#endif

      /* Deal with final states */
      if( final[state] ) {
         switch( state ) {

            /* 1 INT32 difference */
            case _D4_f:
               chunk.l = difs[0];
               c_type = CHUNK_LONG;
               n_samp += 1;
               break;

            /* 2 INT16 differences */
            case _D2_D2_f:
               chunk.w[0] = (UINT16)difs[0];
               chunk.w[1] = (UINT16)difs[1];
               c_type = CHUNK_WORDS;
               n_samp += 2;
               break;

            /* 4 BYTE differences */
            case _D1_D1_D1_D1_f:
               chunk.b[0] = (INT8)difs[0];
               chunk.b[1] = (INT8)difs[1];
               chunk.b[2] = (INT8)difs[2];
               chunk.b[3] = (INT8)difs[3];
               c_type = CHUNK_BYTES;
               n_samp += 4;
               break;
         }

         /* Save this chunk and see if we're done */
         if( ! _save_chunk( rec, &ds, chunk, c_type ) )
            break;

         /* Reset the state */
         state = _START_STATE;
      }
   /* End of compression loop */
   }

   /* If we ran out of data before running out of room, resolve any */
   /* non-final state */
   if( ! final[state] ) {
      switch( state ) {

         /* 1 BYTE or INT16 difference, save as a INT32 */
         case _D1:
         case _D2:
            chunk.l = difs[0];
            c_type = CHUNK_LONG;
            n_samp += 1;
            _save_chunk( rec, &ds, chunk, c_type );
            break;

         /* 2 BYTE differences, save as INT16's */
         case _D1_D1:
            chunk.w[0] = (UINT16)difs[0];
            chunk.w[1] = (UINT16)difs[1];
            c_type = CHUNK_WORDS;
            n_samp += 2;
            _save_chunk( rec, &ds, chunk, c_type );
            break;

         /* 3 BYTE differences, save first 2 as INT16S's, 3rd as a INT32 */
         case _D1_D1_D1:
            chunk.w[0] = (UINT16)difs[0];
            chunk.w[1] = (UINT16)difs[1];
            c_type = CHUNK_WORDS;
            n_samp += 2;
            if( ! _save_chunk( rec, &ds, chunk, c_type ) )
               break;
            chunk.l = difs[2];
            c_type = CHUNK_LONG;
            n_samp += 1;
            _save_chunk( rec, &ds, chunk, c_type );
            break;
      }
   }

   /* Finish out the record */
   if( n_samp > 0 )
      _finish_rec( ptr, &ds );

   /* Return the number of samples actually encoded */
   return( (UINT16)n_samp );
}

/*---------------------------------------------------------------------
 * Save the current chunk, called by encode_steim().
 * Returns TRUE if there is room for another chunk, FALSE if not.
 */

BOOL _save_chunk( STEIM *rec, DATA_STATE *ds, CHUNK chunk, INT32 c_type ) {
   INT8 tmp;

   /* Update frame flags */
   ds->flags |= c_type << (2 * (15 - (ds->c_ndx + 1)));

#ifdef REVERSE_BYTE_ORDER
   /* Perform byte swapping as needed */
   switch( c_type ) {
      case CHUNK_WORDS:
         tmp = chunk.b[0];
         chunk.b[0] = chunk.b[1];
         chunk.b[1] = tmp;
         tmp = chunk.b[2];
         chunk.b[2] = chunk.b[3];
         chunk.b[3] = tmp;
         break;
      case CHUNK_LONG:
         tmp = chunk.b[0];
         chunk.b[0] = chunk.b[3];
         chunk.b[3] = tmp;
         tmp = chunk.b[1];
         chunk.b[1] = chunk.b[2];
         chunk.b[2] = tmp;
         break;
   }
#endif

   /* Save the chunk */
   rec->frame[ds->f_ndx].chunk[ds->c_ndx].l = chunk.l;

#ifdef DEBUG
   printf( "final state, saved type %ld chunk in frame %d, chunk %d\n",
      c_type, ds->f_ndx, ds->c_ndx );
#endif

   /* Increment the chunk index */
   ds->c_ndx++;

   /* See if we need a new frame */
   if( ds->c_ndx >= 15 ) {

      /* Save and reset the flags */
      rec->frame[ds->f_ndx].flags = swap_l( ds->flags );
      ds->flags = 0x00000000;

      /* Reset the chunk index */
      ds->c_ndx = 0;

      /* Increment the frame index */
      ds->f_ndx++;

      /* See if we hit the end of the record */
      if( ds->f_ndx >= 15 )
         return( FALSE );
   }

   return( TRUE );
}

/*---------------------------------------------------------------------
 * Finish off a SEED record, called by encode_steim()
 04Aug04  (rs)  - make sure we don't overwrite end of buffer 
 */

VOID _finish_rec( VOID *ptr, DATA_STATE *ds ) {
   STEIM *rec;

   /* Save integrating constants and flags for last (possibly incomplete) frame */
   rec = (STEIM *)((INT8 *)ptr+64);
   rec->frame[0].chunk[0].l = swap_l( ds->x0 );
   rec->frame[0].chunk[1].l = swap_l( ds->xn );
	if (ds->f_ndx < 15)
   	rec->frame[ds->f_ndx].flags = swap_l( ds->flags );

   return;
}

/*---------------------------------------------------------------------
 * Decode INT32 Steim 1 compressed samples in a PASSCAL data record.
 *
 *    INT16 decode_steim( VOID *ptr, INT16 *n, INT32 *samples );
 *
 * Arguments:
 *    ptr     = VOID pointer to PASSCAL data record block (1024 bytes)
 *    samples = INT32 array for uncompressed samples
 *
 * Returns TRUE if successful, FALSE otherwise
 *
 * samples should be a INT32 array containing at least 892 elements.
 */

BOOL decode_steim( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev ) {
   BOOL  skip_ird;
   INT8 f_ndx, c_ndx, n_difs;
   INT16 i, n_samp;
   INT32 sum, difs[4], fi_con, ri_con;
   UINT32 flags;
   STEIM *rec;

   n_samp = *n;

   /* Get the integration constants */
   rec = (STEIM *)((INT8 *)ptr+64);
   fi_con = swap_l( rec->frame[0].chunk[0].l );
   ri_con = swap_l( rec->frame[0].chunk[1].l );

   /* Init the running sum and save the first sample */
   sum = fi_con;
   n_samp--;
   *samples++ = sum;

   /* Skip the inter-record delta, i.e., the difference between records */
   skip_ird = TRUE;

   /* Process up to 15 frames */
   for( f_ndx=0; f_ndx<15; f_ndx++ ) {

      /* Get the frame flags */
      flags = swap_l( rec->frame[f_ndx].flags );

      /* Process 15 chunks per frame */
      for( c_ndx=0; c_ndx<15; c_ndx++ ) {

         /* Count down samples to 0 */
         if( n_samp <= 0 )
            break;

         /* Process a chunk based on type */
         switch( _chunk_type( flags, c_ndx ) ) {

            case CHUNK_NULL:
               continue;

            case CHUNK_BYTES:
               for( i=0; i<4; i++ )
                  difs[i] = (INT32)rec->frame[f_ndx].chunk[c_ndx].b[i];
               n_difs = 4;
               break;

            case CHUNK_WORDS:
               for( i=0; i<2; i++ )
                  difs[i] = (INT32)swap_w( rec->frame[f_ndx].chunk[c_ndx].w[i] );
               n_difs = 2;
               break;

            case CHUNK_LONG:
               difs[0] = swap_l( rec->frame[f_ndx].chunk[c_ndx].l );
               n_difs = 1;
               break;
         }

         /* Save the decoded samples */
         for( i=0; i<n_difs; i++ ) {
            if( skip_ird ) {
               skip_ird = FALSE;
               *prev = fi_con - difs[0];
//               *prev = 0;
            }
            else {
               sum += difs[i];
               *samples++ = sum;
               n_samp--;
               if( n_samp <= 0 )
                  break;
            }
         }
      }
   }

   if( n_samp > 0 ) {
#ifdef DEBUG
      printf( "WARNING: Partial decompression of sample data, n_samp = %d\n", n_samp );
#endif
      rtp_log(RTP_WARN, "WARNING: Partial decompression of sample data, n_samp = %d\n", n_samp );
      *n = n_samp;
      return( FALSE );
   }

   if( sum != ri_con ) {
#ifdef DEBUG
      printf( "WARNING: Unreconciled reverse integrating constant: %ld, sum: %ld\n",
         ri_con, sum );
#endif
      rtp_log(RTP_WARN, "WARNING: Unreconciled reverse integrating constant: %ld, sum: %ld\n",
         ri_con, sum );
      return( FALSE );
   }

   return( TRUE );
}

/*---------------------------------------------------------------------
 * Returns the chunk type, called by decode_steim( ).
 */

INT8 _chunk_type( UINT32 flags, INT8 c_ndx ) {

   flags >>= (2 * (15 - (c_ndx+1)));

   return( (INT8)(flags & 0x03) );
}

/*---------------------------------------------------------------------
 * Swap byte order in a word integer
 */

INT16 swap_w( INT16 inword ) {

#ifdef REVERSE_BYTE_ORDER
   INT8 tmp;

   union {
      INT8 bytes[2];
      INT16 outword;
   } parts;

   parts.outword = inword;

   tmp = parts.bytes[0];
   parts.bytes[0] = parts.bytes[1];
   parts.bytes[1] = tmp;

   return( parts.outword );
#else
   return( inword );
#endif
}

/*---------------------------------------------------------------------
 * Swap byte order in a long integer
 */

INT32 swap_l( INT32 inlong ) {

#ifdef REVERSE_BYTE_ORDER
   INT8 tmp;

   union {
      INT8 bytes[4];
      INT32 outlong;
   } parts;

   parts.outlong = inlong;

   tmp = parts.bytes[0];
   parts.bytes[0] = parts.bytes[3];
   parts.bytes[3] = tmp;
   tmp = parts.bytes[1];
   parts.bytes[1] = parts.bytes[2];
   parts.bytes[2] = tmp;

   return( parts.outlong );
#else
   return( inlong );
#endif
}

/****************************************************************************
	Purpose	:	This function pulls a single value out of a Steim2 compressed
					buffer.
	Returns	: 	Signed 32 bit value based on the bits found.
	Host     :	Borland C/C++ 2.0  GNU C/C++ 2.x
	Target   :	Win32-Intel, Linux-Intel, Solaris-Intel, Solaris-Sparc
	Packages :	None.
	Author   :	Dale G. Fraunberg
	Company  :	Refraction Technology, Inc.
					2626 Lombardy Lane, Suite 105
					Dallas, Texas  75220
					(214) 353-0609     Fax (214) 353-9659
	Copyright:	(c) 2005 Refraction Technology, Inc.

	Revised:
		18May05	(dgf) initial development
-----------------------------------------------------------------------------
*/

INT32 Steim2_get_bits(INT32 in_value, UINT16 shift_right, UINT16 bit_count)
	{
	INT32 result = in_value;
	INT32 lmask, smask;
	
	/*
	in_value    = 32 bit value to extract bits from
	shift_right = number of bits to shift right
	bit_count   = number of bits to retrieve
	*/
	
	if	((bit_count > 32) || (bit_count == 0) || (shift_right > 32))
		{
		return ( (INT32) 0 ); /* nonsense input, return 0 */
		}
	
	lmask = (INT32) 0xFFFFFFFF;
	lmask = lmask << bit_count;
	lmask = ~lmask; /* flip the mask bits */
	
	result = result >> shift_right;
	
	result = (result & lmask);
	
	/* check high bit, if set, then negative number */
	smask = (INT32) 0x00000001;
	smask = smask << (bit_count - 1);
	if (smask & result)
		{
		lmask = ~lmask; /* flip the mask bits */
		result = (result | lmask);
		}
	
	return (result);
	}  /* end of Steim2_get_bits() */

/*---------------------------------------------------------------------
 * Decode INT32 Steim 2 compressed samples in a PASSCAL data record.
 *
 *    BOOL decode_steim2( VOID *ptr, INT16 *n, INT32 *samples );
 *
 * Arguments:
 *    ptr     = VOID pointer to PASSCAL data record block (1024 bytes)
 *    n       = INT16 pointer for number of samples to decompress/actual count
 *    samples = INT32 array for uncompressed samples
 *
 * Returns TRUE if successful, FALSE otherwise
 *
 * samples should be a INT32 array containing at least 1561 elements.
 *
 * Revised:
 *		26Sep2005	(pld) complete decode_steim2()
 *		03Sep2005	(pld) create a 'shell' for now
 */

BOOL decode_steim2( VOID *ptr, INT16 *n, INT32 *samples, INT32 *prev )
	{
   BOOL		skip_ird;
   INT8		f_ndx, c_ndx;
   INT16		i, n_samp;
   UINT16	n_bits, n_difs;
   INT32		sum, difs[8], fi_con, ri_con, sub_type;
   UINT32	flags, chunk;
   STEIM		*rec;

//	printf( "\nSTM2_DE: %p:%u:%p:%u:%p", ptr, *n, samples );

   n_samp = *n;

   /* Get the integration constants */
   rec = (STEIM *)((INT8 *)ptr+64);
   fi_con = swap_l( rec->frame[0].chunk[0].l );
   ri_con = swap_l( rec->frame[0].chunk[1].l );

   /* Init the running sum and save the first sample */
   sum = fi_con;
   n_samp--;
   *samples++ = sum;							/* output first sample */

   /* Skip the inter-record delta, i.e., the difference between records */
   skip_ird = TRUE;

   /* Process up to 15 frames */
	for( f_ndx = 0; f_ndx < 15; f_ndx++ )
		{
      /* Get the frame flags */
      flags = swap_l( rec->frame[f_ndx].flags );

      /* Process 15 chunks per frame */
      for( c_ndx = 0; c_ndx < 15; c_ndx++ )
      	{
         /* Count down samples to 0 */
         if( n_samp <= 0 )
            break;

         chunk = swap_l( rec->frame[f_ndx].chunk[c_ndx].u );
         /* Process a chunk based on type */
         switch( _chunk_type( flags, c_ndx ) )
				{
				case CHUNK_NULL:
					continue;

				case CHUNK_BYTES:
					n_difs = 4;
					n_bits = 8;
					break;  /* end case CHUNK_BYTES */

				case CHUNK_WORDS:
					sub_type = Steim2_get_bits(chunk, 30, 2);
					sub_type &= 0x00000003;

					switch (sub_type) {
						case 1: /* one 30-bit value */
							n_difs = 1;
							n_bits = 30;
							break;

						case 2: /* two 15-bit values */
							n_difs = 2;
							n_bits = 15;
							break;								

						case 3: /* three 10-bit values */
							n_difs = 3;
							n_bits = 10;
							break;

						default:
							rtp_log(RTP_WARN, "Error in Steim2 sub-type (%u) for C=%u\n", sub_type, CHUNK_WORDS);
							return( FALSE );
							break;
						}	/* end switch (sub_type) */
					break;  /* end case CHUNK_WORDS */

				case CHUNK_LONG:
					sub_type = Steim2_get_bits(chunk, 30, 2);
					sub_type &= 0x00000003;

					switch (sub_type)
						{
						case 0: /* five 6-bit values */
							n_difs = 5;
							n_bits = 6;
							break;

						case 1: /* six 5-bit values */
							n_difs = 6;
							n_bits = 5;
							break;								

						case 2: /* seven 4-bit values */
							n_difs = 7;
							n_bits = 4;
							break;
									
						default:
							rtp_log(RTP_WARN, "Error in Steim2 sub-type (%u) for C=%u\n", sub_type, CHUNK_LONG);
							return( FALSE );
							break;
						}	/* end switch (sub_type) */

               break;  /* end case CHUNK_LONGS */
				}  /* end switch(_chunk_type()) */

         /* Decode the differences */
         for( i = 0; i < n_difs; i++ )
            difs[i] = (INT32)Steim2_get_bits(chunk, (UINT16)(((n_difs-1)-i)*n_bits), n_bits);

         /* Save the decoded samples */
         for( i = 0; i < n_difs; i++ )
				{
            if( skip_ird )
					{
               skip_ird = FALSE;
               *prev = fi_con - difs[0];
//               *prev = 0;
//	printf( "\nSTM2_DE: %d+%d=%d", *prev, difs[0], fi_con );
					}
            else
					{
               sum += difs[i];
               *samples++ = sum;
               n_samp--;
               if( n_samp <= 0 )
                  break;
					}
				}  /* end storage loop */
			}  /* end chunk loop */
		}  /* end frame loop */

	if( n_samp > 0 )
		{
#ifdef DEBUG
		printf( "WARNING: Partial decompression of sample data, n_samp = %d\n", n_samp );
#endif
		rtp_log(RTP_WARN, "WARNING: Partial decompression of sample data, n_samp = %d\n", n_samp );
		*n = n_samp;
		return( FALSE );
		}

	if( sum != ri_con )
		{
#ifdef DEBUG
		printf( "WARNING: Unreconciled reverse integrating constant: %ld, sum: %ld\n",
			ri_con, sum );
#endif
		rtp_log(RTP_WARN, "WARNING: Unreconciled reverse integrating constant: %ld, sum: %ld\n",
			ri_con, sum );
		return( FALSE );
  		}
	return TRUE;
}  /* end decode_steim2() */

/****************************************************************************
	Purpose: Encode a buffer of data into a Ref Tek DT packet using Steim2.
   Returns:	Number of samples encoded into the packet.
   Notes  : 
   Revised:
		26Sep05	(pld) initial development
===========================================================================*/
INT16 encode_steim2( INT32 *samples, INT16 n_rawsamp, VOID *ptr, INT32 prev )
	{
   UINT32		*rec;
   UINT16		smpcnt;
	CMP_INFO_T	cmpr_info;

   /* Initialize the data state */
	cmpr_info.type = 0;
	cmpr_info.cmprsn_residl = 0;
	cmpr_info.last_sample = prev;
	cmpr_info.new_pckt_flg = TRUE; /* Restart packet (wan) */

   /* Initialize the sample counter and record pointer */
   rec = (UINT32 *)((INT8 *)ptr+64);
//	printf( "\nSTM2_EN: %p:%u:%p:%d:%p", samples, n_rawsamp, ptr, prev, rec );

   /* Start of compression loop, compress until out of samples or the */
   /* record is full (steim2_compress() returns 1). */
   for( smpcnt = 0; smpcnt < n_rawsamp; smpcnt++ )
   	{
   	if (steim2_compress(*samples++, &cmpr_info, rec))
   		break;
		}
	steim2_flush(&cmpr_info, rec);
		
//   printf( "  %u", cmpr_info.sample_count );
   return( cmpr_info.sample_count );
	}  /* end encode_steim2() */

/****************************************************************************
	Purpose:	Compress a data sample into an output buffer using STEIM2 
				compression techniques
   Returns:	1 when a new packet need to be started.	
   Revised:
		20Jul05  ---- (tld) renamed several variables for clarity, most
		              were part of the strm_cmp_strct{} structure. 
		26MAY05	---- (tld) New funtion.  Portions were taken from steim_compress
							The key for maintaining backward compatability is the
							passed to this function from pkt_stor.c and the structure
							strm_cmp_strct which holds the data and support variables.
===========================================================================*/
INT32 steim2_compress(
	INT32			sample,						/* sample to be compressed */
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			*data_buffer)				/* output buffer */

	{
	CMP_FLAGS_T	comp;							/* comparison flags */
	UINT16		i;								/* general use integer */
	UINT16		j;
	INT32			beg_sample;					/* begin sample for each difference */
	long			end_sample;					/* end sample for each difference */
	UINT16		number_of_values;			/* returned from differencer */
	UINT16		error;						/* error flag */
	long			prev_sample;				/* = last_sample at start */

	error = FALSE;
	ps->cmprsn_residl = 0;					/* 0024 (pld) assume no residual */
	number_of_values = 0;

	/* set up flags and init data arrays for packet */
	init_packet_flags(ps,data_buffer);

	prev_sample = ps->last_sample;
	number_of_values = steim2_diff (ps, sample, &beg_sample, &end_sample);

	if(number_of_values)
		{
		ps->sample_count += number_of_values;
		if((ps->word_per_frame_cnt == 4) && (ps->tmp_cmprsn_flgs[1] == 0))
			{
			ps->tmp_frame_buffer[1] = beg_sample;
			}
		}

	if (ps->word_per_frame_cnt >= WRDS_PER_FRM)
		{
		/* build compression codes word from its pieces */
		comp.flag.word00 = (unsigned int) 0;
		comp.flag.word01 = (unsigned int) ps->tmp_cmprsn_flgs[1];
		comp.flag.word02 = (unsigned int) ps->tmp_cmprsn_flgs[2];
		comp.flag.word03 = (unsigned int) ps->tmp_cmprsn_flgs[3];
		comp.flag.word04 = (unsigned int) ps->tmp_cmprsn_flgs[4];
		comp.flag.word05 = (unsigned int) ps->tmp_cmprsn_flgs[5];
		comp.flag.word06 = (unsigned int) ps->tmp_cmprsn_flgs[6];
		comp.flag.word07 = (unsigned int) ps->tmp_cmprsn_flgs[7];
		comp.flag.word08 = (unsigned int) ps->tmp_cmprsn_flgs[8];
		comp.flag.word09 = (unsigned int) ps->tmp_cmprsn_flgs[9];
		comp.flag.word10 = (unsigned int) ps->tmp_cmprsn_flgs[10];
		comp.flag.word11 = (unsigned int) ps->tmp_cmprsn_flgs[11];
		comp.flag.word12 = (unsigned int) ps->tmp_cmprsn_flgs[12];
		comp.flag.word13 = (unsigned int) ps->tmp_cmprsn_flgs[13];
		comp.flag.word14 = (unsigned int) ps->tmp_cmprsn_flgs[14];
		comp.flag.word15 = (unsigned int) ps->tmp_cmprsn_flgs[15];

		/* output the frame */
		ps->tmp_frame_buffer[0] = comp.word;

		/* copy words to packet buffer */
		for (j = 0; j < FRAMES_PER_PCKT; j++)
			{
			data_buffer[ps->data_offset++] = swap_l(ps->tmp_frame_buffer[j]);
			}
		
		/* reinit values for next packet */
		if (ps->data_offset >= WRDS_PER_PCKT)
			{
			ps->new_pckt_flg = TRUE+99;
			error = 1;
			/* store end sample we are at the end of this packet*/
			data_buffer[END_SAMPLE_POS] = swap_l(end_sample);
			ps->word_per_frame_cnt = 3;
			}
		else
			{
			ps->word_per_frame_cnt = 1;
			}

		/* set compression flags back to 0 for next frame */
		for (i=0; i < FRAMES_PER_PCKT; i++)
			{
			ps->tmp_cmprsn_flgs[i] = 0;
			/* this is new and may need to be removed */
			ps->tmp_frame_buffer[i] = 0;
			}
		}

	return (error);
	}  /* end steim2_compress() */

/****************************************************************************
	Purpose:	 calculates the difference by subtracting the last value from the 
				 current value.  The result is checked to determine how many 
				 bits are required to store it.
				
   Returns:	 number of differences written out when we have enough to compress
				 into a 32 bit word.
   Revised:
		20Jul05  ---- (tld) renamed several variables for clarity, most
		              were part of the strm_cmp_strct{} structure. 
		01Jun05	---- (tld) New funtion.  
===========================================================================*/
int steim2_diff (
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			this_sample,				/* value of current sample */
	INT32			*beg_sample_ptr,			/* first sample of each output */
	INT32			*end_sample_ptr)			/* last sample of each output */
	{

	int	number_of_values;					/* number of values compressed */
	INT32	this_diff;							/* current difference */
	int	this_type;							/* Steim type, current diff */
	UBYTE	prev_type;							/* Steim type from last diff */
	
	/*----------------------------------------------------------------------- 
		no values currently in queue ie first difference processed for a new 
		word in the frame.  ps->diffs_n_que holds the number of diferences 
		currently being held until we have enough to fill the next word 
		position in the frame.  It is also used to index into the ps->sample_que
		and ps->diff_que arrays that store the current sample and difference.
		Note that the position in the array is one less that the value of 
		condition. 
	-------------------------------------------------------------------------*/

	if ((ps->type == 0) || (ps->diffs_n_que == 0))
		{
		ps->diffs_n_que = 0;
		ps->type = 0;
		}
	number_of_values = 0;

	/* Take difference between this sample and previous one; reset previous */
	this_diff = this_sample - ps->last_sample;

	/*----------------------------------------------------------------------- 
		determine the number of bits required to hold the difference between
		this value and the previous value. If this is a new "word" the 
		difference  is 
		1 = 4 bits required to hold difference
		2 = 5 bits required to hold difference
		3 = 6 bits required to hold difference
		4 = 8 bits required to hold difference
		5 = 10 bits required to hold difference
		6 = 15 bits required to hold difference
		7 = 30 bits required to hold difference
		if this value requires a larger number of bits to hold it than the 
		previous differences set the type equal to the new size otherwise 
		just store it and continue.
	-----------------------------------------------------------------------*/	

	ps->diffs_n_que++;
	prev_type = ps->type;

	if (this_diff >= 0)
		{
		if (this_diff <= st2_pos_dif[1])
			{
			this_type = 1;
			ps->type = max (ps->type, 1);
			}
		else if (this_diff <= st2_pos_dif[2])
			{
			this_type = 2;
			ps->type = max (ps->type, 2);
			}
		else if (this_diff <= st2_pos_dif[3])
			{
			this_type = 3;
			ps->type = max (ps->type, 3);
			}
		else if (this_diff <= st2_pos_dif[4])
			{
			this_type = 4;
			ps->type = max (ps->type, 4);
			}
		else if (this_diff <= st2_pos_dif[5])
			{
			this_type = 5;
			ps->type = max (ps->type, 5);
			}
		else if (this_diff <= st2_pos_dif[6])
			{
			this_type = 6;
			ps->type = max (ps->type, 6);
			}
		else
			{
			this_type = 7;
			ps->type = 7;
			}
		}
	else
		{
		if (this_diff >= st2_neg_dif[1])
			{
			this_type = 1;
			ps->type = max (ps->type, 1);
			}
		else if (this_diff >= st2_neg_dif[2])
			{
			this_type = 2;
			ps->type = max (ps->type, 2);
			}
		else if (this_diff >= st2_neg_dif[3])
			{
			this_type = 3;
			ps->type = max (ps->type, 3);
			}
		else if (this_diff >= st2_neg_dif[4])
			{
			this_type = 4;
			ps->type = max (ps->type, 4);
			}
		else if (this_diff >= st2_neg_dif[5])
			{
			this_type = 5;
			ps->type = max (ps->type, 5);
			}
		else if (this_diff >= st2_neg_dif[6])
			{
			this_type = 6;
			ps->type = max (ps->type, 6);
			}
		else
			{
			this_type = 7;
			ps->type = 7;
			}
		}
		
	/* save sample and difference in array */
	ps->sample_que[ps->diffs_n_que - 1] = this_sample;
	ps->diff_que[ps->diffs_n_que - 1] = this_diff;
	
	/*----------------------------------------------------------------------- 
	  Check to see if the current difference plus the differences in the 
	  queue cause us to have enough samples to write out the next word to the 
	  frame, based on the largest difference size of the samples in the queue.
	-----------------------------------------------------------------------*/	
	if((unsigned int)ps->diffs_n_que >= st2_max_per_word[ps->type])
		{
		*beg_sample_ptr = ps->sample_que[0];
		if((unsigned int)ps->diffs_n_que > st2_max_per_word[ps->type])
			{
			/*------------------------------------------------------------------ 
			  Had more differences that we can fit, save the rest for the next 
			  word we write out.
			  promote type to next larger size until we have the correct type
			  for the number of differences stored prior to the last difference
			  which caused us to go over the threshold.
			------------------------------------------------------------------*/	
			while((unsigned int)(ps->diffs_n_que - 1) < st2_max_per_word[prev_type])
				{
				prev_type++;
				}
			number_of_values = steim2_build_word(ps, prev_type);
			/*------------------------------------------------------------------ 
			  move unused diff down in buffer to position 0 for next pass 
			  move diff sample down to position 0 for next pass 
			  set ps->diffs_n_que = 1  for next pass  
			------------------------------------------------------------------*/
			*end_sample_ptr = ps->sample_que[ps->diffs_n_que - 2];
			ps->sample_que[0] = ps->sample_que[ps->diffs_n_que - 1];
			ps->diff_que[0] = ps->diff_que[ps->diffs_n_que - 1];
			ps->diffs_n_que = 1;
			/* this may be it */
			ps->cmprsn_residl = 1;
			ps->type = this_type;
			}
		else
			{
			/*------------------------------------------------------------------
			  got exactly the right number of diffs to fill the word let's 
			  process them
			------------------------------------------------------------------*/
			*end_sample_ptr = ps->sample_que[ps->diffs_n_que - 1];
			number_of_values = steim2_build_word(ps, ps->type);
			/* setup for next word */
			ps->diffs_n_que = 0;
			ps->type = 0;
			}
		}

	ps->last_sample = this_sample;

	return (number_of_values);
	}  /* end steim2_diff() */


/****************************************************************************
	Purpose:	 builds the actual 32 bit compressed word based on the steim2
				 compression algorithm.  
				
   Returns:	 number of differences compressed.
   Revised:
		20Jul05  ---- (tld) renamed several variables for clarity, most
		              were part of the strm_cmp_strct{} structure. 
		01Jun05	---- (tld) New funtion.  
===========================================================================*/
int steim2_build_word(
	CMP_INFO_T	*ps,							/* Address of compression structure */
	UBYTE			word_type)
	{

	long int comp_word = 0x00000000;
	long int temp_diff;
	unsigned long int upper_mask;
	unsigned int cnt;

	comp_word = comp_word + st2_dnbl_flgs[word_type];

	for ( cnt = 0; cnt < st2_max_per_word[word_type]; cnt++)
		{
		upper_mask = 0xffffffff;
		/*------------------------------------------------------------------ 
		  shift unsigned mask right (current cnt position * size of type + 
		  the dnib # of bits).  This will be used to mask out all bits to
		  the left of the current diff so we can add it to the word we
		  are building.  ***** Mask MUST BE UNSIGNED so 0's are shift in 
		  as we shift to the right!
		------------------------------------------------------------------*/
		upper_mask = upper_mask >> (((st2_max_per_word[word_type] - 1) * 
												st2_diff_size[word_type]) + 
												st2_diff_offset[word_type]);
		temp_diff = upper_mask & (long int)ps->diff_que[cnt];
		/*------------------------------------------------------------------ 
		  Now that we have the mask value we will shift it left to the 
		  correct position so we can do an add operation.
		------------------------------------------------------------------*/
		temp_diff = temp_diff << (WORD_BITS  - (((cnt + 1) * 
																st2_diff_size[word_type]) + 
																st2_diff_offset[word_type]));
		comp_word = comp_word + temp_diff;
		}
	/* save the word we built in the array */
	ps->tmp_frame_buffer[ps->word_per_frame_cnt] = comp_word;
	/* save the ck value for this bit size in the ck array */ 
	ps->tmp_cmprsn_flgs[ps->word_per_frame_cnt] = st2_ck_flgs[word_type];
	ps->word_per_frame_cnt++;

	return((int)cnt);
	}  /* end steim2_build_word() */

/****************************************************************************
	Purpose:	flushes all remaining values from the temporary buffer used to 
				build the compressed words writing them to the 1k output buffer.
				Additional cleanup is performed and the last value is written to 
				the buffer.

  Returns:	0 is always returned from this funtion.  The Steim1 compression 
				can have multiple words that need to be compressed and flushed 
				which can cross the 1k packet size boundary.  This situation 
				requires that an additional packet be created to hold the reamining
				information.  Steim 2 never has more than 1 difference to compress 
				at the time flush is called therefore if the packet size has been
				reached the new packet was already created when steim_compress was 
				called.  
   Revised:
		20Jul05  ---- (tld) renamed several variables for clarity, most
		              were part of the strm_cmp_strct{} structure. 
		19Jul05  ---- (tld) added call to init_packet_flags(), to correct for 
							a single value left over when the flush occurred. 
		06Jun05	---- (tld) New funtion.
===========================================================================*/
int steim2_flush(
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			*data_buffer)				/* output buffer */
	{

	CMP_FLAGS_T	comp;							/* comparison flags */
	UINT16		j;
	UINT16		number_of_values;			/* returned from differencer */
	UINT16		result;						/* error flag */

	result = FALSE;
	ps->cmprsn_residl = 0;						/* 0024 (pld) assume no residual */

	/* set up flags and init data arrays for packet */
	init_packet_flags(ps,data_buffer);


	/* check for uncompressed differences */
	if(ps->diffs_n_que > 0)
		{
		/* just entered a new frame and flush was called, store start */
		if((ps->word_per_frame_cnt == 3) && (ps->tmp_cmprsn_flgs[1] == 0))
			{
			ps->tmp_frame_buffer[1] = ps->sample_que[0];
			}

		/* find the largest type the remainder will fit in */
		while (ps->diffs_n_que < st2_max_per_word[ps->type])
			{
			ps->type++;
			}
		/* compress the reamining differences */ 
		number_of_values = steim2_build_word(ps, ps->type);
		/* add samples to total count */
		ps->sample_count += number_of_values;
		}


	/* build compression codes word from its pieces */
		comp.flag.word00 = (unsigned int) 0;
		comp.flag.word01 = (unsigned int) ps->tmp_cmprsn_flgs[1];
		comp.flag.word02 = (unsigned int) ps->tmp_cmprsn_flgs[2];
		comp.flag.word03 = (unsigned int) ps->tmp_cmprsn_flgs[3];
		comp.flag.word04 = (unsigned int) ps->tmp_cmprsn_flgs[4];
		comp.flag.word05 = (unsigned int) ps->tmp_cmprsn_flgs[5];
		comp.flag.word06 = (unsigned int) ps->tmp_cmprsn_flgs[6];
		comp.flag.word07 = (unsigned int) ps->tmp_cmprsn_flgs[7];
		comp.flag.word08 = (unsigned int) ps->tmp_cmprsn_flgs[8];
		comp.flag.word09 = (unsigned int) ps->tmp_cmprsn_flgs[9];
		comp.flag.word10 = (unsigned int) ps->tmp_cmprsn_flgs[10];
		comp.flag.word11 = (unsigned int) ps->tmp_cmprsn_flgs[11];
		comp.flag.word12 = (unsigned int) ps->tmp_cmprsn_flgs[12];
		comp.flag.word13 = (unsigned int) ps->tmp_cmprsn_flgs[13];
		comp.flag.word14 = (unsigned int) ps->tmp_cmprsn_flgs[14];
		comp.flag.word15 = (unsigned int) ps->tmp_cmprsn_flgs[15];

	/* output the frame */
	ps->tmp_frame_buffer[0] = comp.word;

	/* copy words to packet buffer */
	for (j = 0; j < FRAMES_PER_PCKT; j++)
		{
		data_buffer[ps->data_offset++] = swap_l(ps->tmp_frame_buffer[j]);
		}
		/* store end sample we are at the end of this packet*/
	data_buffer[END_SAMPLE_POS] = swap_l(ps->last_sample);

	if(ps->data_offset >= WRDS_PER_PCKT && ps->diffs_n_que > 0)
		{
		result = 0;
		}

	return (result);
	}  /* end steim2_flush() */
/****************************************************************************
	Purpose:	Initialize packet data and set flags. Used by Steim1 and Steim2

  Returns:	none	
   Revised:
		26Sep05	---- (pld) remove zeroing of last-sample - handled higher up
		20Jul05  ---- (tld) renamed several variables for clarity, most
		              were part of the strm_cmp_strct{} structure. 
		06Jun05	---- (tld) New funtion.  
===========================================================================*/
void init_packet_flags(
	CMP_INFO_T	*ps,							/* Address of compression structure */
	INT32			*data_buffer)				/* output buffer */

	{

	int i;
	/*-----------------------------------------------------------------------
	   Invoke this section if restart flag is set.
		TRUE+99 indicates that we are starting frames 2-15 in the current 
		packet being built.
		TRUE indicates that this is frame 1 of a new packet.  This requires
		additional house keeping.  Compression flag positions 1 and 2 are set
		to 0 because frame_word 1 will hold the beginning value of the packet
		and frame_word 2 will hold the ending value of the packet.  These 
		frame_words are also set to 0 and the frame_word counter is set to 
		frame_word 3.  last_sample is set to zero since it's value is not known. 
	-----------------------------------------------------------------------*/
	if (ps->new_pckt_flg)
		{
		ps->sample_count = 0;
		ps->data_offset = 0;
		ps->tmp_cmprsn_flgs[1] = 0;
		ps->tmp_cmprsn_flgs[2] = 0;
		ps->tmp_frame_buffer[2] = 0;
		if (ps->new_pckt_flg != TRUE+99)
			{
			/* init these values only for new packet */
			ps->word_per_frame_cnt = 3;
//			ps->last_sample = 0;
			ps->tmp_frame_buffer[1] = 0;
			}
		/* clear the compression flag values */
		for (i=ps->word_per_frame_cnt; i < WRDS_PER_FRM; i++)
			{
			ps->tmp_cmprsn_flgs[i] = 0;
			}
		/* clear the data buffer (15 longs) so we can start fresh */
		for (i=0; i < WRDS_PER_PCKT; i += FRAMES_PER_PCKT)
			{
			*(data_buffer + i) = 0;
			}
		ps->new_pckt_flg = FALSE;
		}

	}/* end init_packet_flags() */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:36  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.4  2005/09/03 21:52:31  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.3  2004/08/11 14:50:04  rstavely
 * Changes for version 1.19 Arc utils include multi units & streams, & alignment for
 * arcfetch.
 * Add Rtptrig utility based on rtpftp.
 *
 * Revision 1.2  2002/01/18 17:55:57  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
