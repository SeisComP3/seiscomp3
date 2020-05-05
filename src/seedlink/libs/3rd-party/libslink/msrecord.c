/***************************************************************************
 * msrecord.c:
 *
 * Generic routines to parse Mini-SEED records.
 *
 * Appropriate values from the record header will be byte-swapped to the
 * host order.  The purpose of this code is to provide a portable way of
 * accessing common SEED data record header information.  The recognized
 * structures are the Fixed Section Data Header and Blockettes 100, 1000
 * and 1001.  The Blockettes are optionally parsed and the data samples
 * are optionally decompressed/unpacked.
 *
 * Some ideas and structures were used from seedsniff 2.0
 *
 * Written by Chad Trabant, IRIS Data Managment Center
 *
 * modified: 2016.288
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libslink.h"
#include "unpack.h"

#define SL_ISVALIDYEARDAY(Y, D) (Y >= 1900 && Y <= 2100 && D >= 1 && D <= 366)

/* Declare routines only used in this source file */
void encoding_hash (char enc, char *encstr);
double host_latency (SLMSrecord *msr);

/***************************************************************************
 * sl_msr_new:
 *
 * Allocate, initialize and return a new SLMSrecord struct.
 *
 * Returns a pointer to a SLMSrecord struct on success
 *  or NULL on error.
 ***************************************************************************/
SLMSrecord *
sl_msr_new (void)
{
  SLMSrecord *msr;

  msr = (SLMSrecord *)malloc (sizeof (SLMSrecord));

  if (msr == NULL)
  {
    sl_log_rl (NULL, 2, 0, "sl_msr_new(): error allocating memory\n");
    return NULL;
  }

  msr->fsdh.sequence_number[0] = '\0';
  msr->fsdh.dhq_indicator      = 0;
  msr->fsdh.reserved           = 0;
  msr->fsdh.station[0]         = '\0';
  msr->fsdh.location[0]        = '\0';
  msr->fsdh.channel[0]         = '\0';
  msr->fsdh.network[0]         = '\0';
  msr->fsdh.start_time.year    = 0;
  msr->fsdh.start_time.day     = 0;
  msr->fsdh.start_time.hour    = 0;
  msr->fsdh.start_time.min     = 0;
  msr->fsdh.start_time.sec     = 0;
  msr->fsdh.start_time.unused  = 0;
  msr->fsdh.start_time.fract   = 0;
  msr->fsdh.num_samples        = 0;
  msr->fsdh.samprate_fact      = 0;
  msr->fsdh.samprate_mult      = 0;
  msr->fsdh.act_flags          = 0;
  msr->fsdh.io_flags           = 0;
  msr->fsdh.dq_flags           = 0;
  msr->fsdh.num_blockettes     = 0;
  msr->fsdh.time_correct       = 0;
  msr->fsdh.begin_data         = 0;
  msr->fsdh.begin_blockette    = 0;

  msr->Blkt100  = NULL;
  msr->Blkt1000 = NULL;
  msr->Blkt1001 = NULL;
  msr->Blkt2000 = NULL;
  msr->Blkt2000Ofs = -1;

  msr->msrecord    = NULL;
  msr->datasamples = NULL;
  msr->numsamples  = -1;
  msr->unpackerr   = MSD_NOERROR;

  return msr;
} /* End of sl_msr_new() */

/***************************************************************************
 * sl_msr_free:
 *
 * Free all memory associated with a SLMSrecord struct, except the original
 * record indicated by the element 'msrecord'.
 ***************************************************************************/
void
sl_msr_free (SLMSrecord **msr)
{
  if (msr != NULL && *msr != NULL)
  {
    free ((*msr)->Blkt100);
    free ((*msr)->Blkt1000);
    free ((*msr)->Blkt1001);
    free ((*msr)->Blkt2000);

    if ((*msr)->datasamples != NULL)
      free ((*msr)->datasamples);

    free (*msr);

    *msr = NULL;
  }
} /* End of sl_msr_free() */

