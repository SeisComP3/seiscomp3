/*   Lib330 time series handling definitions
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
    0 2006-09-30 rdr Created
    1 2006-11-28 rdr Remove "last_valid" from com sructure.
    2 2006-12-30 rdr Add cfg_timer.
    3 2007-03-12 rdr Add first_data flag to indicate that continuity needs to be purged at the
                     first second of incoming data.
    4 2008-08-02 rdr Add LO_NSEVT, and SCD_xxxx, scd_evt, and scd_cont.
    5 2008-03-19 rdr Add gap_offset.
*/
#ifndef libsampglob_h
/* Flag this file as included */
#define libsampglob_h
#define VER_LIBSAMPGLOB 5

#ifndef libtypes_h
#include "libtypes.h"
#endif
#ifndef q330types_h
#include "q330types.h"
#endif
#ifndef libseed_h
#include "libseed.h"
#endif
#ifndef libclient_h
#include "libclient.h"
#endif
#ifndef libslider_h
#include "libslider.h"
#endif

/* Detector Options */
#define DO_RUN 1 /* Detector Runs by default */
#define DO_LOG 2 /* Logging enabled */
#define DO_MSG 8 /* put in message log */
/* LCQ Option Bits */
#define LO_EVENT 1 /* Event */
#define LO_DETP 2 /* Write Detector Packets */
#define LO_CALP 4 /* Write Calibration Packets */
#define LO_PEB 8 /* Pre Event Buffers */
#define LO_GAP 0x10 /* Gap Override */
#define LO_CALDLY 0x20 /* Calibration Delay */
#define LO_FRAME 0x40 /* Frame Count */
#define LO_FIRMULT 0x80 /* FIR Filter Multiplier */
#define LO_AVG 0x100 /* Averaging Parameters */
#define LO_CDET 0x200 /* Control Detector */
#define LO_DEC 0x400 /* Decimation */
#define LO_NOUT 0x800 /* No Output */
#define LO_DET1 0x1000 /* Detector 1 */
#define LO_DET2 0x2000 /* Detector 2 */
#define LO_DET3 0x4000 /* Detector 3 */
#define LO_DET4 0x8000 /* Detector 4 */
#define LO_DET5 0x10000 /* Detector 5 */
#define LO_DET6 0x20000 /* Detector 6 */
#define LO_DET7 0x40000 /* Detector 7 */
#define LO_DET8 0x80000 /* Detector 8 */
#define LO_NSEVT 0x8000000 /* Netserv is event only */
#define LO_DOFF 0x10000000 /* Don't generate data */
#define LO_DATAS 0x20000000 /* Enable writing to dataserv */
#define LO_NETS 0x40000000 /* Enable writing to netserv */
#define LO_CNPP 0x80000000 /* Preserve CNP timetags */
/* Detector Equation MS 2 bits */
#define DES_COMM 0x00 /* Comm Event */
#define DES_DET 0x40 /* Murdock-Hutt or Threshold Detector, bits 0-5 are detector number */
#define DES_CAL 0x80 /* Calibration On, Bits 0-5 are LCQ number */
#define DES_OP 0xC0 /* Logical Operator, Bits 0-5 are encoded as DEO_xxx */
/* Detector Operators */
#define DEO_LPAR 0 /* Left Paren */
#define DEO_RPAR 1 /* Right Paren */
#define DEO_NOT 2 /* Not */
#define DEO_AND 3 /* And */
#define DEO_OR 4 /* Or */
#define DEO_EOR 5 /* Exclusive Or */
#define DEO_DONE 63 /* done with detector */
/* Data received values */
#define DR_NEVER 0 /* no data received */
#define DR_HAS 1 /* Has received */
#define DR_ACTIVE 2 /* recently */
/* Send to Client destination bitmaps */
#define SCD_ARCH 1 /* send to archival output */
#define SCD_512 2 /* send to 512 byte miniseed output */
#define SCD_BOTH 3 /* send to both */

