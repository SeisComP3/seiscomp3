/************************************************************************/
/*  Include file from Adebahr DRM data acquisition systems.		*/
/************************************************************************/

/*	$Id: drm_seismo.h 2 2005-07-26 19:28:46Z andres $ 	*/

#ifndef	__seismo_h
#define	__seismo_h

/* Headerfile for "seismo": <all> */

/* general definitions ---------------------------------------------- */

#define	ETC_SEISDIR	"/etc/seisdir"	/* file holds dir for application */
#define	CONFIG_FILE	"CONFIG.DAT"	/* general configuration file */
#define	STATION_FILE	".stations"	/* list of all remote stations */

#define	DATADIR_NUM	3		/* max. number of data directories */
#define	USERDIR_NUM	3		/* max. number of user directories */

#define	SEC_IN_DAY	(60*60*24)	/* seconds of a day */

#define	CLI_HELPDIR	"CLIHELP"	/* Helpdir for Cli-menues */

#define	ETC_PASSWD	  "/etc/passwd"	/* name of /etc/passwd */
#define	ETC_GROUP	  "/etc/group"	/* name of /etc/group */

/* login texts for users in etc_passwd file. The last 5 dots are options */
#define	SEISMO_USER	  "SEISMO_USER__....." 	/* normal seismo user */
#define	SEISMO_GUEST	  "SEISMO_GUEST_....."	/* guest user */
#define	SEISMO_UGLEN	13		/* 13 Bytes to compare max. ^^ */

/* defined options: */
#define	MAX_OPTION	5		/* number of option characters */
#define	FULL_OPTION	"D.XBC"		/* super user full option */
#define	NONE_OPTION	".D..."		/* restricted options */
#define	DIAL_OPTION	0		/* D if user can dial out, else dot */
#define	DAYS_OPTION	1		/* max age of files D,W,M,Y,. = unlim */
#define XFER_OPTION	2		/* X = transfer files option */
#define	BATCH_OPTION	3		/* B = batch file option */
#define	CRON_OPTION	4		/* C = crontab option */


/* user status code */
#define IS_SUPER_USER	0		/* super user status */
#define	IS_NORMAL_USER	1		/* normal user status */
#define	IS_GUEST_USER	2		/* guest user status */

/* DA definitions --------------------------------------------------- */

/* record_types coming from DA-processor */

#define	EMPTY			0	/* empty record (sync packet) */
#define	RECORD_HEADER_1		1	/* data packet header */
#define	RECORD_HEADER_2		2	/* dto. but compression level 2 */
#define	CLOCK_CORRECTION	3	/* clock correction granted */
#define	COMMENTS		4	/* comments packet */
#define	DETECTION_RESULT	5	/* detection packet header */
#define	GOES_MINIHEADER 	6	/* he? */
#define	GOES_DESCRIPTOR		7	/* he? */

#define	DPCOMMO_SHUTDOWN	99	/* shutdown dpsample immediately */




/* clock_correction_types for CLOCK_CORRECTION packets */
/*::	doug@perry.berkeley.edu, per Joe Steim:		*/
/*::	EXPECTED_TIMEMARK == 4				*/
/*::	DAILY_TIME_CORRECTION_REPORT == 5		*/

#define	NO_CLOCK_CORRECTION	0	/* no timemark ever seen so far */
#define	MISSING_TIMEMARK	1	/* no timemark */
#define	EXPECTED_TIMEMARK	2	/* not an error */
#define	VALID_TIMEMARK		3	/* not an error */
#define	UNEXPECTED_TIMEMARK	4	/* unexpected timemark */
#define	DAILY_TIME_CORRECTION_REPORT	5	/* once a day for doc purp. */


/* comment types */

#define	CMT_CLOCK_CORRECTION	127	/* added because event -> comment */
#define CMT_TIME_GAP		126	/* time gap in continues data */

/* other comment types yet not known !!! (he?) */



/* event_detector_types */
/*:: Removed due to conflict with comserv defines - not used by qlib drm.
#define	ANY_DETECTOR		0
#define MURDOCK_HUTT		1
#define THRESHOLD		2
::*/

/* state of health byte */

	/* inaccurate time tagging (no time marks), in SOH */
