#ifndef _titan_h
#define _titan_h

/*=========================================================================
    titan.h (hacked version for SeedLink)
 *========================================================================*/

#include "sismalp.h"
#include "ah.h"
#include "segy.h"
/* #include "seed.h" */
#include "libtitio/titanio.h"


#define  REVISION "7.1"

/* Frames types */
#define DATA_PRE_TRIG       0
#define DATA_POST_TRIG      1
#define TIME                2
#define OFFSET              3
#define INFO                4
#define TIME_CORRECTED      5
#define MISC                6
#define FILLING             7

/* Info frame types */
#define MAX_INFO_FRAME     32
#define LAST_INFO_FRAME    (MAX_INFO_FRAME - 1)
#define CMD_07              7  /* STA_ID */
#define CMD_15             15  /* SET_TIME */
#define INFO_FRAME_16      16
#define INFO_FRAME_17      17
#define INFO_FRAME_20      20

#define PRIM_NUM_DATA     128
#define BASIC_SAMPRATE1 31.25
#define BASIC_SAMPRATE2  20.0
#define CRYSTAL_5323_22       0
#define AD_7710               1
#define HI7190                2
#define CRYSTAL_5321_22       3
#define AD_7710_DEL         1.5
#define HI7190_DEL          1.5
#define CRYSTAL_DEL        29
#define FIR_FILT_NCOEF    128
#define MAX_COMPRESSION    10
#define PRIM                1
#define SECD                2
#define NO_OFS              1
#define REL_OFS             2
#define ABS_OFS             3
#define NCHAN              16 
#define NCOMP               3
#define STANAMELEN          8
#define UNKNOWN        0x1777
#define NOVALUE       9999999
#define LAST                1
#define NINP                (PRIM_NUM_DATA + 10)

#define PATHLEN 255
/*
#define SHIFT_DEFAULT       2
#define SISMALP_MAXOUT      (BLKLEN * 58)
*/
#define MAXBLK              400
#define SISMALP_MAXOUT      (BLKLEN * MAXBLK)

#define GET_SYNC       (sync       = (int) (frame[11] & 0xF0))
#define GET_FRAME_TYPE (frame_type = (int) (frame[11] & 0x0F))
#define SYNC_KO        (sync != 0X50 && sync != 0XA0)
#define SYNC_OK        (sync == 0X50 || sync == 0XA0)

/* Times outside 1990.01.01-00:00:00-2002.01.01-00:00:00 are bad */
#define END_OF_WORLD   (double) 2100000000
#define TIME_OK(t) (((int) t>631152000 && (int) t<(int) END_OF_WORLD) ? 1:0)

typedef struct
{
    char  *cvtit_conf;
    char  stations[255];
    char  timedb[255];
    char  timedt[255];
    char  timedft[255];
    char  extra_tcorr[255];
    char  events[255];
    char  events_lst[255];
    char  events_tbl[255];
    char  meteosat_lst[255];
} Paths;

typedef struct
{
    int on;
    int all;
    int diff;
    int decim;
    int media;
    int batt;
    int coord;
} Titinfo;

typedef struct
{
    int chan;
    int decim_fact;
} Postdecim;

struct option
{
  char   station[9];
  int    chan;
  int    comp;
  int    verb;
  int    do_bindata;
  int    do_asc;
  int    do_sismalp;
  int    do_sac;  /* write data in sac format (native byte ordering)*/
  int    sacsun;  /* if sac format selected, write in sun byte ordering*/
  int    do_segy;
  int    do_ah;
  int    do_mseed;
  int    do_seed;
  int    do_shift;
  int    do_offset;
  int    do_time;
  int    do_coord;
  int    tcorr_mode;
  int    output_delta_t;
  int    use_database;
  int    beg_offset;
  int    end_offset;
  double timespan;
  char   event_list[255];
  char   offset_list[255];
  char   index_list[255];
  int    tjump_seg;
  int    evn_duration;
  int    num_traces;
  int    passcal;           /* OBSOLETE */
  int    daydir;            /* output in day directories; replace passcal */
  char   *force_delta_name; /* permet de forcer le nom du fichier delta */
  int    dtfile;            /* OBSOLETE */
  int    noinfo;            /* do not write ".info" files (ahz/rosalp) */
  int    gain_range;
  int    alt_out_name;      /* OBSOLETE */
  int    no_dt_duplic;      /* discard delta_t duplicate if TRUE */
  int    discard_short_blk; /* discard short (incomplete) data blocks */
  Titinfo info;             /* print out info frames contents */
  Postdecim postdecim;      /* Post decimation variables */
  int    titseg;
  double srate;
};

