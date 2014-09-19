/*======================================================================
    read_titan.c

    Library for Titan converter

    Author: J.-F. Fels, OMP, Toulouse

    Modified for SeedLink: A. Heinloo, GFZ Potsdam

Algorithm

process_titan:
    open input file
    get_station_name
    while (1)
        initilizations
        synchronize_on_frame
        while (read_read_frame)
            processDataFrame
            processTimeFrame
            processInfoFrames
            processOffsetFrame
            processMiscFrames
        end while
    end while

*======================================================================*/
#include "read_titan.h"

/* Prototypes */

#ifdef ANSI_C
static int   readTitanLoop      (void);
static void  print_byte_cnt     (void);
static void  init               (void);
static void  free_dft_list      (void);
#else
static int   readTitanLoop      ();
static void  print_byte_cnt     ();
static void  init               ();
static void  free_dft_list      ();
#endif



TITFILE        *Fp_tit;                      /* TITAN file pointer */
char           dtfname[PATHLEN];             /* delta_t file name  */
int            inp_data[NCHAN][NCOMP][NINP]; /* Input data array */

double         SystTime;      /* System time of the more recent sample */
double         FirstSystTime; /* First System time */
double         LastSystTime;  /* Last System time */
double         ExtPulse;      /* System time of the external pulse    */
double         Observ_dt;     /* Time offset between system and external time */
double         ExtraTcorr;    /* Extra time correction */
TimeReset      ResetTime;     /* System time reset */
struct flags   flags;

struct data_list *list_head = NULL;
struct data_list *list_tail = NULL;
struct dft_list  *dft_head = NULL;
struct dft_list  *dft_tail = NULL;
int              foundDftFiles = -1;
struct xtc_list  *xtc_head = NULL;
struct xtc_list  *xtc_tail = NULL;

struct         acq acq;
struct         Channel Channel[NCHAN];
outData        OutData[NCHAN];
char           Info[32][12];
int            NumEstimDeltaTime;
int            NumFitEntries;
int            totOutSamples;
int            DataOffsetCorr;
Titseg         TitSegment[2048];
int            NTitSegment;
int            TitSegmentNum;

static int    titfile_ofs;
static char   frame[20];

extern struct option opt;
extern FILE   *Fp_log;
extern int    byteswap;
extern Event  evn;
extern Paths  paths;
extern char   Station[8];



/*==================================================================*/
void process_titan(path)
char *path;
{
int i;
char s1[40], s2[40];


    if ((Fp_tit = topenGuess(path)) == NULL)
    {
        fprintf(Fp_log,"process_titan: error opening file %s\n",path);
        return;
    }

    flags.treset_time_out  = FALSE;
    flags.missing_estim_dt = FALSE;
    flags.init_time_log    = TRUE;
    for (i=0; i<NCHAN; i++) Channel[i].tot_inp = 0;
    evn.cur_event = (evn.num_events) ? 0 : -1;
    evn.done      = FALSE;

/*
 * Set begining titan file offset
 */
    titfile_ofs = opt.beg_offset;

    while (1)
    {
        if (readTitanLoop() == FALSE)
        {
            break;
        }
    }

/*
 * Process remaining input data
 */
    output_data(LAST, 0);

/*
 * Process last misc frames
 */
    processMiscFrames(frame, LAST);

/*
 * Log last time
 */
    log_time(LAST, &flags.init_time_log);

/*
 * Close input titan file
 */
    tclose(Fp_tit);
    Fp_tit = NULL;

/*
 * Write last delta time
 */
    write_delta_time(LAST);

/*
 * Print events info
 */
    if ((evn.evn_time != NULL) && (evn.num_events > 0))
    {
        printMissingEvents();
    }

/*
 * Warning for timing problem
 */
    if (opt.do_time == FALSE)
    {
        if (flags.missing_estim_dt == TRUE)
        {
          fprintf(Fp_log,"\n  ====================================");
          fprintf(Fp_log,"=============================\n");
          fprintf(Fp_log,"  WARNING: Couln't find estimated delta ");
          fprintf(Fp_log,"time for some or all traces\n");
          fprintf(Fp_log,"  Time was corrected by observed delta_t: %.4f\n",
                         Observ_dt);
          fprintf(Fp_log,"  ====================================");
          fprintf(Fp_log,"=============================\n");
        }
    }
    else
    {
        if (flags.treset_time_out == TRUE)
        {
          fprintf(Fp_log,"\n  ======================================");
          fprintf(Fp_log,"=============================\n");
          fprintf(Fp_log,"  WARNING: Found time reset by time-out\n");
          fprintf(Fp_log,"  ======================================");
          fprintf(Fp_log,"=============================\n");
        }
    }

    free_dft_list();

    time_asc4(s1, FirstSystTime);
    time_asc4(s2, LastSystTime);
    fprintf(Fp_log,"  System Time: from %.19s to %.19s\n", s1, s2);
}