#define	SOH_INACCURATE			0x80
	/* time gap detected, in SOH */
#define	SOH_GAP				0x40
	/* record contains event data */
#define	SOH_EVENT_IN_PROGRESS		0x20
	/* record is first of event sequence */
#define SOH_BEGINNING_OF_EVENT		0x10
	/* record contains calibration data */
#define SOH_CAL_IN_PROGRESS		0x08
	/* too many time marks received during this record */
#define	SOH_EXTRANEOUS_TIMEMARKS	0x04
	/* record is time-tagged at a mark */
#define SOH_EXTERNAL_TIMEMARK_TAG	0x02
	/* last record containing calibration data */
#define	SOH_RECEPTION_GOOD		0x01


/* control-codes in packets from DA-processor */

#define	NUL				0	/* nul code */
#define	SOH				1	/* 1st byte of sync */
#define	ETX				3	/* end of packet */
#define	SYN				0x15	/* 2nd byte of sync */
#define	DLE				0x10	/* escape special codes */




/* currently we support only one command to the DA-processor: */

#define	ACK_MSG				48	/* acknowledge command */
#define LEADIN_ACKNAK			'Z'	/* data for ack-packet */



/* CRC-polynomial for DA-check */
#define	CRC_POLYNOMIAL			0x56070368L

#define	FRAMES_TOTAL	8		/* FRAMES_PER_DA+1 = tot. packet size */
#define	FRAMES_PER_DA	7		/* max. data frames per packet */
#define	FRAMES_PER_SEED	63		/* frames in SEED-record */

	/* NOTE: FRAMES_PER_SEED and FRAMES_PER_DA must be dividable!
		 63/7 = 9. See tools/seedconv.c why */

#define	MAXBYTES_TILLSYNC  1024		/* max. number of bytes till SYNC */
#define	SEED_REC_SIZE	4096		/* size of SEED record */


	/* highest allowable packet number */
#define	MAXSEQ			7
#define	MODULUS			(MAXSEQ + 1)

	/* for DP reset: seqnum is MODULUS for init-packet */
#define	DP_COLDSTART_SEQ_NUM	MODULUS

	/* ctrl code in DA_TO_DP_BUFFER */
#define	SEQUENCE_CONT		0	/* continue in sequence */
#define	SEQUENCE_RESET		1	/* take next seq.number without chk */


#define	DA_COMSTRLEN	190	/* length of incoming comment string */
#define	DP_COMSTRLEN	192	/* length of comment-strings stored */



/* Steim-Compression bit patterns for difference lengths */

#define	D4_BYTE		0x03	/* 1 4 byte difference */
#define	D2_BYTE		0x02	/* 2 2 byte differences */
#define	D1_BYTE		0x01	/* 4 1 byte differences */
#define	IG_BYTE		0x00	/* ignore that item */




/* DP-Definitions -------------------------------------------------- */



/* Art der Kommunikation zwischen DP und DA-Prozessor */ 

#define	DA_COM_TTY	'T' /* tty-line	*/
#define	DA_COM_ETH	'E' /* TCP/IP	*/
#define	DA_COM_MSG	'Q' /* via message queues	*/


#define	RESTART_MAX	10	/* max. restart count for died processes */

/* Magic Number for all Continues Data, Event-Data and Log-Data files */
#define	STORE_MAGIC	663541234L

/* storage file type */
#define	STYPE_CONT	1	/* continues data */
#define	STYPE_EVENT	2	/* events */
#define	STYPE_LOG	3	/* log data */

#define	MAX_STORETAB	90	/* max. number of concurrently open tables.
				   NOTE: the unix-files should be 100/process */

/* status of storage file */
#define	ST_ACTIVE	1	/* file is in use by dpsample */
#define	ST_DETACH	0	/* file is detached (no more in use) */
#define	ST_SAVED	2	/* file is archived on tape */



/* for dpsample debugging */
#ifdef	DEBUG_DPSAMPLE
extern	char	Verbosity;	/* verbosity level */
#define	VERBOSE(s) if(Verbosity)strout(s)
#define	VERBOSE2(s) if(Verbosity>1)strout(s)
#define VERBOSE3(s) if(Verbosity>2)strout(s)
#else	
#define	VERBOSE(s) 
#define	VERBOSE2(s) 
#define	VERBOSE3(s) 
#endif


