/*   Lib330 common type definitions
     Copyright 2006-2010 Certified Software Corporation

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
    1 2007-07-16 rdr Add tback and LIBSTATE_ANNC.
    2 2007-08-04 rdr tcp/ip structure definitions moved here from q330types.h
    3 2008-01-09 rdr Add 230400 baud and baler flags.
    4 2008-07-31 rdr Add met3 support.
    5 2008-08-05 rdr Add flag bits for requesting baler configuration block.
    6 2008-08-20 rdr Add TCP Support.
    7 2009-02-08 rdr Add Environmental Processor support.
    8 2009-03-11 rdr Add SDI status definitions.
    9 2009-04-18 rdr Changes in EP structures.
   10 2009-06-27 rdr Increase size of opaque structure in tback.
   11 2010-03-27 rdr Add Q335 definitions.
   12 2010-05-08 rdr Add minimum on time to tcomm.
   13 2010-12-26 rdr Add sensor currents to boom status.
*/
/* Flag this file as included */
#ifndef libtypes_h
#define libtypes_h
#define VER_LIBTYPES 13

#include "pascal.h"

/* The following are generally the exact data structures as returned from
   the Q330. In a few cases a change has been made for easier host use */
#define MAXARP 16 /* maximum arp history */
#define MAX_SAT 12 /* Maximum number of satellites */
#define MAXROUTE 40 /* Maximum number of routes */
#define MAXDEV 20 /* Maximum number of devices */
#define PORT_OS 65535 /* OS assigned web/data/net server port */
#define MAXLINT 2147483647 /* maximum positive value for a longint */
#define CHANNELS 6 /* Number of digitizer/boom channels */
#define FREQS 8 /* Number of frequencies possible from Q330 */
#define FREQUENCIES 8 /* Same as FREQS */
#define HIGH_FREQ_BIT 7 /* Bit 7 frequency is variable rate */
#define MAX_RATE 1000 /* highest frequency for detector */
#define MAXMTU 576 /* Maximum MTU allowed */
#define MINMTU 276 /* Minimum MTU allowed */
#define MAXDATA 536 /* maximum size of QDP payload */
#define TCPBUFSZ (MAXMTU + 4) /* with room for two extra words */
#define MAXDATA96 722 /* maxumum size of QDP payload if Base96 */
#define COMMLENGTH 11 /*number of characters in a comm name*/
#define CE_MAX 32 /* maximum number of comm events */
#define NTP_SERV_COUNT 2 /* Number of NTP server definitions */

#ifndef OMIT_SEED
#define DETECTOR_NAME_LENGTH 31 /* Maximum number of characters in a detector name */
#endif

#ifndef X86_WIN32
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

