/*   Lib330 Seed Definitions
     Copyright 2006 Certified Software Corporation

    This file is part of Lib330

    Lib330 is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Lib330 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lib330; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Edit History:
   Ed Date       By  Changes
   -- ---------- --- ---------------------------------------------------
    0 2006-09-10 rdr Created
    1 2007-01-08 hjs prefaced some functions with lib330 to avoid collisions
    2 2011-03-17 rdr Add new deb_flags definitions.
*/
#ifndef libseed_h
/* Flag this file as included */
#define libseed_h
#define VER_LIBSEED 3

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
/* Make sure q330types.h is included */
#ifndef q330types_h
#include "q330types.h"
#endif

/* NOTE: Don't use sizeof for these values, we care about the actual SEED spec, not host sizes */
#define MAXSAMPPERWORD 7 /* maximum samples per 32-bit word */

#ifndef OMIT_SEED
#define WORDS_PER_FRAME 16 /* flag word and 15 actual entries */
#define FRAME_SIZE (WORDS_PER_FRAME * 4) /* 64 bytes per frame */
#define FRAMES_PER_RECORD 8 /* 512 byte base record */
#define LIB_REC_SIZE (FRAME_SIZE * FRAMES_PER_RECORD) /* normal miniseed is 512 byte record */
#define MAX_FRAMES_PER_RECORD 256 /* 16384 record */
#define MAX_REC_SIZE (FRAME_SIZE * MAX_FRAMES_PER_RECORD) /* largest that archive can handle */
#define DATA_OVERHEAD 64 /* data starts at second frame */
#define DATA_AREA (LIB_REC_SIZE - DATA_OVERHEAD) /* data area in normal miniseed */
#define NONDATA_OVERHEAD 56 /* can't use these bytes in record */
#define NONDATA_AREA (LIB_REC_SIZE - NONDATA_OVERHEAD) /* blockette/message area in normal miniseed */
#define OPAQUE_HDR_SIZE 20 /* for all 5 characters */
#define TIMING_BLOCKETTE_SIZE 200
#define RECORD_EXP 9
#define DEB_LOWV 4 /* In low range */
#define DEB_Q330 6 /* Data originates from a Q330 */
#define DEB_NEWBITS 0x20 /* New bit meanings if set */
#define DEB_PB14_FIX 0x40 /* fixed leap day handling */
#define DEB_EVENT_ONLY 0x80 /* event only channel */
/* Seed activity Flags */
#define SAF_BEGIN_EVENT 0x4
#define SAF_END_EVENT 0x8
#define SAF_LEAP_FORW 0x10
#define SAF_LEAP_REV 0x20
#define SAF_EVENT_IN_PROGRESS 0x40
/* Seed quality flags */
#define SQF_AMPERR 0x1
#define SQF_MISSING_DATA 0x10
#define SQF_CHARGING 0x40
/* LOG_CFG Flags */
#define LC_START 1 /* Write configuration at start of session */
#define LC_END 2 /* Write configuration at end of session */
#define LC_INT 4 /* Write configuration at least interval minutes */
#endif

/* Seed flags also used by one second data */
#define SAF_CAL_IN_PROGRESS 0x1
#define SQF_QUESTIONABLE_TIMETAG 0x80
#define SIF_LOCKED 0x20

/* NOTE: The original paxcal definitions were 1 .. n instead of 0 .. n-1 */
typedef char tseed_name[3] ;
typedef char tseed_net[2] ;
typedef char tseed_stn[5] ;
typedef char tlocation[2] ;
typedef word tseeda12[6] ;