/* startup commands */
#define	DPSAMPLE_CMD	"dpsample da=%|s v=0 >> %{^|s/DPSAMPLE.LOG"
#define	DPCOMMO_CMD	"dpcommo da=%|s v=0 >> %{^|s/DPCOMMO.LOG"

#define	DPC_TO_DPS_MSGTYPE	1	/* message type for dp-commo */

#define	DA_SYNC_TIME	300	/* give 5 minutes to signal DA is down */
#define	DA_UPD_TIME	30	/* update da_alive twice every minute */


/* x-axis resolutions for graphic display of continues data */

#define	XT_10S		10	/* 10 seconds */
#define	XT_30S		30	/* 30 seconds */
#define	XT_1M		60	/* 1 minute */
#define	XT_2M		120	/* 2 minutes */
#define	XT_5M		300	/* 5 minutes */
#define	XT_10M		600	/* 10 Minutes */
#define	XT_30M		1800	/* 30 Minutes */
#define	XT_1H		3600	/* 1 hour */
#define	XT_2H		7200	/* 2 hours */
#define	XT_6H		21600	/* 6 hours */
#define	XT_12H		43200	/* 12 hours */
#define	XT_24H		86400	/* 24 hours, i.e. 1 day */


#define	GRN_SAMPMAX	20480	/* max. number of values per TEK-display */

#define	GRN_MAXDECIM	100	/* max. decimation factor for TEK-display */

#define	MAX_FIRCHAIN	4	/* max. 4 chained decimation filters */


/* ---------- file types in private directory ------------------- */

#define	ALL_FILE_TYPE	0xff	/* all file types */
#define	SEED_FILE_TYPE	0x01	/* SEED files */
#define	AGSE_FILE_TYPE	0x02	/* AGSE files */
#define	DETC_FILE_TYPE	0x04	/* list of detections */
#define	OPST_FILE_TYPE	0x08	/* op-status */
#define	RDAT_FILE_TYPE	0x10	/* raw data file */
#define	RDET_FILE_TYPE	0x20	/* raw detections file */
#define	RLOG_FILE_TYPE	0x40	/* raw logging file */
#define	OTHR_FILE_TYPE	0x80	/* other unknown files */


/* ---------- AQCFG Definitions --------------------------------- */

#define	AQCFG_STATION	1000	/* get station */
#define	AQCFG_SIGNON	2000	/* get signon message */
#define	AQCFG_COMPS	3000	/* get components */
#define	AQCFG_LCQ	4000	/* get (4000-n)th channel description */


/* UNIX definitions */
#define	UNIX_BLK_SIZE	512	/* number of bytes per logical block */
#define	UNIX_BLK_SHIFT	9	/* number of bits to shift to get blocks */




typedef struct _commo_comment { /* Comment-packet rcvd from DA */

	long	header_flag;	/* flag word for header frame */
	unsigned char	frame_type;	/* The type of a frame */
	char	comment_type;	/* type of comment */
	unsigned char	time_trans[6];	/* time stamp on message */
	unsigned char	da_c[DA_COMSTRLEN];	/* comment string in comment packet rcvd from DA */

}	COMMO_COMMENT;



typedef union _word { /* compressed word (Steim's compr. algorithm) */

	char	bdiff[4];	/* byte difference */
	short	sdiff[2];	/* short difference */
	long	ldiff;	/* long difference */

}	CWORD;



typedef struct _header_type { /* 1/2 full of first full frame in each data-record */

	long	header_flag;	/* flag word for header frame */
	unsigned char	frame_type;	/* The type of a frame */
	unsigned char	component;	/* component (vertical, N-S, E-W etc.) */
	unsigned char	stream;	/* Stream */
	unsigned char	soh;	/* state of health bits */
	unsigned long	station;	/* 4 byte station name */
	short	millisec;	/* 0..999 fractional part of time */
	short	time_mark;	/* sample number of time tag */
	long	firstword;	/* first word in the record */
	short	clock_corr;	/* Signed msec offset of clock */
	short	num_samples;	/* total in this record */
	char	rate;	/* +=samp/sec, -=sec/samp */
	char	spare;	/* Spare byte for filling */
	unsigned char	time_sample[6];	/* time of the tagged sample */

}	HEADER_TYPE;