/* Fixed flag bits */
#define FF_DYN 2 /* Bit 1 of fixed.flags indicates dynamic addresses supported */
#define FF_AUX 4 /* Bit 2 in fixed.flags indicates aux board status supported */
#define FF_NWEB 8 /* Bit 3 in fixed.flags indicates can handle new web server advertisement */
#define FF_SS 0x10 /* Bit 4 in fixed.flags indicates supports serial sensor status */
#define FF_HIPWR 0x20 /* Bit 5 in fixed.flags indicates > 250ma current limit */
#define FF_EP 0x40 /* Bit 6 in fixed.flags indicates at least 1 EP configured */
#define FF_335 0x100 /* Bit 8 in fixed.flags indicates this is a Q335 */
/* Clock Type */
#define CLK_NONE 0 /* No internal clock */
#define CLK_M12 1 /* Motorola M12 */
/* Calibrator Types */
#define CAL_QCAL330 33
#define CAL_QCAL335 35
/* Aux. board types */
#define AUXAD_ID 32
#define AUXAD_PORT 140
/* Unknown Seismo Temperature */
#define TEMP_UNKNOWN 666
/* Logical Ports and bit numbers in physical log_map */
#define LP_NONE 6
#define LP_TEL1 0
#define LP_TEL2 1
#define LP_TEL3 2
#define LP_TEL4 3
#define LP_CTRL 4
#define LP_SPEC 5
/* Physical Ports */
#define PP_NONE -1
#define PP_SER1 0
#define PP_SER2 1
#define PP_SER3 2
#define PP_GPS 3
#define PP_ETH 3
/* Ethernet Link Status */
#define LS_LINKOK 0x8000
#define LS_POLOK 0x10
/* Logical Port Flags */
#define LNKFLG_FILL 1 /* Fill mode on */
#define LNKFLG_FLUSH 2 /* Flushed based on time */
#define LNKFLG_FREEZE 4 /* Freeze data output */
#define LNKFLG_FRACQ 8 /* Freeze Acquisition */
#define LNKFLG_OLD 16 /* Keep oldest data */
#define LNKFLG_PIGGY 0x100 /* Piggyback sequence requests on data acks */
#define LNKFLG_FLUSH_FAULT 0x200 /* Data LED if flush didn't go well */
#define LNKFLG_HOTSWAP 0x400 /* Hotswap OK */
#define LNKFLG_WFLUSH 0x800 /* Flushed window buffers based on time */
#define LNKFLG_BASE96 0x4000 /* Send data using base 96 instead of binary */
#define LNKFLG_SAVE 0x8000 /* Save in EEPROM */
/* Logical Port Status Flags */
#define LPSF_RECON 1 /* Vacuum reconnect request */
#define LPSF_POWER 2 /* Keep powered on */
#define LPSF_PWROFF 4 /* Turn off power */
#define LPSF_BADMEM 0x8000 /* Packet buffer size reduced due to bad packet memory */
/* Cal Status Bits */
#define CAL_ENON 1 /* Calibration enable on */
#define CAL_SGON 2 /* Calibration signal on */
#define CAL_ERROR 4 /* Calibration on */
/* Charger Status */
#define CHRG_NOT 0 /* Not Charging */
#define CHRG_BULK 1 /* Bulk */
#define CHRG_ABS 2 /* Absorption */
#define CHRG_FLOAT 3 /* Float */
#define CHRG_EQ 4 /* Equalization */
/* Status request bits */
#define SRB_GLB 0 /* Global Status */
#define SRB_GST 1 /* GPS Status */
#define SRB_PWR 2 /* Power supply Status */
#define SRB_BOOM 3 /* Boom positions ... */
#define SRB_THR 4 /* Thread Status */
#define SRB_PLL 5 /* PLL Status */
#define SRB_GSAT 6 /* GPS Satellites */
#define SRB_ARP 7 /* ARP Status */
#define SRB_LOG1 8 /* Logical Port 1 Status */
#define SRB_LOG2 9 /* Logical Port 2 Status */
#define SRB_LOG3 10 /* Logical Port 3 Status */
#define SRB_LOG4 11 /* Logical Port 4 Status */
#define SRB_SER1 12 /* Serial Port 1 Status */
#define SRB_SER2 13 /* Serial Port 2 Status */
#define SRB_SER3 14 /* Serial Port 3 Status */
#define SRB_ETH 15 /* Ethernet Status */
#define SRB_BALER 16 /* Baler Status */
#define SRB_DYN 17 /* Dynamic IP Address */
#define SRB_AUX 18 /* Aux Board Status */
#define SRB_SS 19 /* Serial Sensor Status */
#define SRB_EP 20 /* Environmental Processor Status */
#define SRB_FES 21 /* Q335 Front End Status */
/* Configuration Request Bits */
#define CRB_GLOB 0 /* Global configuration */
#define CRB_FIX 1 /* Fixed configuration */
#define CRB_LOG 2 /* Logical Data port configuration */
#define CRB_GPSIDS 3 /* GPS ID's */
#define CRB_ROUTES 4 /* Routes */
#define CRB_DEVS 5 /* CNP Devices */
#define CRB_SENSCTRL 6 /* Sensor Control */
/* baler flags */
#define BA_PDOWN 1 /* power down when done */
#define BA_MANUAL 2 /* powered on by command, use access timeout */
#define BA_CMDS 4 /* Commands OK */
#define BA_CFG 8 /* Config block follows */
/* baler ready flags */
#define BR_RQCFG 1 /* Request configuration blockette */
/* Baler Status */
#define BS_AUTO 0x8000 /* Automatic On */
#define BS_MANUAL 0x4000 /* Manual On */
#define BS_CONT 0xC000 /* Continuous */
#define BS_YES 0x2000 /* Is a Baler */
#define BS_OFFTIME 0x1000 /* Forced Off */
#define BS_MASK 0xFFF /* Timeouts */
/* Dialer Status */
#define DS_DIAL 0x8000 /* Is a dialer */
#define DS_DAUTO 0x1800 /* Dialer auto-on */
#define DS_DMAN 0x1000 /* Dialer manual-on */
#define DS_DANS 0x800 /* Dialer answer mode */
#define DS_DOFF 0x4000 /* Locked out */
#define DS_PMASK 0x3E0 /* These bits have dialer phase */
#define DS_TMASK 0x1F /* These bits have timeouts */
/* Clock Quality Flag Bits */
#define CQ_LOCK 1 /* Has been locked, else internal time */
#define CQ_2D 2 /* 2D Lock */
#define CQ_3D 4 /* 3D Lock */
#define CQ_1D 8 /* No fix, but time should be good */
#define CQ_FILT 0x10 /* Filtering in progress */
#define CQ_SPEC 0x20 /* Speculative 1-D startup */
/* PLL State */
#define PLL_OFF 0x00 /* Not on */
#define PLL_HOLD 0x40
#define PLL_TRACK 0x80
#define PLL_LOCK 0xC0
/* Environmental Processor defines */
#define EP_MAXCHAN 128
#define EP_MAXAD 3
#define MAX_SDI 4 /* Status for 1st 4 SDI devices only */
/* Version Info */
#define FIRST_WITH_CURIP 0x138 /* 1.56 is the first with current ip address in interface status */
#define FIRST_WITH_PERC_TRIG 0x14C /* 1.76 is first with percentage trigger capability */
#define FIRST_WITH_BASE96 0x14D /* 1.77 is first with base96 data capability */
#define FIRST_WITH_RPOC 0x159 /* 1.89 is first with random poc source port */
#define FIRST_WITH_SS 0x15E /* 1.94 is first with serial sensor */
/* Communications Ethernet Flags */
#define CEF_DEFAULT     0   /* Use default address */
#define CEF_CONST       1   /* Use constant address */
#define CEF_DHCP        2   /* Use DHCP */
#define CEF_UPINGBLOCK  0x80 /* Blocks pings from unregistered hosts */
#define CEF_UNLOCK      0x400 /* Unlock always */
#define CEF_BLK_ICMP    0x1000 /* Block ICMP Packets */
#define CEF_MTU         0x2000 /* MTU Override is active */
/* Communications Power Cycling Flags */
#define CPF_CONT        0x1000000 /* Continuously powered baler */
#define CPF_MAXOFF      0x2000000 /* At Interval */
#define CPF_INHIBIT     0x4000000 /* Inhibit baler below percentage */
#define CPF_PERC1       0x10000000 /* At data port 1 percentage */
#define CPF_PERC2       0x20000000 /* At data port 1 percentage */
#define CPF_PERC3       0x40000000 /* At data port 1 percentage */
#define CPF_PERC4       0x80000000 /* At data port 1 percentage */
/* Listopts flags */
#define WBL_WHITE       0x8000 /* List is a whitelist instead of blacklist */
#define IP_LIST_SIZE    8   /* White/Blacklist entries */

