/************************************************************************/
/*  Field definitions used in SEED data record headers.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
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

/*	$Id: sdr.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__sdr_h
#define __sdr_h

#define	SDR_HDR_SIZE	64		/* SDR hdr size in file.	*/

#define	ACTIVITY_CALIB_PRESENT	0x1
#define	ACTIVITY_TIME_GAP	0x2	/* for backwards compatibility.	*/
#define	ACTIVITY_TIME_CORR_APPLIED	02
#define	ACTIVITY_BEGINNING_OF_EVENT	0x4
#define	ACTIVITY_END_OF_EVENT	0x8
#define	ACTIVITY_POS_LEAP_SECOND    0x10
#define	ACTIVITY_NEG_LEAP_SECOND    0x20
#define	ACTIVITY_EVENT_IN_PROGRESS  0x40

#define	IO_PARITY_ERROR		0x2
#define	IO_LONG_RECORD		0x2
#define	IO_SHORT_RECORD		0x4

#define	QUALITY_SATURATION	0x1
#define	QUALITY_CLIPPING	0x2
#define	QUALITY_SPIKES		0x4
#define	QUALITY_GLITHES		0x8
#define	QUALITY_MISSING		0x10
#define	QUALITY_TELEMETRY_ERROR	0x20
#define	QUALITY_FILTER_CHARGE	0x40
#define	QUALITY_QUESTIONABLE_TIMETAG	0x80

#define	MSHEAR_TIMETAG_FLAG	0x01

typedef	char		SEED_BYTE;	/* signed byte			*/
typedef	unsigned char	SEED_UBYTE;	/* unsigned byte		*/
typedef	short		SEED_WORD;	/* 16 bit signed		*/
typedef	unsigned short	SEED_UWORD;	/* 16 bit unsigned		*/
typedef	int		SEED_LONG;	/* 32 bit signed		*/
typedef	unsigned int	SEED_ULONG;	/* 32 bit unsigned		*/
typedef	char		SEED_CHAR;	/* 7 bit character, high bit 0	*/
typedef unsigned char	SEED_UCHAR;	/* 8 bit character, unsigned.	*/
typedef signed char	SEED_SCHAR;	/* 8 bit character, signed.	*/
typedef	float		SEED_FLOAT;	/* IEEE floating point		*/

#define PACKED __attribute__ ((packed))

typedef struct _sdr_time {
    SEED_UWORD	year;		/* Year					*/
    SEED_UWORD	day;		/* Day of year (1-366)			*/
    SEED_UBYTE	hour;		/* Hour (0-23)				*/
    SEED_UBYTE	minute;		/* Minute (0-59)			*/
    SEED_UBYTE	second;		/* Second (0-60 (leap))			*/
    SEED_UBYTE	pad;		/* Padding				*/
    SEED_UWORD	ticks;		/* 1/10 millisecond (0-9999)		*/
} PACKED SDR_TIME;

#define	SDR_SEQ_LEN		6
#define	SDR_STATION_LEN		5
#define	SDR_LOCATION_LEN	2
#define	SDR_CHANNEL_LEN		3
#define	SDR_NETWORK_LEN		2

/************************************************************************/
/*  SEED Fixed Data Record Header (SDR)					*/
/************************************************************************/
typedef struct _sdr_hdr {			/* byte offset  */
    SEED_CHAR	seq_no[SDR_SEQ_LEN];		/*	0   */
    SEED_CHAR	data_hdr_ind;			/*	6   */
    SEED_CHAR	space_1;			/*	7   */
    SEED_CHAR	station_id[SDR_STATION_LEN];	/*	8   */
    SEED_CHAR	location_id[SDR_LOCATION_LEN];	/*	13  */
    SEED_CHAR	channel_id[SDR_CHANNEL_LEN];	/*	15  */
    SEED_CHAR	network_id[SDR_NETWORK_LEN];	/*	18  */
    SDR_TIME	time;				/*	20  */
    SEED_UWORD	num_samples;			/*	30  */
    SEED_WORD	sample_rate_factor;		/*	32  */
    SEED_WORD	sample_rate_mult;		/*	34  */
    SEED_UBYTE	activity_flags;			/*	36  */
    SEED_UBYTE	io_flags;			/*	37  */
    SEED_UBYTE	data_quality_flags;		/*	38  */
    SEED_UBYTE	num_blockettes;			/*	39  */
    SEED_LONG	num_ticks_correction;		/*	40  */
    SEED_UWORD	first_data;			/*	44  */
    SEED_UWORD	first_blockette;		/*	46  */
} PACKED SDR_HDR;

/************************************************************************/
/*  Blockette Definitions.						*/
/************************************************************************/
typedef struct _blockette_hdr {		/*  Common binary blockette hdr.*/
    SEED_UWORD	type;			/*  binary blockette number.	*/
    SEED_UWORD	next;			/*  byte offset from sdr to next*/
} PACKED BLOCKETTE_HDR;			/*  blockette for this record.	*/