typedef struct _header_data { /* Header for data-packets from DA */

	HEADER_TYPE	header_type;	/* 1/2 full of first full frame in each data-record */
	long	packet_seq;	/* stream-local record number */
	char	spares_he_data[28];	/* filler bytes to get 64 bytes full frame */
	CWORD	da_d[FRAMES_PER_DA][16];	/* Frames in continues data record rcvd from DA */

}	HEADER_DATA;



typedef struct _goes_mini_header { /* a mini header (?) */

	long	header_flag;	/* flag word for header frame */
	unsigned char	frame_type;	/* The type of a frame */
	unsigned char	component;	/* component (vertical, N-S, E-W etc.) */
	char	rate;	/* +=samp/sec, -=sec/samp */
	unsigned char	soh;	/* state of health bits */
	unsigned char	time_sample[6];	/* time of the tagged sample */
	short	millisec;	/* 0..999 fractional part of time */
	long	firstword;	/* first word in the record */

}	GOES_MINI_HEADER;



typedef union _commo_data { /* Compressed Record from DA */

	CWORD	da_f[FRAMES_TOTAL][16];	/* Complete Dataframe */
	HEADER_DATA	header_data;	/* Header for data-packets from DA */
	GOES_MINI_HEADER	goes_mini_header;	/* a mini header (?) */

}	COMMO_DATA;



typedef struct _event_empty { /* only record 0 in log file */

	unsigned char	frame_type;	/* The type of a frame */
	long	nextinrec;	/* next record to read or write */
	long	nrecs;	/* number of records w/o header rec */
	long	lastaccess;	/* incrementing sequenece number */
	unsigned char	upda_at[6];	/* time of last update */

}	EVENT_EMPTY;



/*::	doug@perry.berkeley.edu, per Joe Steim:			*/
/*::	secs_elaps and cons_expect_marks are really a union of	*/
/*::	a single field.						*/
/*::	    Use secs_elaps when clock_except =			*/
/*::		    MISSING_TIMEMARK, UNEXPECTED_TIMEMARK	*/
/*::		    NO_CLOCK_CORRECTION (?)			*/
/*::	    Use con_expect_marks when clock_except =		*/
/*::		    EXPECTED_TIMEMARK, VALID_TIMEMARK,		*/
/*::		    DAILY_TIME_CORRECTION_REPORT		*/
/*::	time_of_mark is not defined when clock_except = 	*/
/*::		    MISSING_TIMEMARK				*/

typedef struct _event_clock_corr { /* eventlog struc if frame_type=CLOCK_CORRECTION */

	unsigned char	frame_type;	/* The type of a frame */
	unsigned char	clock_except;	/* type of exception condition */
	unsigned char	time_of_mark[6];	/* if clock_except=EXPECTED: VALID,DAILY,UNEXPECTED */
	union {
	    long	secs_elaps;	/* seconds elapsed since last time mark */
	    long	cons_expect_marks;	/* consecutive expected time marks */
	} val;
	long	msec_corr;	/* last value correcting time */
	unsigned char	rec_qual;	/* reception quality indicator from Kinemetrics clck */
	unsigned char	timbas_vco;	/* HRDDCU VCO control byte */

}	EVENT_CLOCK_CORR;



typedef struct _sq_rep { /* squeezed event detection report */

	long	jdate;	/* seconds since Jan 1, 1984 00:00:00 */
	short	millisec;	/* 0..999 fractional part of time */
	unsigned char	component;	/* component (vertical, N-S, E-W etc.) */
	unsigned char	stream;	/* Stream */
	long	motion_qual;	/* motion pick lookback quality */
	long	peak_amp;	/* peak amplitude */
	long	period100;	/* 100 times detected period */
	long	back_amp;	/* background amplitude */

}	SQ_REP;