typedef longword tbauds[10] ;
extern const tbauds bauds ;

/* Clock processing information */
typedef struct {
  longint zone ; /* timezone offset in seconds */
  word degrade_time ; /* loss in lock in minutes before degrading 1% */
  byte q_locked ; /* PLL Locked quality */
  byte q_track ; /* PLL Tracking quality */
  byte q_hold ; /* PLL Holding quality */
  byte q_off ; /* Currently Locked, PLL Off */
  byte q_spare ; /* Spare */
  byte q_high ; /* has been locked highest quality */
  byte q_low ; /* has been locked lowest quality */
  byte q_never ; /* Never been locked quality */
  word clock_filt ; /* minimum seconds between clock messages */
} tclock ;
extern const tclock default_clock ;

typedef word tsix[3] ;
typedef longword t64[2] ; /* sixty four bit fields */
typedef longword t128[4] ; /* 128 bit fields */
typedef word tfreqs[CHANNELS] ;
typedef byte *pbyte ;
typedef char *pchar ;
typedef word *pword ;
typedef longint *plong ;
typedef double tfloat ;
typedef void *pointer ;
typedef byte dms_type[13] ; /* elements 1 .. 12 used for days per month */
/* Following represent null terminated strings */
typedef char string7[8] ;
typedef char string15[16] ;
typedef char string31[32] ;
typedef char string63[64] ;
typedef char string79[80] ;
typedef char string95[96] ;
typedef char string[256] ;