/***************************************************************************
 * sl_littleendianhost:
 *
 * Determine the byte order of the host machine.  Due to the lack of
 * portable defines to determine host byte order this run-time test is
 * provided.  This function originated from a similar function in libmseed.
 *
 * Returns 1 if the host is little endian, otherwise 0.
 ***************************************************************************/
static uint8_t
sl_littleendianhost (void)
{
  uint16_t host = 1;
  return *((uint8_t *)(&host));
} /* End of sl_littleendianhost() */

/***************************************************************************
 * sl_msr_parse:
 *
 * A wrapper for sl_msr_parse_size()
 ***************************************************************************/
SLMSrecord *
sl_msr_parse (SLlog *log, const char *msrecord, SLMSrecord **ppmsr,
              int8_t blktflag, int8_t unpackflag)
{
  return sl_msr_parse_size (log, msrecord, ppmsr, blktflag, unpackflag, SLRECSIZE);
}

/***************************************************************************
 * sl_msr_parse_size:
 *
 * Parses a SEED record header/blockettes and populates a SLMSrecord struct.
 *
 * If 'blktflag' is true the blockettes will also be parsed.  The parser
 * recognizes Blockettes 100, 1000 and 1001.
 *
 * If 'unpackflag' is true the data samples are unpacked/decompressed and
 * the SLMSrecord->datasamples pointer is set appropriately.  The data samples
 * will be 32-bit integers with the same byte order as the host.  The
 * SLMSrecord->numsamples will be set to the actual number of samples
 * unpacked/decompressed and SLMSrecord->unpackerr will be set to indicate
 * any errors encountered during unpacking/decompression (MSD_NOERROR if
 * no errors).
 *
 * All appropriate values will be byte-swapped to the host order, including
 * the data samples.
 *
 * All header values, blockette values and data samples will be overwritten
 * by subsequent calls to this function.
 *
 * If the msr struct is NULL it will be allocated.
 *
 * Returns a pointer to the SLMSrecord struct populated on success.  On
 * error *ppmsr is set to NULL and NULL is returned.
 *
 * 6 Apr 2012 - Jacob Crummey:
 * Added sl_msr_parse_size() to allow for selecting SeedLink record
 * packet size.  Possible values for slrecsize are 128, 256, 512.
 * There is no error checking, so the value of slrecsize must be
 * checked before passing it to sl_msr_parse_size().
 ***************************************************************************/