#ifndef OMIT_SEED
#define MAXSAMP 38
#define FIRMAXSIZE 400
#define MAXPOLES 8       /* Maximum number of poles in recursive filters */
#define MAXSECTIONS 4    /* Maximum number of sections in recursive filters */
#define FILTER_NAME_LENGTH 31 /* Maximum number of characters in an IIR filter name */
#define PEEKELEMS 16
#define PEEKMASK 15 /* TP7 doesn't optimize mod operation */
#define CFG_TIMEOUT 120 /* seconds since last config blockette added before flush */
#endif

#define SS_100 448 /* bytes needed for 100hz segment buffer */
#define SS_200 888 /* bytes needed for 200hz segment buffer */
#define NO_LAST_DATA_QUAL 999 /* initial value */
#define MHOLDQSZ 500 /* Queue for messages prior to lcq allocation */

#ifndef OMIT_SEED
enum tevent_detector {MURDOCK_HUTT, THRESHOLD} ;
/*
  tiirdef is a definition of an IIR filter which may be used multiple places
*/
typedef double tvector[MAXPOLES + 1] ;
typedef struct {
  byte poles ;
  boolean highpass ;
  byte spare ;
  single ratio ; /* ratio * sampling_frequency = corner */
  tvector a ;
  tvector b ;
} tsection_base ;
typedef struct tiirdef {
  struct tiirdef *link ;
  byte sects ;
  byte iir_num ; /* filter number */
  single gain ; /* filter gain */
  single rate ; /* reference frequency */
  char fname[FILTER_NAME_LENGTH] ;
  tsection_base filt[MAXSECTIONS + 1] ;
} tiirdef ;
typedef tiirdef *piirdef ;
/*
  tiirfilter is one implementation of a filter on a specific LCQ
*/
typedef struct {
  byte poles ;
  boolean highpass ;
  byte spare ;
  single ratio ; /* ratio * sampling_frequency = corner */
  tvector a ;
  tvector b ;
  tvector x ;
  tvector y ;
} tiirsection ;
typedef struct tiirfilter {
  struct tiirfilter *link ; /* next filter */
  piirdef def ; /* definition of this filter */
  integer sects ;
  word packet_size ; /* total size of this packet */
  tiirsection filt[MAXSECTIONS + 1] ;
  tfloat out ; /*may be an array*/
} tiirfilter ;
typedef tiirfilter *piirfilter ;
/*
  Tfir_packet is the actual implementation of one FIR filter on a particular LCQ
*/
typedef tfloat *pfloat ;
typedef struct {
  pfloat fbuf ; /* pointer to FIR filter buffer */
  pfloat f ; /* working ptr into FIR buffer */
  pfloat fcoef ; /* ptr to floating pnt FIR coefficients */
  longint flen ; /* number of coef in FIR filter */
  longint fdec ; /* number of FIR inp samps per output samp */
  longint fcount ; /* current number of samps in FIR buffer */
} tfir_packet ;
typedef tfir_packet *pfir_packet ;
/*
  Tavg_packet is only used if averaging reports are requested on an LCQ
*/
typedef struct {
  tfloat running_avg ;
  tfloat signed_sum ;
  tfloat sqr_sum ;
  tfloat peak_abs ;
  longword avg_count ;
} tavg_packet ;
typedef tavg_packet *pavg_packet ;
/*
  tdetload defines operating constants for murdock-hutt and threshold detectors
*/
typedef struct {
  longint filhi, fillo ; /* threshold limits */
  longint iwin ; /* window length in samples & threshold hysterisis */
  longint n_hits ; /* #P-T >= th2 for detection & threshold min. dur. */
  longint xth1, xth2, xth3, xthx ; /* coded threshold factors */
  longint def_tc ; /* time correcton for onset (default) */
  longint wait_blk ; /* controls re-activation of detector
                        and recording time in event code & threshold too */
  integer val_avg ; /* the number of values in tsstak[] */
} tdetload ;
typedef tfloat tsinglearray[MAX_RATE] ;
typedef tsinglearray *psinglearray ;
typedef longint tinsamps[MAXSAMP] ;
typedef tfloat trealsamps[MAXSAMP] ;
#endif