typedef int16 tscaling[CHANNELS][FREQS];
typedef int16 toffsets[CHANNELS];
typedef int16 tgains[CHANNELS];
typedef struct {  /* Format of C1_GLOB */
  word clock_to;   /* Clock Timeout */
  word initial_vco;   /* Initial VCO Value */
  word gps_backup;   /* non-zero to apply backup power */
  word samp_rates;   /* aux. and status sampling rates */
  word gain_map;   /* gain bitmap */
  word filter_map;   /* filter bitmap */
  word input_map;   /* input bitmap */
  word web_port;   /* web server port */
  word server_to;   /* server timeout */
  word drift_tol;   /* Drift Tolerance */
  word jump_filt;   /* jump filter */
  word jump_thresh;   /* jump threshold */
  int16 cal_offset;   /* calibrator D/A offset */
  word sensor_map;   /* sensor control bitmap */
  word sampling_phase, gps_cold;   /* GPS coldstart seconds */
  longword user_tag ; /* User settable value */
  tscaling scaling;
  toffsets offsets;
  tgains gains;
  longword msg_map;   /* message enable bitmap */
} tglobal;
typedef struct { /* Format for C1_FIX */
  longword last_reboot;   /* Time of last reboot */
  longword reboots;   /* Number of reboots */
  longword backup_map;   /* Backup data structure bitmap */
  longword default_map;   /* Default data structure bitmap */
  word cal_type;   /* Calibrator Type */
  word cal_ver;   /* Calibrator Version */
  word aux_type;   /* Aux. Type */
  word aux_ver;   /* Aux. Version */
  word clk_type;   /* Clock Type */
  word flags;   /* Ethernet installed */
  word sys_ver;   /* System software version */
  word sp_ver;   /* Slave processor version */
  word pld_ver;   /* PLD version */
  word mem_block;   /* Memory block size */
  longword property_tag ;
  t64 sys_num;   /* System serial number */
  t64 amb_num;   /* Analog mother board number */
  t64 seis1_num;   /* Seismometer 1 serial number */
  t64 seis2_num;   /* Seismometer 2 serial number */
  longword qapchp1_num ; /* QAPCHP1 serial number */
  longword int_sz ;  /* Internal Data Memory size */
  longword int_used;   /* Internal Data memory used */
  longword ext_sz;   /* External Data memory size */
  longword flash_sz;   /* Flash memory size */
  longword ext_used;   /* External Data memory used */
  longword qapchp2_num ; /* QAPCHP2 serial number */
  longword log_sz[LP_TEL4 - LP_TEL1 + 1];   /* Packet Memory Size */
  byte freq7 ; /* bit 7 frequency */
  byte freq6 ; /* bit 6 frequency */
  byte freq5 ; /* bit 5 frequency */
  byte freq4 ; /* bit 4 frequency */
  byte freq3 ; /* bit 3 frequency */
  byte freq2 ; /* bit 2 frequency */
  byte freq1 ; /* bit 1 frequency */
  byte freq0 ; /* bit 0 frequency */
  longint ch13_delay[FREQS];   /* channels 1-3 delays */
  longint ch46_delay[FREQS];   /* channels 4-6 delays */
} tfixed ;
typedef longword tsensctrl[8];   /* for each bit */
typedef struct {  /* Format of C1_LOG */
  word lport;   /* Logical Port Number */
  word flags;   /* Logical Port flags */
  word perc;   /* Percent of packet buffer * 2.56 */
  word mtu;   /* Size of data packet */
  word grp_cnt;   /* Group Count */
  word rsnd_max;   /* Maximum Resend Timeout */
  word grp_to;   /* Group Timeout */
  word rsnd_min;   /* Minimum Resend Timeout */
  word window;   /* Window Size */
  word dataseq;   /* Data Sequence Number */
  word freqs[CHANNELS];   /* frequency map */
  word ack_cnt;   /* Acknowledge Skip Count */
  word ack_to;   /* Acknowledge Timeout */
  longword olddata;   /* flush threshold */
  word eth_throttle;   /* ethernet throttling for this port */
  word full_alert ;  /* Data Fault LED if buffer over this percentage * 2.56 */
  word auto_filter ; /* Blockettes to block based on SEED channels */
  word man_filter ; /* Blockettes to block based on manual selections */
  longword spare ;
} tlog;
typedef struct {  /* global status */
  word aqctrl;   /* Acquisition Control */
  word clock_qual;   /* Current clock quality */
  word clock_loss;   /* Minutes since lock */
  word current_voltage; /* Current AMB DAC control value */
  longword sec_offset;   /* Seconds offset + data seq + 2000 = time */
  longword usec_offset;   /* Usec offset for data */
  longword total_time;   /* Total time in seconds */
  longword power_time;   /* Total power on time in seconds */
  longword last_resync;   /* Time of last resync */
  longword resyncs;   /* Total number of resyncs */
  word gps_stat;   /* Gps status */
  word cal_stat;   /* Calibrator Status */
  word sensor_map;   /* Sensor control bitmap */
  word cur_vco;   /* Current VCO Value */
  word data_seq;   /* Data sequence number */
  word pll_flag;   /* PLL enabled */
  word stat_inp;   /* Status Inputs */
  word misc_inp;   /* Misc. Input */
  longword cur_sequence ; /* latest digitizer sequence */
} tstat_global;
typedef struct { /* SMU status */
  word phase;   /* Charging Phase */
  int16 battemp;   /* Battery temperature */
  word capacity;   /* Battery capacity */
  word depth;   /* Depth of discharge */
  word batvolt;   /* Battery Voltage */
  word inpvolt;   /* Input Voltage */
  int16 batcur;   /* Battery current */
  word absorption;   /* Absorption setpoint */
  word float_ ;   /* float setpoint */
  byte alerts ; /* bitmap of loads about to be turned off */
  byte loads_off ; /* bitmap of loads currently on */
} tstat_pwr;
typedef struct {  /* GPS status */
  word gpstime;   /* GPS Power on/off time in seconds */
  word gpson;   /* GPS power on if non-zero */
  word sat_used;   /* Number of satellite used */
  word sat_view;   /* Number of satellites in view */
  char time[10];   /* GPS Time */
  char date[12];   /* GPS Date */
  char fix[6];   /* GPS Fix type */
  char height[12];   /* GPS Height */
  char lat[14];   /* GPS Latitude */
  char longt[14];   /* GPS Longitude */
  longword last_good;   /* Time of last good 1PPS */
  longword check_err;   /* Number of checksum errors */
} tstat_gps;
typedef char tgpsid[9][32];   /* GPS IDs */
typedef struct  {  /* Boom positions and other stuff */
  int16 booms[CHANNELS];
  word amb_pos;   /* analog mother board positive - 10mv */
  word amb_neg;   /* analog mother board negative - 10mv */
  word supply; /* input voltage - 150mv */
  int16 sys_temp;   /* system temperature - celsius */
  word main_cur;   /* main current - 1ma */
  word ant_cur;   /* GPS antenna current - 1ma */
  int16 seis1_temp;   /* seismo 1 temperature - celsius */
  int16 seis2_temp;   /* seimso 2 temperature - celsius */
  longword cal_timeouts; /* calibrator timeouts */
  int16 sensa_cur ; /* Sensor A current - 5ma - Q335 only */
  int16 sensb_cur ; /* Sensor B current - 5ma - Q335 only */
} tstat_boom;
typedef struct { /* PLL Status */
  single start_km, time_error, rms_vco, best_vco;
  longint spare;   /* was target */
  longword ticks_track_lock ;   /* ticks since last track or lock */
  int16 km;
  word state;   /* hold/track/lock */
} tstat_pll;