typedef struct _blockette_100 {		/*  Sample Rate Blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	actual_rate;
    SEED_CHAR	flags;
    SEED_CHAR	reserved[3];
} PACKED BLOCKETTE_100;

/*  Quanterra Threshold Detector. Comments refer to Quanterra usage.	*/
typedef struct _blockette_200 {		/*  Generic Event Detection blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	signal_amplitude;	/*  Amp that caused detection.	*/
    SEED_FLOAT	signal_period;		/*  Not used by Quanterra.	*/
    SEED_FLOAT	background_estimate;	/*  Limit that was exceeded.	*/
    SEED_UBYTE	detection_flags;	/*  Not used by Quanterra.	*/
    SEED_UBYTE	reserved;		/*  Not used.			*/
    SDR_TIME	time;			/*  Onset time of detector.	*/
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_CHAR	detector_name[24];	/*  Quanterra detector name.	*/
} PACKED BLOCKETTE_200;
#define	BLOCKETTE_200_STD_SIZE	(sizeof(BLOCKETTE_200)-24)

typedef struct _blockette_201 {		/*  Murdock Event Detection blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	signal_amplitude;	/*  Amplitude of signal (counts)*/
    SEED_FLOAT	signal_period;		/*  Period of signal in seconds.*/
    SEED_FLOAT	background_estimate;	/*  Background estimates (counts)*/
    SEED_UBYTE	detection_flags;	/*  bit 0: 1=dilitational,0=compression */
    SEED_UBYTE	reserved;		/*  Unused - set to 0.		*/
    SDR_TIME	time;			/*  Signal onset time.		*/
    SEED_UBYTE	signal_to_noise[6];	/*  sn ratios - only use first 5*/
    SEED_UBYTE	loopback_value;		/*  Loopback value (0, 1, or 2).*/
    SEED_UBYTE	pick_algorithm;		/*  Pick algorithm - (0 or 1).	*/
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_CHAR	detector_name[24];	/*  Quanterra detector name.	*/
} PACKED BLOCKETTE_201;
#define	BLOCKETTE_201_STD_SIZE	(sizeof(BLOCKETTE_201)-24)

typedef struct _blockette_300 {		/*  Step Calibration blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;
    SEED_UBYTE	num_step_calibrations;
    SEED_UBYTE	calibration_flags;
    SEED_ULONG	step_duration;
    SEED_ULONG	interval_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
} PACKED BLOCKETTE_300;
#define	BLOCKETTE_300_STD_SIZE	(sizeof(BLOCKETTE_300)-8)

typedef struct _blockette_310 {		/*  Sine Calibration Blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_ULONG	calibration_duration;
    SEED_FLOAT	calibration_period;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
} PACKED BLOCKETTE_310;
#define	BLOCKETTE_310_STD_SIZE	(sizeof(BLOCKETTE_310)-8)

typedef struct _blockette_320 {		/*  Pseudo-random Calibration blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_FLOAT	calibration_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
    SEED_CHAR	noise_type[8];
} PACKED BLOCKETTE_320;
#define	BLOCKETTE_320_STD_SIZE	(sizeof(BLOCKETTE_320)-8)

typedef struct _blockette_390 {		/*  Generic Calibration blockette*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_FLOAT	calibration_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
} PACKED BLOCKETTE_390;

typedef struct _blockette_395 {		/*  Calibration Abort blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UWORD	reserved;
} PACKED BLOCKETTE_395;

typedef struct _blockette_400 {		/*  Beam blockette.		*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	azimuth;
    SEED_FLOAT	slowness;
    SEED_UWORD	config;
    SEED_UWORD	reserved;
} PACKED BLOCKETTE_400;

typedef struct _blockette_405 {		/*  Beam Delay blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_UWORD	delay;
} PACKED BLOCKETTE_405;

typedef struct _blockette_500 {		/*  Timing blockette (Quanterra). */
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	vco_correction;		/*  VCO correction.		*/
    SDR_TIME	time;			/*  Time of timing exception.	*/
    SEED_SCHAR	usec99;			/*  time extension to microsec.	*/
    SEED_SCHAR	reception_quality;	/*  Clock Reception quality.	*/
    SEED_LONG	count;			/*  Count (for exception type).	*/
    SEED_CHAR	exception_type[16];	/*  Type of timing exception.	*/
    SEED_CHAR	clock_model[32];	/*  Type of clock in use.	*/
    SEED_CHAR	clock_status[128];	/*  Clock status string.	*/
} PACKED BLOCKETTE_500;

typedef struct _blockette_1000 {	/*  Data format blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_CHAR	format;			/*  data format.		*/
    SEED_CHAR	word_order;		/*  word order.			*/
    SEED_CHAR	data_rec_len;		/*  record length in 2**n.	*/
    SEED_CHAR	reserved;		/*  unused.			*/
} PACKED BLOCKETTE_1000;

typedef struct _blockette_1001 {	/*  Data extention blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_SCHAR	clock_quality;		/*  clock quality.		*/
    SEED_SCHAR	usec99;			/*  time extension to microsec.	*/
    SEED_UCHAR	flags;			/*  flags.			*/
    SEED_SCHAR	frame_count;		/*  # of 64-byte steim frames.	*/
} PACKED BLOCKETTE_1001;

/* Variable length blockettes.						*/
typedef struct _blockette_2000 {	/*  Opaque data blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_WORD	    blockette_len;	/*  total blockete length.	*/
    SEED_WORD	    data_offset;	/*  offset to opaque data.	*/
    SEED_LONG	    record_num;		/*  record number.		*/
    SEED_UCHAR	    word_order;		/*  opaque data word order.	*/
    SEED_UCHAR	    data_flags;		/*  opaque data flags.		*/
    SEED_UCHAR	    num_hdr_strings;	/*  number of opaque hdr strings*/
} PACKED BLOCKETTE_2000;

typedef struct _blockette_unknown {	/*  Unknown blockette.		*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_CHAR	body[128];		/*  body, suitably large.	*/
} PACKED BLOCKETTE_UNKNOWN;

#endif

