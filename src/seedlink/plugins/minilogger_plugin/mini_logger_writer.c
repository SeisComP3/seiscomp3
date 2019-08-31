/***************************************************************************
 * mini_logger_writer.c
 *
 * Read serial data from a serial port and write 512-byte Mini-SEED data records in mseed files.
 *
 * Written by Anthony Lomax
 *   ALomax Scientific www.alomax.net
 *
 * created: 2013.10.28
 ***************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/resource.h>
#include <errno.h>

#ifndef WIN32
#include <signal.h>
static void term_handler(int sig);
#endif

#include "libmseed.h"
#include "mini_logger_writer.h"
#include "minisepdevice.h"
#include "settings/settings.h"
#include "plugin.h"

#define DEBUG 0

#define STANDARD_STRLEN 4096


#ifdef EXTERN_MODE
#define	EXTERN_TXT extern
#else
#define EXTERN_TXT
#endif

EXTERN_TXT int verbose;
EXTERN_TXT char propfile[STANDARD_STRLEN];

// properties
// Logging
EXTERN_TXT char port_path_hint[STANDARD_STRLEN];
#define PORT_PATH_HINT_DEFAULT "/dev/ttyACM0"   // rasberry pi
//#define PORT_PATH_HINT_DEFAULT "/dev/usbdev1.1" // iMac
EXTERN_TXT int allow_set_interface_attribs;
#define ALLOW_SET_INTERFACE_ATTRIBS_DEFAULT 0

EXTERN_TXT double mswrite_header_sample_rate;
#define MSWRITE_HEADER_SAMPLE_RATE_DEFAULT -1
EXTERN_TXT char mswrite_data_encoding_type[STANDARD_STRLEN];
#define MSWRITE_DATA_ENCODING_TYPE_DEFAULT "DE_INT32"
EXTERN_TXT int mswrite_data_encoding_type_code;

// Station
EXTERN_TXT char station_network[STANDARD_STRLEN];
#define STA_NETWORK_DEFAULT "UK"
EXTERN_TXT char station_name[STANDARD_STRLEN];
#define STA_NAME_DEFAULT "TEST"
EXTERN_TXT char channel_prefix[STANDARD_STRLEN];
#define STA_CHANNEL_PREFIX_DEFAULT "BH"
EXTERN_TXT char component[STANDARD_STRLEN];
#define STA_COMPONENT_DEFAULT "Z"
//
EXTERN_TXT int nominal_sample_rate;
#define STA_NOMINAL_SAMPLE_RATE_DEFAULT 20
//Nominal sample rate in seconds, one of 20, 40 or 80. type=string default=20
//   For SEISMOMETER INTERFACE (USB) (CODE: SEP 064):
// 	The SPS can be adjusted by sending single characters to the Virtual Com Port:
//	‘a’: 20.032 SPS
//	‘b’: 39.860 SPS
//	‘c’: 79.719 SPS
//
EXTERN_TXT int nominal_gain;
#define STA_NOMINAL_GAIN_DEFAULT 1
//Nominal gain, one of 1, 2 or 4. type=string default=1
//   For SEISMOMETER INTERFACE (USB) (CODE: SEP 064):
// 	The gain can be adjusted by sending single characters to the Virtual Com Port:
//	‘1’: ×1 = 0.64μV/count
//	‘2’: ×2 = 0.32μV/count
//	‘4’: ×2 = 0.32μV/count
//
EXTERN_TXT int do_settings_sep064;
#define DO_SETTINGS_SEP064_DEFAULT 0


#define PROP_FILE_NAME_DEFAULT "mini_logger_writer.prop"
static Settings *settings = NULL;

static char* port_path;

#define SLRECSIZE 512   // Mini-SEED record size
#define SLREC_DATA_SIZE 456     // apparent size of time-seris data in a 512 byte mseed record written by libmseed
//static int num_samples_in_record = SLRECSIZE / sizeof (int);
static int num_samples_in_record = -1;



int find_device_and_connect(char *port_path_hint);
int collect_and_write();

/***************************************************************************
 * usage():
 * Print the usage message and exit.
 ***************************************************************************/
