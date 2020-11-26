/************************************************************************
 * Routines for decoding INT16, INT32, STEIM1 and STEIM2 encoded data.
 *
 * These are routines extracted from libmseed 2.18 and adapted for use
 * in this code base.
 *
 * modified: 2016.288
 ************************************************************************/

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "libslink.h"

/* Supported SEED data encodings */
#define DE_ASCII 0
#define DE_INT16 1
#define DE_INT32 3
#define DE_FLOAT32 4
#define DE_FLOAT64 5
#define DE_STEIM1 10
#define DE_STEIM2 11

/* Internal decoding routines */
static int decode_int16 (int16_t *input, int samplecount, int32_t *output,
                         int outputlength, int swapflag);
static int decode_int32 (int32_t *input, int samplecount, int32_t *output,
                         int outputlength, int swapflag);
static int decode_steim1 (int32_t *input, int inputlength, int samplecount,
                          int32_t *output, int outputlength, char *srcname,
                          int swapflag, SLlog *log);
static int decode_steim2 (int32_t *input, int inputlength, int samplecount,
                          int32_t *output, int outputlength, char *srcname,
                          int swapflag, SLlog *log);

/* Control for printing debugging information */
static int decodedebug = 0;

/* Extract bit range and shift to start */
#define EXTRACTBITRANGE(VALUE, STARTBIT, LENGTH) ((VALUE & (((1 << LENGTH) - 1) << STARTBIT)) >> STARTBIT)

/************************************************************************
 *  sl_msr_unpack():
 *  Unpack Mini-SEED data for a given SLMSrecord.  The data is accessed
 *  in the record indicated by SLMSrecord->msrecord and the unpacked
 *  samples are placed in SLMSrecord->datasamples.  The resulting data
 *  samples are 32-bit integers in host byte order.
 *
 *  Return number of samples unpacked or -1 on error.
 ************************************************************************/
int
sl_msr_unpack (SLlog *log, SLMSrecord *msr, int swapflag)
{
  const char *dbuf; /* Encoded data buffer */
  char srcname[50]; /* Source name, "Net_Sta_Loc_Chan[_Qual] */
  int blksize;      /* byte size of Mini-SEED record */
  int format;       /* SEED data encoding */
  int datasize;     /* byte size of data samples in record */
  int nsamples;     /* number of samples unpacked */
  int unpacksize;   /* byte size of unpacked samples */
  int i;

  /* Reset the error flag */
  msr->unpackerr = MSD_NOERROR;

  /* Determine data format and blocksize from Blockette 1000 */
  if (msr->Blkt1000 != NULL)
  {
    format = msr->Blkt1000->encoding;
    for (blksize = 1, i = 1; i <= msr->Blkt1000->rec_len; i++)
      blksize *= 2;
  }
  else
  {
    sl_log_rl (log, 2, 0, "msr_unpack(): No Blockette 1000 found!\n");
    return (-1);
  }

  /* Calculate buffer size needed for unpacked samples */
  unpacksize = msr->fsdh.num_samples * sizeof (int32_t);

  /* Allocate space for the unpacked data */
  if (msr->datasamples != NULL)
    msr->datasamples = (int32_t *)malloc (unpacksize);
  else
    msr->datasamples = (int32_t *)realloc (msr->datasamples, unpacksize);

  datasize = blksize - msr->fsdh.begin_data;
  dbuf     = msr->msrecord + msr->fsdh.begin_data;

  /* Decide if this is a format that we can decode. */
  switch (format)
  {

  case DE_STEIM1:
    sl_log_rl (log, 1, 2, "Unpacking Steim1 data frames\n");

    sl_msr_srcname (msr, srcname, 0);

    nsamples = decode_steim1 ((int32_t *)dbuf, datasize, msr->fsdh.num_samples,
                              msr->datasamples, unpacksize, srcname, swapflag, log);

    break;

  case DE_STEIM2:
    sl_log_rl (log, 1, 2, "Unpacking Steim2 data frames\n");

    sl_msr_srcname (msr, srcname, 0);

    nsamples = decode_steim2 ((int32_t *)dbuf, datasize, msr->fsdh.num_samples,
                              msr->datasamples, unpacksize, srcname, swapflag, log);

    break;

  case DE_INT32:
    sl_log_rl (log, 1, 2, "Unpacking INT32 data samples\n");

    nsamples = decode_int32 ((int32_t *)dbuf, msr->fsdh.num_samples,
                             msr->datasamples, unpacksize, swapflag);

    break;

  case DE_INT16:
    sl_log_rl (log, 1, 2, "Unpacking INT16 data samples\n");

    nsamples = decode_int16 ((int16_t *)dbuf, msr->fsdh.num_samples,
                             msr->datasamples, unpacksize, swapflag);

    break;

  default:
    sl_log_rl (log, 2, 0, "Unable to unpack format %d for %.5s.%.2s.%.2s.%.3s\n", format,
               msr->fsdh.station, msr->fsdh.network,
               msr->fsdh.location, msr->fsdh.channel);

    msr->unpackerr = MSD_UNKNOWNFORMAT;
    return (-1);
  }

  if (nsamples > 0 || msr->fsdh.num_samples == 0)
  {
    return (nsamples);
  }

  if (nsamples < 0)
  {
    msr->unpackerr = nsamples;
  }

  return (-1);
} /* End of sl_msr_unpack() */

