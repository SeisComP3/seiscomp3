/*********************************************************************
 * seedutil.c:
 *
 * Utility routines for Mini-SEED records.
 *
 * Written by Chad Trabant, IRIS Data Management Center
 *
 * modified: 2006.006
 *********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "seedutil.h"


/*********************************************************************
 * find_reclen:
 *
 * Search for a 1000 blockette in a MiniSEED data record and return
 * the record size.
 *
 * Returns size of the record in bytes, 0 if 1000 blockette was not
 * found or -1 on error.
 *********************************************************************/
int
find_reclen ( const char *msrecord, int maxheaderlen )
{
  unsigned short begin_blockette; /* byte offset for next blockette */
  char swap_flag = 0;             /* is swapping needed? */
  char found1000 = 0;             /* found 1000 blockette? */
  char idx;
  int reclen = -1;                /* size of record in bytes */
  
  struct s_fsdh_data *fsdh_data;
  struct s_blk_1000  *blk_1000;
  struct s_blk_head  blk_head;

  /* Simple verification of a data record */
  if ( *(msrecord+6) != 'D' &&
       *(msrecord+6) != 'R' &&
       *(msrecord+6) != 'Q' )
    return -1;
  
  fsdh_data = (struct s_fsdh_data *) (msrecord+20);
  
  /* Check to see if byte swapping is needed (bogus year makes good test) */
  if ( (fsdh_data->start_time.year < 1960) ||
       (fsdh_data->start_time.year > 2050) )
    swap_flag = 1;
  
  begin_blockette = fsdh_data->begin_blockette;
  
  /* Swap order of begin_blockette field if needed */
  swap_2bytes(&begin_blockette, swap_flag);
  
  /* loop through blockettes as long as number is non-zero and viable */
  while ((begin_blockette != 0) &&
	 (begin_blockette <= (unsigned short) maxheaderlen))
    {
      memcpy((void *)&blk_head, msrecord+begin_blockette,
	     sizeof(struct s_blk_head));
      swap_2bytes(&blk_head.blk_type, swap_flag);
      swap_2bytes(&blk_head.next_blk, swap_flag);
      
      if (blk_head.blk_type == 1000)  /* Found the 1000 blockette */
	{
	  blk_1000 = (struct s_blk_1000 *) (msrecord+begin_blockette);
	  
	  found1000 = 1;
	  
	  /* Calculate record size in bytes as 2^(blk_1000->rec_len) */
	  for (reclen=1, idx=1; idx <= blk_1000->rec_len; idx++)
	    reclen *= 2;      
	}    

      begin_blockette = blk_head.next_blk;
    }
  
  if ( !found1000 )
    return 0;
  else
    return reclen;
}


void
swap_2bytes (unsigned short *a, char f)
{
  union {
    unsigned short i;
    char b[2];
  } word;
  char temp;

  if (f == 1){  /* f is the flag to trigger byte swapping */
  word.i = *a;
  temp = word.b[0];
  word.b[0] = word.b[1];
  word.b[1] = temp;
  memcpy((void *)a,(void *)&(word.i),sizeof(short));
  }
}


void
swap_4bytes (unsigned int *a, char f)
{
  union {
    unsigned int i;
    char b[4];
  } word;
  char temp;

  if (f == 1){  /* f is the flag to trigger byte swapping */
  word.i = *a;
  temp = word.b[0];
  word.b[0] = word.b[3];
  word.b[3] = temp;
  temp = word.b[1];
  word.b[1] = word.b[2];
  word.b[2] = temp;
  memcpy((void *)a,(void *)&(word.i),sizeof(int));
  }
}
