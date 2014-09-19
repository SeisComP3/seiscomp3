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
*/
#ifndef q330types_h
/* Flag this file as included */
#define q330types_h
#define VER_Q330TYPES 0

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif


#define WINBUFS 128 /* Number of window buffers */
#define MANF_26QAP1 8 /* Use 26 bit output for channels 1-3 */
#define MANF_26QAP2 16 /* Use 26 bit output for channels 4-6 */
/* QDP */
#define QDP_VERSION 2 /* QDP Version */
#define QDP_HDR_LTH 12 /* in actual network traffic bytes */
#define UDP_HDR_LTH 8 /* ditto */
#define IP_HDR_LTH 20 /* ditto */
/* Commands and Response */
#define C1_CACK 0xA0 /* Command Acknowledge */
#define C1_RQSRV 0x10 /* Request Server Registration */
#define C1_SRVCH 0xA1 /* Server Challenge */
#define C1_SRVRSP 0x11 /* Server Response */
#define C1_CERR 0xA2 /* Command Error */
#define C1_DSRV 0x12 /* Delete Server */
#define C1_POLLSN 0x14 /* Poll for Serial Number */
#define C1_MYSN 0xA3 /* My Serial Number */
#define C1_SLOG 0x17 /* Set Logical Port */
#define C1_RQLOG 0x18 /* Request Logical Port */
#define C1_LOG 0xA5 /* Logical Port */
#define C1_RQSTAT 0x1F /* Request Status */
#define C1_STAT 0xA9 /* Status */
#define C1_RQRT 0x25 /* Request Routing Table */
#define C1_RT 0xAA /* Routing Table */
#define C1_RQGID 0x28 /* Request GPS ID Strings */
#define C1_GID 0xAC /* GPS ID Strings */
#define C1_UMSG 0x30 /* Send User Message */
#define C1_WEB 0x33 /* Webserver advertisement */
#define C1_RQFGLS 0x34 /* Request fixed values, global programming, one logical port, and sensor control */
#define C1_FGLS 0xB1 /* Combination of fixed values, global programming, one logical port, and sensor control */
#define C1_RQDCP 0x35 /* Request Digitizer Calibration Packet */
#define C1_DCP 0xB2 /* Digitizer Calibration Packet */
#define C1_RQDEV 0x36 /* Request Devices */
#define C1_DEV 0xB3 /* Device list */
#define C1_PING 0x38 /* Ping Q330 */
#define C1_RQMAN 0x1E /* Request Manufacturer's Area */
#define C1_MAN 0xA8 /* Manufacturer's Area */
/* Memory Commands */
#define C1_RQMEM 0x41 /* Request Memory Contents */
#define C1_MEM 0xB8 /* Memory Contents */
/* Secondary Commands */
#define C2_RQGPS 0x53 /* Request GPS Parameters */
#define C2_GPS 0xC1 /* GPS Parameters */
/* Cal Status Bits */
#define CAL_ENON 1 /* Calibration enable on */
#define CAL_SGON 2 /* Calibration signal on */
#define CAL_ERROR 4 /* Calibration on */
/* Memory Types */
#define MT_CFG1 1 /* Configuration Memory for logical port 1 */
#define MT_CFG2 2 /* Configuration Memory for logical port 2 */
#define MT_CFG3 3 /* Configuration Memory for logical port 3 */
#define MT_CFG4 4 /* Configuration Memory for logical port 4 */
#define MAXSEG 438 /* Maximum number of real bytes */
/* Command Error Codes */
#define CERR_PERM 0 /* No Permission */
#define CERR_TMSERV 1 /* Too many servers */
#define CERR_NOTR 2 /* You are not registered */
#define CERR_INVREG 3 /* Invalid Registration Request */
#define CERR_PAR 4 /* Parameter Error */
#define CERR_SNV 5 /* Structure not valid */
#define CERR_CTRL 6 /* Control Port Only */
#define CERR_SPEC 7 /* Special Port Only */
#define CERR_MEM 8 /* Memory operation already in progress */
#define CERR_CIP 9 /* Calibration in Progress */
#define CERR_DNA 10 /* Data not available */
#define CERR_DB9 11 /* Console Port Only */
#define CERR_MEMEW 12 /* Memory erase or Write Error */
/* PLL Flag Bits */
#define PLL_AUTO 0x8000 /* PLL Operation on */
/* Status request bits */
#define SRB_TOKEN 29 /* DP Tokens have changed */
#define SRB_LCHG 30 /* Logical Port programming change */
#define SRB_UMSG 31 /* User Message */
/* advanced GPS modes */
#define AG_INT 0 /* internal GPS */
#define AG_EXT 1 /* external GPS */
#define AG_ESEA 2 /* external seascan */
#define AG_NET 3 /* network timing */
#define AG_EACC 4 /* external access to internal GPS */
#define AG_EXPORT 8 /* export NMEA and 1PPS output */
#define AG_422 0x10 /* Use RS-422 for import/export, else RS-232 */
#define AG_SDGPS 0x20 /* Serial DGPS input */
#define AG_NDGPS 0x40 /* Network DGPS input */
/* advanced GPS flags */
#define AG_CONT 0 /* Continuous Operation */
#define AG_MAX 1 /* Until maximum on time */
#define AG_PLL 2 /* Until PLL lock */
#define AG_GPS 3 /* Until GPS lock */
/* Gain Bitmap */
#define GAIN_DIS 0 /* disabled */
#define GAIN_POFF 1 /* preamp off */
#define GAIN_PON 2 /* preamp on */

