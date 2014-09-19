
/*
 * swap.h taken from the Earthworm v6.2 distribution.
 */

#ifndef SWAP_H
#define SWAP_H

#include "ewdefs.h"
#include "trace_buf.h"

/* include file for swap.c: handy routines for swapping earthwormy things */

void SwapShort( short * );                  /* swap.c       sys-independent  */
void SwapInt( int * );                      /* swap.c       sys-independent  */
void SwapDouble( double * );                /* swap.c       sys-independent  */
void SwapFloat( float * );                  /* swap.c       sys-independent  */

/* fixes wave message into local byte order, based on globals _SPARC and _INTEL */
int WaveMsgMakeLocal( TRACE_HEADER* );
int WaveMsg2MakeLocal( TRACE2_HEADER* );

#endif
