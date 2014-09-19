/*   Lib330 structures relating to Q330 communications
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
    0 2006-09-09 rdr Created
    1 2006-10-26 rdr Add last_data_time, current_ip, and current_port to topstat.
    2 2006-11-30 rdr Add Definitions for module directory.
    3 2008-04-03 rdr Add opt_compat.
    4 2008-08-19 rdr Add tcp support.
*/
#ifndef libclient_h
/* Flag this file as included */
#define libclient_h
#define VER_LIBCLIENT 7

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif

/* one second filtering bit masks */
#define OSF_ALL 1 /* bit set to send all one second data */
#define OSF_DATASERV 2 /* bit set to send dataserv lcqs */
#define OSF_1HZ 4 /* bit set to send 1hz main digitizer data */
/* miniseed filtering bit masks */
#define OMF_ALL 1 /* bit set to send all miniseed data */
#define OMF_NETSERV 2 /* bit set to send netserv data */
#define OMF_CFG 4 /* pass configuration opaque blockettes */
#define OMF_TIM 8 /* pass timing records */
#define OMF_MSG 16 /* pass message records */
#define MAX_LCQ 128 /* maximum number of lcqs that can be reported */

#ifndef OMIT_SEED
#define FILTER_NAME_LENGTH 31 /* Maximum number of characters in an IIR filter name */
#define FIRMAXSIZE 400
#define MAX_DETSTAT 40 /* maximum number that library will return */
#define MAX_CTRLSTAT 20 /* maximum number that library will return */
#define MAX_MODULES 30
#else
#define MAX_MODULES 23
#endif

typedef char string2[3] ;
typedef char string3[4] ;
typedef char string5[6] ;
typedef char string9[10] ;
typedef char string250[252] ;
typedef pointer tcontext ; /* a client doesn't need to know the contents of the context */
typedef void (*tcallback)(pointer p) ; /* prototype for all the callback procedures */
enum thost_mode {HOST_ETH, HOST_SER, HOST_TCP} ;

#ifndef OMIT_SEED
/*
  Tfilter_base/tfilter is a definition of a FIR filter which may be used
  multiple places
*/
typedef struct tfilter { /* coefficient storage for FIR filters */
  struct tfilter *link ; /* list link */
  char fname[FILTER_NAME_LENGTH + 1] ; /* name of this FIR filter */
  byte fir_num ; /* filter number */
  double coef[FIRMAXSIZE] ; /* IEEE f.p. coef's */
  longint len ; /* actual length of filter */
  double gain ; /* gain factor of filter */
  double dly ; /* delay in samples */
  longint dec ; /* decimation factor of filter */
} tfilter ;
typedef tfilter *pfilter ;
#endif

