/** *************************************************************************
 * minisepdevice.h
 *
 * Interface for managing SEP seismometer over serial connection.
 *
 * Modified from:
 * device/minisepdevice.py
 * Jon Gilbert 27/01/2011
 *
 * Written by Anthony Lomax
 *   ALomax Scientific www.alomax.net
 *
 * created: 2013.10.28
 ***************************************************************************/

// device settings for USB SEP 064 Seismometer Interface
// SEP 064 16 bit data
#define MIN_DATA -32768
#define MAX_DATA 32767
// a device with <= 32bit data
//#define MIN_DATA -2147483648
//#define MAX_DATA 2147483647
// SEP 064 specific nominal sample rates
#define SAMP_PER_SEC_20 20.032
#define SAMP_PER_SEC_40 39.860
#define SAMP_PER_SEC_80 79.719
#define DT_MIN_EXPECTED (1.0 / SAMP_PER_SEC_80)

#define TIMEOUT_SMALL 100000    // read timout in microseconds
#define TIMEOUT_LARGE 1000000
#define READ_ERROR LONG_MIN

#define MSG_FLAG 0
#define ERROR_FLAG 1

int find_device(char* port_path_hint, int verbose, char** pport_path, int allow_set_interface_attribs);
long read_next_value(hptime_t *phptime, long timeout);
hptime_t current_utc_hptime();
int logprintf(int is_error, char *fmt, ...);
void disconnect(int verbose);
int set_seo064_sample_rate_and_gain(int nominal_sample_rate, int nominal_gain, long timeout, int verbose);
