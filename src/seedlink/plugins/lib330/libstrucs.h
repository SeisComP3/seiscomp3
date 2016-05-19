/*   Lib330 internal data structures
     Copyright 2006-2013 Certified Software Corporation

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
    1 2006-11-23 rdr Change "accum" in taccmstat to longint so it can store INVALID_ENTRY.
    2 2006-12-18 rdr Add ctrlport and dataport to store current control and data ports
                     rather than modifying the parameters passed by the host.
    3 2007-08-01 rdr Add baler callback definitions.
    4 2007-08-07 rdr Add Q330 continuity cache definitions. Add memory management for
                     structures that live throughout the duration of the thread.
    5 2007-10-30 rdr Remove q330phy.
    6 2008-01-09 rdr Add freeze_timer. access_timer moved to shared area for locking.
                     Add adjustable status and data timeouts and retries. Add msgmutex,
                     msglock, and msgunlock.
    7 2008-08-20 rdr Add TCP Support.
    8 2009-02-09 rdr Add EP support.
    9 2009-07-28 rdr Add DSS support.
   10 2009-09-15 rdr Add DSS support when connected to 330 via serial.
   11 2010-03-27 rdr Add Q335 flag.
   12 2010-05-07 rdr Add comm structure.
   13 2013-02-02 rdr Add high_socket.
}*/
#ifndef libstrucs_h
/* Flag this file as included */
#define libstrucs_h
#define VER_LIBSTRUCS 18

/* Make sure libtypes.h is included */
#ifndef libtypes_h
#include "libtypes.h"
#endif
/* Make sure q330types.h is included */
#ifndef q330types_h
#include "q330types.h"
#endif
/* Make sure libclient.h is included */
#ifndef libclient_h
#include "libclient.h"
#endif
/* Make sure libseed.h is included */
#ifndef libseed_h
#include "libseed.h"
#endif

#define CMDQSZ 32 /* Maximum size of command queue */
#define MAX_HISTORY 16
#define MAXCFG 7884 /* actual number of characters allowed */
#define WINWRAP (WINBUFS - 1)
#define SKIPCNT 24
#define MAXSPREAD 128 /* now that we have the reboot time saved.. */
#define DEFAULT_MEMORY 131072
#define DEFAULT_MEM_INC 65536
#define DEFAULT_THRMEM 65536
#define DEFAULT_THR_INC 32768
#define RAW_GLOBAL_SIZE 160
#define RAW_FIXED_SIZE 188
#define RAW_LOG_SIZE 52
#define INITIAL_ACCESS_TIMOUT 60 /* Less than 1 minute poweron is pointless */
#define DEFAULT_PIU_RETRY 5 * 60 /* Port in Use */
#define DEFAULT_DATA_TIMEOUT 10 * 60 /* Data timeout */
#define DEFAULT_DATA_TIMEOUT_RETRY 10 * 60 ;
#define DEFAULT_STATUS_TIMEOUT 5 * 60 /* Status timeout */
#define DEFAULT_STATUS_TIMEOUT_RETRY 5 * 60 ;
#define MIN_MSG_QUEUE_SIZE 10 /* Minimum number of client message buffers */

typedef struct { /* One command to be sent to Q330 */
  byte cmd ;
  byte subcmd ;
  boolean tunneled ;
  word sendsz ; /* sending size */
  word estsz ; /* estimated return packet size */
  word retsz ; /* return size accumulator */
  double sent ;
} tcmdq ;
typedef tcmdq tcmdqs[CMDQSZ] ;

enum tcphase {CP_IDLE, CP_NEED, CP_WAIT} ;
typedef struct { /* command queue fields */
  enum tcphase cphase ; /* command handling phase */
  word ctrlrecnt ; /* control port retry down counter */
  word lastctrlseq ; /* the last one we sent */
  word ctrlseq ; /* incrementing control sequence counter */
  word ctrl_retries ; /* number of retries so far */
  integer cmdin, cmdout ; /* command queue pointers */
  tcmdqs cmdq ; /* command dispatch queue */
  tany cmsgin ; /* command message from Q330 */
  tany cmsgout ; /* command messout to Q330 */
  integer history_count ;
  integer history_idx ; /* next entry */
  double histories[MAX_HISTORY] ;
} tcommands ;
typedef longint taccminutes[60] ;
typedef struct { /* for building one dp statistic */
  longint accum ;
  longint accum_ds ; /* for datastream use */
  pointer ds_lcq ; /* lcq associated with this statistic, if any */
  taccminutes minutes ;
  longint hours[24] ;
} taccmstat ;
/* Compiler doesn't like this typedef taccmstat taccmstats[tacctype] ; */
typedef taccmstat taccmstats[AC_LAST + 1] ;
typedef byte tcfgbuf[MAXCFG] ;
typedef tcfgbuf *pcfgbuf ;
typedef struct {
  boolean valid ;
  byte spare ;
  tany buf ;
} tpkt_buf ;
typedef word tcbuf[10000] ; /* continuity buffer */
typedef struct tmem_manager { /* Linked list of memory segments for token expansion and buffers */
  struct tmem_manager *next ; /* next block */
  integer alloc_size ; /* allocated size of this block */
  integer sofar ; /* amount used in this block */
  pointer base ; /* start of the allocated memory in this block */
} tmem_manager ;
typedef tmem_manager *pmem_manager ;