typedef struct {  /* entry for 1 satellite */
  word num;   /* satellite number */
  int16 elevation;   /* elevation in meters */
  int16 azimuth;   /* azimuth in degrees */
  word snr;   /* signal to noise ratio */
} tstat_sat1;
typedef struct {  /* tells how many satellites reported */
  word sat_count;   /* number of satellites */
  word blk_size;   /* size of this block */
} tstat_sathdr;
typedef struct { /* total structure */
  tstat_sathdr sathdr;
  tstat_sat1 sats[MAX_SAT];
} tstat_sats;
typedef struct tstat_log { /* logical port status */
  longword sent;   /* Total Data Packets Sent */
  longword resends;   /* Total Packets re-sent */
  longword fill;   /* Total Fill Packets sent */
  longword seq;   /* Receive Sequence errors */
  longword pack_used;   /* Bytes of packet buffer used */
  longword last_ack;   /* time of last packet acked */
  word phy_num;   /* physical port number used */
  word log_num;   /* logical port we are reporting */
  word retran;   /* retransmission timer */
  word flags;    /* LPSF_xxx flags */
} tstat_log;
typedef struct {  /* serial port status */
  longword check;   /* Receive Checksum errors */
  longword ioerrs;   /* Total I/O errors */
  word phy_num;   /* Physical port we are reporting */
  word spare;
  longword unreach;   /* Destination Unreachable ICMP Packets Received */
  longword quench;   /* Source Quench ICMP Packets Received */
  longword echo;   /* Echo Request ICMP Packets Received */
  longword redirect;   /* Redirection Packets Received */
  longword over;   /* Total overrun errors */
  longword frame;   /* Total framing errors */
} tstat_serial;
typedef struct {  /* ethernet port status */
  longword check;   /* Receive Checksum errors */
  longword ioerrs;   /* Total I/O errors */
  word phy_num;   /* Physical port we are reporting */
  word spare;
  longword unreach;   /* Destination Unreachable ICMP Packets Received */
  longword quench;   /* Source Quench ICMP Packets Received */
  longword echo;   /* Echo Request ICMP Packets Received */
  longword redirect;   /* Redirection Packets Received */
  longword runt;   /* Total runt frames */
  longword crc_err;   /* CRC errors */
  longword bcast;   /* Broadcast frames */
  longword ucast;   /* Unicast frames */
  longword good;   /* Good frames */
  longword jabber;   /* Jabber errors */
  longword outwin;   /* Out the window */
  longword txok;   /* Transmit OK */
  longword miss;   /* Receive packets missed */
  longword collide;   /* Transmit collisions */
  word linkstat;   /* Link status */
  word spare2;
  longword spare3 ;
} tstat_ether;

typedef struct {  /* format of one entry of C1_ARP */
  longword ip;   /* IP address */
  tsix mac;   /* MAC address */
  word timeout;   /* timeout in seconds */
} tarp1;
typedef struct {  /* header for C1_ARP */
  word arp_count;   /* number of entries */
  word blk_size;   /* total size of this block */
} tstat_arphdr;
typedef struct {  /* Entire structure */
  tstat_arphdr arphdr ;
  tarp1 arps[MAXARP] ;
} tstat_arp;

typedef struct {  /* one route entry */
  longword rt_ip;   /* IP address of connected device */
  word rt_pp;   /* Physical port it is attached to */
  word rt_lp;   /* Logical port it talked to */
  longword heard;   /* Seconds since we last heard from this device */
} troute1;
typedef struct { /* Entire structure as reported to host */
  integer count ; /* number of entries */
  troute1 routes[MAXROUTE] ;
} troutelist ;
typedef struct { /* Format of Baler Status for one physical port */
  longword last_on ; /* Time last turned on */
  longword powerups ; /* Total number of power ups since reset */
  word baler_status;   /* BS_XXXX and timeouts */
  word baler_time; /* minutes since baler was activated */
} tsbaler1 ;
typedef tsbaler1 tstat_baler[PP_ETH - PP_SER1 + 1] ; /* baler status */