static void usage(void) {

    fprintf(stdout, "%s version: %s (%s)\n", PACKAGE, VERSION, VERSION_DATE);
    fprintf(stdout, "Usage: %s [options]\n", PACKAGE);
    fprintf(stdout,
            " Options:\n"
            " -p propfile    properties file name (default: config.prop)\n"
            " -V             Report program version\n"
            " -h             Show this usage message\n"
            " -v             Be more verbose, multiple flags can be used\n"
            );

} /* End of usage() */

/***************************************************************************
 * parameter_proc():
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int parameter_proc(int argcount, char **argvec) {

    int optind = 1;
    int error = 0;

    if (argcount <= 1)
        error++;

    // Process all command line arguments
    for (optind = 1; optind < argcount; optind++) {

        if (strcmp(argvec[optind], "-V") == 0) {
            fprintf(stdout, "%s version: %s (%s)\n", PACKAGE, VERSION, VERSION_DATE);
            exit(0);
        } else if (strcmp(argvec[optind], "-h") == 0) {
            (*usage)();
            exit(0);
        } else if (strncmp(argvec[optind], "-v", 2) == 0) {
            verbose += strspn(&argvec[optind][1], "v");
        } else if (strcmp(argvec[optind], "-p") == 0) {
            strcpy(propfile, argvec[++(optind)]);
        } else if (strncmp(argvec[optind], "-", 1) == 0) {
            fprintf(stdout, "%s: Unknown option: %s\n", PACKAGE, argvec[optind]);
            exit(1);
        }
    }

    // Report the program version
    if (verbose) {
        logprintf(MSG_FLAG, "%s version: %s (%s)\n", PACKAGE, VERSION, VERSION_DATE);
    }

    return 0;

} /* End of parameter_proc() */

/***************************************************************************
 * init_properties():
 * Initialize properties from properties file
 ***************************************************************************/
int init_properties(char *propfile) {

    // read properties file
    FILE *fp_prop = fopen(propfile, "r");
    if (fp_prop == NULL) {
        logprintf(MSG_FLAG, "Info: Cannot open application properties file: %s\n", propfile);
        settings = NULL;
        return (0);
    }
    settings = settings_open(fp_prop);
    fclose(fp_prop);
    if (settings == NULL) {
        logprintf(ERROR_FLAG, "Reading application properties file: %s\n", propfile);
        return (-1);
    }

    //
    if (settings_get_helper(settings,
            "Logging", "port_path_hint", port_path_hint, sizeof (port_path_hint), PORT_PATH_HINT_DEFAULT,
            verbose) == 0) {
        ; // handle error
    }
    //
    if (settings_get_int_helper(settings,
            "Logging", "allow_set_interface_attribs", &allow_set_interface_attribs, ALLOW_SET_INTERFACE_ATTRIBS_DEFAULT,
            verbose) == 0) {
        ; // handle error
    }

    //
    if (settings_get_double_helper(settings,
            "Logging", "mswrite_header_sample_rate", &mswrite_header_sample_rate, MSWRITE_HEADER_SAMPLE_RATE_DEFAULT,
            verbose
            ) == DBL_INVALID) {
        ; // handle error
    }
    //
    if (settings_get_helper(settings,
            "Logging", "mswrite_data_encoding_type", mswrite_data_encoding_type, sizeof (mswrite_data_encoding_type), MSWRITE_DATA_ENCODING_TYPE_DEFAULT,
            verbose
            ) == 0) {
        ; // handle error
    }

    //
    if (settings_get_helper(settings,
            "Station", "station_network", station_network, sizeof (station_network), STA_NETWORK_DEFAULT,
            verbose
            ) == 0) {
        ; // handle error
    }
    //
    if (settings_get_helper(settings,
            "Station", "station_name", station_name, sizeof (station_name), STA_NAME_DEFAULT,
            verbose
            ) == 0) {
        ; // handle error
    }
    //
    if (settings_get_helper(settings,
            "Station", "channel_prefix", channel_prefix, sizeof (channel_prefix), STA_CHANNEL_PREFIX_DEFAULT,
            verbose
            ) == 0) {
        ; // handle error
    }
    //
    if (settings_get_helper(settings,
            "Station", "component", component, sizeof (component), STA_COMPONENT_DEFAULT,
            verbose
            ) == 0) {
        ; // handle error
    }
    //
    if (settings_get_int_helper(settings,
            "Station", "nominal_sample_rate", &nominal_sample_rate, STA_NOMINAL_SAMPLE_RATE_DEFAULT,
            verbose
            ) == DBL_INVALID) {
        ; // handle error
    }
    //
    if (settings_get_int_helper(settings,
            "Station", "nominal_gain", &nominal_gain, STA_NOMINAL_GAIN_DEFAULT,
            verbose
            ) == DBL_INVALID) {
        ; // handle error
    }
    //
    if (settings_get_int_helper(settings,
            "Station", "do_settings_sep064", &do_settings_sep064, DO_SETTINGS_SEP064_DEFAULT,
            verbose) == 0) {
        ; // handle error
    }

    return (0);

}