typedef struct _event_det_result { /* eventlog_struc, if frame_type=DEtECTION_RESULT */

	unsigned char	frame_type;	/* The type of a frame */
	unsigned char	detection_type;	/* detection type */
	unsigned char	time_of_rep[6];	/* time that detection is reported */
	SQ_REP	sq_rep;	/* squeezed event detection report */

}	EVENT_DET_RESULT;



typedef union _eventlog_struc { /* eventlog_struc as a union */

	EVENT_EMPTY	event_empty;	/* only record 0 in log file */
	EVENT_CLOCK_CORR	event_clock_corr;	/* eventlog struc if frame_type=CLOCK_CORRECTION */
	EVENT_DET_RESULT	event_det_result;	/* eventlog_struc, if frame_type=DEtECTION_RESULT */

}	EVENTLOG_STRUC;



typedef struct _commo_event { /* Event received from DA */

	long	header_flag;	/* flag word for header frame */
	EVENTLOG_STRUC	eventlog_struc;	/* eventlog_struc as a union */

}	COMMO_EVENT;



typedef union _commo_record { /* data packet rcvd from DA emb.in DA_TO_DP_BUFFER */

	COMMO_DATA	commo_data;	/* Compressed Record from DA */
	COMMO_EVENT	commo_event;	/* Event received from DA */
	COMMO_COMMENT	commo_comment;	/* Comment-packet rcvd from DA */

}	COMMO_RECORD;



typedef struct _da_time { /* Struktur fuer Zeitstempel von DA-Rechner */

	unsigned char	time_sample[6];	/* time of the tagged sample */
	short	millisec;	/* 0..999 fractional part of time */

}	DA_TIME;



typedef struct _preval_struc { /* struct to hold values to be compressed (Compress) */

	long	value;	/* seismic long value (amplitude) */
	DA_TIME	da_time;	/* Struktur fuer Zeitstempel von DA-Rechner */
	int	difftype;	/* difference type (D1_BYTE, D2_BYTE, D4_BYTE) */
	long	ldiff;	/* long difference */

}	PREVAL_STRUC;



typedef struct _store_data { /* Datapacket stored in continues data file */

	DA_TIME	da_begtime;	/* Time of first item in packet in storage file */
	DA_TIME	da_endtime;	/* Time for last item in data-packet */
	short	num_samples;	/* total in this record */
	char	rate;	/* +=samp/sec, -=sec/samp */
	unsigned char	soh;	/* state of health bits */
	long	packet_seq;	/* stream-local record number */
	short	clock_corr;	/* Signed msec offset of clock */
	CWORD	da_d[FRAMES_PER_DA][16];	/* Frames in continues data record rcvd from DA */

}	STORE_DATA;



typedef struct _compress_ctrl { /* structure to control the Compress machine */

	PREVAL_STRUC	preval[10];	/* buffer of values to be compressed (Compress) */
	long	predec;	/* predecessor value to calc differences (Compress) */
	DA_TIME	prev_time;	/* previous da_time for detecting time-gaps (Compr.) */
	int	state;	/* state of the Compress machine */
	int	prenum;	/* number of values in preval (Compress) */
	double	gaplim;	/* Limit Factor for detecting time-gaps in Compress */
	int	maxframe;	/* Number of frames per DA-packet (Compr.) */
	int	frame;	/* actual no. of frame in compressed packet */
	int	cell;	/* actual no. of long cell in a frame (Compress) */
	int	framefull;	/* indicates: the actual frame is full */
	int	(*compwrite)();	/* ptr to write func in compress ctrl-struc */
	STORE_DATA	store_data;	/* Datapacket stored in continues data file */

}	COMPRESS_CTRL;



typedef struct _config { /* Allgemeine Konfigurationsstruktur */

	char	datadir[DATADIR_NUM][61];	/* Array of directories for data-files */
	char	userdir[USERDIR_NUM][61];	/* Directories for users */
	long	pid_daemon;	/* PID des Daemon-Prozesses drmdaemon */
	int	seismo_gid;	/* GID of all seismo users */
	int	seismo_uid_start;	/* first possible UID for guest and normal users */
	int	seismo_uid_end;	/* last possible uid for guest and normal users */

}	CONFIG;



typedef struct _data_value { /* value and time of one sample */

	long	value;	/* seismic long value (amplitude) */
	DA_TIME	da_time;	/* Struktur fuer Zeitstempel von DA-Rechner */

}	DATA_VALUE;