/************************************************************************
 * decode_int16:
 *
 * Decode 16-bit integer data and place in supplied buffer as 32-bit
 * integers.
 *
 * Return number of samples in output buffer on success, -1 on error.
 ************************************************************************/
static int
decode_int16 (int16_t *input, int samplecount, int32_t *output,
              int outputlength, int swapflag)
{
  int16_t sample;
  int idx;

  if (samplecount <= 0)
    return 0;

  if (!input || !output || outputlength <= 0)
    return -1;

  for (idx = 0; idx < samplecount && outputlength >= (int)sizeof (int32_t); idx++)
  {
    sample = input[idx];

    if (swapflag)
      sl_gswap2a (&sample);

    output[idx] = (int32_t)sample;

    outputlength -= sizeof (int32_t);
  }

  return idx;
} /* End of decode_int16() */

/************************************************************************
 * decode_int32:
 *
 * Decode 32-bit integer data and place in supplied buffer as 32-bit
 * integers.
 *
 * Return number of samples in output buffer on success, -1 on error.
 ************************************************************************/
static int
decode_int32 (int32_t *input, int samplecount, int32_t *output,
              int outputlength, int swapflag)
{
  int32_t sample;
  int idx;

  if (samplecount <= 0)
    return 0;

  if (!input || !output || outputlength <= 0)
    return -1;

  for (idx = 0; idx < samplecount && outputlength >= (int)sizeof (int32_t); idx++)
  {
    sample = input[idx];

    if (swapflag)
      sl_gswap4a (&sample);

    output[idx] = sample;

    outputlength -= sizeof (int32_t);
  }

  return idx;
} /* End of decode_int32() */

/************************************************************************
 * decode_steim1:
 *
 * Decode Steim1 encoded miniSEED data and place in supplied buffer
 * as 32-bit integers.
 *
 * Return number of samples in output buffer on success, -1 on error.
 ************************************************************************/