typedef struct { /* parameters for lib_create call */
  t64 q330id_serial ; /* serial number */
  word q330id_dataport ; /* Data port, use LP_TEL1 .. LP_TEL4 */
  string5 q330id_station ; /* initial station name */
  longint host_timezone ; /* seconds offset of host time. Initial value if auto-adjust enabled */
  string95 host_software ; /* host software type and version */
  string250 opt_contfile ; /* continuity root file path and name, null for no continuity */
  word opt_verbose ; /* VERB_xxxx bitmap */
  word opt_zoneadjust ; /* calculate host's timezone automatically */
  word opt_secfilter ; /* OSF_xxx bits */
#ifndef OMIT_SEED
  word opt_compat ; /* Compatibility Mode */
  word opt_minifilter ; /* OMF_xxx bits */
  word opt_aminifilter ; /* OMF_xxx bits */
  word amini_exponent ; /* 2**exp size of archival miniseed, range of 9 to 14 */
  integer amini_512highest ; /* rates up to this value are updated every 512 bytes */
  word mini_embed ; /* 1 = embed calibration and event blockettes into data */
  word mini_separate ; /* 1 = generate separate calibration and event records */
  pfilter mini_firchain ; /* FIR filter chain for decimation */
  tcallback call_minidata ; /* address of miniseed data callback procedure */
  tcallback call_aminidata ; /* address of archival miniseed data callback procedure */
#endif
  enum tliberr resp_err ; /* non-zero for error code creating context */
  tcallback call_state ; /* address of state change callback procedure */
  tcallback call_messages ; /* address of messages callback procedure */
  tcallback call_secdata ; /* address of one second data callback procedure */
  tcallback call_lowlatency ; /* address of low latency data callback procedure */
} tpar_create ;
typedef struct { /* parameters for lib_register call */
  t64 q330id_auth ; /* authentication code */
  string250 q330id_address ; /* domain name or IP address in dotted decimal */
  word q330id_baseport ; /* base UDP port number */
  enum thost_mode host_mode ;
  string250 host_interface ; /* ethernet or serial port path name */
  word host_mincmdretry ; /* minimum command retry timeout */
  word host_maxcmdretry ; /* maximum command retry timeout */
  word host_ctrlport ; /* set non-zero to use specified UDP port at host end */
  word host_dataport ; /* set non-zero to use specified UDP port at host end */
#ifndef OMIT_SERIAL
  word serial_flow ; /* 1 = hardware flow control */
  longword serial_baud ; /* in bps */
  longword serial_hostip ; /* IP address to identify host */
#endif
  word opt_latencytarget ; /* seconds latency target for low-latency data */
  word opt_closedloop ; /* 1 = enable closed loop acknowledge */
  word opt_dynamic_ip ; /* 1 = dynamic IP address */
  word opt_hibertime ; /* hibernate time in minutes if non-zero */
  word opt_conntime ; /* maximum connection time in minutes if non-zero */
  word opt_connwait ; /* wait this many minutes after connection time or buflevel shutdown */
  word opt_regattempts ; /* maximum registration attempts before hibernate if non-zero */
  word opt_ipexpire ; /* dyanmic IP address expires after this many minutes since last POC */
  word opt_buflevel ; /* terminate connection when buffer level reaches this value if non-zero */
} tpar_register ;
enum tpacket_class {PKC_DATA, PKC_EVENT, PKC_CALIBRATE, PKC_TIMING, PKC_MESSAGE, PKC_OPAQUE} ;
enum tacctype {AC_GAPS, AC_BOOTS, AC_READ, AC_WRITE, AC_COMATP, AC_COMSUC, AC_PACKETS, AC_COMEFF,
            AC_POCS, AC_NEWIP, AC_DUTY, AC_THROUGH, AC_MISSING, AC_FILL, AC_CMDTO, AC_SEQERR,
            AC_CHECK, AC_IOERR} ;

#define AC_FIRST AC_GAPS
#define AC_LAST AC_IOERR /* CHANGE THESE WHENEVER ADDING A NEW TACCTYPE */
#define AC_DATA_LATENCY ((integer)AC_LAST + 1)
#define AC_STATUS_LATENCY ((integer)AC_DATA_LATENCY + 1)
#define INVALID_ENTRY -1 /* no data for this time period */
#define INVALID_LATENCY -66666666 /* not yet available */

typedef struct {
  word low_seq ; /* last packet number acked */
  word latest ; /* latest packet received */
  longword validmap[8] ;
} tslidestat ;
enum taccdur {AD_MINUTE, AD_HOUR, AD_DAY} ;
/* Compiler doesn't understand this typedef longint taccstats[tacctype][taccdur] ; */
typedef longint taccstats[AC_LAST + 1][AD_DAY + 1] ;
typedef struct { /* operation status */
  string9 station_name ; /* network and station */
  word station_port ; /* data port number */
  longword station_tag ; /* tagid */
  t64 station_serial ; /* q330 serial number */
  longword station_reboot ; /* time of last reboot */
  longint timezone_offset ; /* seconds to adjust computer's clock */
  taccstats accstats ; /* accumulated statistics */
  word minutes_of_stats ; /* how many minutes of data available to make hour */
  word hours_of_stats ; /* how many hours of data available to make day */
  word auxinp ; /* bitmap of Aux. inputs */
  longint data_latency ; /* data latency (calculated based on host clock) in seconds or INVALID_LATENCY */
  longint status_latency ; /* seconds since received status from 330 or INVALID_LATENCY */
  longint runtime ; /* running time in seconds since current connection (+) or time it has been down (-) */
  longword totalgaps ; /* total number of data gaps since context created */
  single pkt_full ; /* percent of Q330 packet buffer full */
  word clock_qual ; /* Percent clock quality */
  longint clock_drift ; /* Clock drift from GPS in microseconds */
  integer mass_pos[6] ; /* mass positions */
  integer calibration_errors ; /* calibration error bitmap */
  integer sys_temp ; /* Q330 temperature in degrees C */
  single pwr_volt ; /* Q330 power supply voltage in volts */
  single pwr_cur ; /* Q330 power supply current in amps */
  longint gps_age ; /* age in seconds of last GPS clock update, -1 for never updated */
  enum tgps_stat gps_stat ; /* GPS Status */
  enum tgps_fix gps_fix ; /* GPS Fix */
  enum tpll_stat pll_stat ; /* PLL Status */
  double gps_lat ; /* Latitude */
  double gps_long ; /* Longitude */
  double gps_elev ; /* Elevation */
  tslidestat slidecopy ; /* sliding window status */
  longword last_data_time ; /* Latest data received, 0 for none */
  longword current_ip ; /* current IP Address of Q330 */
  word current_port ; /* current Q330 UDP Port */
} topstat ;
typedef struct { /* for 1 second and low latency callback */
  longword total_size ; /* number of bytes in buffer passed */
  tcontext context ;
  string9 station_name ; /* network and station */
  string2 location ;
  byte chan_number ; /* channel number according to tokens */
  string3 channel ;
  word padding ;
  integer rate ; /* sampling rate */
  longword cl_session ; /* closed loop session number */
  longword reserved ; /* must be zero */
  double cl_offset ; /* closed loop time offset */
  double timestamp ; /* Time of data, corrected for any filtering */
  word filter_bits ; /* OSF_xxx bits */
  word qual_perc ; /* time quality percentage */
  word activity_flags ; /* same as in Miniseed */
  word io_flags ; /* same as in Miniseed */
  word data_quality_flags ; /* same as in Miniseed */
  byte src_channel ; /* source blockette channel */
  byte src_subchan ; /* source blockette sub-channel */
  longint samples[MAX_RATE] ; /* decompressed samples */
} tonesec_call ;
#ifndef OMIT_SEED
enum tminiseed_action {MSA_512, /* new 512 byte packet */
                      MSA_ARC, /* new archival packet, non-incremental */
                      MSA_FIRST, /* new archival packet, incremental */
                      MSA_INC, /* incremental update to archival packet */
                      MSA_FINAL, /* final incremental update */
                      MSA_GETARC, /* request for last archival packet written */
                      MSA_RETARC} ; /* client is returning last packet written */