typedef struct _da_config { /* Konfigsatz pro DA-Prozessor */

	char	da_name[14];	/* Symb. Name des DA-Prozessors */
	char	valid;	/* Valid- Flag */
	char	da_bemerk[61];	/* Bemerkung zum DA-Prozessor */
	char	da_conf_name[61];	/* Name des DA-Konfig files */
	char	da_com_type;	/* Kommunikationstyp zum DA-Prozessor */
	char	ttyport[21];	/* Port fuer tty-communication */
	char	xon_xoff;	/* Flag fuer Xon/Xoff Protokoll */
	long	baudrate;	/* Baudrate falls fuer tty communication */
	char	parity;	/* Parity fuer tty-communication */
	char	stopbits;	/* Anzahl Stopbits fuer tty communication */
	char	hostname[21];	/* Name des Hosts fuer TCP/IP Kommunikation */
	long	tcp_port;	/* Port-Nummer fuer TCP/IP Kommunikation */
	long	msg_key;	/* Message Queue Key fuer lokale Kommunikation */
	char	chk_dp_run;	/* Run-Modus fuer alle DP-Prozesse pro DA-Prozessor */
	char	dp_run;	/* Run-Command for Daemon for this da-processor */
	long	pid_dpsample;	/* PID fuer dpsample prozess */
	long	dpsample_start;	/* Time when dpsample has been started */
	long	pid_dpcommo;	/* PID of the dpcommo-process */
	long	dpcommo_start;	/* Starting time for dpcommo-process */
	long	msgkey_commo;	/* Message-Q-Key for dpcommo/dpsample communication */
	long	da_alive;	/* once every minute DA alive signal */

}	DA_CONFIG;

#define KEY_DA_NAME 0




typedef struct _errctl_struc { /* error control structure in packets */

	unsigned short	chksum;	/* 16-bit unsigned checksum */
	unsigned long	crc;	/* 32-Bit CRC value (DA-fashion, URG!!!) */

}	ERRCTL_STRUC;



typedef struct _da_to_dp_buffer { /* buffer to assemble a packet rcvd from DA */

	unsigned char	seq;	/* sequence number for packets */
	unsigned char	ctrl;	/* what to do with 'seq' in DP */
	COMMO_RECORD	commo_record;	/* data packet rcvd from DA emb.in DA_TO_DP_BUFFER */
	ERRCTL_STRUC	errctl;	/* error control struc in DA_TO_DP_BUFFER */

}	DA_TO_DP_BUFFER;



typedef struct _dpc_to_dps_msg { /* dpcommo to dpsample message structure */

	long	mtype;	/* message type (in message structure) */
	COMMO_RECORD	commo_record;	/* data packet rcvd from DA emb.in DA_TO_DP_BUFFER */

}	DPC_TO_DPS_MSG;



typedef struct _dp_to_da_header { /* structure to send ack to the DA */

	ERRCTL_STRUC	errctl;	/* error control struc in DA_TO_DP_BUFFER */
	unsigned char	cmd;	/* Command value for DP to DA acking */
	unsigned char	seq;	/* sequence number for packets */

}	DP_TO_DA_HEADER;



typedef struct _extrct_stream { /* Extracted Stream info from DA-Config-File */

	char	station_name[5];	/* Name of a Station in ASCII */
	char	station_params[61];	/* ASCII-Text for Station Parameters */
	char	comp_name[5];	/* Name of a Component */
	char	stream_name[5];	/* Name of a stream */
	char	detect_flag;	/* TRUE if detector specified for that stream */

}	EXTRCT_STREAM;



typedef struct _resfir { /* Resampling FIR-filter control structure */

	short	decim;	/* Decimation factor for resampling filter */
	short	firlen;	/* Length of resampling FIR-Filter */
	short	numcoeff;	/* Number of resampling FIR-filter coefficients */
	char	fireven;	/* TRUE if even filter length, else FALSE */
	double	*fircoeff;	/* FIR filter coefficients */

}	RESFIR;



typedef struct _retry_da { /* Retry Control Structure for drmdaemon */

	char	da_name[14];	/* Symb. Name des DA-Prozessors */
	int	retries;	/* number of retries in drmdaemon to start processes */

}	RETRY_DA;