static int
decode_steim1 (int32_t *input, int inputlength, int samplecount,
               int32_t *output, int outputlength, char *srcname,
               int swapflag, SLlog *log)
{
  int32_t *outputptr = output; /* Pointer to next output sample location */
  uint32_t frame[16];          /* Frame, 16 x 32-bit quantities = 64 bytes */
  int32_t X0    = 0;           /* Forward integration constant, aka first sample */
  int32_t Xn    = 0;           /* Reverse integration constant, aka last sample */
  int maxframes = inputlength / 64;
  int frameidx;
  int startnibble;
  int nibble;
  int widx;
  int diffcount;
  int idx;

  union dword {
    int8_t d8[4];
    int16_t d16[2];
    int32_t d32;
  } SLP_PACKED *word;

  if (inputlength <= 0)
    return 0;

  if (!input || !output || outputlength <= 0 || maxframes <= 0)
    return -1;

  if (decodedebug)
    sl_log_rl (log, 1, 0, "Decoding %d Steim1 frames, swapflag: %d, srcname: %s\n",
               maxframes, swapflag, (srcname) ? srcname : "");

  for (frameidx = 0; frameidx < maxframes && samplecount > 0; frameidx++)
  {
    /* Copy frame, each is 16x32-bit quantities = 64 bytes */
    memcpy (frame, input + (16 * frameidx), 64);

    /* Save forward integration constant (X0) and reverse integration constant (Xn)
       and set the starting nibble index depending on frame. */
    if (frameidx == 0)
    {
      if (swapflag)
      {
        sl_gswap4a (&frame[1]);
        sl_gswap4a (&frame[2]);
      }

      X0 = frame[1];
      Xn = frame[2];

      startnibble = 3; /* First frame: skip nibbles, X0, and Xn */

      if (decodedebug)
        sl_log_rl (log, 1, 0, "Frame %d: X0=%d  Xn=%d\n", frameidx, X0, Xn);
    }
    else
    {
      startnibble = 1; /* Subsequent frames: skip nibbles */

      if (decodedebug)
        sl_log_rl (log, 1, 0, "Frame %d\n", frameidx);
    }

    /* Swap 32-bit word containing the nibbles */
    if (swapflag)
      sl_gswap4a (&frame[0]);

    /* Decode each 32-bit word according to nibble */
    for (widx = startnibble; widx < 16 && samplecount > 0; widx++)
    {
      /* W0: the first 32-bit contains 16 x 2-bit nibbles for each word */
      nibble = EXTRACTBITRANGE (frame[0], (30 - (2 * widx)), 2);

      word      = (union dword *)&frame[widx];
      diffcount = 0;

      switch (nibble)
      {
      case 0: /* 00: Special flag, no differences */
        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 00=special\n", widx);
        break;

      case 1: /* 01: Four 1-byte differences */
        diffcount = 4;

        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 01=4x8b  %d  %d  %d  %d\n",
                     widx, word->d8[0], word->d8[1], word->d8[2], word->d8[3]);
        break;

      case 2: /* 10: Two 2-byte differences */
        diffcount = 2;

        if (swapflag)
        {
          sl_gswap2a (&word->d16[0]);
          sl_gswap2a (&word->d16[1]);
        }

        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 10=2x16b  %d  %d\n", widx, word->d16[0], word->d16[1]);
        break;

      case 3: /* 11: One 4-byte difference */
        diffcount = 1;
        if (swapflag)
          sl_gswap4a (&word->d32);

        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 11=1x32b  %d\n", widx, word->d32);
        break;
      } /* Done with decoding 32-bit word based on nibble */

      /* Apply accumulated differences to calculate output samples */
      if (diffcount > 0)
      {
        for (idx = 0; idx < diffcount && samplecount > 0; idx++, outputptr++)
        {
          if (outputptr == output) /* Ignore first difference, instead store X0 */
            *outputptr = X0;
          else if (diffcount == 4) /* Otherwise store difference from previous sample */
            *outputptr = *(outputptr - 1) + word->d8[idx];
          else if (diffcount == 2)
            *outputptr = *(outputptr - 1) + word->d16[idx];
          else if (diffcount == 1)
            *outputptr = *(outputptr - 1) + word->d32;

          samplecount--;
        }
      }
    } /* Done looping over nibbles and 32-bit words */
  }   /* Done looping over frames */

  /* Check data integrity by comparing last sample to Xn (reverse integration constant) */
  if (outputptr != output && *(outputptr - 1) != Xn)
  {
    sl_log_rl (log, 1, 0, "%s: Warning: Data integrity check for Steim1 failed, Last sample=%d, Xn=%d\n",
               srcname, *(outputptr - 1), Xn);
  }

  return (outputptr - output);
} /* End of decode_steim1() */

/************************************************************************
 * decode_steim2:
 *
 * Decode Steim2 encoded miniSEED data and place in supplied buffer
 * as 32-bit integers.
 *
 * Return number of samples in output buffer on success, -1 on error.
 ************************************************************************/
