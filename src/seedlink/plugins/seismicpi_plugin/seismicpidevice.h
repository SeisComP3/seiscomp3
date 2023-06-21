/** *************************************************************************
 * seismicpidevice.h
 *
 * Interface for managing SEP seismometer over serial connection.
 *
 * Modified from:
 * device/seismicpidevice.py
 * Jon Gilbert 27/01/2011
 *
 * Written by Anthony Lomax
 *   ALomax Scientific www.alomax.net
 *
 * created: 2013.10.28
 * Updated for Mindhat Seismic PI HAT by Kostas Boukouras (kbouk@noa.gr): 2020.11.08
 ***************************************************************************/

// device settings for Seicmic PI HAT 
// PI HAT 16 bit data
#define MIN_DATA -32768
#define MAX_DATA 32767
// a device with <= 32bit data
//#define MIN_DATA -2147483648
//#define MAX_DATA 2147483647
// SEP 064 specific nominal sample rates
// TC1 MIN/MAX Settings
//#define MIN_DATA 0
//#define MAX_DATA 65000

#define SAMP_PER_SEC_32 32
#define SAMP_PER_SEC_64 64
#define SAMP_PER_SEC_128 128
#define DT_MIN_EXPECTED (1.0 / SAMP_PER_SEC_32)

#define TIMEOUT_SMALL 200000    // read timout in microseconds
#define TIMEOUT_LARGE 2000000
#define READ_ERROR LONG_MIN

#define MSG_FLAG 2
#define ERROR_FLAG 2 

int find_device(char* port_path_hint, int verbose, char** pport_path, int allow_set_interface_attribs, int single_multi, int source_select);
long * read_next_value(hptime_t *phptime, int single_multi, long timeout, int source_select);
hptime_t current_utc_hptime();
int logprintf(int is_error, char *fmt, ...);
void disconnect(int verbose);
int set_pihat_sample_rate_and_gain(int nominal_sample_rate, int nominal_gain, int single_multi, int source_select, int  long timeout, int verbose);
