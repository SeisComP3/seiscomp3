/*=========================================================================
        proto.h (hacked version for SeedLink)
 *========================================================================*/
#ifndef _proto_h
#define _proto_h

#include "sismalp.h"
#include "titan.h"
#include "ah.h"
#include "segy.h"
#include "libtitio/titanio.h"

#ifdef ANSI_C

/* libtitio */
int teof         (TITFILE*);
int tread        (void*, int, int, TITFILE*);
int tseek        (TITFILE*, int, int);
int ttell        (TITFILE*);
int getEventOfs  (int, TITFILE*);

/* synchro.c */
int sync_on_frame  (int);
int sync_phase1    (int);
int sync_phase2    (void);
void check_sync    (char*);

/* libtitan.c */
int     read_fit_list      (char*);
int     processDataFrame   (char*);
int     processTimeFrame   (char*);
int     processInfoFrames  (char*, char*);
void    processMiscFrames  (char*, int);
void    processOffsetFrame (char*, int);
void    check_staname      (char*);
void    flushInputData     (int, int);
int     get_chan_parms     (char*, double*, int*);
double  samprate           (char*);
int     data_continuity    (int);
void    decompress         (char*, int, int[][MAX_COMPRESSION], int);
void    clearOutputParms   (int);
void    saveOutputParms    (int);
TITFILE *open_frd          (char*);
void    log_time           (int, int*);
void    write_delta_time   (int);
void    get_alt_name       (double, char*, double);
int     get_output_name    (double, char*);
void    ReadDbStation      (char*);
int     HasStatDbParms     (void);

/* libtime.c */
double GetOutputDataTime (int, char*, int, char**);
double GetEstimDt        (double, char*, char**);
double get_smoothed_dt   (double, char*, char**);
double calc_delta_t      (double, Tcoef);
void   readDftFiles      (void);
void   readXtcorrFile    (void);
int    loadDftFile       (double, char*);
void   freeTimeDrift     (void);

/* libinfo.c */
void decode_infos        (char*, int);

/* libsism.c */
void   output_sismalp    (int, int);
char   *sismalp_desc1    (int,int,int,float,double,struct desc *);
char   *sismalp_desc2    (char*,int,int,int*,int,int);
void   rd_desc           (char*, struct desc *);

/* libahz.c */
void   write_ahz_head   (int, int, double);
void   write_ahz_data   (int);

/* libasc.c */
void    output_asc      (int, int);

/* libevn.c */
void sism_month_events_tbl (Paths);
int  read_event_list       (int);

/* libout.c */
void OutputData       (int, int);
void wrt_info         (int, int, double, FILE*);

/* libsac.c */
void write_sac_head   (int, int, double);

/* libsegy.c */
void write_segy_head  (int, int, double);

/* mkseed.c */
void mkseed           (void);

char *mem             (int);

/* loadresp.c */
void get_channel_gain    (char*, char*, double, double, double*);

/* libutil.c */
void paths_init      (void);
void get_dbltime1    (char*, char*, double*);
void get_dbltime2    (char*, double*);
void get_dbltime3    (char*, double*);
void get_dbltime4    (char*, double*);
int  str2utime2      (char*, char*, double*);
int  str2utime3      (char*, double*);
int  str2utime4      (char*, double*);
int  str2utime1      (char*, double*);
void get_secs        (double, double*);
void time_asc0       (char*, char*, double);
void time_asc1       (char*, double);
void time_asc2       (char*, double);
void time_asc3       (char*, double);
void time_asc4       (char*, double);
void find_wordorder  (char*);
int  bytes2int4      (char, char, char, char);
void swap_4byte      (void*);
void swap_2byte_array(short*, int);
void swap_4byte_array(void*,int);
long open_Frd        (char*, FILE**);
long open_Frw        (char*, FILE**);
long open_Frd_noex   (char*, FILE**);
long open_Fapp       (char*, FILE**);
void open_Fwr        (char*, FILE**);
long fcreat_append   (char*, FILE**);

/* conflicts with getline() in libc (stdio.h) (different prototype)
int  getline         (FILE*, char*); */

int  getLine         (FILE*, char*, int, char, int*);
int  sparse          (char*, char**, char*, int);

/* conflicts with trim() in qlib (same prototype, different behaviour)
int  trim            (char*); */

int  trim_cr         (char*);
void ucase           (char*);
void lcase           (char*);
void min_max         (short*, int , short*, short*);
int  isdir           (char*);
void mk_year_day_dir (char*, char*, char*);
int  isfile          (char*);
int  read_dir        (char*, char*, char*);
void sort            (long, long*);
void sort_table      (char*, int);
int  worder          (int);
int  isinteger       (char*);
char *mem_alloc      (int, char*);

/* readtitan.c */
void process_titan   (char*);
void output_data     (int, int);
void output_miniseed (int, int);
void free_data_list  (void);

/* segtitan.c */
void  Event2TitanSeg     (int, int);
void  Tjump2TitanSeg     (int, int);
void  writeTitseg        (void);
void  read_offset_list   (void);
void  readTitanIndex     (void);
void  printMissingEvents (void);