#ifndef WIN32

/** *************************************************************************
 * term_handler:
 * Signal handler routine.
 ************************************************************************* **/
static void term_handler(int sig) {
    disconnect(verbose);
    exit(0);
}
#endif

int main(int argc, char **argv) {

#ifndef WIN32
    // Signal handling, use POSIX calls with standardized semantics
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = term_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
#endif

    /*
    printf("LONG_MIN %ld\n", LONG_MIN);
    printf("LONG_MAX %ld\n", LONG_MAX);
    printf("INT_MIN %d\n", INT_MIN);
    printf("INT_MAX %d\n", INT_MAX);
     */

    // set default error message prefix
    ms_loginit(NULL, NULL, NULL, "ERROR: ");

    // defaults
    verbose = 0;
    strcpy(port_path_hint, "/dev/usbdev1.1");
    strcpy(propfile, PROP_FILE_NAME_DEFAULT);

    // Process input parameters
    if (parameter_proc(argc, argv) < 0)
        return 1;
    init_properties(propfile);

    // set encoding type
    // possible: DE_ASCII, DE_INT16, DE_INT32, DE_FLOAT32, DE_FLOAT64, DE_STEIM1, DE_STEIM2
    // supported: DE_INT16, DE_INT32, DE_STEIM1, DE_STEIM2
    if (strcmp(mswrite_data_encoding_type, "DE_INT16") == 0) {
        mswrite_data_encoding_type_code = DE_INT16;
        num_samples_in_record = (SLREC_DATA_SIZE) / 2;
    } else if (strcmp(mswrite_data_encoding_type, "DE_INT32") == 0) {
        mswrite_data_encoding_type_code = DE_INT32;
        num_samples_in_record = (SLREC_DATA_SIZE) / 4;
        /*
    } else if (strcmp(mswrite_data_encoding_type, "DE_ASCII") == 0) {
        mswrite_data_encoding_type_code = DE_ASCII;
    } else if (strcmp(mswrite_data_encoding_type, "DE_FLOAT32") == 0) {
        mswrite_data_encoding_type_code = DE_FLOAT32;
    } else if (strcmp(mswrite_data_encoding_type, "DE_FLOAT64") == 0) {
        mswrite_data_encoding_type_code = DE_FLOAT64;
         */
    } else if (strcmp(mswrite_data_encoding_type, "DE_STEIM1") == 0) {
        mswrite_data_encoding_type_code = DE_STEIM1;
        num_samples_in_record = (SLREC_DATA_SIZE) / 4; // estimate, inefficient, assumes int32 data
    } else if (strcmp(mswrite_data_encoding_type, "DE_STEIM2") == 0) {
        mswrite_data_encoding_type_code = DE_STEIM2;
        num_samples_in_record = (SLREC_DATA_SIZE) / 4; // estimate, inefficient, assumes int32 data
    }

    // enter infinite loop, term_handler() performs cleanup
    while (1) {

        // find device and connect
        find_device_and_connect(port_path_hint);

        // set sample rate and gain for SEP064
        if (do_settings_sep064) {
            if (set_seo064_sample_rate_and_gain(nominal_sample_rate, nominal_gain, TIMEOUT_SMALL, verbose)) {
                continue;
            }
        }

        // collect data and write
        if (collect_and_write()) { // collect_and_write() returned error
            logprintf(ERROR_FLAG, "Reading from %s, will try reconnecting...\n", port_path);
            disconnect(verbose);
        } else {
            break; // collect_and_write() returned normally
        }

    }

    return (0);

} /* End of main() */

