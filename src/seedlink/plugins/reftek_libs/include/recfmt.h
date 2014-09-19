#pragma ident "$Id: recfmt.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  :	Any
 Task     :	
 File     : RECFMT.H
 Purpose  : PASSCAL recording format data packet definition and related typedefs
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
             2626 Lombardy Lane, Suite 105
             Dallas, Texas  75220  USA
             (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
 14-Sep-2005, PLD change max streams from 8 to 9
 03-Sep-2005, PLD
    Initial (crude) support for filter description pkts (FD)
    Initial support for Steim2 compression:
       Increase max samples to 1561
       New data type
 11-Feb-1992, RLB
    Packets for DAS versions <= 2.46.
 28-Oct-1992, RLB
    Packets for DAS version 2.47, EH implemented, ET has major
    changes, minor changes to other structures.
    Restructured, nested structures are arrays and/or unions.
    Fields marked <reserved in 2.47> are no longer defined in
    version 2.47 but are retained for backward compatability.
    FD is not defined in 2.47 but as it does not conflict with
    other structures, it is retained.
 23-Apr-1993, RLB
    Conforms with version 2.50 DAS code.  Added DR, updated EH and
    added compressed data to DT.  Also added disk and tape format
    block typedefs.
 22-Sep-1993, RLB
    Changes all types to stdtypes.
  6-Nov-1996, RLB
    Updated to 120 format, dropped FD and other obsolete fields

-------------------------------------------------------------------- */

#ifndef _RECFMT_
#define _RECFMT_

#include "stdtypes.h"

/* Undefined integer types */
#ifndef VOID_INT8
#   define VOID_INT8               -127    /* For signed types on ones's complement machines */
#   define VOID_INT16            -32767    /* this is the largest negative value, on two's complement */
#   define VOID_INT32       -2147483647    /* machines, it the largest negative value plus one. */
#   define VOID_INT64 -9223372036854775807
#   define VOID_UINT8              0xFF    /* For unsigned types this is simply the largest possible */
#   define VOID_UINT16           0xFFFF    /* value for each type. */
#   define VOID_UINT32       0xFFFFFFFF
#   define VOID_UINT64 0xFFFFFFFFFFFFFFFF
#endif

/* Constants ----------------------------------------------------------- */

#define NO_PACKET 0							/* Packet types */
#define AD        1
#define CD        2
#define DR        3
#define DS        4
#define DT        5
#define EH        6
#define ET        7
#define OM        8
#define NH        9
#define NT        10
#define SC        11
#define SH        12
#define FD        13
#define MAX_RF_PACKET_TYPE   13

#define RF_PACKET_SIZE     1024			/* RECFMT packet size */
#define RF_MAX_UNIT      0xFFFE
//#define RF_MAX_STREAMS        9
#define RF_MAX_STREAMS       10			/* PLDSTRM: really want just 9 */
#define RF_MAX_CHANNELS      16
#define RF_MAX_SAMPLES     1561
#define RF_MAX_EVENT_NO    9999
#define RF_MAX_SEQ_NO	   9999

#define DT_NONE               0			/* Data types */
#define DT_16BIT              1
#define DT_32BIT              2
#define DT_COMP               3
#define DT_COMP2              4

#define IVOID            -32767

/* Typedefs ----------------------------------------------------------- */
typedef UINT8 BCD;							/* Binary-coded-decimal */

/* Format blocks */
typedef union _RF_FMTBLK
	{
	struct _DISK_FMT
		{
		UINT32 wri_sec;						/* Current write location */
		CHAR label[16];						/* Volume label */
		UINT32 wrap_cnt;					/* Wrap count; */
		UINT32 rd_sec;						/* Current read location */
		UINT32 cpy_sec;						/* RT44D copy sector */
		UINT32 leod;						/* Logical end-of-data  */
		UINT32 first_dr;					/* First directory packet location */
		UINT32 last_dr;						/* Last directory packet location */
		UINT32 overwrite;					/* Overwrite flag, 0=disable */
		UINT32 n_dir_blk;					/* Number of directory blocks */
		UINT32 n_dir_ent;					/* Number of directory entries */
		UINT32 swi_sec;						/* Actual end of data on disk switch */
		UINT32 peod;						/* Physical end-of-disk */
		UINT8 reserved[960];				/* All set to 0 */
		} disk;
	struct _TAPE_FMT
		{
		CHAR label[16];						/* Volume label */
		UINT32 type;						/* 0 or 32=Exabyte, 1=DDS */
		UINT8 reserved[1004];				/* All set to 0 */
		} tape;
	} RF_FMTBLK;

/* RECFMT data packet ------------------------------------------------- */
typedef struct _RF_PACKET
	{

	/* Packet Header, first 16 bytes of all packets -------------------- */
	struct _PH
		{
		CHAR type[2];						    /* Packet type (ASCII) */
		BCD exp;							    /* Experiment number (2 digit BCD) */
		BCD year;							    /* Year (2 digit BCD) */
		UINT16 unit;							/* Unit id number (NBO) */
		BCD time[6];							/* Time (BCD DDDHHMMSSTTT) */
		BCD bpp[2];								/* Bytes per packet (4 digit BCD) */
		BCD packet[2];							/* Packet sequence number (4 digit BCD) */
		} hdr;

	/* Union of all packet types --------------------------------------- */
	union
		{

		/* AD - Auxiliary data ------------------------------------------ */
		struct _AD
			{
			UINT8 reserved[2];
			CHAR chans[16];					    /* Active channels (ASCII) */
			CHAR samp_per[8];					/* Sample period, seconds (ASCII) */
			CHAR data_type[2];				    /* 16, 32 or C0 (ASCII) */
			CHAR rec_len[8];					/* Record length (ASCII) */
			} ad;

		/* CD - Calibration definition ---------------------------------- */
		struct _CD
			{
			CHAR time[14];						/* Start time (YYYYDDDHHMMSS) */
			CHAR interval[8];					/* Repeat interval (DDHHMMSS) */
			CHAR n_int[4];						/* # of intervals (ASCII) */
			CHAR length[8];					    /* Length, seconds (ASCII) */
			CHAR step[4];						/* On/Off (ASCII) */
			CHAR st_period[8];				    /* Seconds (ASCII) */
			CHAR st_size[8];					/* Seconds (ASCII) */
			CHAR st_amp[8];					    /* Volts (ASCII) */
			CHAR st_sel[4];					    /* Coil/Ampl select (ASCII) */
			} cd;

		/* DR - Directory block ----------------------------------------- */
		struct _DR
			{
			UINT32 prev;						/* Previous DR block */
			UINT32 next;						/* Next DR block */
			UINT8 reserved;
			UINT8 sectors;						/* Blocks/sectors flag, 0=blocks, 1=sectors */
			UINT16 entries;						/* Number of directory entries */
			UINT32 last_ent;					/* Last directory entry */
			struct _DRE
				{
				UINT32 entry_num;				/* Entry number */
				UINT8 entry_stat;				/* Entry status */
				UINT8 entry_fmt;				/* Entry format, 0=standard entry */
				UINT16 entry_len;				/* Entry length, 80 for standard entry */
				UINT16 unit_id;					/* Unit ID */
				UINT16 evn_num;					/* Event number, 0=SOH */
				UINT8 eh_flag;					/* Event starts here */
				UINT8 et_flag;					/* Event ends here */
				UINT16 str_num;					/* Stream number, 65535=SOH */
				CHAR str_name[8];				/* Stream name (ASCII), 'SOH     '=SOH */
				CHAR trig_type[4];			    /* Trigger type (ASCII) 'SOH '=SOH */
				UINT32 tr_sec;					/* Trigger time */
				UINT32 is_sec;					/* Initial sample time */
				UINT32 dt_sec;					/* Detrigger time */
				UINT32 ls_sec;					/* Last sample time */
				UINT32 re_sec;					/* Record (entry) time */
				UINT16 tr_msec;
				UINT16 is_msec;
				UINT16 dt_msec;
				UINT16 ls_msec;
				UINT16 re_msec;
				UINT16 data_type;				/* Data type */
				UINT32 start;					/* Start of data */
				UINT32 n_blks;					/* Number of blocks at this location */
				UINT32 n_errs;					/* Number of errors in these blocks */
				UINT8 reserved[8];
				} de[7];
			} dr;

		/* DS - Data stream definition ---------------------------------- */
		/* Note that this is just an array of 4 data stream info structs */
		struct _DS
			{
			CHAR str_num[2];					/* Stream number (ASCII) */
			CHAR str_name[24];				    /* Stream name (ASCII) */
			CHAR channels[16];				    /* Channels included (ASCII) */
			CHAR rate[4];						/* Sample rate (ASCII) */
			CHAR data_type[2];				    /* Data type (ASCII) */
			UINT8 reserved[16];
			CHAR trig_type[4];				    /* Trigger type  (ASCII) */
			/* Union of all trigger information -------------------------- */
			union
				{
				/* CON - Continuous trigger information ------------------- */
				struct _CON
					{
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					UINT8 reserved[154];		/* Fill, required to fill out union */
					} con;
				/* CRS - Cross-Stream trigger information ----------------- */
				struct _CRS
					{
					CHAR tr_str_num[2];		    /* Trigger stream number (ASCII) */
					CHAR pre_tr_len[8];		    /* Pre trigger length, seconds (ASCII) */
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					} crs;
				/* EVT - Event trigger information ------------------------ */
				struct _EVT
					{
					CHAR tr_chans[16];		    /* Trigger channels (ASCII) */
					CHAR min_chans[2];		    /* Minimum channels (ASCII) */
					CHAR tr_window[8];		    /* Trigger window (ASCII) */
					CHAR pre_tr_len[8];		    /* Pre trigger length, seconds (ASCII) */
					CHAR post_tr_len[8];		/* Post trigger length, seconds (ASCII) */
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					UINT8 reserved[8];
					CHAR sta_len[8];			/* STA length, seconds (ASCII) */
					CHAR lta_len[8];			/* LTA length, seconds (ASCII) */
					CHAR mean_rem[8];			/* Mean removal, seconds (ASCII) */
					CHAR tr_ratio[8];			/* Trigger ratio (ASCII) */
					CHAR de_tr_ratio[8];		/* De-trigger ratio (ASCII) */
					CHAR lta_hold[8];			/* LTA hold (ASCII) */
					} evt;
				/* EXT, RAD - External or Radio trigger information ------- */
				struct _EXT
					{
					CHAR pre_tr_len[8];		    /* Pre trigger length, seconds (ASCII) */
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					} ext;
				/* LVL - Level trigger information ------------------------ */
				struct _LVL
					{
					CHAR tr_level[8];			/* Trigger level (ASCII) */
					CHAR pre_tr_len[8];		    /* Pre trigger length, seconds (ASCII) */
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					} lvl;
				/* TIM - Time trigger information ------------------------- */
				struct _TIM
					{
					CHAR time[14];				/* Start time (YYYYDDDHHMMSS) */
					CHAR interval[8];			/* Repeat interval (DDHHMMSS) */
					CHAR n_int[8];				/* # of intervals (ASCII) */
					CHAR rec_len[8];			/* Record length, seconds (ASCII) */
					} tim;
				} trg;
			} ds[4];

		/* DT - Data ---------------------------------------------------- */
		struct _DT
			{
			BCD event_num[2];					/* Event number (4 digit BCD) */
			BCD stream;							/* Stream number (2 digit BCD) */
			BCD channel;						/* Channel number (2 digit BCD) */
			BCD len[2];							/* Length in samples (4 digit BCD) */
			UINT8 reserved;
			BCD data_type;						/* Data type (2 digit BCD; 16, 32 or C0) */
			/* Sample data in various formats */
			union
				{
				INT8 udata[1000];			    /* Individual bytes */
				INT16 wdata[500];				/* 16 bit signed two's complement, big endian byte order */
				INT32 ldata[250];				/* 32 bit signed two's complement, big endian byte order */
				/* Steim1 compressed sample data, 223 (worst case) to 892 (best case) 32 bit samples */
				struct _CDT
					{
					UINT8 fill[40];			    /* Filler bytes */
					struct _FRM
						{
						UINT32 c;				/* 16 two bit codes */
						union
							{
							INT8 b[4];			/* 4 one byte differences */
							INT16 w[2];			/* 2 two byte differences */
							INT32 l;			/* 1 four byte difference */
							} s[15];			/* 15 segments */
						} frm[15];				/* 15 frames */
					} cdata;
				} data;
			} dt;

		/* EH/ET - Event header / trailer ------------------------------- */
		struct _EH
			{
			BCD event_num[2];					/* Event number (4 digit BCD) */
			BCD stream;							/* Stream number (2 digit BCD) */
			UINT8 reserved[4];
			BCD data_type;						/* Data type (2 digit BCD, 16, 32, C0) */
			CHAR tr_message[33];				/* Trigger time message */
			UINT8 fill[7];						/* 7 spaces */
			CHAR str_name[24];		    		/* Stream name */
			CHAR rate[4];						/* Sample rate (ASCII) */
			CHAR trig_type[4];			    	/* Trigger type (ASCII) */
			CHAR tr_time[16];					/* Trigger time (YYYYDDDHHMMSSTHS) */
			CHAR ist[16];						/* Initial sample time (YYYYDDDHHMMSSTHS) */
			CHAR de_tr_time[16];				/* Detrigger time (YYYYDDDHHMMSSTHS) */
			CHAR lst[16];						/* Last sample time (YYYYDDDHHMMSSTHS) */
			CHAR nom_bit_volts[16][8];		/* Bit count voltage (ASCII) */
			CHAR true_bit_volts[16][8];	/* Bit count voltage (ASCII) */
			CHAR gain[16];						/* Gain */
			CHAR ad_res[16];					/* A/D resolution */
			CHAR fsa[16];						/* full scale analog */
			CHAR seed_code[16][4];
			CHAR sensor_fsa[16];
			CHAR sensor_vpu[16][6];			/* volts per unit */
			CHAR sensor_units[16];
			CHAR rsvd2[206];
			CHAR st_comment[40];				/* station comment */
			CHAR filter_list[16];
			CHAR position[26];				/* gps position */
			CHAR rsvd3[80];
			
			} eh;

		/* 120 EH/ET - Event header / trailer --------------------------- */
		struct _EH120
			{
			BCD event_num[2];					/* Event number (4 digit BCD) */
			BCD stream;							/* Data class / stream number (2 digit BCD) */
			BCD event_msb;						/* Most significant 2 digits of event number (2 digit BCD) */
			UINT8 reserved[3];
			BCD data_type;						/* Data type (2 digit BCD, 16, 32, C0) */
			CHAR tr_message[33];				/* Trigger time message */
			UINT8 fill[7];						/* 7 spaces */
			CHAR str_name[24];				    /* Stream name */
			CHAR rate[8];						/* Sample rate (ASCII) */
			CHAR tr_time[16];					/* Trigger time (YYYYDDDHHMMSSTHS) */
			CHAR ist[16];						/* Initial sample time (YYYYDDDHHMMSSTHS) */
			CHAR de_tr_time[16];				/* Detrigger time (YYYYDDDHHMMSSTHS) */
			CHAR lst[16];						/* Last sample time (YYYYDDDHHMMSSTHS) */
			CHAR bit_volt[8];					/* Bit count voltage (ASCII) */
			UINT8 reserved1[824];
			CHAR sensor[8];					    /* DAU sensor description */
			} eh120;

		/* OM - Operating mode ------------------------------------------ */
		struct _OM
			{
			CHAR state[2];						/* Power state (SL or CP) */
			CHAR mode[2];						/* Recording mode (RM, SC or SR) */
			INT8 reserved[28];
			/* Wake-up sequence information ------------------------------ */
			struct _WAK
				{
				CHAR seq_num[2];				/* Sequence number (ASCII) */
				CHAR time[12];					/* Start time (YYYYDDDHHMM) */
				CHAR duration[6];				/* Power duration (DDHHMM) */
				CHAR interval[6];				/* Repeat interval (DDHHMM) */
				CHAR n_int[2];					/* # of intervals (ASCII) */
				UINT8 reserved[36];
				} wake[8];
			} om;

		/* NH/NT - Network event header / trailer ----------------------- */
		struct _NH
			{
			BCD event_num[2];					/* Event number (4 digit BCD) */
			BCD stream;							/* Data class / stream number (2 digit BCD) */
			BCD event_msb;						/* Most significant 2 digits of event number (2 digit BCD) */
			INT8 reserved[44];
			CHAR str_name[24];				    /* Data class / stream name */
			CHAR rate[8];						/* Sample rate (ASCII) */
			CHAR tr_time[16];					/* Trigger time (YYYYDDDHHMMSSTHS) */
			CHAR ist[16];						/* Initial sample time (YYYYDDDHHMMSSTHS) */
			CHAR de_tr_time[16];				/* Detrigger time (YYYYDDDHHMMSSTHS) */
			CHAR lst[16];						/* Last sample time (YYYYDDDHHMMSSTHS) */
			INT8 reserved1[832];
			CHAR xmit_freq[8];				    /* CRU transmit freq */
			CHAR xmit_site[8];				    /* CRU transmit site */
			INT8 reserved2[4];
			CHAR cru_prop_delay[4];			    /* CRU propagation delay */
			CHAR cru_data_chn[2];			    /* CRU data channel */
			CHAR cru_seq[4];					/* CRU sequence */
			CHAR reserved3[2];
			} nh;

		/* SC - Station/Channel ----------------------------------------- */
		struct _SC
			{
			CHAR ex_hdr[2];					    /* Experiment header (ASCII) */
			CHAR ex_name[24];					/* Experiment name (ASCII) */
			CHAR ex_com[40];					/* Experiment comment (ASCII) */
			CHAR st_num[4];					    /* Station number (ASCII) */
			CHAR st_name[24];					/* Station name (ASCII) */
			CHAR st_com[40];					/* Station comment (ASCII) */
			CHAR das_mod[12];					/* DAS model number (ASCII) */
			CHAR das_ser[12];					/* DAS serial number (ASCII) */
			CHAR ex_start[14];				    /* Experiment start time (YYYYDDDHHMMSS) */
			CHAR clk_type[4];					/* Clock type (ASCII) */
			CHAR clk_ser[10];					/* Clock serial number (ASCII) */
			/* Channel information --------------------------------------- */
			struct _CHN
				{
				CHAR num[2];					/* Channel number (ASCII) */
				CHAR name[10];					/* Channel name (ASCII) */
				CHAR azim[10];					/* Azimuth (ASCII) */
				CHAR incli[10];				    /* Inclination (ASCII) */
				CHAR local_x[10];				/* Local X coord (ASCII) */
				CHAR local_y[10];				/* Local Y coord (ASCII) */
				CHAR local_z[10];				/* Local Z coord (ASCII) */
				CHAR xy_type[4];				/* Data unit for X & Y */
				CHAR z_type[4];				    /* Data unit for Z */
				CHAR gain[4];					/* Preamp gain (ASCII, Magnification) */
				CHAR sens_model[12];			/* Sensor model (ASCII) */
				CHAR sens_ser[12];			    /* Sensor serial number (ASCII) */
				CHAR comment[40];				/* Comments (ASCII) */
				CHAR bit_volt[8];				/* Bit count voltage (ASCII) */
				} chn[5];
			} sc;

		/* SH - State of health ----------------------------------------- */
		struct _SH
			{
			INT8 reserved[8];
			CHAR data[1000];					/* Entries take the form: DDD:HH:MM:SS followed */
			/* by a <SP>, the entry, and a <CR><LF>.  An */
			/* entry may span two packets. */
			} sh;

		/* FD - Filter Description  -------------------------------------- */
		struct _FD
			{
			CHAR data[1008];					/* don't specify for now */
			} fd;
		} pac;
	} RF_PACKET;

/* This struct is filled by RCDecodeHeader(  ) */
typedef struct RF_HEADER
	{
	UINT8 type;									/* ^ Packet type */
	UINT8 stream;								/* 1 Stream number */
	UINT8 channel;								/* 2 Channel number */
	UINT8 data_type;							/* 3 Data type, see DT_???? constants above */
	REAL64 time;								/* ^ Packet time stamp (MS_TIME) */
	UINT16 unit;								/* ^ Unit ID number (host byte order) */
	UINT16 length;								/* 3 Number of samples */
	UINT16 seq;									/* ^ Packet sequence number */
	UINT16 bytes;								/* ^ Bytes in packet */
	UINT16 exp;									/* ^ Experiment number */
	UINT16 evn_no;								/* 1 Event number */
	} RF_HEADER;

/* ^ = Fields in packet header, always defined */
/* 1 = DT, EH, and ET packets only, otherwise = IVOID */
/* 2 = -1 for EH, -2 for ET, chn for DT, otherwise = IVOID */
/* 3 = DT packets only, otherwise = DT_NONE */


/* Globals ------------------------------------------------------------ */

#ifndef _RECFMT_C
#define _RECFMT_C extern
_RECFMT_C CHAR PacketCodes[14][2];
_RECFMT_C CHAR *DataTypes[4];
#else
CHAR PacketCodes[MAX_RF_PACKET_TYPE+2][2] = {
	"??",											/* 0  Unrecognnized packet */
	"AD",											/* 1  Auxiliary data packet */
	"CD",											/* 2  Calibration definition packet */
	"DR",											/* 3  Directory packet */
	"DS",											/* 4  Data stream definition packet */
	"DT",											/* 5  Data packet */
	"EH",											/* 6  Event header packet */
	"ET",											/* 7  Event trailer packet */
	"OM",											/* 8  Operating mode packet */
	"NH",											/* 9  Network event header */
	"NT",											/* 10 Network event trailer */
	"SC",											/* 11 Station/channel definition packet */
	"SH",											/* 12 State-of-health packet */
	"FD",											/* 13 Filter description packet */
	"\0"
	};
CHAR *DataTypes[5] = {
    "?Data?",
    " 16Bit",
    " 24Bit",
    "24Comp",
    "24Cmp2"
    };

#endif

/* Prototypes --------------------------------------------------------- */
_RECFMT_C UINT8 RFPacketType( RF_PACKET * pac );
_RECFMT_C BOOL IsDataPacket( UINT8 type );
_RECFMT_C UINT16 ExpectedSeqNumber( UINT16 seq );
_RECFMT_C REAL64 RFPacketTime( RF_PACKET * pac );
_RECFMT_C VOID RFDecodeHeader( RF_PACKET * pac, RF_HEADER * hdr );
_RECFMT_C VOID Decode( CHAR * string, CHAR * fmt, VOID * val );
_RECFMT_C CHAR *UnBCD( BCD * dest, BCD * source, INT16 sbytes );

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:59  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.4  2005/09/03 21:52:29  pdavidson
 *
 * Minimal modifications to support Steim2 recording format, 0.1 sps sample
 * rate and FD packets.
 *
 * Revision 1.3  2004/08/11 14:50:04  rstavely
 * Changes for version 1.19 Arc utils include multi units & streams, & alignment for
 * arcfetch.
 * Add Rtptrig utility based on rtpftp.
 *
 * Revision 1.2  2002/01/18 17:49:01  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