/*==================================================================*/
static int readTitanLoop()
{
int   sync, frame_type;

    init();

    if (evn.cur_event >= evn.num_events)
    {
        fprintf(Fp_log,"  End readTitanLoop: last event found; ofs=%d\n",
             ttell(Fp_tit));
        return 0;
    }
    if ((titfile_ofs = sync_on_frame(titfile_ofs)) < 0)
    {
        fprintf(Fp_log,"  End readTitanLoop: sync_on_frame failed; ofs=%d\n",
             ttell(Fp_tit));
        return 0;
    }
    if (teof(Fp_tit))
    {
        fprintf(Fp_log,"  End readTitanLoop: EOF at file ofs %d\n",
             ttell(Fp_tit));
        return 0;
    }
    if (opt.end_offset > 0 && (ttell(Fp_tit) >= opt.end_offset))
    {
        fprintf(Fp_log,"  End readTitanLoop: end offset reached; ofs=%d\n",
             ttell(Fp_tit));
        return 0;
    }

    fprintf(Fp_log,"  readTitanLoop: frame synchro @ ofs %d\n",
        ttell(Fp_tit));

    while (tread(frame, 1, 12, Fp_tit) == 12)
    {
        GET_SYNC;
        if (!SYNC_OK)
        {
            titfile_ofs = ttell(Fp_tit);
            check_sync(frame);
            output_data(LAST, 0);
            log_time(LAST, &flags.init_time_log);
            return 1;
        }

        GET_FRAME_TYPE;
        switch (frame_type)
        {

case DATA_PRE_TRIG:
case DATA_POST_TRIG:
            titfile_ofs = ttell(Fp_tit);
            if (processDataFrame(frame) == -1)
            {
              fprintf(Fp_log,"  readTitanLoop: processDataFrame failed at ");
              fprintf(Fp_log,"file ofs: %d\n", ttell(Fp_tit));
              output_data(LAST, 0);
              log_time(LAST, &flags.init_time_log);
              return 1;
            }

            if (evn.done == TRUE)
            {
               evn.done = FALSE;
            }
            if (evn.cur_event >= evn.num_events)
            {
              output_data(LAST, 0);
              fprintf(Fp_log,"  End readTitanLoop: last event found; ofs=%d\n",
                   ttell(Fp_tit));
              return 0;
            }
            break;

case TIME: 
            if (processTimeFrame(frame) == -1)
            {
              fprintf(Fp_log,"  readTitanLoop: processTimeFrame failed at ");
              fprintf(Fp_log,"file ofs: %d\n", ttell(Fp_tit));
              output_data(LAST, 0);
              log_time(LAST, &flags.init_time_log);
              return 1;
            }
            log_time(!LAST, &flags.init_time_log);
            write_delta_time(!LAST);
            if (opt.end_offset > 0 && (ttell(Fp_tit) >= opt.end_offset))
            {
              output_data(LAST, 0);
              return 1;
            }
            break;

case OFFSET:
            processOffsetFrame(frame, 0);
            break;

case INFO:
            if (processInfoFrames(frame, "readTitanLoop") == FALSE)
            {
              fprintf(Fp_log,"  readTitanLoop: WRONG INFO  at ");
              fprintf(Fp_log,"file ofs %d\n", ttell(Fp_tit));
              output_data(LAST, 0);
              log_time(LAST, &flags.init_time_log);
              return 1;
            }
            print_byte_cnt();
            break;

case MISC:
            processMiscFrames(frame, !LAST);
            break;

case TIME_CORRECTED:
case FILLING:
            break;

default:
            fprintf(Fp_log,"  WARNING: readTitanLoop: ");
            fprintf(Fp_log,"frame type %d not supported ",frame_type);
            fprintf(Fp_log,"@ file ofs %d\n", ttell(Fp_tit));
            output_data(LAST, 0);
            return 1;

        }  /* end switch */

    }  /* end while */

    if (teof(Fp_tit))
    {
        fprintf(Fp_log,"  End readTitanLoop: EOF at file ofs %d\n", ttell(Fp_tit));
        return 0;
    }
    fprintf(Fp_log,"  WARNING: End readTitanLoop: ??? file ofs %d\n", ttell(Fp_tit));
    return 0;
}


