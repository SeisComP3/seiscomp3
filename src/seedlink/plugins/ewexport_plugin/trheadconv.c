/*
      Function TrHeadConv():
      Converts a structure of type TRACE_HEADER to a structure
      of type TRACE2_HEADER.  The location code field in the
      TRACE2_HEADER is set to the null string, ie "--".
      Note that conversion occurs "in-place", so the original
      TRACE_HEADER is not saved.   WMK 4/23/04
*/

#include <string.h>
#include <ewdefs.h>

TRACE2_HEADER *TrHeadConv( TRACE_HEADER *th )
{
   TRACE2_HEADER *t2h = (TRACE2_HEADER *)th;

   t2h->chan[TRACE2_CHAN_LEN-1] = '\0';  /* Null-terminate channel field */
   strcpy( t2h->loc, LOC_NULL_STRING );
   t2h->version[0] = TRACE2_VERSION0;
   t2h->version[1] = TRACE2_VERSION1;
   return t2h;
}