typedef struct _rf_config { /* Description for chained FIR-filters */

	short	decim;	/* Decimation factor for resampling filter */
	short	rfirs[MAX_FIRCHAIN];	/* Array of chained decimation FIR-Filters */

}	RF_CONFIG;



typedef struct _rpacket { /* Resampling ring packet buffer for FIR-filters */

	int	rp_size;	/* Size of a resampling packet buffer */
	double	*rp_buf;	/* Buffer that holds the samples in an rpacket */
	int	rp_num;	/* Number of entries in an rpacket buffer */
	int	rp_start;	/* Start of samples for next filter step in rpacket */
	int	rp_write;	/* index to next sample to write into rpacket */
	int	rp_step;	/* Decimation stepwidth */

}	RPACKET;



typedef struct _store_event { /* Data packet for events stored in file */

	unsigned char	detection_type;	/* detection type */
	unsigned char	time_of_rep[6];	/* time that detection is reported */
	DA_TIME	da_time;	/* Struktur fuer Zeitstempel von DA-Rechner */
	long	motion_qual;	/* motion pick lookback quality */
	long	peak_amp;	/* peak amplitude */
	long	period100;	/* 100 times detected period */
	long	back_amp;	/* background amplitude */

}	STORE_EVENT;



typedef struct _stream_config { /* Konfigurationsrecord fuer Streams */

	char	da_name[14];	/* Symb. Name des DA-Prozessors */
	char	station_name[5];	/* Name of a Station in ASCII */
	char	comp_name[5];	/* Name of a Component */
	char	stream_name[5];	/* Name of a stream */
	char	valid;	/* Valid- Flag */
	char	span_type;	/* Type for timespan (seconds, minutes, hours, days) */
	long	span_time;	/* Time for span according to SPAN_TYPE */
	short	sto_max_files;	/* max. number of storage files per stream */
	char	cont_data_name[61];	/* Pathname for Continues Data */
	char	event_data_name[61];	/* Pathname for Events */
	char	log_data_name[61];	/* Pathname for Error-Logging */
	double	str_calib;	/* Calibration of a single stream */
	double	seismo_calib;	/* Calibration of a single seismometer */
	char	seedchannel[4];	/* SEED Format Channel Identifier */
	char	unit[8];	/* Engineering Units for a stream (typ nm/sec) */

}	STREAM_CONFIG;



typedef struct _store_file_head { /* Header fuer STorage file */

	long	store_magic;	/* Magic number for all storage files */
	char	store_file_type;	/* File type of a storage file */
	int	store_pack_size;	/* size of a packet */
	long	store_offset;	/* start of first packet in storage file */
	long	store_span;	/* Zeitspanne in Sekunden fuer Speichertabelle */
	char	store_flag;	/* Status der Storage datei */
	long	store_pred_seq;	/* sequence number of predecessor storage file */
	long	store_create_time;	/* Unix-Time of creation for a storage file */
	unsigned char	store_maxtime[6];	/* Endzeitpunkt zum Umschalten auf andere Datei */
	DA_TIME	store_begin;	/* Time of first item in storage file */
	DA_TIME	store_end;	/* Time of last item in storage file */
	long	store_count;	/* number of packets stored in a file */
	STREAM_CONFIG	stream_config;	/* Konfigurationsrecord fuer Streams */
	DA_CONFIG	da_config;	/* Konfigsatz pro DA-Prozessor */

}	STORE_FILE_HEAD;



typedef struct _store_log { /* comment packet to be stored in a storage file */

	unsigned char	time_trans[6];	/* time stamp on message */
	char	comment_type;	/* type of comment */
	char	comment[DP_COMSTRLEN];	/* Comment Text stored in file */

}	STORE_LOG;

#define KEY_STREAM 0

/*  Berkeley additions.	*/
#define	DRM_HDR_SIZE	64	/* This is WRONG -- look it up! */
#define	DRM_FILE_HDR_SIZE	sizeof(STORE_FILE_HEAD)
#define	MAX_HDR_SIZE		sizeof(STORE_FILE_HEAD)

#endif