typedef struct tcont_cache { /* Linked list of Q330 continuity segments */
  struct tcont_cache *next ; /* next block */
  pbyte payload ; /* address of payload */
  integer size ; /* current payload size */
  integer allocsize ; /* size of this allocation */
  /* contents follow */
} tcont_cache ;

typedef tpkt_buf *ppkt_buf ;
typedef struct { /* for tunnelling commands */
  byte reqcmd ; /* what to send */
  byte respcmd ; /* what we expect */
  integer paysize ;
  byte payload[MAXDATA] ;
} ttunnel ;
enum tclient_ping {CLP_IDLE, CLP_REQ, CLP_SENT} ; /* client has requested a ping */
enum ttunnel_state {TS_IDLE, TS_REQ, TS_SENT, TS_READY} ; /* tunnel state */

typedef struct { /* shared variables with client. Require mutex protection */
  enum tliberr liberr ; /* last error condition */
  enum tlibstate target_state ; /* state the client wants */
  integer stat_minutes ;
  integer stat_hours ;
  longint total_minutes ;
  taccmstats accmstats ;
  topstat opstat ; /* operation status */
  word first_share_clear ; /* start of shared fields cleared after de-registration */
  longword extra_status ; /* client wants for status than default */
  longword have_status ; /* this status is currently available */
  longword have_config ; /* have configuration info from Q330 bitmap */
  longword want_config ; /* bitmap of perishable configuration requested */
  longword check_ip ; /* IP address to check for registration with Willard */
  word access_timer ; /* keeps connected to Q330 while baler active */
  word status_interval ; /* status request interval in seconds */
  word interval_counter ; /* automatic status timer */
  integer freeze_timer ; /* if > 0 then don't process data from Q330 */
  boolean log_changed ; /* client wants to change programming */
  boolean abort_requested ; /* client requests aborting command queue */
  boolean usermessage_requested ; /* client wants a user message sent */
  boolean webadv_requested ; /* client wants to advertise a web server */
  enum tclient_ping client_ping ; /* client has requested a ping */
  enum ttunnel_state tunnel_state ; /* tunnel state */
  ttunnel tunnel ; /* for tunnelling commands */
  tsensctrl sensctrl ; /* sensor control */
  tfixed fixed ; /* fixed values after reboot */
  tglobal global ; /* global programming */
  tlog log ; /* current logical port programming */
  tlog newlog ; /* to change programming */
  tstat_global stat_global ; /* global status */
  tstat_pwr stat_pwr ; /* Power supply status */
  tstat_gps stat_gps ; /* GPS status */
  tgpsid gpsids ; /* GPS ID Strings */
  tstat_boom stat_boom ; /* Boom positions and other stuff */
  tstat_pll stat_pll ; /* PLL Status */
  tstat_sats stat_sats ; /* GPS Satellites */
  tstat_log stat_log ; /* logical port status */
  tstat_serial stat_serial ; /* serial port status */
  tstat_ether stat_ether ; /* ethernet port status */
  tstat_arp stat_arp ; /* arp status */
  tstat_baler stat_baler ; /* baler status */
  tdyn_ips stat_dyn ;
  tstat_auxad stat_auxad ; /* AuxAD status */
  tstat_sersens stat_sersens ; /* Serial sensor status */
  tstat_ep stat_ep ; /* Environmental Processor status */
  tstat_fes stat_fes ; /* Front-End Status for Q335 */
  tepdelay epdelay ; /* EP Delays from Q330 */
  tepcfg epcfg ; /* EP Configuration */
  tepcfg newepcfg ; /* To change it */
  troutelist routelist ; /* route list */
  tnew_webadv new_webadv ; /* new format web advertisement */
  told_webadv old_webadv ; /* old format web advertisement */
  tuser_message user_message ; /* as requested by client */
  tuser_message usermsg ; /* user message received */
  tuser_message newuser ; /* user message to send */
  tdevs devs ; /* CNP Devices */
  tpingreq pingreq ; /* for pinging Q330 */
  tslidestat slidestat ; /* sliding window status */
  word last_share_clear ; /* last address cleared after de-registration */
} tshare ;
typedef struct { /* this is the actual context which is hidden from clients */
#ifndef CMEX32
#ifdef X86_WIN32
  HANDLE mutex ;
  HANDLE msgmutex ;
  HANDLE threadhandle ;
  longword threadid ;
#else
  pthread_mutex_t mutex ;
  pthread_mutex_t msgmutex ;
  pthread_t threadid ;
#endif
#endif
  enum tlibstate libstate ; /* current state of this station */
  boolean terminate ; /* set TRUE to terminate thread */
  boolean needtosayhello ; /* set TRUE to generate created message */
  boolean q330cont_updated ; /* Has been updated since writing */
  tseed_net network ;
  tseed_stn station ;
  tpar_create par_create ; /* parameters to create context */
  tpar_register par_register ; /* registration parameters */
  pmem_manager memory_head ; /* start of memory management blocks */
  pmem_manager cur_memory ; /* current block we are allocating from */
  integer cur_memory_required ; /* for continuity */
  pmem_manager thrmem_head ; /* start of thread memory management blocks */
  pmem_manager cur_thrmem ; /* current block we are allocating from */
  integer cur_thrmem_required ; /* for thread continuity */
  tcont_cache *conthead ; /* head of active segments */
  tcont_cache *contfree ; /* head of inactive but available segments */
  tcont_cache *contlast ; /* last active segment during creation */
  tshare share ; /* variables shared with client */
  pointer aqstruc ; /* opaque pointer to acquisition structures */
  pointer dssstruc ; /* opaque pointer to dss handler */
  pointer md5buf ; /* opaque pointer to md5 working buffer */
  pointer lastuser ;
  longword msg_count ; /* message count */
  word rsum ; /* random number generator static storage */
  integer reg_wait_timer ; /* stay in wait state until decrements to zero */
  integer dynip_age ; /* age in seconds */
  integer reg_tries ; /* number of times we have tried to register */
  integer minute_counter ; /* 6 * 10 seconds = 60 seconds */
  integer data_timeout ; /* threshold seconds for data_timer */
  integer data_timeout_retry ; /* seconds to wait before retrying after data timeout */
  integer status_timeout ; /* threshold seconds for status_timer */
  integer status_timeout_retry ; /* seconds to wait before retrying after status timeout */
  integer piu_retry ; /* seconds to wait before retrying after port in use error */
  integer update_ep_timer ; /* seconds to wait before manually checking EP Delays after token change */
  longint last_ten_sec ; /* last ten second value */
  double last_100ms ; /* last time ran 100ms timer routine */
  double saved_data_timetag ; /* for latency calculations */
  double q330_cont_written ; /* last time Q330 continuity was written to disk */
  double boot_time ; /* for DSS use */
  longword dpstat_timestamp ; /* for dp statistics */
  word cur_verbosity ; /* current verbosity */
  pcfgbuf cfgbuf ;
  tcbuf *cbuf ; /* continuity buffer */
  boolean media_error ; /* for continuity writing */
  boolean tcp ; /* Using TCP ethernet connection */
  boolean got_connected ; /* UDP or immediate TCP connection */
  boolean q335 ; /* Connected to Q335 */
#ifdef X86_WIN32
  HANDLE comid ; /* for serial communications */
  SOCKET cpath ; /* commands socket */
  SOCKET dpath ; /* data socket */
  SOCKET dsspath ; /* dss socket */
  struct sockaddr csockin, csockout ; /* commands socket address descriptors */
  struct sockaddr dsockin, dsockout ; /* data socket address descriptors */
#else
  integer comid ; /* for serial communications */
  integer cpath ; /* commands socket */
  integer dpath ; /* data socket */
  integer dsspath ; /* dss socket */
  integer high_socket ; /* Highest socket number */
  struct sockaddr csockin, csockout ; /* commands socket address descriptors */
  struct sockaddr dsockin, dsockout ; /* data socket address descriptors */
#endif
  word ctrlport, dataport ; /* currently used control and data ports */
  longword serial_ip ; /* Host serial IP */
  tany datain, dataout, datasave ;
  crc_table_type crc_table ;
  tstate_call state_call ; /* buffer for building state callbacks */
  tmsg_call msg_call ; /* buffer for building message callbacks */
  tonesec_call onesec_call ; /* buffer for building one second callbacks */
  tbaler_call baler_call ; /* buffer for buiding baler callbacks */
#ifndef OMIT_SEED
  tminiseed_call miniseed_call ; /* buffer for building miniseed callbacks */
#endif
  tclock qclock ; /* default clock or loaded from tokens */
  longint zone_adjust ; /* timezone adjust */
  double last_status_received ; /* last time status was received */
  string contmsg ; /* any errors from continuity checking */
  string9 station_ident ; /* network-station */
  ppkt_buf pkt_bufs[256] ;
  /* following are cleared after de-registering */
  word first_clear ; /* first byte to clear */
  integer ack_delay ;
  integer ack_timeout ; /* to send DT_NOP packets */
  word ack_counter ;
  word timercnt ; /* count up getting one second intervals from 100ms */
  word q330cport ; /* Q330's command port */
  word q330dport ; /* Q330's data port */
  longword q330ip ; /* current working IP address */
  longword web_ip ; /* possible web server IP address based on server challenge */
  boolean usesock ; /* use sockets for I/O */
  boolean registered ; /* registered with Q330 */
  boolean balesim ; /* baler simulation */
  boolean stalled_link ; /* link is currently stalled */
  boolean link_recv ; /* have link parameters from Q330 */
  boolean need_regmsg ; /* need a registration user message */
  boolean piggyok ; /* piggyback status OK */
  boolean reboot_done ; /* has reboot been completed? */
  boolean need_sats ; /* need satellite status for clock logging */
  boolean nested_log ; /* we are actually writing a log record */
  boolean flush_all ; /* flush DA and DP LCQ's */
  boolean dss_gate_on ; /* Data packets are recent */
  integer comtimer ; /* timeout for above action */
  integer reg_timer ; /* registration timeout */
  integer data_timer ; /* since got data */
  longint conn_timer ; /* seconds in run state */
  integer status_timer ; /* count up for seconds since status */
  longword last_sent_count, lastds_sent_count ; /* packets sent at start of this minute */
  longword last_resent_count, lastds_resent_count ; /* packets resent at start of this minute */
  word cfgsize, cfgnow, cfgoffset ;
  word ipid ;
  word pingid ;
  word window_size ;
  boolean escpend ;
  boolean needframe ;
  pbyte bufptr ;
  double ping_send ; /* when ping was sent out */
  tcommands commands ;
  tsrvch srvch ; /* Server challenge packet */
  tsrvresp srvresp ; /* Server response packet */
  tbrdy brdy ; /* baler ready message */
#ifndef OMIT_SDUMP
  tgps2 gps2 ; /* for status dump only */
  tman man ;
  tdcp dcp ;
#endif
  tpoll newpoll ; /* poll for serial number */
  tpinghdr pinghdr ; /* for sending pings */
  tpingbuffer pingbuffer ;
  tback back ; /* Baler Acknowledge received */
  tcomm comm ; /* Communications structure */
  tbalecfg balecfg ; /* Baler configuration */
  byte raw_global[RAW_GLOBAL_SIZE] ;
  byte raw_fixed[RAW_FIXED_SIZE] ;
  byte raw_log[RAW_LOG_SIZE] ;
  tmem mem_req ; /* memory request */
  tmem mem_hdr ; /* memory response header */
  tip recvip ; /* received ip header */
  tudp recvudp ; /* received udp header */
  tqdp recvhdr ; /* received and decoded qdp header */
  byte tcpbuf[TCPBUFSZ] ; /* TCP is basically a serial link, must buffer */
  word tcpidx ; /* How many bytes are currently valid in tcpbuf */
  word last_packet ;
  double lasttime ;
  longword lastseq ;
  longword lastfill ;
  longword stat_request ; /* bitmap of status to request */
  boolean autowin ;
  word last_clear ; /* end of clear */
} tq330 ;
typedef tq330 *pq330 ;