/***************************************************************************
 * find_device_and_connect:
 *
 * Attempt to connect to a device, slows down the loop checking
 * after 20 attempts with a larger delay to reduce pointless
 * work being done.
 *
 * Returns 0 on success and -1 otherwise.
 ***************************************************************************/
int find_device_and_connect(char *port_path_hint) {

    int counter = 0;
    int slow_mode = 0;
    if (verbose)
        logprintf(MSG_FLAG, "Searching for device...\n");
    if (!find_device(port_path_hint, verbose, &port_path, allow_set_interface_attribs)) {
        if (verbose)
            while (!find_device(port_path_hint, verbose, &port_path, allow_set_interface_attribs)) {
                //logprintf(MSG_FLAG, "Still searching for device...\n");
                logprintf(ERROR_FLAG, "Still searching for device.   Try reconnecting (unplug and plug in) the USB Seismometer Interface device.\n");
                if (counter < 20) {
                    counter++;
                    sleep(5);
                } else if (!slow_mode) {
                    if (verbose)
                        logprintf(MSG_FLAG, "Entering slow mode after 20 attempts.\n");
                    slow_mode = 1;
                    sleep(30);
                } else {
                    sleep(30);
                }
            }
    }
    if (verbose)
        logprintf(MSG_FLAG, "Device connected successfully: %s\n", port_path);

    return (0);

}

/***************************************************************************
 * hptime2timestr:
 *
 * Build a time string in filename format from a high precision
 * epoch time.
 *
 * The provided isostimestr must have enough room for the resulting time
 * string of 24 characters, i.e. '2001.07.29.12.38.00.000' + NULL.
 *
 * The 'subseconds' flag controls whether the sub second portion of the
 * time is included or not.
 *
 * Returns a pointer to the resulting string or NULL on error.
 ***************************************************************************/
char *
hptime2timestr(hptime_t hptime, char *timestr, flag subseconds, char* datepath) {
    struct tm tms;
    time_t isec;
    int ifract;
    int ret;

    if (timestr == NULL)
        return NULL;

    /* Reduce to Unix/POSIX epoch time and fractional seconds */
    isec = MS_HPTIME2EPOCH(hptime);
    ifract = (int) (hptime - (isec * HPTMODULUS));

    /* Adjust for negative epoch times */
    if (hptime < 0 && ifract != 0) {
        isec -= 1;
        ifract = HPTMODULUS - (-ifract);
    }

    if (!(gmtime_r(&isec, &tms)))
        return NULL;

    if (subseconds) {
        ifract /= (HPTMODULUS / 1000); // tp milliseconds (assumes HPTMODULUS mulitple of 1000)
        /* Assuming ifract has millisecond precision */
        ret = snprintf(timestr, 24, "%4d.%02d.%02d.%02d.%02d.%02d.%03d",
                tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday,
                tms.tm_hour, tms.tm_min, tms.tm_sec, ifract);
    } else {
        ret = snprintf(timestr, 20, "%4d.%02d.%02d.%02d.%02d.%02d",
                tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday,
                tms.tm_hour, tms.tm_min, tms.tm_sec);
    }
    //printf("DEBUG: timestr= %s\n", timestr);

    if (ret != 23 && ret != 19)
        return NULL;

    ret = snprintf(datepath, 17, "%4d/%02d/%02d/%02d/%02d",
            tms.tm_year + 1900, tms.tm_mon + 1, tms.tm_mday,
            tms.tm_hour, tms.tm_min);

    if (ret != 16)
        return NULL;

    return timestr;

} /* End of ms_hptime2isotimestr() */