SLMSrecord *
sl_msr_parse_size (SLlog *log, const char *msrecord, SLMSrecord **ppmsr,
                   int8_t blktflag, int8_t unpackflag, int slrecsize)
{
  uint8_t headerswapflag = 0; /* is swapping needed? */
  uint8_t dataswapflag   = 0;
  SLMSrecord *msr        = NULL;

  if (ppmsr == NULL)
  {
    sl_log_rl (log, 2, 1, "msr_parse(): pointer to SLMSrecord cannot be NULL\n");
    *ppmsr = NULL;
    return NULL;
  }
  else
  {
    msr = *ppmsr;
  }

  /* If this record is new init a new one otherwise clean it up */
  if (msr == NULL)
  {
    msr = sl_msr_new ();
  }
  else
  {
    if (msr->Blkt100 != NULL)
    {
      free (msr->Blkt100);
      msr->Blkt100 = NULL;
    }
    if (msr->Blkt1000 != NULL)
    {
      free (msr->Blkt1000);
      msr->Blkt1000 = NULL;
    }

    if (msr->Blkt1001 != NULL)
    {
      free (msr->Blkt1001);
      msr->Blkt1001 = NULL;
    }

    if (msr->Blkt2000 != NULL)
    {
      free (msr->Blkt2000);
      msr->Blkt2000 = NULL;
      msr->Blkt2000Ofs = -1;
    }

    if (msr->datasamples != NULL)
    {
      free (msr->datasamples);
      msr->datasamples = NULL;
    }
  }

  msr->msrecord = msrecord;

  /* Copy the fixed section into msr */
  memcpy ((void *)&msr->fsdh, msrecord, 48);

  /* Sanity check for msr/quality indicator */
  if (msr->fsdh.dhq_indicator != 'D' &&
      msr->fsdh.dhq_indicator != 'R' &&
      msr->fsdh.dhq_indicator != 'Q')
  {
    sl_log_rl (log, 2, 0, "record header/quality indicator unrecognized: %c\n",
               msr->fsdh.dhq_indicator);
    sl_msr_free (&msr);
    *ppmsr = NULL;
    return NULL;
  }

  /* Check to see if byte swapping is needed by testing the year and day */
  if (!SL_ISVALIDYEARDAY (msr->fsdh.start_time.year, msr->fsdh.start_time.day))
    headerswapflag = dataswapflag = 1;

  /* Change byte order? */
  if (headerswapflag)
  {
    SL_SWAPBTIME (&msr->fsdh.start_time);
    sl_gswap2 (&msr->fsdh.num_samples);
    sl_gswap2 (&msr->fsdh.samprate_fact);
    sl_gswap2 (&msr->fsdh.samprate_mult);
    sl_gswap4 (&msr->fsdh.time_correct);
    sl_gswap2 (&msr->fsdh.begin_data);
    sl_gswap2 (&msr->fsdh.begin_blockette);
  }

  /* Parse the blockettes if requested */
  if (blktflag)
  {
    /* Define some structures */
    struct sl_blkt_head_s *blkt_head;
    struct sl_blkt_100_s *blkt_100;
    struct sl_blkt_1000_s *blkt_1000;
    struct sl_blkt_1001_s *blkt_1001;
    struct sl_blkt_2000_s *blkt_2000;
    uint16_t begin_blockette; /* byte offset for next blockette */

    /* Initialize the blockette structures */
    blkt_head = (struct sl_blkt_head_s *)malloc (sizeof (struct sl_blkt_head_s));
    blkt_100  = NULL;
    blkt_1000 = NULL;
    blkt_1001 = NULL;
    blkt_2000 = NULL;

    /* loop through blockettes as long as number is non-zero and viable */
    begin_blockette = msr->fsdh.begin_blockette;

    while ((begin_blockette != 0) &&
           (begin_blockette <= slrecsize))
    {

      memcpy ((void *)blkt_head, msrecord + begin_blockette,
              sizeof (struct sl_blkt_head_s));
      if (headerswapflag)
      {
        sl_gswap2 (&blkt_head->blkt_type);
        sl_gswap2 (&blkt_head->next_blkt);
      }

      if (blkt_head->blkt_type == 100)
      { /* found a 100 blockette */
        blkt_100 = (struct sl_blkt_100_s *)malloc (sizeof (struct sl_blkt_100_s));
        memcpy ((void *)blkt_100, msrecord + begin_blockette,
                sizeof (struct sl_blkt_100_s));

        if (headerswapflag)
        {
          sl_gswap4 (&blkt_100->sample_rate);
        }

        blkt_100->blkt_type = blkt_head->blkt_type;
        blkt_100->next_blkt = blkt_head->next_blkt;

        msr->Blkt100 = blkt_100;
      }

      if (blkt_head->blkt_type == 1000)

      { /* found the 1000 blockette */
        blkt_1000 =
            (struct sl_blkt_1000_s *)malloc (sizeof (struct sl_blkt_1000_s));
        memcpy ((void *)blkt_1000, msrecord + begin_blockette,
                sizeof (struct sl_blkt_1000_s));

        blkt_1000->blkt_type = blkt_head->blkt_type;
        blkt_1000->next_blkt = blkt_head->next_blkt;

        msr->Blkt1000 = blkt_1000;
      }

      if (blkt_head->blkt_type == 1001)
      { /* found a 1001 blockette */
        blkt_1001 =
            (struct sl_blkt_1001_s *)malloc (sizeof (struct sl_blkt_1001_s));
        memcpy ((void *)blkt_1001, msrecord + begin_blockette,
                sizeof (struct sl_blkt_1001_s));

        blkt_1001->blkt_type = blkt_head->blkt_type;
        blkt_1001->next_blkt = blkt_head->next_blkt;

        msr->Blkt1001 = blkt_1001;
      }

      if (blkt_head->blkt_type == 2000)
      { /* found a 2000 blockette */
        blkt_2000 =
            (struct sl_blkt_2000_s *)malloc (sizeof (struct sl_blkt_2000_s));
        memcpy ((void *)blkt_2000, msrecord + begin_blockette,
                sizeof (struct sl_blkt_2000_s));

        blkt_2000->blkt_type = blkt_head->blkt_type;
        blkt_2000->next_blkt = blkt_head->next_blkt;

        msr->Blkt2000 = blkt_2000;
        msr->Blkt2000Ofs = begin_blockette;
      }

      /* Point to the next blockette */
      begin_blockette = blkt_head->next_blkt;
    } /* End of while looping through blockettes */

    if (blkt_1000 == NULL)
    {
      sl_log_rl (log, 1, 0, "1000 blockette was NOT found for %s.%s.%s.%s!",
                 msr->fsdh.network, msr->fsdh.station,
                 msr->fsdh.location, msr->fsdh.channel);
    }
    else
    {
      /* no byte swapping of data if little-endian host and little-endian data */
      if (sl_littleendianhost () && blkt_1000->word_swap == 0)
        dataswapflag = 0;
      /* no byte swapping of data if big-endian host and big-endian data */
      else if (!sl_littleendianhost () && blkt_1000->word_swap == 1)
        dataswapflag = 0;
    }

    free (blkt_head);
  }

  /* Unpack the data samples if requested */
  if (unpackflag)
  {
    msr->numsamples = sl_msr_unpack (log, msr, dataswapflag);
  }
  else
  {
    msr->numsamples = -1;
  }

  /* Re-direct the original pointer and return the new */
  *ppmsr = msr;
  return msr;
} /* End of sl_msr_parse_size() */