typedef longword tdyn_ips[PP_ETH - PP_SER1 + 1] ; /* dynamic ip address for each port */
typedef struct { /* AuxAD board programming */
  byte chancount ; /* Number of active channels */
  byte calpga ; /* pga gain to use during calibration */
  word auxchans[9] ; /* per channel programming */
} tauxadcfg ;

typedef struct { /* Format of Aux status header */
  word size ; /* Size in bytes */
  word packver ; /* Version */
  word aux_type;   /* Aux. Type */
  word aux_ver;   /* Aux. Version */
} tauxhdr ;
typedef struct { /* Format of AuxAD packet, worst case */
  tauxhdr hdr ;
  longint conversions[8] ;
} tstat_auxad ;

typedef struct { /* header for serial status */
  word totalsize ; /* in bytes */
  word count ; /* number of sub-blocks */
} tsshdr ;
typedef struct { /* a sub-block for each sensor */
  word size ; /* only of this subblock */
  word sensor_type ; /* 1 for paroscientific pressure */
  word phyport ; /* serial port */
  word sps ; /* seconds per sample */
  word units ; /* engineering units */
  word int_time ; /* integration time in milliseconds */
  word fracdig ; /* fractional digits */
  word validmeas ; /* bit 0 for pressure, bit 1 for temperature */
  longint pressure ; /* pressure measurement */
  longint temperature ; /* temperature measurement */
  longint humidity ; /* humidity in 0.1% increments */
  longint exttemp ; /* external temperature in 0.01C increments */
} tssstat ;
typedef struct { /* Total serial sensor status */
  tsshdr hdr ;
  tssstat sensors[PP_SER2 - PP_SER1 + 1] ;
} tstat_sersens ;

typedef struct {  /* one device entry */
  word dev_addr;   /* device address */
  word dev_id;   /* device id */
  word dev_ver;   /* device version */
  word dev_opt;   /* device option */
  t64 dev_num;   /* serial number */
  word dev_static ; /* device static storage */
  word heard;   /* Seconds since we last heard from this device */
} tdev1;
typedef struct { /* As reported to host */
  word count ;
  tdev1 alldev[MAXDEV];
} tdevs ;

/* The following are not from the Q330 */
typedef struct { /* for gregorian calcs */
  word wyear ;
  word wmonth ;
  word dayofweek ;
  word wday ;
  word whour ;
  word wminute ;
  word wsecond ;
  word wmilliseconds ;
} tsystemtime ;

enum tgps_stat {GPS_OFF, /* GPS is off */
                GPS_OFF_LOCK, /* Off due to GPS Lock */
                GPS_OFF_PLL, /* Off due to PLL Lock */
                GPS_OFF_LIMIT, /* Off due to Time Limit */
                GPS_OFF_CMD, /* Off due to Command */
                GPS_ON, /* On */
                GPS_ON_AUTO, /* Powered on automatically */
                GPS_ON_CMD, /* Powered on by command */
                GPS_COLDSTART} ; /* In cold start */
enum tgps_fix  {GPF_LF, /* GPS Off, never locked */
                GPF_OFF, /* GPS Off, unknown lock */
                GPF_1DF, /* GPS Off, last fix was 1D */
                GPF_2DF, /* GPS Off, last fix was 2D */
                GPF_3DF, /* GPS Off, last fix was 3D */
                GPF_NL, /* GPS On, never locked */
                GPF_ON, /* GPS On, unknown lock */
                GPF_1D, /* 1D Fix */
                GPF_2D, /* 2D Fix */
                GPF_3D, /* 3D Fix */
                GPF_NB} ; /* No GPS board */
enum tpll_stat {PLS_OFF, /* PLL Off */
                PLS_HOLD, /* PLL Hold */
                PLS_TRACK, /* PLL Tracking */
                PLS_LOCK} ; /* PLL Locked */

typedef struct { /* DSS configuration from tokens */
  char high_pass[8] ;    /* highest priority password */
  char mid_pass[8] ;
  char low_pass[8] ;     /* lowest priority password */
  longint timeout ;
  longint max_bps ;
  byte verbosity ;
  byte max_cpu_perc ;
  word port_number ;
  word max_mem ;     /* Max memory in KB */
  word reserved ;
} tdss ;

typedef struct { /* one comm event */
  char name[COMMLENGTH+1] ;
  boolean ison ;
} tonecomm ;
typedef tonecomm tcommevents[CE_MAX] ;