typedef struct { /* format for miniseed and archival miniseed */
  tcontext context ;
  string9 station_name ; /* network and station */
  string2 location ;
  byte chan_number ; /* channel number according to tokens */
  string3 channel ;
  integer rate ; /* sampling rate */
  longword cl_session ; /* closed loop session number */
  double cl_offset ; /* closed loop time offset */
  double timestamp ; /* Time of data, corrected for any filtering */
  word filter_bits ; /* OMF_xxx bits */
  enum tpacket_class packet_class ; /* type of record */
  enum tminiseed_action miniseed_action ; /* what this packet represents */
  word data_size ; /* size of actual miniseed data */
  pointer data_address ; /* pointer to miniseed record */
} tminiseed_call ;
typedef tminiseed_call *pminiseed_call ;
#endif
enum tstate_type {ST_STATE, /* new operational state */
                 ST_STATUS, /* new status available */
                 ST_CFG, /* new configuration available */
                 ST_STALL, /* change in stalled comlink state */
                 ST_PING, /* subtype has ping type */
                 ST_TICK, /* info has seconds, subtype has usecs */
                 ST_OPSTAT, /* new operational status minute */
                 ST_TUNNEL} ; /* tunnel response available */
typedef struct { /* format for state callback */
  tcontext context ;
  enum tstate_type state_type ; /* reason for this message */
  string9 station_name ;
  longword subtype ; /* to further narrow it down */
  longword info ; /* new highest message for ST_MSG, new state for ST_STATE,
                      or new status available for ST_STATUS */
} tstate_call ;
typedef struct { /* format for messages callback */
  tcontext context ;
  longword msgcount ; /* number of messages */
  word code ;
  longword timestamp, datatime ;
  string95 suffix ;
} tmsg_call ;
typedef struct { /* format of data provided by received POC */
  longword new_ip_address ; /* new dynamic IP address */
  word new_base_port ; /* port translation may have changed it */
  string95 log_info ; /* any additional information the POC receiver wants logged */
} tpocmsg ;
#ifndef OMIT_SEED
typedef struct { /* format of one detector status entry */
  char name[DETECTOR_NAME_LENGTH + 11] ;
  boolean ison ; /* last record was detected on */
  boolean declared ; /* ison filtered with first */
  boolean first ; /* if this is the first detection after startup */
  boolean enabled ; /* currently enabled */
} tonedetstat ;
typedef struct { /* format of the result */
  integer count ; /* number of valid entries */
  tonedetstat entries[MAX_DETSTAT] ;
} tdetstat ;
typedef struct { /* record to change detector enable */
  char name[DETECTOR_NAME_LENGTH + 11] ;
  boolean run_detector ;
} tdetchange ;
typedef struct { /* format of one control detector status entry */
  string79 name ;
  boolean ison ; /* currently on */
} tonectrlstat ;
typedef struct { /* format of the result */
  integer count ; /* number of valid entries */
  tonectrlstat entries[MAX_CTRLSTAT] ;
} tctrlstat ;
typedef struct { /* format of one lcq status entry */
  string2 location ;
  byte chan_number ; /* channel number according to tokens */
  string3 channel ;
  longint rec_cnt ; /* number of records */
  longint rec_age ; /* number of seconds since update */
  longint rec_seq ; /* current record sequence */
  longint det_count ; /* number of detections */
  longint cal_count ; /* number of calibrations */
  longint arec_cnt ; /* number of archive new records */
  longint arec_over ; /* number of archive overwritten records */
  longint arec_age ; /* since last update */
  longint arec_seq ; /* current record sequence */
} tonelcqstat ;
typedef struct { /* format of the result */
  integer count ; /* number of valid entries */
  tonelcqstat entries[MAX_LCQ] ;
} tlcqstat ;
#endif
typedef struct { /* format of essential items from tokens */
  string9 station_name ;
  word web_port ; /* web server TCP port */
  word net_port ; /* netserver (LISS) TCP port */
  word datas_port ; /* dataserver TCP port */
  longword webip ; /* IP address according to server challenge */
  tdss dss ; /* DSS configuration */
  word buffer_counts[MAX_LCQ] ; /* pre-event buffers + 1 */
} tdpcfg ;
typedef struct { /* one module */
  string15 name ;
  integer ver ;
} tmodule ;
typedef tmodule tmodules[MAX_MODULES] ; /* Null name to indicate end of list */
typedef tmodules *pmodules ;