typedef longint tdataarray[MAX_RATE] ;
typedef tdataarray *pdataarray ;
typedef byte tidxarray[MAX_RATE + 1] ;
typedef tidxarray *pidxarray ;
typedef longword tmergedbuf[MAX_RATE] ;
typedef tmergedbuf *pmergedbuf ;

#ifndef OMIT_SEED
/*
  tdetector defines a type of detector that can be used multiple times
*/
typedef struct tdetector {
  struct tdetector *link ; /* next in list of detectors */
  piirdef detfilt ; /* detector pre-filter, if any */
  tdetload uconst ; /*detector parameters*/
  byte detector_num ; /* detector number */
  enum tevent_detector dtype ; /* detector type */
  char detname[DETECTOR_NAME_LENGTH] ; /* detector name */
} tdetector ;
typedef tdetector *pdetector ;
/*  PDOPs are a representation of the actual equation, not what is run */
typedef struct tdop {
  struct tdop *link ;
  pointer point ; /* needed for DES_DET and DES_CAL */
  byte tok ;
} tdop ;
typedef tdop *pdop ;
/*
  Detector operations allow the results from multiple detectors combine to
  form a control detector output
*/
typedef boolean *pboolean ;
typedef struct tdetector_operation {
  struct tdetector_operation *link ;
  byte op ;
  integer temp_num ;
  pboolean tospt, nospt ;
} tdetector_operation ;
typedef tdetector_operation *pdetector_operation ;
/*
  A control detector is what is actually referenced by a LCQ to know if it
  should output event data
*/
typedef struct tcontrol_detector {
  struct tcontrol_detector *link ;  /* link to next control detectors */
  pdetector_operation pdetop ; /* the actual equations for execution */
  pdop token_list ; /* these were the tokens that were parsed */
  boolean logmsg ; /* if TRUE, send message to auxout on change */
  boolean ison ; /* current status */
  boolean wason ; /* previous status */
  byte ctrl_num ; /* control detector number */
  char cdname[79] ;
} tcontrol_detector ;
typedef tcontrol_detector *pcontrol_detector ;
/*
  Compressed buffer rings are used as pre-event buffers
*/
typedef struct tcompressed_buffer_ring {
  struct tcompressed_buffer_ring *link ; /* list link */
  seed_header hdr_buf ; /* for building header */
  completed_record rec ; /* ready to write format */
  boolean full ; /* if this record full */
} tcompressed_buffer_ring ;
typedef tcompressed_buffer_ring *pcompressed_buffer_ring ;
/*
  Downstream packets are used to filter a data stream and produce a new LCQ
*/
typedef struct tdownstream_packet {
  struct tdownstream_packet *link ; /* list link, NIL if end or no derived q's */
  pointer derived_q ; /* pointer to the lcq who looks at this flag, NIL if none */
} tdownstream_packet ;
typedef tdownstream_packet *pdownstream_packet ;
#endif

/*
  Segments are used for >50hz data re-assembly
*/
typedef struct tsegment_ring {
  struct tsegment_ring *link ;
  tdp_mult seg ; /* first is tdp_comp, rest are pdp_mult */
} tsegment_ring ;
typedef tsegment_ring *psegment_ring ;
/*
  The data holding queue are used to save out-of-order segments in the continuity
  structure. this should be a queue, to accomodate small MTU's which cause more splitting
  of seconds across DC_MULT messages. For standard MTU, a single buffer (two segments) works
*/
typedef struct {
  pdp_mult ppkt ; /* this points to the following pkt */
  byte pkt[MAXMTU - 40] ;
} dholdqtype ;
typedef dholdqtype *tdhqp ;