/*================================================================*/
static void init()
{
int i, chan;

    acq.AdcDelay      = (double) UNKNOWN;

    for (chan=0; chan<NCHAN; chan++)
    {
        Channel[chan].isnew         = TRUE;
        Channel[chan].numcomp       = 0;
        Channel[chan].ninp          = 0;
        Channel[chan].nout          = 0;
        Channel[chan].srate         = (double) UNKNOWN;
        Channel[chan].decim         = (short)  UNKNOWN;
        Channel[chan].adcdelay      = (double) UNKNOWN;
        Channel[chan].filtdelay     = (double) UNKNOWN;

        clearOutputParms(chan);
    }

    memset(Info, 0, (32*12));
    for (i=0; i<32; i++) Info[i][10] = -1;
    flags.first_info_frame     = TRUE;
    flags.first_data_frame     = TRUE;
    flags.first_time_frame     = TRUE;
    flags.TimeFrame            = FALSE;
    SystTime             = 0.0;
    ExtPulse             = (double) UNKNOWN;
    Observ_dt            = (double) UNKNOWN;
    ResetTime.curr       = (double) UNKNOWN;
    ResetTime.prev       = (double) UNKNOWN;
    ResetTime.reboot     = -1;
    DataOffsetCorr       = ABS_OFS;
}


/*================================================================*
 * free the data info list after a call to process_titan
 *================================================================*/
void free_data_list()
{
struct data_list *pl,*plf;
 
if (0) printf("  ==== running free_data_list\n");

    for (pl=list_head; pl!=NULL;)
    {
        free(pl->header);
        plf=pl;
        pl=pl->next;
        free(plf);
    }
    list_head=NULL;
    list_tail=NULL;
}


/*================================================================*/
void free_dft_list()
{
struct dft_list *pl,*plf;
 
if (0) printf("  ==== running free_dft_list\n");

    for (pl=dft_head; pl!=NULL;)
    {
        plf=pl;
        pl=pl->next;
        free(plf);
    }
    dft_head=NULL;
    dft_tail=NULL;
    foundDftFiles = -1;
}


/*==================================================================*/
static void print_byte_cnt()
{
static int  kkk;
char str[40];

  if (++kkk > 80000)
  {
    printf("  process_titan: ................   %7.2f MBytes read ",
              (double)(ttell(Fp_tit)) / 1000000.0);
    time_asc4(str, SystTime);
    printf("%.19s\n", str);
    kkk = 0;
  }
}