/***************************************************************************
 * sl_msr_print:
 *
 * Prints header values, if 'details' is greater than zero then
 * detailed information (values in the fixed header and following
 * blockettes) is printed.
 *
 * Returns non-zero on success or 0 on error.
 ***************************************************************************/
int
sl_msr_print (SLlog *log, SLMSrecord *msr, int details)
{
  char sourcename[50];
  char stime[25];
  double latency;
  double dsamprate = 0.0;
  int usec;

  /* Build the source name string */
  sl_msr_srcname (msr, sourcename, 0);

  usec = msr->fsdh.start_time.fract * 100;

  if (msr->Blkt1001)
  {
    usec += msr->Blkt1001->usec;

    if (usec > 1000000 || usec < 0)
    {
      sl_log_rl (log, 1, 0, "Cannot apply microsecond offset\n");
      usec -= msr->Blkt1001->usec;
    }
  }

  /* Build a start time string */
  snprintf (stime, sizeof (stime), "%04d,%03d,%02d:%02d:%02d.%06d",
            msr->fsdh.start_time.year, msr->fsdh.start_time.day,
            msr->fsdh.start_time.hour, msr->fsdh.start_time.min,
            msr->fsdh.start_time.sec, usec);

  /* Calculate the latency */
  latency = host_latency (msr);

  /* Report information in the fixed header */
  if (details > 0)
  {
    dsamprate = sl_msr_dnomsamprate (msr);
    sl_log_rl (log, 0, 0, "                 source: %s\n", sourcename);
    sl_log_rl (log, 0, 0, "             start time: %s  (latency ~%1.1f sec)\n",
               stime, latency);
    sl_log_rl (log, 0, 0, "      number of samples: %d\n", msr->fsdh.num_samples);
    sl_log_rl (log, 0, 0, "     sample rate factor: %d\n", msr->fsdh.samprate_fact);
    sl_log_rl (log, 0, 0, " sample rate multiplier: %d  (%.10g samples per second)\n",
               msr->fsdh.samprate_mult, dsamprate);
    sl_log_rl (log, 0, 0, "     num. of blockettes: %d\n",
               msr->fsdh.num_blockettes);
    sl_log_rl (log, 0, 0, "        time correction: %ld\n",
               msr->fsdh.time_correct);
    sl_log_rl (log, 0, 0, "      begin data offset: %d\n", msr->fsdh.begin_data);
    sl_log_rl (log, 0, 0, "  fist blockette offset: %d\n",
               msr->fsdh.begin_blockette);
  }
  else
  {
    sl_msr_dsamprate (msr, &dsamprate);
    sl_log_rl (log, 0, 0, "%s, %d samples, %.10g Hz, %s (latency ~%1.1f sec)\n",
               sourcename, msr->fsdh.num_samples, dsamprate, stime, latency);
  }

  if (details > 0)
  {
    if (msr->Blkt100 != NULL)
    {
      sl_log_rl (log, 0, 0, "          BLOCKETTE 100:\n");
      sl_log_rl (log, 0, 0, "              next blockette: %d\n",
                 msr->Blkt100->next_blkt);
      sl_log_rl (log, 0, 0, "          actual sample rate: %.4f\n",
                 msr->Blkt100->sample_rate);
      sl_log_rl (log, 0, 0, "                       flags: %d\n",
                 msr->Blkt100->flags);
    }

    if (msr->Blkt1000 != NULL)
    {
      int reclen;
      char order[40];
      char encstr[100];

      encoding_hash (msr->Blkt1000->encoding, &encstr[0]);

      /* Calculate record size in bytes as 2^(Blkt1000->rec_len) */
      reclen = (unsigned int)1 << msr->Blkt1000->rec_len;

      /* Big or little endian reported by the 1000 blockette? */
      if (msr->Blkt1000->word_swap == 0)
        strncpy (order, "Little endian (Intel/VAX)", sizeof (order) - 1);
      else if (msr->Blkt1000->word_swap == 1)
        strncpy (order, "Big endian (SPARC/Motorola)", sizeof (order) - 1);
      else
        strncpy (order, "Unknown value", sizeof (order) - 1);

      sl_log_rl (log, 0, 0, "         BLOCKETTE 1000:\n");
      sl_log_rl (log, 0, 0, "              next blockette: %d\n",
                 msr->Blkt1000->next_blkt);
      sl_log_rl (log, 0, 0, "                    encoding: %s\n", encstr);
      sl_log_rl (log, 0, 0, "                  byte order: %s\n", order);
      sl_log_rl (log, 0, 0, "               record length: %d (val:%d)\n",
                 reclen, msr->Blkt1000->rec_len);
      sl_log_rl (log, 0, 0, "                    reserved: %d\n",
                 msr->Blkt1000->reserved);
    }

    if (msr->Blkt1001 != NULL)
    {
      sl_log_rl (log, 0, 0, "         BLOCKETTE 1001:\n");
      sl_log_rl (log, 0, 0, "              next blockette: %d\n",
                 msr->Blkt1001->next_blkt);
      sl_log_rl (log, 0, 0, "              timing quality: %d%%\n",
                 msr->Blkt1001->timing_qual);
      sl_log_rl (log, 0, 0, "                micro second: %d\n",
                 msr->Blkt1001->usec);
      sl_log_rl (log, 0, 0, "                    reserved: %d\n",
                 msr->Blkt1001->reserved);
      sl_log_rl (log, 0, 0, "                 frame count: %d\n",
                 msr->Blkt1001->frame_cnt);
    }
  }

  return 1;
} /* End of sl_msr_print() */