#ifndef OMIT_SEED
typedef longword compressed_frame[WORDS_PER_FRAME] ; /* 1 frame = 64 bytes */
typedef compressed_frame compressed_frame_array[FRAMES_PER_RECORD] ;
typedef compressed_frame max_compressed_frame_array[MAX_FRAMES_PER_RECORD] ;
typedef max_compressed_frame_array *pmax_cfr ;
typedef byte completed_record[LIB_REC_SIZE] ; /* ready for storage */
typedef completed_record *pcompleted_record ;
typedef struct { /* minimum required fields for all blockettes */
  word blockette_type ;
  word next_blockette ; /*offset to next blockette*/
} blk_min ;
typedef struct { /* Data Only Blockette */
  word blockette_type ;
  word next_blockette ; /*offset to next blockette*/
  byte encoding_format ; /*11 = Steim2, 0 = none*/
  byte word_order ; /*1 always*/
  byte rec_length ; /*8=256, 9=512, 12=4096*/
  byte dob_reserved ; /*0 always*/
} data_only_blockette ;
typedef struct { /* Data Extension Blockette */
  word blockette_type ;
  word next_blockette ; /*offset to next blockette*/
  byte qual ; /*0 to 100 quality indicator*/
  byte usec99 ; /*0 - 99 microseconds*/
  byte deb_flags ; /* DEB_XXXX */
  byte frame_count ; /* number of 64 byte data frames */
} data_extension_blockette ;
typedef struct {
  union {
    struct {
      word yr ;
      word jday ;
      byte hr ;
      byte minute ;
      byte seconds ;
      byte unused ;
      word tenth_millisec ;
    } STGREG ;
    double fpt ;
  } STUNION ;
} tseed_time ;
#define seed_yr STUNION.STGREG.yr
#define seed_jday STUNION.STGREG.jday
#define seed_hr STUNION.STGREG.hr
#define seed_minute STUNION.STGREG.minute
#define seed_seconds STUNION.STGREG.seconds
#define seed_unused STUNION.STGREG.unused
#define seed_tenth_millisec STUNION.STGREG.tenth_millisec
#define seed_fpt STUNION.fpt

typedef struct {
  union {
    char ch[6] ;
    longword num ;
  } SSUNION ;
} tseed_sequence ;
#define seed_ch SSUNION.ch
#define seed_num SSUNION.num

typedef struct {
  tseed_sequence sequence ;                   /* record number */
  char seed_record_type ;                     /* D for data record */
  char continuation_record ;                  /* space normally */
  tseed_stn station_id_call_letters ;         /* last char must be space */
  tlocation location_id ;                     /* non aligned! */
  tseed_name channel_id ;                     /* non aligned! */
  tseed_net seednet ;                         /* space filled */
  tseed_time starting_time ;
  word samples_in_record ;
  int16 sample_rate_factor ;
  int16 sample_rate_multiplier ;              /* always 1 */
  byte activity_flags ;                       /* ?I?LEBTC */
  byte io_flags ;                             /* ??L????? */
  byte data_quality_flags ;                   /* ???G???? */
  byte number_of_following_blockettes ;       /* normally 2 for data */
  longint tenth_msec_correction ;             /* always 0 */
  word first_data_byte ;                      /* 0, 56, or 64 - data starts in frame 1 */
  word first_blockette_byte ;                 /* 48 */
  data_only_blockette dob ;
  data_extension_blockette deb ;              /* used for data only */
} seed_header ;
typedef struct {
  single signal_amplitude ;
  single signal_period ;
  single background_estimate ;
  byte event_detection_flags ;                /* 0/1 for MH */
  byte reserved_byte ;                        /* 0 */
  tseed_time signal_onset_time ;
} tonset_base ;
typedef struct {
  single signal_amplitude ;
  single signal_period ;
  single background_estimate ;
  byte event_detection_flags ;               /* 0/1 for MH */
  byte reserved_byte ;                       /* 0 */
  tseed_time signal_onset_time ;
  byte snr[6] ;                              /* only first 5 used */
  byte lookback_value ;                      /* 0, 1, or 2 */
  byte pick_algorithm ;                      /* 0 or 1 */
} tonset_mh ;
typedef struct {            /* 201 */
  word blockette_type ;
  word next_blockette ;     /*offset to next blockette*/
  /* Extensions */
  tonset_mh mh_onset ;
  char s_detname[24] ;
} murdock_detect ;
typedef struct {            /* 200 */
  word blockette_type ;
  word next_blockette ;
  tonset_base thr_onset ;
  /* Extensions */
  char s_detname[24] ;
} threshold_detect ;
typedef struct {                          /* 500 */
  word blockette_type ;
  word next_blockette ;
  single vco_correction ;                 /* 0 to 100% of control value */
  tseed_time time_of_exception ;
  byte usec99 ;                           /* -50 to +99 usec correction */
  byte reception_quality ;                /* 0 to 100% */
  longint exception_count ;               /* exception specific count */
  char exception_type[16] ;               /* description of exception */
  char clock_model[32] ;                  /* type of clock */
  char clock_status[128] ;                /* condition of clock */
} timing ;
typedef struct {
  single calibration_amplitude ;          /* volts or amps, depending on LCQ */
  tseed_name calibration_input_channel ;  /* monitor channel, if any */
  byte cal2_res ;                         /* zero */
  single ref_amp ;                        /* reference amplitude */
  char coupling[12] ;                     /* coupling method to seismo */
  char rolloff[12] ;                      /* type of filtering used */
} cal2 ;
typedef cal2 *pcal2 ;
typedef struct {
  word blockette_type ;
  word next_blockette ;
  tseed_time calibration_time ;              /* start or stop */
  byte number_of_steps ;                     /* 1 */
  byte calibration_flags ;                   /* bit 0 = +, bit 2 = automatic */
  longword calibration_duration ;            /* 0.0001 seconds / count */
  longword interval_duration ;               /* 0 */
  cal2 step2 ;
} step_calibration ;
typedef struct {
  word blockette_type ;
  word next_blockette ;
  tseed_time calibration_time ;              /* start or stop */
  byte res ;                                 /* 0 */
  byte calibration_flags ;                   /* bit 2 = automatic, bit 4 set */
  longword calibration_duration ;            /* 0.0001 seconds / count */
  single sine_period ;                       /* in seconds */
  cal2 sine2 ;
} sine_calibration ;
typedef struct {
  word blockette_type ;
  word next_blockette ;
  tseed_time calibration_time ;              /* start or stop */
  byte res ;                                 /* 0 */
  byte calibration_flags ;                   /* bit 2 = automatic, bit 4 = ? */
  longword calibration_duration ;            /* 0.0001 seconds / count*/
  cal2 random2 ;
  char noise_type[8] ;                       /* frequency distribution */
} random_calibration ;
typedef struct {
  word blockette_type ;
  word next_blockette ;
  tseed_time calibration_time ;              /* start or stop */
  word res ;                                 /* 0 */
} abort_calibration ;
typedef struct {                             /* first part of every opaque blockette */
  word blockette_type ;
  word next_blockette ;
  word blk_lth ;                             /* Total blockette length in bytes */
  word data_off ;                            /* Byte offset to start of actual data */
  longword recnum ;                          /* Record number */
  byte word_order ;                          /* 1 = big endian */
  byte opaque_flags ;                        /* orientation, packaging, etc. */
  byte hdr_fields ;                          /* number of header fields */
  char rec_type[5] ;                         /* 4 character plus ~ terminator (most use only two characters) */
} topaque_hdr ;
#endif