extern void lib_create_context (tcontext *ct, tpar_create *cfg) ; /* If ct = NIL return, check resp_err */
extern enum tliberr lib_destroy_context (tcontext *ct) ; /* Return error if any */
extern enum tliberr lib_register (tcontext ct, tpar_register *rpar) ;
extern enum tlibstate lib_get_state (tcontext ct, enum tliberr *err, topstat *retopstat) ;
extern void lib_change_state (tcontext ct, enum tlibstate newstate, enum tliberr reason) ;
extern void lib_request_status (tcontext ct, longword bitmap, word interval) ;
extern enum tliberr lib_get_status (tcontext ct, longword bitnum, pointer buf) ;
extern enum tliberr lib_set_config (tcontext ct, longword bitnum, pointer buf) ;
extern void lib_abort_command (tcontext ct) ;
extern enum tliberr lib_get_config (tcontext ct, longword bitnum, pointer buf) ;
extern void lib_ping_request (tcontext ct, tpingreq *ping_req) ;
extern enum tliberr lib_unregistered_ping (tcontext ct, tpar_register *rpar) ;
extern word lib_change_verbosity (tcontext ct, word newverb) ;
extern enum tliberr lib_get_slidestat (tcontext ct, tslidestat *slidecopy) ;
extern void lib_send_usermessage (tcontext ct, string79 *umsg) ;
extern void lib_poc_received (tcontext ct, tpocmsg *poc) ;
extern enum tliberr lib_get_commevents (tcontext ct, tcommevents *commevents) ;
#ifndef OMIT_SEED
extern void lib_set_commevent (tcontext ct, integer number, boolean seton) ;
extern enum tliberr lib_get_detstat (tcontext ct, tdetstat *detstat) ;
extern enum tliberr lib_get_ctrlstat (tcontext ct, tctrlstat *ctrlstat) ;
extern enum tliberr lib_get_lcqstat (tcontext ct, tlcqstat *lcqstat) ;
extern void lib_change_enable (tcontext ct, tdetchange *detchange) ;
#endif
extern enum tliberr lib_get_dpcfg (tcontext ct, tdpcfg *dpcfg) ;
extern void lib_msg_add (tcontext ct, word msgcode, longword dt, string95 *msgsuf) ;
extern void lib_webadvertise (tcontext ct, string15 *stnname, string *dpaddr) ;
extern enum tliberr lib_send_tunneled (tcontext ct, byte cmd, byte response, pointer buf, integer req_size) ;
extern enum tliberr lib_get_tunneled (tcontext ct, byte *response, pointer buf, integer *resp_size) ;
extern pmodules lib_get_modules (void) ;
#endif