/***************************************************************************
 * sl_ms_srcname:
 *
 * Generate a source name string for a specified raw data record in
 * the format: 'NET_STA_LOC_CHAN' or, if the quality flag is true:
 * 'NET_STA_LOC_CHAN_QUAL'.  The passed srcname must have enough room
 * for the resulting string.
 *
 * Returns a pointer to the resulting string or NULL on error.
 ***************************************************************************/
char *
sl_msr_srcname (SLMSrecord *msr, char *srcname, int8_t quality)
{
  char network[8];
  char station[8];
  char location[8];
  char channel[8];

  if (!msr)
    return NULL;

  sl_strncpclean (network, msr->fsdh.network, 2);
  sl_strncpclean (station, msr->fsdh.station, 5);
  sl_strncpclean (location, msr->fsdh.location, 2);
  sl_strncpclean (channel, msr->fsdh.channel, 3);

  /* Build the source name string including the quality indicator*/
  if (quality)
    sprintf (srcname, "%s_%s_%s_%s_%c",
             network, station, location, channel, msr->fsdh.dhq_indicator);

  /* Build the source name string without the quality indicator*/
  else
    sprintf (srcname, "%s_%s_%s_%s", network, station, location, channel);

  return srcname;
} /* End of sl_msr_srcname() */