extern void lock (pq330 q330) ;
extern void unlock (pq330 q330) ;
extern void msglock (pq330 q330) ;
extern void msgunlock (pq330 q330) ;
extern void sleepms (integer ms) ;
extern void getbuf (pq330 q330, pointer *p, integer size) ;
extern void mem_release (pq330 q330) ;
extern void getthrbuf (pq330 q330, pointer *p, integer size) ;
extern void gcrcinit (crc_table_type *crctable) ;
extern void lib_create_330 (tcontext *ct, tpar_create *cfg) ;
extern enum tliberr lib_destroy_330 (tcontext *ct) ;
extern enum tliberr lib_register_330 (pq330 q330, tpar_register *rpar) ;
extern enum tliberr lib_unregping_330 (pq330 q330, tpar_register *rpar) ;
extern longint gcrccalc (crc_table_type *crctable, pbyte p, longint len) ;
extern void new_state (pq330 q330, enum tlibstate newstate) ;
extern void new_cfg (pq330 q330, longword newbitmap) ;
extern void new_status (pq330 q330, longword newbitmap) ;
extern void state_callback (pq330 q330, enum tstate_type stype, longword val) ;
extern longword baler_callback (pq330 q330, enum tbaler_type btype, longword val) ;
extern longword make_bitmap (longword bit) ;
extern void set_liberr (pq330 q330, enum tliberr newerr) ;
#endif