#ifndef OMIT_SEED
/*
  A com_packet is used to build up a compressed record using input from either
  the Q330 or another LCQ
*/
typedef struct {
  longint last_sample ; /* most recent sample for compression */
  longint flag_word ; /* for construction the flag longword */
  longint records_written ; /* count of buffers written */
  pcompressed_buffer_ring ring ; /* current element of buffer ring */
  pcompressed_buffer_ring last_in_ring ; /* last record in ring if non-NIL */
  compressed_frame frame_buffer ; /* frame we are currently compressing */
  word frame ; /* current compression frame */
  word maxframes ; /* maximum number of frames in a com record */
  integer ctabx ; /* current compression table index */
  integer block ; /* current compression block */
  integer peek_total ; /* number of samps in peek buffer */
  integer next_in ; /* peek buffer next-in index */
  integer next_out ; /* peek buffer next-out index */
  integer time_mark_sample ; /* sample number of time mark */
  integer next_compressed_sample ; /* next-in samp num in rec buf */
  integer blockette_count ; /* number of extra blockettes */
  integer blockette_index ; /* byte offset in record for next blockette (CNP) */
  integer last_blockette ; /* byte offset of last blockette */
  boolean charging ; /* filter charging */
  longint diffs[MAXSAMPPERWORD] ;
  longint sc[MAXSAMPPERWORD + 2] ;
  longint peeks[PEEKELEMS] ; /* compression buffer */
} tcom_packet ;
typedef tcom_packet *pcom_packet ;
#endif

/*
  A precomp record holds all the values associated with pre-compressed data from the Q330
*/
typedef struct {
  longint prev_sample ; /* previous sample from Q330 for decompression */
  longint prev_value ; /* from last decompression */
  integer block_idx ; /* index into source blocks */
  pbyte pmap ; /* pointer into blockette map */
  pbyte pdata ; /* pointer into blockette data */
  word mapidx ; /* indexes two bits at a time into pmap^ */
  word curmap ; /* current map word if mapidx <> 0 */
  integer blocks ; /* number of blocks to be decompressed */
} tprecomp ;

#ifndef OMIT_SEED
/* to build archival miniseed */
typedef struct {
  boolean appended ; /* data has been added */
  boolean existing_record ; /* from preload or incremental update */
  boolean incremental ; /* update every 512 byte record */
  boolean leave_in_buffer ; /* set to not clear out buffer after sending */
  word amini_filter ; /* OMF_xxx bits */
  integer total_frames ; /* sequential record filling index */
  integer frames_outstanding ; /* frames updated but not written */
  longint records_written ; /* count of buffers written */
  longint records_written_session ; /* this session */
  longint records_overwritten_session ; /* count of records overwritten */
  longword last_updated ; /* seconds since 2000 */
  seed_header hdr_buf ; /* for building header */
  pmax_cfr pcfr ;
} tarc ;
#endif