static void record_handler (char *record, int reclen, void *handlerdata) {
    char sta_id[11];
    snprintf(sta_id, 11, "%s.%s", station_network, station_name);
    if (send_mseed(sta_id, record, reclen) < 0) {
        logprintf(ERROR_FLAG, "Error sending data to seedlink!\n");
        exit(1);
    }
}

#define DOUBLE long double
//#define DOUBLE double

/***************************************************************************
 * collect_and_write:
 *
 * Attempt to connect to a device, slows down the loop checking
 * after 20 attempts with a larger delay to reduce pointless
 * work being done.
 *
 * Returns 0 on success and -1 otherwise.
 ***************************************************************************/
int collect_and_write() {

    int32_t idata[2000]; // enough space for data of 2 records
    hptime_t hptime;
    hptime_t start_hptime_est = 0;
    hptime_t last_hptime;
    DOUBLE dt, dt_est, sample_rate_est;
    DOUBLE start_hptime_current, record_window_current, record_window_est;
    DOUBLE prev_start_hptime_est = -1;
    int n_start_hptime_est;

    // debug
    hptime_t start_hptime_nominal = 0;
    hptime_t prev_start_next_hptime_est = 0;
    double diff_end, diff_end_cumul = 0.0;

    char seedtimestr[64];

    // decay constant depends on required decay time and sample rate
    //double decay_minutes = 60.0; // 1 hour
    double decay_minutes = 1.0;
    double decay_consant = 1.0 / (decay_minutes * 60.0 * (double) nominal_sample_rate);


    // initialize last_hptime to current time
    last_hptime = current_utc_hptime();

    // initialize dt_est based on nominal sample rate
    dt_est = (nominal_sample_rate == 80) ? 1.0 / SAMP_PER_SEC_80 : (nominal_sample_rate == 40) ? 1.0 / SAMP_PER_SEC_40 : 1.0 / SAMP_PER_SEC_20;
    //	‘a’: 20.032 SPS
    //	‘b’: 39.860 SPS
    //	‘c’: 79.719 SPS
    // initialize record_window_est based on  nominal sample rate and record length
    record_window_est = dt_est * num_samples_in_record;

    if (DEBUG) {
        logprintf(MSG_FLAG, "Initialize: last_hptime=%lld, dt_est=%lld, dt=%lf, dt_end=%lf, dt_end_cumul=%lf)\n",
                last_hptime, dt_est, record_window_est);
    }

    int first = 1;
    MSRecord *pmsrecord = msr_init(NULL);
    strcpy(pmsrecord->network, station_network);
    strcpy(pmsrecord->station, station_name);
    strcpy(pmsrecord->location, "");
    sprintf(pmsrecord->channel, "%s%s", channel_prefix, component);
    pmsrecord->samprate = 1.0;
    pmsrecord->reclen = SLRECSIZE;
    pmsrecord->encoding = mswrite_data_encoding_type_code;
    pmsrecord->byteorder = 1;
    pmsrecord->datasamples = idata;
    pmsrecord->numsamples = 0;
    pmsrecord->sampletype = 'i';

    while (1) {

        // load data up to SLRECSIZE
        long ivalue;
        int nsamp = 0;
        start_hptime_current = 0;
        n_start_hptime_est = 0;
        while (nsamp < num_samples_in_record) {
            ivalue = read_next_value(&hptime, TIMEOUT_LARGE);
            if (ivalue == READ_ERROR || ivalue < MIN_DATA || ivalue > MAX_DATA) {
                logprintf(MSG_FLAG, "READ_ERROR: port=%s, nsamp=%d, ivalue=%ld\n", port_path, nsamp, ivalue);
                pmsrecord->datasamples = NULL;
                msr_free(&pmsrecord);
                return (-1);
            }
            if (DEBUG && nsamp == 0) {
                start_hptime_nominal = hptime;
            }
            idata[pmsrecord->numsamples + nsamp] = (int32_t) ivalue;
            dt = (DOUBLE) (hptime - last_hptime) / (DOUBLE) HPTMODULUS;
            last_hptime = hptime;
            if (verbose > 3) {
                logprintf(MSG_FLAG, "%d %ld %s (dt=%lf)\n", nsamp, ivalue, ms_hptime2seedtimestr(hptime, seedtimestr, 1), (double) dt);
            }
            // estimate start time and dt
            // use only later samples in record since writing previous record may delay reading of first samples of this record
            if (nsamp >= num_samples_in_record / 2) {
                // 20131107 AJL - use all samples, may give better start time estimate, since buffering should compensate for any delay of first samples
                //if (1) {
                // start time estimate is timestamp of current data minus dt_est*nsamp
                start_hptime_current += (hptime - (hptime_t) ((DOUBLE) 0.5 + dt_est * (DOUBLE) HPTMODULUS * (DOUBLE) nsamp));
                n_start_hptime_est++;
                // accumulate dt_est using low-pass filter
                //dt_est = dt_est + (DOUBLE) decay_consant * (dt - dt_est);
            }
            nsamp++;
        }
        start_hptime_current /= n_start_hptime_est;
        if (prev_start_hptime_est > 0) {
            record_window_current = (DOUBLE) (start_hptime_current - prev_start_hptime_est) / (DOUBLE) HPTMODULUS;
        } else {
            record_window_current = record_window_est;
        }
        // accumulate record_window_est using low-pass filter
        record_window_est = record_window_est + (DOUBLE) decay_consant * (record_window_current - record_window_est);
        if (prev_start_hptime_est > 0) {
            start_hptime_est = prev_start_hptime_est + (hptime_t) ((DOUBLE) 0.5 + record_window_est * (DOUBLE) HPTMODULUS);
        } else {
            start_hptime_est = start_hptime_current;
        }
        prev_start_hptime_est = start_hptime_est;
        // test - truncate dt to 1/10000 s to match precision of miniseed btime
        //logprintf(MSG_FLAG, "0 sample_rate_est=%lf (dt=%lfs)\n", (double) ((DOUBLE) 1.0 / dt_est), (double) dt_est);
        dt_est = record_window_est / (DOUBLE) num_samples_in_record;
        sample_rate_est = (DOUBLE) 1.0 / dt_est;
        if (DEBUG) {
            diff_end = (double) (start_hptime_est - prev_start_next_hptime_est) / (double) HPTMODULUS;
            if (!first)
                diff_end_cumul += diff_end;
            logprintf(MSG_FLAG, "sample_rate_est=%lf (dt=%lfs)\n", (double) sample_rate_est, (double) dt_est);
            logprintf(MSG_FLAG, "start_hptime_est=%lld, start_hptime_nominal=%lld, dt=%lf, dt_end=%lf, dt_end_cumul=%lf)\n", start_hptime_est, start_hptime_nominal,
                    (double) ((DOUBLE) (start_hptime_est - start_hptime_nominal) / (DOUBLE) HPTMODULUS), diff_end, diff_end_cumul);
            prev_start_next_hptime_est = start_hptime_est + (hptime_t) ((DOUBLE) 0.5 + dt_est * (DOUBLE) HPTMODULUS * (DOUBLE) nsamp);
        }

        pmsrecord->starttime = start_hptime_est - (DOUBLE) HPTMODULUS * pmsrecord->numsamples / pmsrecord->samprate;
        pmsrecord->samprate = mswrite_header_sample_rate > 0.0 ? mswrite_header_sample_rate : sample_rate_est;
        pmsrecord->numsamples += nsamp;

        int64_t npackedsamples = 0;
        if (msr_pack(pmsrecord, record_handler, NULL, &npackedsamples, 0, verbose) < 0) {
            logprintf(ERROR_FLAG, "Error encoding data!\n");
            exit(1);
        }

        pmsrecord->numsamples -= npackedsamples;
        memmove(&idata[0], &idata[npackedsamples], pmsrecord->numsamples * 4);
    }

    return (0);

}

