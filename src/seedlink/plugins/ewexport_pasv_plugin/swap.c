
/*
 * swap.c taken from the Earthworm v6.2 distribution.
 *
 * Modified WaveMsgMakeLocal() to work without defines.
 *
 */

/*
 * SWAP.C
 *
 *  Byte swapping functions
 */

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "swap.h"


void SwapShort( short *data )
{
   char temp;

   union {
      char  c[2];
   } dat;

   memcpy( &dat, data, sizeof(short) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[1];
   dat.c[1] = temp;
   memcpy( data, &dat, sizeof(short) );
   return;
}

void SwapInt( int *data )
{
   char temp;

   union {
      char c[4];
   } dat;

   memcpy( &dat, data, sizeof(int) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( data, &dat, sizeof(int) );
   return;
}

void SwapDouble( double *data )
{
   char temp;

   union {
       char   c[8];
   } dat;

   memcpy( &dat, data, sizeof(double) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[7];
   dat.c[7] = temp;

   temp     = dat.c[1];
   dat.c[1] = dat.c[6];
   dat.c[6] = temp;

   temp     = dat.c[2];
   dat.c[2] = dat.c[5];
   dat.c[5] = temp;

   temp     = dat.c[3];
   dat.c[3] = dat.c[4];
   dat.c[4] = temp;
   memcpy( data, &dat, sizeof(double) );
   return;
}


/**************************** swapWaveMsg ***************************
*       Byte-swap a univerals Waveform message in place.            *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/

int WaveMsgMakeLocal( TRACE_HEADER* wvmsg )
{
  return( WaveMsg2MakeLocal( (TRACE2_HEADER *) wvmsg ) );
}


int WaveMsg2MakeLocal( TRACE2_HEADER* wvmsg )
{
   int    dataSize;  /* flag telling us how many bytes in the data */
   int    i;
   char   byteOrder;
   char   hostOrder;
   int    nsamp;
   int   *intPtr;
   short *shortPtr;
   double samprate,starttime,endtime;
   double tShouldEnd;
   double dFudgeFactor;

   /* Check local machine order with htons() */
   if ( htons(0x1234) == 0x1234 )
     hostOrder = 's';
   else
     hostOrder = 'i';

   /* See what sort of data it carries
    **********************************/
   dataSize=0;
   if ( strcmp(wvmsg->datatype, "s4")==0)
   {
        dataSize=4; byteOrder='s';
   }
   else if ( strcmp(wvmsg->datatype, "i4")==0)
   {
        dataSize=4; byteOrder='i';
   }
   else if ( strcmp(wvmsg->datatype, "s2")==0)
   {
        dataSize=2; byteOrder='s';
   }
   else if ( strcmp(wvmsg->datatype, "i2")==0)
   {
        dataSize=2; byteOrder='i';
   }
   else
        return(-1); /* We don't know this message type*/



   /* SWAP the header (if neccessary) */
   if ( byteOrder != hostOrder )
   {
        /* swap the header
        *****************/
        SwapInt( &(wvmsg->pinno) );
        SwapInt( &(wvmsg->nsamp) );
        SwapDouble( &(wvmsg->starttime) );
        SwapDouble( &(wvmsg->endtime)   );
        SwapDouble( &(wvmsg->samprate)  );
   }

  if (wvmsg->nsamp > 4000/dataSize) {
     printf( "WaveMsg2MakeLocal error: nsamps (%d) bigger than allowed for a tbuf with data type %s.",
		wvmsg->nsamp, wvmsg->datatype );
     return(-1);
  }

/* Perform a CheckSumish kind of calculation on the header
 * ensure that the tracebuf ends within 5 samples of the given endtime.
 * DK 2002/03/18
 *********************************************************************/

/* moved nsamp memcpy to here to avoid byte-alignment with next statement */
   memcpy( &nsamp,     &wvmsg->nsamp,     sizeof(int)    );
   memcpy( &samprate,  &wvmsg->samprate,  sizeof(double) );
   memcpy( &starttime, &wvmsg->starttime, sizeof(double) );
   memcpy( &endtime,   &wvmsg->endtime,   sizeof(double) );

   tShouldEnd   = starttime + ((nsamp - 1) / samprate);
   dFudgeFactor = 5.0 / samprate;

/* This is supposed to be a simple sanity check to ensure that the
 * endtime is within 5 samples of where it should be.  We're not
 * trying to be judgemental here, we're just trying to ensure that
 * we protect ourselves from complete garbage, so that we don't segfault
 * when allocating samples based on a bad nsamp
 ***********************************************************************/
   if( endtime < (tShouldEnd-dFudgeFactor) ||
       endtime > (tShouldEnd+dFudgeFactor)    )
   {
      return(-2);
   }

   /* SWAP the data (if neccessary) */
   if (byteOrder != hostOrder)
   {
   /* Swap the data.  */
        intPtr=(int*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        shortPtr=(short*) ((char*)wvmsg + sizeof(TRACE2_HEADER) );
        for( i=0; i<nsamp; i++)
        {
                if(dataSize==2) SwapShort( &(shortPtr[i]) );
                if(dataSize==4) SwapInt(  &(intPtr[i])  );
        }
        /* Re-write the data type field in the message
        **********************************************/
        if(dataSize==2) strcpy(wvmsg->datatype,"s2");
        if(dataSize==4) strcpy(wvmsg->datatype,"s4");
   }

   return(0);
}


void SwapFloat( float *data )
{
   char temp;

   union {
      char c[4];
   } dat;

   memcpy( &dat, data, sizeof(float) );
   temp     = dat.c[0];
   dat.c[0] = dat.c[3];
   dat.c[3] = temp;
   temp     = dat.c[1];
   dat.c[1] = dat.c[2];
   dat.c[2] = temp;
   memcpy( data, &dat, sizeof(float) );
   return;
}