typedef struct
{   
    int    year;
    int    month;
    int    day;
    int    hour;
    int    min;
    float  sec;
} Tstruct;

#ifndef leap_year
#define leap_year(i) ((i % 4 == 0 && i % 100 != 0) || i % 400 == 0)
#endif

typedef struct
{
    int    *evn_time;        /* array of event starting time */
    int    *evn_duration;    /* array of events duration */
    int    num_events;       /* number of events */
    int    cur_event;        /* current event being processed */
    int    done;             /* TRUE if current event processe */
    int    *found;           /* array of flags: TRUE if event found */
    int    *file_offset;     /* array of titan file offset for the events */
    FILE   *Fp_sism_evntbl;  /* Sismalp event table */
} Event;

#define SMOOTHED       1
#define FITTED         2
#define NOCORRECTION   0
#define CORRECT_DFT    1
#define CORRECT_TOP    2


struct flags
{
    int  first_info_frame;
    int  first_data_frame;
    int  first_time_frame;
    int  init_time_log;
    int  missing_estim_dt;
    int  treset_time_out;
    int  TimeFrame;
};

typedef struct
{
    double beg_systime;
    double end_systime;
    int    ncoef;
    int    type;
    double coef[6];
} Tcoef;

typedef struct
{
    double curr;
    double prev;
    int reboot;
} TimeReset;

/*
SAC format:
----------
sach.cmpaz = azimuth;
sach.cmpinc = dip + 90.0;
    Z sach.cmpaz =  0.0; sach.cmpinc =  0.0;
    N sach.cmpaz =  0.0; sach.cmpinc = 90.0;
    E sach.cmpaz = 90.0; sach.cmpinc = 90.0;
*/

struct sismchan
{
    int  on;
    char sensor[8];
    char compname[3];
    double azim;
    double dip;
};


struct StationParm
{
    char    station[9];
    double  begtime;
    double  endtime;
    double  lat;
    double  lon;
    double  elev;
    double  depth;
    struct  sismchan sismchan[4][3];
    struct StationParm *next;
};

struct acq
{
    short  AdcType;
    double AdcDelay;
    short  NumChan;
    double PrimSrate;
    int    on_off[NCHAN];
    char   Id[12];
};

struct Channel
{
/* conflicts with C++'s operator 'new', so renamed to 'isnew'
    short    new; */
    short    isnew;
    short    numcomp;
    short    ratecode;
    double   srate;
    short    decim;
    double   adcdelay;
    double   filtdelay;
    short    ninp;
    int      nout;
    int      tot_inp;
    double   uncorrected_time;
    double   start_time;
    int      rel_ofs[NCOMP];
    int      abs_ofs[NCOMP];
    int      offset[NCOMP];
};

typedef struct
{
    short    numcomp;
    double   srate;
    short    decim;
    double   adcdelay;
    double   filtdelay;
    int      nsamples;
    char     dftfile[80];
    double   uncorrected_time;
    double   start_time;
    double   resettime;
    int      reboot;
    double   extpulse;
    double   observ_dt;
    double   estim_dt;
    double   extra_tcorr;
} outData;

struct data_list
{
    outData *header;
    char   station[9];
    short  chan;
    short  comp;
    char   *data_fname;
    struct data_list *next;
};

struct dft_list
{
     char    dft_name[80];
     int     time_reset;
     int     beg_systime;
     double  beg_dt;
     int     end_systime;
     double  end_dt;
     double  clock_drift;
     struct  dft_list *next;
};

struct xtc_list
{
    char dft_name[80];
    double tcorr;
    struct xtc_list *next;
};

typedef struct
{
    int beg_offset;
    int end_offset;
    int utime;
} Titseg;

typedef struct
{
    double sta;
    double lta;
    double ratio;
    float  fl;
    float  fh;
    int    plot;
} Trigparm;

typedef struct
{
    double t;
    double dft;
} TimeDrift;


#define append_linklist_element(new, head, tail) \
        new->next = NULL; \
        if (head != NULL) tail->next = new; \
        else head = new; \
        tail = new;

#ifdef LIB2
#define TITFILE FILE
TITFILE *open_frd(char *);
#define topenGuess(pt)        open_frd(pt)
#define tclose(file)          fclose(file)
#define tread(pt, n, m, file) fread(pt, n, m, file)
#define tseek(file, n, m)     fseek(file, n, m)
#define ttell(file)           ftell(file)
#define teof(file)            feof(file)
#endif


#endif