typedef struct { /* Ping request from host */
  word pingtype ; /* 0 for normal ping */
  word pingopt ;
  longword pingreqmap ;
} tpingreq ;
typedef struct { /* Header for type 3 response */
  word drift_tol ;
  word umsg_count ;
  longword last_reboot  ;
  longint spare1 ;
  longint spare2 ;
} tpingstathdr ;
typedef struct { /* Format of type 5 response */
  word version ;
  word flags ;
  longword tag_id ;
  t64 serialnum  ;
  longword packetsizes[LP_TEL4 - LP_TEL1 + 1] ;
  longword triggers[PP_ETH - PP_SER1 + 1] ;
  word advflags[PP_ETH - PP_SER1 + 1] ;
  word dataport[PP_ETH - PP_SER1 + 1] ;
  word calibration_errors ;
  word sys_ver ;
} tpinglimits ;
typedef struct { /* Used internally by library */
  word ping_type ;
  word ping_id ;
} tpingbuffer ;
typedef struct { /* C2_BACK */
  t64 sernum ; /* Q330 serial number */
  longword q330_ip ; /* Q330 IP address */
  longword poc_ip ; /* POC/Baler IP address */
  longword log2_ip ; /* Logger alternative address */
  word bport ; /* Q330 base port */
  word lport ; /* Logical Port number */
  word webbps ; /* web bps / 10 */
  word flags ; /* baler/dial-out flags */
  word access_to ; /* access timeout in seconds */
  word spare2 ;
  t64 balersn ; /* Baler serial number */
  /* Optional baler configuration block */
  word size ; /* Byte count of structure */
  word phyport ; /* physical interface it is tied to */
  word balertype ; /* 44 = Baler44 and Q330/S */
  word version ;
  word opaque[116] ; /* only select portions of willard and the baler know */
} tback ;

typedef struct ttimeouts {
  word idle_timeout ; /* minutes for this data port once packet buffer is empty */
  word busy_timeout ; /* minutes for this data port while packet buffer not empty */
} ttimeouts ;
typedef struct tiplist {
  longword low ;
  longword high ;
} tiplist ;

typedef struct tcomm { /* Format of CPC_COM */
  t64 serial ; /* serial number for broadcast */
  word version ; /* structure version */
  word active_lth ; /* active length */
  word mtu ; /* MTU Override */
  word base_port ; /* UDP/TCP base port */
  longword eth_ip ; /* Ethernet IP address */
  longword eth_mask ; /* Ethernet Netmask */
  longword eth_gate ; /* Ethernet Gateway address */
  longword pwr_cycling ; /* Power Cycling Flags */
  ttimeouts timeouts[LP_TEL4 - LP_TEL1 + 1] ; /* timeouts per data port */
  word triggers[LP_TEL4 - LP_TEL1 + 1] ; /* percentage for each data port */
  word min_off ; /* Minimum Power off time */
  word listopts ; /* Black/Whitelist options */
  word max_off ; /* Maximum Power off time */
  word baler_min_perc ; /* Baler minimum percent */
  tiplist iplist[IP_LIST_SIZE] ; /* white or blacklists */
  longword eth_flags ; /* Ethernet flags */
  word min_on ; /* Minimum On time */
  word spare ;
  byte other_exp[364] ; /* Other expansion area */
} tcomm ;

typedef struct { /* Format of C3_BCFG */
  word sub_command ;
  word sub_response ;
  word size ; /* Byte count of structure */
  word phyport ; /* physical interface it is tied to */
  word balertype ; /* 44 = Baler44 and Q330/S */
  word version ;
  char opaque[236] ; /* only select portions of willard and the baler know */
} tbalecfg ;

typedef struct {
  longword ress[PP_SER2 - PP_SER1 + 1] ;
  word chancnt ; /* number of 32 bit channel entries */
  word spare ;
  longword chandlys[2] ; /* 8 bit channel and 24 filter delay in 5usec inc */
} tepdelay ;
/* SDI Defs */
enum tsdi_phase {SP_UNKNOWN, SP_ENUMADDR, SP_ENUMID, SP_CFGRD, SP_CFGWR, SP_CFGDONE,
                 SP_READY, SP_SAMPLE, SP_SAMPWAIT} ;
enum tsdi_driver {SD_UNKNOWN, SD_WXT520} ;
typedef struct tsdistat { /* for 1 device */
  char address ; /* device address, '0' - '9' */
  enum tsdi_phase phase ; /* see enum definition above */
  enum tsdi_driver driver ; /* Driver ID */
  byte spare1 ; /* for alignment */
  char model[6] ; /* Sensor model */
  char serial[13] ; /* Sensor serial number */
  char spare2 ;
  char version[3] ; /* Sensor version */
  char spare3 ;
} tsdistat ;
enum tep_model {EM_UNKNOWN,   /* DS2431 not set yet */
                EM_BASE1} ;   /* Production prototype boards */
enum tadc_model {AM_NONE,     /* No analog board or DS2431 not set yet */
                 AM_SETRA1,   /* One Analog channel with Setra Interface */
                 AM_GEN1,     /* One Analog channel, general purpose */
                 AM_GEN3} ;   /* Three Analog channels, general purpose */