/***************************************************************************
 * sl_msr_dsamprate:
 *
 * Calculate a double precision sample rate for the specified SLMSrecord and
 * store it in the passed samprate argument.  If a 100 Blockette was
 * included and parsed, the "Actual sample rate" (Blockette 100, field 3)
 * will be returned, otherwise a nominal sample rate will be calculated
 * from the sample rate factor and multiplier in the fixed section data
 * header.
 *
 * Returns 1 if actual sample rate (from 100 Blockette), 2 if nomial sample
 * rate (from factor and multiplier) or 0 on error.
 ***************************************************************************/
int
sl_msr_dsamprate (SLMSrecord *msr, double *samprate)
{
  if (!msr)
    return 0;

  if (msr->Blkt100)
  {
    *samprate = (double)msr->Blkt100->sample_rate;
    return 1;
  }
  else
  {
    *samprate = sl_msr_dnomsamprate (msr);

    if (*samprate == -1.0)
      return 0;

    return 2;
  }
} /* End of sl_msr_dsamprate() */

/***************************************************************************
 * sl_msr_dnomsamprate:
 *
 * Calculate a double precision nominal sample rate for the specified
 * SLMSrecord from the sample rate factor and multiplier in the fixed
 * section data header.
 *
 * Returns the nominal sample rate or -1.0 on error.
 ***************************************************************************/
double
sl_msr_dnomsamprate (SLMSrecord *msr)
{
  double srcalc = 0.0;
  int factor;
  int multiplier;

  if (!msr)
    return -1.0;

  /* Calculate the nominal sample rate */
  factor     = msr->fsdh.samprate_fact;
  multiplier = msr->fsdh.samprate_mult;

  if (factor > 0)
    srcalc = (double)factor;
  else if (factor < 0)
    srcalc = -1.0 / (double)factor;

  if (multiplier > 0)
    srcalc = srcalc * (double)multiplier;
  else if (multiplier < 0)
    srcalc = -1.0 * (srcalc / (double)multiplier);

  return srcalc;
} /* End of sl_msr_dnomsamprate() */

/***************************************************************************
 * sl_msr_depochstime:
 *
 * Convert a btime struct of a FSDH struct of a SLMSrecord (the record
 * start time) into a double precision (Unix/POSIX) epoch time.
 * Include microsecond offset in blockette 1001 if it was included.
 *
 * Returns double precision epoch time or 0 for error.
 ***************************************************************************/