/*
  tlcq define one "Logical Channel Queue", corresponding to one SEED channel.
*/
typedef struct tlcq {
  struct tlcq *link ; /* forward link */
  struct tlcq *dispatch_link ; /* to next lcq that gets similar input data */
  tlocation location ; /* Seed Location */
  tseed_name seedname ; /* Seed Channel Name */
  byte lcq_num ; /* reference number for this LCQ */
  byte raw_data_source ; /* from Q330 channel */
  byte raw_data_field ; /* adds more information */
  longword lcq_opt ; /* LCQ options */
  string2 slocation ; /* dynamic length version */
  string3 sseedname ;
  word caldly ; /* number of seconds after cal over to turn off detection */
  word calinc ; /* count up timer for turning off detect flag*/
  integer rate ; /* + => samp per sec; - => sec per samp */
  boolean timemark_occurred ; /* set at the first sample */
  boolean cal_on ; /* calibration on */
  boolean calstat ; /* unfiltered calibration status */
  boolean variable_rate_set ; /* if any new data has been added to variable rate LCQ */
  boolean validated ; /* DP LCQ is still in tokens */
  enum tpacket_class pack_class ; /* for sending to client */
  longword dtsequence ; /* data record sequence number currently being processed */
  tfloat delay ; /* total FIR delay including digitizer delay */
  longword seg_seq ; /* sequence number for segment collection */
  psegment_ring segbuf ; /* only used for > 50hz */
  psegment_ring pseg ; /* the actual start of the linked list */
  psegment_ring seg_next ; /* next available ring buffer space */
  word segsize ; /* size of segment buffer */
  word seg_count ; /* number of segments so far */
  word seg_high ; /* highest segment, zero if not yet known */
  pmergedbuf mergedbuf ; /* continguous version of data from segments, same size as segbuf */
  word onesec_filter ; /* OSF_xxx bits */
  pidxarray idxbuf ; /* for converting frames into samples */
  pdataarray databuf ; /* raw input data */
  word datasize ; /* size of above structure */
  tdhqp dholdq ; /* data holding queue for DC_MULT pkts */
  double timetag ; /* seconds since 2000 */
  double backup_tag ; /* in case >1hz data gets flushed between seconds */
  double last_timetag ; /* if not zero, timetag of last second of data */
  word timequal ; /* quality from 0 to 100% */
  word backup_qual ; /* in case >1hz data gets flushed between seconds */
  single gap_threshold ; /* number of samples that constitutes a gap */
  tfloat gap_secs ; /* number of seconds constituting a gap */
  tfloat gap_offset ; /* expected number of seconds between new incoming samples */
  tprecomp precomp ; /* precompressed data fields */
#ifndef OMIT_SEED
  boolean slipping ; /* is derived stream, waiting for sync */
  longint slip_modulus ;
  tfloat input_sample_rate ; /* sample rate of input to decimation filter */
  pdownstream_packet downstream_link ; /* "stream_avail"'s for derived lcq's */
  struct tlcq *prev_link ; /* back link for checking fir-derived queue order */
  longword avg_length ; /* interval in samples between reports */
  single firfixing_gain ; /* normally 1.0, typically <1.0 for goes */
  pcom_packet com ; /* this stream's compression packet(s) */
  pcontrol_detector ctrl ; /* pointer to general detector stack */
  pointer det ; /* head of this channel's detector chain */
  pfilter source_fir ; /* pointer to where "fir" came from */
  pfir_packet fir ; /* this stream's fir filter */
  piirdef avg_source ; /* where the average filter came from */
  piirfilter avg_filt ; /* prefilter for averaging, if any */
  boolean gen_on ; /* general detector on */
  boolean gen_last_on ;
  boolean data_written ;
  byte scd_evt, scd_cont ; /* SCD_xxx flags for event and continuous */
  word pre_event_buffers ; /* number of pre-event buffers */
  tfloat processed_stream ; /* output of this stream's FIR filter */
  longint records_generated_session ; /* count of buffers generated this connection */
  longword last_record_generated ; /* seconds since 2000 */
  longint detections_session ; /* number of detections during session */
  longint calibrations_session ; /* number of calibrations during session */
  longword gen_next ; /* general next to send */
  piirfilter stream_iir ; /* head of this channel's IIR filter chain */
  pavg_packet avg ; /* structure for doing averaging */
  word mini_filter ; /* OMF_xxx bits */
  tarc arc ; /* archival miniseed structure */
  char control_detector_name[79] ; /* for later conversion to pointer */
#endif
} tlcq ;
typedef tlcq *plcq ;