extern char *seed2string(tlocation *loc, tseed_name *sn, pchar result) ;
extern void string2fixed(pointer p, pchar s) ;

#ifndef OMIT_SEED
extern void fix_seed_header (seed_header *hdr, tsystemtime *greg,
                             longint usec, boolean setdeb) ;
extern void lib330_seed_time (tseed_time *st, tsystemtime *greg, longint usec) ;
extern void storeseedhdr (pbyte *pdest, seed_header *hdr, boolean hasdeb) ;
extern void storemurdock (pbyte *pdest, murdock_detect *mdet) ;
extern void storethreshold (pbyte *pdest, threshold_detect *tdet) ;
extern void storetiming (pbyte *pdest, timing *tim) ;
extern void storestep (pbyte *pdest, step_calibration *stepcal) ;
extern void storesine (pbyte *pdest, sine_calibration *sinecal) ;
extern void storerandom (pbyte *pdest, random_calibration *randcal) ;
extern void storeabort (pbyte *pdest, abort_calibration *abortcal) ;
extern void storeopaque (pbyte *pdest, topaque_hdr *ophdr, integer rectypelth,
                         pointer pbuf, integer psize) ;
extern void storeframe (pbyte *pdest, compressed_frame *cf) ;
extern void loadblkhdr (pbyte *p, blk_min *blk) ;
extern void loadtime (pbyte *p, tseed_time *seedtime) ;
extern void loadseedhdr (pbyte *psrc, seed_header *hdr, boolean hasdeb) ;
extern void convert_time (double fp, tsystemtime *greg, longint *usec) ;
extern double extract_time (tseed_time *st, byte usec) ;
extern void loadtiming (pbyte *psrc, timing *tim) ;
extern void loadmurdock (pbyte *psrc, murdock_detect *mdet) ;
extern void loadthreshold (pbyte *psrc, threshold_detect *tdet) ;
extern void loadstep (pbyte *psrc, step_calibration *stepcal) ;
extern void loadsine (pbyte *psrc, sine_calibration *sinecal) ;
extern void loadrandom (pbyte *psrc, random_calibration *randcal) ;
extern void loadabort (pbyte *psrc, abort_calibration *abortcal) ;
extern void loadopaquehdr (pbyte *psrc, topaque_hdr *ophdr) ;
#endif

#endif