/* ioroutin.c */
int    xdr_puthead    (ahhed*, XDR*);
int    xdr_putdata    (ahhed*, char*, XDR*);
void   get_null_head  (ahhed*);
int    xdr_ahhead     (XDR*, ahhed*);
int    xdr_gethead    (ahhed*, XDR*);
int    skip_data      (ahhed*, XDR*);
bool_t xdr_short12    (XDR*, float*, float*);
bool_t xdr_short16    (XDR*, float*, float*);

/* decimate.c */
int postDecimate      (struct data_list*);


TITFILE *topen(char*, int);
TITFILE *topenGuess(char*);
void    tclose(TITFILE*);
int     tSetPart(TITFILE*,int);
int     tGetSize(TITFILE*);
int     teof(TITFILE*);
FILE    *tFILE(TITFILE*);
void    trewind(TITFILE*);
int     tread(void*,int,int,TITFILE*);
int     ttell(TITFILE*);
int     testByteSwap(void);
int     isDir(char*);
int     isFile(char*);
int     isTitanDisk(char*);
int     isCharDev(char*);
int     hasIndex(TITFILE*);

#else

/* libtitio */
int teof         ();
int tread        ();
int tseek        ();
int ttell        ();
int getEventOfs  ();

/* synchro.c */
int sync_on_frame  ();
int sync_phase1    ();
int sync_phase2    ();
void check_sync    ();

/* libtitan.c */
int     read_fit_list      ();
int     processDataFrame   ();
int     processTimeFrame   ();
int     processInfoFrames  ();
void    processMiscFrames  ();
void    processOffsetFrame ();
void    check_staname      ();
void    flushInputData     ();
int     get_chan_parms     ();
double  samprate           ();
int     data_continuity    ();
void    decompress         ();
void    clearOutputParms   ();
void    saveOutputParms    ();
TITFILE *open_frd          ();
void    log_time           ();
void    write_delta_time   ();
void    get_alt_name       ();
int     get_output_name    ();
void    ReadDbStation      ();
int     HasStatDbParms     ();

/* libtime.c */
double GetOutputDataTime ();
double GetEstimDt        ();
double get_smoothed_dt   ();
double calc_delta_t      ();
void   readDftFiles      ();
void   readXtcorrFile    ();
int    loadDftFile       ();
void   freeTimeDrift     ();

/* libinfo.c */
void decode_infos       ();

/* libsism.c */
void   output_sismalp     ();
char   *sismalp_desc1     ();
char   *sismalp_desc2     ();
void   rd_desc            ();

/* libahz.c */
void   write_ahz_head   ();
void   write_ahz_data   ();

/* libasc.c */
void    output_asc      ();

/* libevn.c */
void sism_month_events_tbl ();
int  read_event_list       ();

/* libout.c */
void OutputData       ();
void wrt_info         ();

/* libsac.c */
void write_sac_head   ();

/* libsegy.c */
void write_segy_head  ();

/* mkseed.c */
void mkseed           ();

/* libseed.c */
char *mem             ();
void dbt_to_tstruct   ();
void tstruct_to_dbt   ();
void time_asc         ();
void asc_to_dbt       ();
int  seed_srate       ();

/* loadresp.c */
void get_channel_gain    ();
void print_filter        ();


/* libutil.c */
void paths_init      ();
void get_dbltime1    ();
void get_dbltime2    ();
void get_dbltime3    ();
void get_dbltime4    ();
int  str2utime2      ();
int  str2utime3      ();
int  str2utime4      ();
int  str2utime1      ();
void get_secs        ();
void time_asc0       ();
void time_asc1       ();
void time_asc2       ();
void time_asc3       ();
void time_asc4       ();
void find_wordorder  ();
int  bytes2int4      ();
void swap_4byte      ();
void swap_2byte_array();
void swap_4byte_array();
long open_Frd        ();
long open_Frw        ();
long open_Frd_noex   ();
long open_Fapp       ();
void open_Fwr        ();
long fcreat_append   ();
int  getline         ();
int  getLine         ();
int  sparse          ();
int  trim            ();
int  trim_cr         ();
void ucase           ();
void lcase           ();
void min_max         ();
int  isdir           ();
void mk_year_day_dir ();
int  isfile          ();
int  read_dir        ();
void sort            ();
void sort_table      ();
int  worder          ();
int  isinteger       ();
char *mem_alloc      ();

/* readtitan.c */
void process_titan   ();
void output_data     ();
void output_miniseed ();
void free_data_list  ();

/* segtitan.c */
void  Event2TitanSeg     ();
void  Tjump2TitanSeg     ();
void  writeTitseg        ();
void  read_offset_list   ();
void  readTitanIndex     ();
void  printMissingEvents ();


/* ioroutin.c */
int    xdr_puthead    ();
int    xdr_putdata    ();
void   get_null_head  ();
int    xdr_ahhead     ();
int    xdr_gethead    ();
int    skip_data      ();
bool_t xdr_short12    ();
bool_t xdr_short16    ();

/* decimate.c */
int postDecimate      ();


TITFILE *topen       ();
TITFILE *topenGuess  ();
void    tclose       ();
int     tSetPart     ();
int     tGetSize     ();
int     teof         ();
FILE    *tFILE       ();
void    trewind      ();
int     tread        ();
int     ttell        ();
int     testByteSwap ();
int     isDir        ();
int     isFile       ();
int     isTitanDisk  ();
int     isCharDev    ();
int     hasIndex     ();

#endif

#endif