double
sl_msr_depochstime (SLMSrecord *msr)
{
  struct sl_btime_s *btime;
  double dtime;

  if (!msr)
    return 0;

  btime = &msr->fsdh.start_time;

  dtime = (double)(btime->year - 1970) * 31536000 +
          ((btime->year - 1969) / 4) * 86400 +
          (btime->day - 1) * 86400 +
          btime->hour * 3600 +
          btime->min * 60 +
          btime->sec +
          (double)btime->fract / 10000.0;

  if (msr->Blkt1001)
  {
    dtime += msr->Blkt1001->usec / 1000000;
  }

  return dtime;
} /* End of sl_msr_depochstime() */

/***************************************************************************
 * encoding_hash:
 *
 * Set encstr to a string describing the data frame encoding.
 ***************************************************************************/
void
encoding_hash (char enc, char *encstr)
{
  switch (enc)
  {
  case 0:
    strcpy (encstr, "ASCII text (val:0)");
    break;
  case 1:
    strcpy (encstr, "16 bit integers (val:1)");
    break;
  case 2:
    strcpy (encstr, "24 bit integers (val:2)");
    break;
  case 3:
    strcpy (encstr, "32 bit integers (val:3)");
    break;
  case 4:
    strcpy (encstr, "IEEE floating point (val:4)");
    break;
  case 5:
    strcpy (encstr, "IEEE double precision float (val:5)");
    break;
  case 10:
    strcpy (encstr, "STEIM 1 Compression (val:10)");
    break;
  case 11:
    strcpy (encstr, "STEIM 2 Compression (val:11)");
    break;
  case 12:
    strcpy (encstr, "GEOSCOPE Muxed 24 bit int (val:12)");
    break;
  case 13:
    strcpy (encstr, "GEOSCOPE Muxed 16/3 bit gain/exp (val:13)");
    break;
  case 14:
    strcpy (encstr, "GEOSCOPE Muxed 16/4 bit gain/exp (val:14)");
    break;
  case 15:
    strcpy (encstr, "US National Network compression (val:15)");
    break;
  case 16:
    strcpy (encstr, "CDSN 16 bit gain ranged (val:16)");
    break;
  case 17:
    strcpy (encstr, "Graefenberg 16 bit gain ranged (val:17)");
    break;
  case 18:
    strcpy (encstr, "IPG - Strasbourg 16 bit gain (val:18)");
    break;
  case 19:
    strcpy (encstr, "STEIM 3 Compression (val:19)");
    break;
  case 30:
    strcpy (encstr, "SRO Format (val:30)");
    break;
  case 31:
    strcpy (encstr, "HGLP Format (val:31)");
    break;
  case 32:
    strcpy (encstr, "DWWSSN Gain Ranged Format (val:32)");
    break;
  case 33:
    strcpy (encstr, "RSTN 16 bit gain ranged (val:33)");
    break;
  default:
    sprintf (encstr, "Unknown format code: (%d)", enc);
  } /* end switch */

} /* End of encoding_hash() */

/***************************************************************************
 * host_latency:
 *
 * Calculate the latency based on the system time in UTC accounting for
 * the time covered using the number of samples and sample rate given.
 * Double precision is returned, but the true precision is dependent on
 * the accuracy of the host system clock among other things.
 *
 * Returns seconds of latency.
 ***************************************************************************/
double
host_latency (SLMSrecord *msr)
{
  double dsamprate = 0.0; /* Nominal sampling rate */
  double span      = 0.0; /* Time covered by the samples */
  double epoch;           /* Current epoch time */
  double sepoch;          /* Epoch time of the record start time */
  double latency = 0.0;

  sl_msr_dsamprate (msr, &dsamprate);

  /* Calculate the time covered by the samples */
  if (dsamprate)
    span = (double)msr->fsdh.num_samples * (1.0 / dsamprate);

  /* Grab UTC time according to the system clock */
  epoch = sl_dtime ();

  /* Now calculate the latency */
  sepoch  = sl_msr_depochstime (msr);
  latency = epoch - sepoch - span;

  return latency;
} /* End of host_latency() */