static int
decode_steim2 (int32_t *input, int inputlength, int samplecount,
               int32_t *output, int outputlength, char *srcname,
               int swapflag, SLlog *log)
{
  int32_t *outputptr = output; /* Pointer to next output sample location */
  uint32_t frame[16];          /* Frame, 16 x 32-bit quantities = 64 bytes */
  int32_t X0 = 0;              /* Forward integration constant, aka first sample */
  int32_t Xn = 0;              /* Reverse integration constant, aka last sample */
  int32_t diff[7];
  int32_t semask;
  int maxframes = inputlength / 64;
  int frameidx;
  int startnibble;
  int nibble;
  int widx;
  int diffcount;
  int dnib;
  int idx;

  union dword {
    int8_t d8[4];
    int32_t d32;
  } SLP_PACKED *word;

  if (inputlength <= 0)
    return 0;

  if (!input || !output || outputlength <= 0 || maxframes <= 0)
    return -1;

  if (decodedebug)
    sl_log_rl (log, 1, 0, "Decoding %d Steim2 frames, swapflag: %d, srcname: %s\n",
               maxframes, swapflag, (srcname) ? srcname : "");

  for (frameidx = 0; frameidx < maxframes && samplecount > 0; frameidx++)
  {
    /* Copy frame, each is 16x32-bit quantities = 64 bytes */
    memcpy (frame, input + (16 * frameidx), 64);

    /* Save forward integration constant (X0) and reverse integration constant (Xn)
       and set the starting nibble index depending on frame. */
    if (frameidx == 0)
    {
      if (swapflag)
      {
        sl_gswap4a (&frame[1]);
        sl_gswap4a (&frame[2]);
      }

      X0 = frame[1];
      Xn = frame[2];

      startnibble = 3; /* First frame: skip nibbles, X0, and Xn */

      if (decodedebug)
        sl_log_rl (log, 1, 0, "Frame %d: X0=%d  Xn=%d\n", frameidx, X0, Xn);
    }
    else
    {
      startnibble = 1; /* Subsequent frames: skip nibbles */

      if (decodedebug)
        sl_log_rl (log, 1, 0, "Frame %d\n", frameidx);
    }

    /* Swap 32-bit word containing the nibbles */
    if (swapflag)
      sl_gswap4a (&frame[0]);

    /* Decode each 32-bit word according to nibble */
    for (widx = startnibble; widx < 16 && samplecount > 0; widx++)
    {
      /* W0: the first 32-bit quantity contains 16 x 2-bit nibbles */
      nibble    = EXTRACTBITRANGE (frame[0], (30 - (2 * widx)), 2);
      diffcount = 0;

      switch (nibble)
      {
      case 0: /* nibble=00: Special flag, no differences */
        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 00=special\n", widx);

        break;
      case 1: /* nibble=01: Four 1-byte differences */
        diffcount = 4;

        word = (union dword *)&frame[widx];
        for (idx = 0; idx < diffcount; idx++)
        {
          diff[idx] = word->d8[idx];
        }

        if (decodedebug)
          sl_log_rl (log, 1, 0, "  W%02d: 01=4x8b  %d  %d  %d  %d\n", widx, diff[0], diff[1], diff[2], diff[3]);
        break;

      case 2: /* nibble=10: Must consult dnib, the high order two bits */
        if (swapflag)
          sl_gswap4a (&frame[widx]);
        dnib = EXTRACTBITRANGE (frame[widx], 30, 2);

        switch (dnib)
        {
        case 0: /* nibble=10, dnib=00: Error, undefined value */
          sl_log_rl (log, 2, 0, "%s: Impossible Steim2 dnib=00 for nibble=10\n", srcname);

          return -1;
          break;

        case 1: /* nibble=10, dnib=01: One 30-bit difference */
          diffcount = 1;
          semask    = 1U << (30 - 1); /* Sign extension from bit 30 */
          diff[0]   = EXTRACTBITRANGE (frame[widx], 0, 30);
          diff[0]   = (diff[0] ^ semask) - semask;

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 10,01=1x30b  %d\n", widx, diff[0]);
          break;

        case 2: /* nibble=10, dnib=10: Two 15-bit differences */
          diffcount = 2;
          semask    = 1U << (15 - 1); /* Sign extension from bit 15 */
          for (idx = 0; idx < diffcount; idx++)
          {
            diff[idx] = EXTRACTBITRANGE (frame[widx], (15 - idx * 15), 15);
            diff[idx] = (diff[idx] ^ semask) - semask;
          }

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 10,10=2x15b  %d  %d\n", widx, diff[0], diff[1]);
          break;

        case 3: /* nibble=10, dnib=11: Three 10-bit differences */
          diffcount = 3;
          semask    = 1U << (10 - 1); /* Sign extension from bit 10 */
          for (idx = 0; idx < diffcount; idx++)
          {
            diff[idx] = EXTRACTBITRANGE (frame[widx], (20 - idx * 10), 10);
            diff[idx] = (diff[idx] ^ semask) - semask;
          }

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 10,11=3x10b  %d  %d  %d\n", widx, diff[0], diff[1], diff[2]);
          break;
        }

        break;

      case 3: /* nibble=11: Must consult dnib, the high order two bits */
        if (swapflag)
          sl_gswap4a (&frame[widx]);
        dnib = EXTRACTBITRANGE (frame[widx], 30, 2);

        switch (dnib)
        {
        case 0: /* nibble=11, dnib=00: Five 6-bit differences */
          diffcount = 5;
          semask    = 1U << (6 - 1); /* Sign extension from bit 6 */
          for (idx = 0; idx < diffcount; idx++)
          {
            diff[idx] = EXTRACTBITRANGE (frame[widx], (24 - idx * 6), 6);
            diff[idx] = (diff[idx] ^ semask) - semask;
          }

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 11,00=5x6b  %d  %d  %d  %d  %d\n",
                       widx, diff[0], diff[1], diff[2], diff[3], diff[4]);
          break;

        case 1: /* nibble=11, dnib=01: Six 5-bit differences */
          diffcount = 6;
          semask    = 1U << (5 - 1); /* Sign extension from bit 5 */
          for (idx = 0; idx < diffcount; idx++)
          {
            diff[idx] = EXTRACTBITRANGE (frame[widx], (25 - idx * 5), 5);
            diff[idx] = (diff[idx] ^ semask) - semask;
          }

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 11,01=6x5b  %d  %d  %d  %d  %d  %d\n",
                       widx, diff[0], diff[1], diff[2], diff[3], diff[4], diff[5]);
          break;

        case 2: /* nibble=11, dnib=10: Seven 4-bit differences */
          diffcount = 7;
          semask    = 1U << (4 - 1); /* Sign extension from bit 4 */
          for (idx = 0; idx < diffcount; idx++)
          {
            diff[idx] = EXTRACTBITRANGE (frame[widx], (24 - idx * 4), 4);
            diff[idx] = (diff[idx] ^ semask) - semask;
          }

          if (decodedebug)
            sl_log_rl (log, 1, 0, "  W%02d: 11,10=7x4b  %d  %d  %d  %d  %d  %d  %d\n",
                       widx, diff[0], diff[1], diff[2], diff[3], diff[4], diff[5], diff[6]);
          break;

        case 3: /* nibble=11, dnib=11: Error, undefined value */
          sl_log_rl (log, 2, 0, "%s: Impossible Steim2 dnib=11 for nibble=11\n", srcname);

          return -1;
          break;
        }

        break;
      } /* Done with decoding 32-bit word based on nibble */

      /* Apply differences to calculate output samples */
      if (diffcount > 0)
      {
        for (idx = 0; idx < diffcount && samplecount > 0; idx++, outputptr++)
        {
          if (outputptr == output) /* Ignore first difference, instead store X0 */
            *outputptr = X0;
          else /* Otherwise store difference from previous sample */
            *outputptr = *(outputptr - 1) + diff[idx];

          samplecount--;
        }
      }
    } /* Done looping over nibbles and 32-bit words */
  }   /* Done looping over frames */

  /* Check data integrity by comparing last sample to Xn (reverse integration constant) */
  if (outputptr != output && *(outputptr - 1) != Xn)
  {
    sl_log_rl (log, 1, 0, "%s: Warning: Data integrity check for Steim2 failed, Last sample=%d, Xn=%d\n",
               srcname, *(outputptr - 1), Xn);
  }

  return (outputptr - output);
} /* End of decode_steim2() */