#ifndef OMIT_SEED
/*
  tdet_packet defines the implementation of one detector on a LCQ. First part is not saved
  for continuity.
*/
typedef struct tdet_packet {
  struct tdet_packet *link ;
  pdetector detector_def ; /* definition of the detector */
  plcq parent ; /* the LCQ that owns this packet */
  byte det_options ; /* detector options */
  byte det_num ; /* ID for my copy of the detector */
  boolean singleflag ; /* true if data points are actually floating point */
  boolean remaining ; /* true if more samples to process in current rec */
  integer datapts ; /* Number of data points processed at a time */
  integer grpsize ; /* Samples per group, submultiple of datapts */
  integer sam_ch ;
  integer sam_no ; /*the number of the current seismic sample*/
  word insamps_size ; /* size of the tinsamps buffer, if any */
  word cont_size ; /* size of the continuity structure */
  double samrte ; /* Sample rate for this detector */
  pdataarray indatar ; /* ptr to data array */
  tinsamps *insamps ; /* ptr to low freq input buffer */
  pointer cont ; /* pointer to continuity structure */
  tonset_mh onset ; /* returned onset parameters */
  tdetload ucon ; /*user defined constants*/
} tdet_packet ;
typedef tdet_packet *pdet_packet ;
#endif

typedef struct {
  tlocation log_location ;
  tseed_name log_seedname ;
  tlocation tim_location ;
  tseed_name tim_seedname ;
} tlog_tim ;
typedef struct {
  tlocation cfg_location ;
  tseed_name cfg_seedname ;
  byte flags ;
  word interval ;
} tlog_cfg ;
typedef plcq tdispatch[96] ; /* handlers for non-main data */
typedef plcq tmdispatch[CHANNELS][FREQUENCIES] ; /* for main data */
typedef struct mholdqtype {
  struct mholdqtype *link ;
  string m ;
} mholdqtype ;
typedef mholdqtype *tmhqp ;
typedef struct {
  tcontext owner ;
#ifndef OMIT_SEED
  integer arc_size ; /* size of archival mini-seed records */
  integer arc_frames ; /* number of frames in an archival record */
#endif
  word first_sg ; /* start of cleard fields */
  word webport ;
  word netport ;
  word dservport ;
  tlog_tim log_tim ;
  tlog_cfg log_cfg ; /* NOTE: interval is in NBO */
  boolean contingood ; /* continuity good */
  boolean non_comp ; /* non-compliant DP */
  boolean first_data ;
  longword dt_data_sequence ; /* global data record sequence number */
  plcq lcqs ;    /* first lcq from this server */
  plcq proc_lcq ; /* lcq referenced by sliding window processing */
  byte calerr_bitmap ;
  byte highest_lcqnum ; /* highest LCQ number from tokens */
  word last_data_qual ;
  word data_qual ; /* 0-100% */
  double data_timetag ;
  tdss dss_def ; /* token definition */
  tcommevents commevents ;
#ifndef OMIT_SEED
  boolean daily_done ; /*daily timemark has been done*/
  double last_update ; /* time of last clock update */
  longint except_count ; /* for timing blockette exception_count */
  plcq cfg_lcq ; /* For configuration data */
  plcq msg_lcq ;
  plcq tim_lcq ;
  plcq cnp_lcqs ;
  piirdef iirchain ; /* start of iir filter chain */
  pdetector defchain ;
  pcontrol_detector ctrlchain ;
  double cfg_lastwritten ;
  word total_detectors ;
  pchar opaque_buf ;
  word opaque_size ;
  timing timing_buf ; /* need a place to keep this between log_clock and finish_log_clock */
  tcompressed_buffer_ring detcal_buf ; /* used for building event and calibration only records */
#endif
  word last_sg ;
  plcq dplcqs ; /* For statistics */
  pointer data_latency_lcq ; /* dp lcq for data latency */
  pointer status_latency_lcq ; /* dp lcq for status latency */
  tdispatch dispatch ;
  tmdispatch mdispatch ;
#ifndef OMIT_SEED
  tmhqp mholdq, mhqnxi, mhqnxo ;
  pfilter firchain ; /* start of fir filter chain */
  integer cfg_timer ; /* count-down since last added configuration data */
#endif
} taqstruc ;
typedef taqstruc *paqstruc ;

#endif