#define PIU_TIME 5 * 60 /* Port in Use */
#define NR_TIME 30 /* Not Registered */
#define CRC_POLYNOMIAL 1443300200

typedef longint crc_table_type[256] ;

/* IP */
typedef struct { /* IP Header */
  byte ip_verlen ; /* IP version & header length (in longs) */
  byte ip_tos ; /* type of service */
  word ip_len ;  /* total packet length (in octets) */
  word ip_id ;  /* datagram id */
  word ip_fragoff ; /* fragment offset (in 8-octet's) */
  byte ip_ttl ;  /* time to live */
  byte ip_proto ; /* IP protocol */
  word ip_cksum ; /* header checksum */
  longword ip_src ;  /* IP address of source */
  longword ip_dst ;  /* IP address of destination */
} tip ;
/* UDP */
typedef struct { /* UDP header */
  word u_src ; /* source UDP port number */
  word u_dst ; /* destination UDP port number */
  word u_len ; /* length of UDP data */
  word u_cksum ; /* UDP checksum (0 => none) */
} tudp ;
/* QDP */
typedef struct {     /* common header for all UDP messages */
  longint crc ;      /* over all the packet */
  byte command ;     /* Command */
  byte version ;     /* version */
  word datalength ;  /* not including header */
  word sequence ;    /* Sender's sequence */
  word acknowledge ; /* and acknowledge */
} tqdp ;
typedef tqdp *pqdp ;
typedef struct { /* UDP psuedo-header */
  longword up_src ; /* IP address of source */
  longword up_dst ; /* IP address of destination */
  byte up_zero ; /* zero byte */
  byte up_proto ; /* protocol */
  word up_length ; /* UDP length */
} tpsuedo ;
typedef tpsuedo *ppsuedo ;
/*
Any qdp packet at the Q330 binary level
*/
typedef struct {
  byte headers[28] ; /* for ip and udp */
  byte qdp[QDP_HDR_LTH] ; /* size of qdp header */
  byte qdp_data[MAXDATA] ;
} tany ;
typedef tany *pany ;
typedef struct {
  longword sender ;
  string79 msg ;
} tuser_message ;
typedef struct {  /* C1_SRVCH */
  t64 challenge ; /* challenge value */
  longword dpip ; /* DP IP */
  word dpport ; /* PORT */
  word dpreg ; /* Registration */
} tsrvch ;
typedef struct {  /* C1_SRVRSP */
  t64 serial ; /* serial number */
  t64 challenge ; /* challenge value */
  longword dpip ; /* DP IP */
  word dpport ; /* PORT */
  word dpreg ; /* Registration */
  t64 counter_chal ; /* server's counter challege value */
  t128 md5result ;
} tsrvresp ;
typedef struct { /* C1_POLLSN */
  word mask ; /* serial number mask */
  word match ; /* serial number match */
} tpoll ;
typedef struct { /* header for memory requests */
  longword start ; /* Starting address */
  word count ; /* Byte Count */
  word memtype ; /* Memory Type */
} tmem ;
typedef tmem *pmem ;
typedef struct { /* for reading tokens */
  word segnum ;
  word segtotal ;
} tseghdr ;
typedef struct { /* old format for webserver advertisement */
  char ip_port[24] ; /* webserver's ip and port number */
  char name[8] ; /* station name */
} told_webadv ;
typedef struct { /* new format for webserver advertisement */
  char name[8] ; /* station name */
  char dpaddress[256] ;
} tnew_webadv ;
typedef struct { /* format of C1_FGL */
  word gl_off ; /* offset to start of global programming block */
  word sc_off ; /* offset to start of sensor control */
  word lp_off ; /* offset to start of logical port block */
  word spare ;
} tfgl ;
typedef struct { /* Format of C1_MYSN */
  t64 sys_num ; /* System serial number */
  longword property_tag ; /* For Ian */
  longword user_tag ;
} tmysn ;
typedef tmysn *pmysn ;
typedef struct { /* ping header (first 2 words) */
  word ping_type ;
  word ping_opt ; /* id or user message number */
} tpinghdr ;
typedef struct { /* data acknowledgement */
  word new_throttle ; /* if not $FFFF */
  word spare2 ;
  longword acks[4] ;
  longword spare3 ;
} tdp_ack ;
typedef struct { /* one NTP server */
  longword ip ;
  longword router_ip ;
  longint offset ; /* offset in usec */
  word port ;
  word timeout ; /* timeout in seconds */
  word init_int ; /* initial interval in seconds */
  word ss_int ; /* steady_state interval in seconds */
  longword spare ;
} tntpserv ;
typedef struct { /* format of C2_GPS */
  word mode ; /* timing mode */
  word flags ;
  word off_time ; /* in minutes */
  word resync ; /* resync hour */
  word max_on ; /* in minutes */
  word lock_usec ; /* maximum usec offset for lock condition */
  longword spare2 ;
  word interval ; /* PLL update interval */
  word initial_pll ;
  single pfrac ;
  single vco_slope ;
  single vco_intercept ;
  single max_ikm_rms ;
  single ikm_weight ;
  single km_weight ;
  single best_weight ;
  single km_delta ;
  longword spare4 ;
  longword spare5 ;
  /* following is preliminary */
  word ntpflags ; /* NTPF_xxxx constants */
  word gpstontpdly ; /* Delay in minutes between switching from GPS to NTP */
  word ntptogpsdly ; /* Delay in minutes between switching from NTP to GPS */
  word spare3 ;
  tntpserv ntp_servers[NTP_SERV_COUNT] ;
} tgps2 ;
typedef struct { /* Format for C1_MAN */
  t128 password ; /* password to gain access */
  word qap13_type ; /* QAPCHP channels 1-3 type */
  word qap13_ver ; /* QAPCHP channels 1-3 version */
  word qap46_type ; /* QAPCHP channels 4-6 type */
  word qap46_ver ; /* QAPCHP channels 4-6 version */
  longword qap13_num ; /* QAPCHP channels 1-3 serial number */
  longword qap46_num ; /* QAPCHP channels 4-6 serial number */
  longword ref_counts[CHANNELS] ; /* desired counts with reference conn. */
  longword born_on ; /* "born on" time */
  longword packet_sz ; /* packet buffer size */
  word clk_type ; /* Clock type */
  word model ; /* system model number */
  int16 def_cal_offset ; /* default calibrator offset */
  int16 flags ;
  longword property_tag ; /* For Ian */
  longword expiration_time ; /* Maximum running time */
} tman ;
typedef struct { /* format of C1_DCP */
  longint offsets[CHANNELS] ; /* offset with preamp on */
  longint gains[CHANNELS] ; /* channel gains */
} tdcp ;
#endif