#define LAST_EP_MODEL EM_BASE1
#define LAST_ADC_MODEL AM_GEN3
typedef struct {
  single start_km ;
  single time_error ;
  single best_vco ;
  longword ticks_track_lock ; /* ticks since last track or lock */
  longint km ;
  word state ; /* hold/track/lock */
  word spare1 ;
  t64 serial ;
  longword procid ;
  longword secs_boot ; /* seconds since boot */
  longword secs_resync ; /* seconds since last resync */
  longword resyncs ; /* Total number of resyncs */
  longword q330_comm_errors ; /* communications errors from 330 */
  longword ep_comm_errors ; /* communications errors from EP */
  word spare2 ;
  word sdi_count ; /* number of SDI-12 devices active */
  word version ; /* firmware version and revision */
  word flags ;
  word analog_chans ; /* Number of analog channels */
  enum tep_model ep_model ;
  byte ep_rev ;
  longword gains ; /* 4 bits per channel */
  word inp_volts ; /* in .1 volt increments */
  word humidity ; /* in percent */
  longword pressure ; /* in ubar */
  longint temperature ; /* in 0.1C increments */
  longint adcounts[4] ;
  tsdistat sdistats[MAX_SDI] ;
  /* Following replaces 5th SDI status in previous versions */
  t64 adc_serial ;
  enum tadc_model adc_model ;
  byte adc_rev ;
  word adc_spare1 ;
  longword adc_spare2 ;
  longword spares[3] ;
} tstat_oneep ;
typedef tstat_oneep tstat_ep[2] ;

typedef struct tfestat { /* Status for one FE */
  single start_km ;
  single time_error ;
  single best_vco ;
  longword ticks_track_lock ; /* ticks since last track or lock */
  longint km ;
  word state ; /* hold/track/lock */
  word flags ; /* Bit 0 means sensor temperature is valid */
  longword secs_resync ; /* seconds since last resync */
  longword resyncs ; /* Total number of resyncs */
  longword secs_boot ; /* seconds since boot */
  longword cp_comm_errors ; /* communications errors from CP */
  word inp_volts ; /* in .1 volt increments */
  word sensor_bitmap ; /* current bitmap */
  word cal_status ; /* what phase it is in */
  int16 sensor_temp ;
  t64 sensor_serial ;
  shortint booms[4] ; /* 3 boom positions plus spare*/
} tfestat ;
typedef struct tfestat_hdr {
  word count ; /* number of boards */
  word lth ; /* status length */
} tfestat_hdr ;
typedef struct tstat_fes {
  tfestat_hdr hdr ;
  tfestat boards[2] ;
} tstat_fes ;

enum tliberr {LIBERR_NOERR, /* No error */
              LIBERR_PERM, /* No Permission */
              LIBERR_TMSERV, /* Port in Use */
              LIBERR_NOTR, /* You are not registered */
              LIBERR_INVREG, /* Invalid Registration Request */
              LIBERR_PAR, /* Parameter Error */
              LIBERR_SNV, /* Structure not valid */
              LIBERR_CTRL, /* Control Port Only */
              LIBERR_SPEC, /* Special Port Only */
              LIBERR_MEM, /* Memory operation already in progress */
              LIBERR_CIP, /* Calibration in Progress */
              LIBERR_DNA, /* Data not available */
              LIBERR_DB9, /* Console Port Only */
              LIBERR_MEMEW, /* Memory erase or Write Error */
              LIBERR_THREADERR, /* Could not create thread */
              LIBERR_BADDIR, /* Bad continuity directory */
              LIBERR_REGTO, /* Registration Timeout */
              LIBERR_STATTO, /* Status Timeout */
              LIBERR_DATATO, /* Data Timeout */
              LIBERR_NOSTAT, /* Your requested status is not yet available */
              LIBERR_INVSTAT, /* Your requested status in not a valid selection */
              LIBERR_CFGWAIT, /* Your requested configuration is not yet available */
              LIBERR_INVCFG, /* Your can't set that configuration */
              LIBERR_TOKENS_CHANGE, /* Tokens Changed */
              LIBERR_INVAL_TOKENS, /* Invalid Tokens */
              LIBERR_BUFSHUT, /* Shutdown due to reaching buffer percentage */
              LIBERR_CONNSHUT, /* Shutdown due to reaching buffer percentage */
              LIBERR_CLOSED, /* Closed by host */
              LIBERR_NETFAIL, /* Networking Failure */
              LIBERR_TUNBUSY, /* Tunnelling in progress */
              LIBERR_INVCTX} ; /* Invalid Context */
enum tlibstate {LIBSTATE_IDLE, /* Not connected to Q330 */
                LIBSTATE_TERM, /* Terminated */
                LIBSTATE_PING, /* Un-registered Ping, returns to LIBSTATE_IDLE when done */
                LIBSTATE_CONN, /* TCP Connect wait */
                LIBSTATE_ANNC, /* Announce baler */
                LIBSTATE_REG, /* Requesting Registration */
                LIBSTATE_READCFG, /* Reading Configuration */
                LIBSTATE_READTOK, /* Reading Tokens */
                LIBSTATE_DECTOK, /* Decoding Tokens and allocating structures */
                LIBSTATE_RUNWAIT, /* Waiting for command to run */
                LIBSTATE_RUN, /* Running */
                LIBSTATE_DEALLOC, /* De-allocating structures */
                LIBSTATE_DEREG, /* De-registering */
                LIBSTATE_WAIT} ; /* Waiting for a new registration */
#endif
