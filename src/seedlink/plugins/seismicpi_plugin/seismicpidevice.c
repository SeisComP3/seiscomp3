/** *************************************************************************
 * seismicpidevice.c
 *
 * Interface for managing SeismicPi HAT over serial connection.
 *
 * Modified from:
 * device/seismicpidevice.py
 * Jon Gilbert, 27/01/2011
 * ALomax Scientific www.alomax.net for SEP device (minilogger_plugin), 28/10/2013
 *
 * By Kostas Boukouras (kbouk@noa.gr) 
 * created: 2020.10.15
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
//
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <usb.h>
#include<termio.h>
#include <linux/serial.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifdef __linux
#include <linux/usbdevice_fs.h>

#endif

#include "libmseed.h"
#include "seismicpi_writer.h"
#include "seismicpidevice.h"

// Device constants
#define DEVICE_RATE 32      // (Hz)

// Excluded ports
// If for any reason some port is off limits, place it in this comma separated list.
//#define EXCLUDED_PORTS "/dev/console,/dev/null,/dev/zero,/dev/full,/dev/random,/dev/urandom"
#define EXCLUDED_PORTS "/dev/stdin,/dev/stdout,/dev/stderr,/dev/console,/dev/null,/dev/zero,/dev/full,/dev/random,/dev/urandom"

#define MAX_NUM_AVAIL_PORTS 4096
static char avail_ports[MAX_NUM_AVAIL_PORTS][512];
static int num_avail_ports = 0;
static char seismometer_port[512];

static int fd_port = -1;
static struct termios termios_orig;
// Serial configuration
#define     SER_BAUD B115200
#define     SER_PARITY 0
#define     SER_TIMEOUT 1
#define     SER_RTS 0

int reset_interface_attribs(int fn_io, struct termios *ptermios_orig) {

    if (tcsetattr(fn_io, TCSANOW, ptermios_orig) != 0) {
        fprintf(stderr, "error %d from reset_interface_attribs:tcsetattr", errno);
        return -1;
    }
    return 0;
}

int set_interface_attribs(int fn_io, int speed, int parity, int timeout, int rts, struct termios *ptermios_orig) {

    // save copy of original attributes
    //if (tcgetattr(fn_io, ptermios_orig) != 0) {
    //    fprintf(stderr, "error %d from set_interface_attribs:tcgetattr", errno);
    //    return -1;
    //}

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    // Set in/out baud rate to be 9600
    //cfsetispeed(&tty, B9600);
    //cfsetospeed(&tty, B9600);

    tty.c_cflag &= ~PARENB; /* no parity */
    tty.c_cflag &= ~CSTOPB; /* 1 stop bit */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; /* 8 bits */
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON; /* non canonical mode */
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_lflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_lflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    //tty.c_oflag &= ~OPOST; /* raw output */

    tcsetattr(fn_io, TCSANOW, &tty); /* apply the settings */
    tcflush(fn_io, TCOFLUSH);

    //if (tcgetattr(fn_io, &tty) != 0) {
    //    fprintf(stderr, "error_0 %d from set_interface_attribs:tcgetattr", errno);
    //    return -1;
    //}


    return 0;
}

/** function to look for available ports and store them in an array of strings.
 */
int scan_for_ports(char* port_path_hint, int verbose) {

    num_avail_ports = 0;

    if (verbose > 2) {
        logprintf(MSG_FLAG, "Adding ports to avail_ports:\n");
    }

    // Make sure set port is at front of list
    strcpy(avail_ports[num_avail_ports], port_path_hint);
    num_avail_ports++;
    if (verbose > 2)
        logprintf(MSG_FLAG, "%s", port_path_hint);

    char dirpath[] = "/dev";
    char device_path[512];
    DIR* pDir = opendir(dirpath);
    struct dirent *pFile = NULL;
    while ((pFile = readdir(pDir)) && num_avail_ports < MAX_NUM_AVAIL_PORTS) {

        sprintf(device_path, "%s/%s", dirpath, pFile->d_name);

        if (!strcmp(pFile->d_name, ".") || !strcmp(pFile->d_name, "..")) {
            continue;
        }

        // Remove any excluded entries.
        if (strstr(EXCLUDED_PORTS, device_path) != NULL) {
            continue;
        }

        // Add port to list
        strcpy(avail_ports[num_avail_ports], device_path);
        num_avail_ports++;

        if (verbose > 2)
            logprintf(MSG_FLAG, "%s\n", device_path);

    }
    closedir(pDir);

    if (verbose > 1) {
        logprintf(MSG_FLAG, "num_avail_ports=%d\n", num_avail_ports);
    }

    return (num_avail_ports);

}

int inputAvailable(int fn_io) {

    static struct timeval tv;
    fd_set rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(fn_io, &rfds);
    //select(fn_io + 1, &rfds, NULL, NULL, &tv);
    //return (FD_ISSET(fn_io, &rfds));
    int retval = select(fn_io + 1, &rfds, NULL, NULL, &tv);
    if (retval == -1) {
        //perror("select()");
        return (0);
    } else if (retval) {
        return (1);
    } else {
        return (0);
    }


}

/* function to return the value in the next line in the buffer. Timeouts based on settings.
 * On a timeout or no connection present, returns READ_ERROR.
 */
long * read_next_value(hptime_t *phptime, int single_multi, long timeout, int source_select) {

#define MAX_LINE_DATA 32

    static char line_data[MAX_LINE_DATA + 1];
    static char line_data1[MAX_LINE_DATA + 1];
    static char line_data2[MAX_LINE_DATA + 1];
    static char line_data3[MAX_LINE_DATA + 1];
    static char line_data4[MAX_LINE_DATA + 1];
    static int n_line_data;
    char buf[1];

    n_line_data = 0;
    memset(line_data, 0, sizeof line_data1); 
    long total_sleep = 0;

    // skip any leading \r\n chars
    // if (total_sleep >= timeout) {
    //     return (READ_ERROR);
    // }

    // skip any leading \r\n chars
    while (total_sleep < timeout && n_line_data < MAX_LINE_DATA) {
        //if (timeout == TIMEOUT_LARGE)
        //    printf("inputAvailable(fd_port):%d\n", inputAvailable(fd_port));
        if (inputAvailable(fd_port) && read(fd_port, buf, sizeof buf) == 1) { // read up to 1 characters if ready to read)
            if (buf[0] == '\r' || buf[0] == '\n') {
            //if (buf[0] > -32769 && buf[0] < 327678) 
              //  continue;
            }
			line_data[n_line_data] = buf[0];
			n_line_data++;
            break;
        } else {
            usleep(10000);
           total_sleep += 10000;
        }
    }
    
   // if (total_sleep >= timeout) {
   //     return (READ_ERROR);
   // }

    // get data timestamp
    *phptime = current_utc_hptime();

    // read value chars
    while (total_sleep < timeout && n_line_data < MAX_LINE_DATA) {
        if (inputAvailable(fd_port) && read(fd_port, buf, sizeof buf)) { // read up to 1 characters if ready to read)
            if (buf[0] == '\r' || buf[0] == '\n') {
                break;
            }
            line_data[n_line_data] = buf[0];
            
            n_line_data++;
        } else {
            usleep(10000);
            total_sleep += 10000;
        }
    }
    	
    if (single_multi != 1) {
      memset(line_data1, 0, sizeof line_data1);   
      memset(line_data2, 0, sizeof line_data2);   
      memset(line_data3, 0, sizeof line_data3);   
      memset(line_data4, 0, sizeof line_data4);   
      int ind0=0;
      int ind1=0;
      int ind2=0;
      int ind3=0;
      int ind4=0;
      int sel_ch1=1;
      int sel_ch2=0;
      int sel_ch3=0;
      int sel_ch4=0;
     /* Write 4-Channel Data*/
     
      while (ind0 <= n_line_data) {

    	      if ((line_data[ind0] != ',')) {

                if ((sel_ch1==1) && (sel_ch2==0) && (sel_ch3==0) && (sel_ch4==0)) {
					line_data1[ind1]=line_data[ind0];
					//logprintf(MSG_FLAG, "LINE_DATA1: %s\n", line_data1);
					ind1++;
                }
		        else if ((sel_ch1==0) && (sel_ch2==1) && (sel_ch3==0) && (sel_ch4==0)) {
                  line_data2[ind2]=line_data[ind0];
                  //logprintf(MSG_FLAG, "LINE_DATA2: %s\n", line_data2);
                  ind2++;
                }
                else if ((sel_ch1==0) && (sel_ch2==0) && (sel_ch3==1) && (sel_ch4==0)) {
                  line_data3[ind3]=line_data[ind0];
                  //logprintf(MSG_FLAG, "LINE_DATA3: %s\n", line_data3);
                  ind3++;
                }
		        else if ((sel_ch1==0) && (sel_ch2==0) && (sel_ch3==0) && (sel_ch4==1)) {
                  line_data4[ind4]=line_data[ind0];
                  //logprintf(MSG_FLAG, "LINE_DATA4: %s\n", line_data4);
                  ind4++;
                }
	      }

	    if (line_data[ind0]==',') {
                if ((sel_ch1==1) && (sel_ch2==0) && (sel_ch3==0) && (sel_ch4==0)) {
                  sel_ch1=0;
				  sel_ch2=1;
				  sel_ch3=0;
				  sel_ch4=0;
	        }
		        else if ((sel_ch1==0) && (sel_ch2==1) && (sel_ch3==0) && (sel_ch4==0)) {
                  sel_ch1=0;
                  sel_ch2=0;
                  sel_ch3=1;
                  sel_ch4=0;
                }
		        else if ((sel_ch1==0) && (sel_ch2==0) && (sel_ch3==1) && (sel_ch4==0)) {
                  sel_ch1=0;
                  sel_ch2=0;
                  sel_ch3=0;
                  sel_ch4=1;
                }
              }

              ind0++;
          }
    }

    //if (total_sleep >= timeout) {
    //    return (READ_ERROR);
   // }

    if (single_multi != 1) {
			static long values[4];
			long value1 = atol(line_data1);
			long value2 = atol(line_data2);
			long value3 = atol(line_data3);
			long value4 = atol(line_data4);
			values[0]=value1;
			values[1]=value2;
			values[2]=value3;
			values[3]=value4;
		    
      return values;
    }
    else {
     static long value[1];
     value[0] = atol(line_data);
     //logprintf(MSG_FLAG, "N_LINE_DATA: %d\n", n_line_data);
     //logprintf(MSG_FLAG, "LINE_DATA: %s\n", line_data);
     //logprintf(MSG_FLAG, "VALUE: %d\n", value);

     return value;
   }
}

#define NUM_DATA_TEST 20
#define MAX_NUM_TEST_READS 1000
#define SAMP_RATE_TOLERANCE 0.05    // 5%

/* function to find a serial or serial/usb connection to the seismometer.
 */
int find_device(char* port_path_hint, int verbose, char** pport_path, int allow_set_interface_attribs, int single_multi, int source_select) {

    hptime_t hptime;
    hptime_t last_hptime;
    double dt, dt_sum;
    int n_dt, n_dt_sum;

    *pport_path = NULL;

    //init_usb();

    scan_for_ports(port_path_hint, verbose);

    // check ports one at a time. Will return when the device is found or when all are checked.
    int n;
    for (n = 0; n < num_avail_ports; n++) {
        //for (n = num_avail_ports - 1; n >= 0; n--) {

        if (verbose > 1) {
            logprintf(MSG_FLAG, "Trying port: %s\n", avail_ports[n]);
        }

        // 20131219 AJL fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK | O_NOCTTY | O_DSYNC);
        // 20140108 AJL fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK | O_NOCTTY | O_SYNC);
        fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK);
        if (fd_port < 0) {
            //fprintf(stderr, "error %d opening %s: %s", errno, portname, strerror(errno));
            if (verbose > 1)
                logprintf(MSG_FLAG, "   Failed to open port: %s\n", avail_ports[n]);
           //continue;
        }
        /*#ifdef __linux
                //
                printf("Resetting USB device %s: ", avail_ports[n]);
                int rc = ioctl(fd_port, USBDEVFS_RESET, 0);
                if (rc < 0) {
                    printf(" error: %d\n", rc);
                } else {
                    printf(" success\n");
                }
        #endif*/
        // close and re-open port as this may help re-set port hardware (???)
        close(fd_port);
        fd_port = -1;
        // 20131219 AJL fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK | O_NOCTTY | O_DSYNC);
        // 20140108 AJL fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK | O_NOCTTY | O_SYNC);
        fd_port = open(avail_ports[n], O_RDWR | O_NONBLOCK);
        if (fd_port < 0) {
            //fprintf(stderr, "error %d opening %s: %s", errno, portname, strerror(errno));
            if (verbose > 1)
                logprintf(MSG_FLAG, "   Failed to re-open port: %s\n", avail_ports[n]);
            //continue;
        }
        // 20140108 AJL - added:
        //if (allow_set_interface_attribs)
        set_interface_attribs(fd_port, SER_BAUD, SER_PARITY, SER_TIMEOUT, SER_RTS, &termios_orig);

        if (verbose > 1) {
            logprintf(MSG_FLAG, "   Port successfully opened: %s\n", avail_ports[n]);
        }
        //sleep(2);

        // Test, take 3 values, if they are all good, return true.
        if (verbose > 1)
            logprintf(MSG_FLAG, "   Testing serial connection...\n");
        dt_sum = 0.0;
        n_dt = 0;
        n_dt_sum = 0;
        last_hptime = -1;
        int i = 0;
        for (i = 0; i < MAX_NUM_TEST_READS && n_dt < NUM_DATA_TEST; i++) {
            long *testd;
            testd = read_next_value(&hptime, single_multi, TIMEOUT_SMALL, source_select);
	    long testdata = *(testd);
            // message
            if (verbose > 1) {
                printf("   i=%d testdata=%ld", i, testdata);
            }
            // check value
            if (testdata == READ_ERROR || testdata < MIN_DATA || testdata > MAX_DATA) {
                if (allow_set_interface_attribs)
                    //reset_interface_attribs(fd_port, &termios_orig);
                close(fd_port);
                fd_port = -1;
                if (verbose > 1) {
                    printf("\n");
                }
                break;
            }
            // get dt
            dt = (double) (hptime - last_hptime) / (double) HPTMODULUS;
            if (verbose > 1) {
                printf(" dt=%lf", dt);
            }
            // if dt much less than expected, may be buffered data
            if (n_dt < 1 && dt < DT_MIN_EXPECTED / 10.0) {
                if (verbose > 1) {
                    printf("\n");
                }
                continue;
            }
            // accumulate dt
            if (n_dt > NUM_DATA_TEST / 2 && last_hptime > 0) {
                dt_sum += dt;
                n_dt_sum++;
            } else {
                if (verbose > 1) {
                    printf("X");
                }
            }
            n_dt++;
            if (verbose > 1) {
                printf("\n");
            }
            last_hptime = hptime;
        }
        // Got some data, connection may be good.
        if (n_dt >= NUM_DATA_TEST) {
            // check sample rate against USB Seismometer Interface rates
            double sample_rate_mean = (double) n_dt_sum / dt_sum;
            if (verbose > 1)
                logprintf(MSG_FLAG, "   sample_rate_mean=%lf\n", sample_rate_mean);

            if (
                    (fabs(sample_rate_mean - SAMP_PER_SEC_32) / SAMP_PER_SEC_32 < SAMP_RATE_TOLERANCE) ||
                    (fabs(sample_rate_mean - SAMP_PER_SEC_64) / SAMP_PER_SEC_64 < SAMP_RATE_TOLERANCE) ||
                    (fabs(sample_rate_mean - SAMP_PER_SEC_128) / SAMP_PER_SEC_128 < SAMP_RATE_TOLERANCE)
                    ) {
                if (verbose > 1)
                    logprintf(MSG_FLAG, "   Serial connection good.\n");
                strcpy(seismometer_port, avail_ports[n]);
                *pport_path = seismometer_port;
                return (1);
            }

        }
        if (verbose > 1)
            logprintf(MSG_FLAG, "   Serial connection failed.\n");

        if (allow_set_interface_attribs)
            //reset_interface_attribs(fd_port, &termios_orig);
        close(fd_port);
        fd_port = -1;

    }

    return (0);

}

/* Disconnect from any opened ports.
 */
void disconnect(int verbose) {

    if (fd_port >= 0) {
        //reset_interface_attribs(fd_port, &termios_orig);
        if (close(fd_port)) {
            logprintf(ERROR_FLAG, "closing port: %s : %s\n", seismometer_port, strerror(errno));
        } else {
            if (verbose)
                logprintf(MSG_FLAG, "Port successfully closed: %s\n", seismometer_port);
        }
    }

}

/* function to set device sample rate and gain on device port.
 */
int set_pihat_sample_rate_and_gain(int nominal_sample_rate, int nominal_gain, int single_multi, int source_select, long timeout, int verbose) {

    char buf[1];

    long total_sleep = 0;

    // nominal_gain
    //Nominal gain, one of 1, 2 or 4. type=string default=1
    // 	The gain can be adjusted by sending single characters to the Virtual Com Port:
    //	‘1’: ×1 = 0.64μV/count
    //	‘2’: ×2 = 0.32μV/count
    //	‘4’: ×2 = 0.16μV/count
    //	‘8’: ×2 = 0.08μV/count

    if (nominal_gain == 8) {
        buf[0] = '8';
    } else if (nominal_gain == 4) {
        buf[0] = '4';
    } else if (nominal_gain == 2) {
        buf[0] = '2';
    } else {
        buf[0] = '1';
    }
    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            if (verbose) {
                logprintf(MSG_FLAG, "Set nominal_gain to: %d (\'%c\' on port)\n", nominal_gain, buf[0]);
            }
            break;
        }
        usleep(10000);
        //tal_sleep += 10000;
    }

    buf[0] = '\n';
    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            if (verbose) {
                logprintf(MSG_FLAG, "Set nominal_gain to: %d (\'%c\' on port)\n", nominal_gain, buf[0]);
            }
            break;
        }
        usleep(10000);
        //tal_sleep += 10000;
    }

    if (nominal_sample_rate == 32) {
	    buf[0] = 'a';
    } else if (nominal_sample_rate == 64) {
            buf[0] = 'b'; 
    } else {
	    buf[0] = 'c';
    }

    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            if (verbose) {
                logprintf(MSG_FLAG, "Set nominal_sample_rate to: %d (\'%c\' on port)\n", nominal_sample_rate, buf[0]);
            }
            break;
        }
        usleep(10000);
        //total_sleep += 10000;
    }

    buf[0] = '\n';
    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            break;
        }
        usleep(10000);
        //total_sleep += 10000;
    }

    if (source_select == 1) {
        buf[0] = 'i';
    }
    else {
        buf[0] = 'e';
    }

    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            if (verbose) {
                logprintf(MSG_FLAG, "Set source_select to: %d (\'%c\' on port)\n", source_select, buf[0]);
            }
            break;
        }
        usleep(10000);
        //total_sleep += 10000;
    }

    buf[0] = '\n';
    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            break;
        }
        usleep(10000);
        //total_sleep += 10000;
    }

    if (single_multi == 1) {
        buf[0] = 's';
    }
    else {
        buf[0] = 'm';
    }

    while (total_sleep < timeout) {
        if (write(fd_port, buf, sizeof buf) == 1) {
            if (verbose) {
                logprintf(MSG_FLAG, "Set single_multi to: %d (\'%c\' on port)\n", single_multi, buf[0]);
            }
            break;
        }
        usleep(10000);
        //total_sleep += 10000;
    }


    //total_sleep = 0;

    return (0);

}

void current_utc_time(struct timespec * ts) {

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif

}


#define NANO 1000000000

hptime_t timespec2hptime(struct timespec* ts) {

    time_t itime_sec = (time_t) ts->tv_sec;
    struct tm* tm_time = gmtime(&itime_sec);

    long hptime_sec_frac = (long) ((double) ts->tv_nsec / ((double) NANO / (double) HPTMODULUS));
    int year = tm_time->tm_year + 1900;
    int month = tm_time->tm_mon + 1;
    int mday = tm_time->tm_mday;
    int jday;
    ms_md2doy(year, month, mday, &jday);

    //printf("DEBUG: timespec2hptime(): %d.%d-%d:%d:%d.(%ld) -> %lld\n",
    //        year, jday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, hptime_sec_frac,
    //        ms_time2hptime(year, jday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, hptime_sec_frac));
    return (ms_time2hptime(year, jday, tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, hptime_sec_frac));

}

hptime_t current_utc_hptime() {

    static struct timespec time_spec;

    // get data timestamp
    current_utc_time(&time_spec);
    //printf("DEBUG: time_spec.tv_sec %ld, time_spec.tv_nsec %ld, timespec2hptime(&time_spec) %lld\n", time_spec.tv_sec, time_spec.tv_nsec, timespec2hptime(&time_spec));
    return (timespec2hptime(&time_spec));

}

static char NO_PREFIX[] = "";
static char ERROR_PREFIX[] = "ERROR: ";

/***************************************************************************
 * logprintf:
 *
 * A generic log message handler, pre-pends a current date/time string
 * to each message.  This routine add a newline to the final output
 * message if it is not included with the message.
 *
 * Returns the number of characters in the formatted message.
 ***************************************************************************/
int logprintf(int is_error, char *fmt, ...) {
    int rv = 0;
    char message[200];
    va_list argptr;
    struct tm *tp;
    time_t curtime;

    char *day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
        "Aug", "Sep", "Oct", "Nov", "Dec"};

    /* Build local time string and generate final output */
    curtime = time(NULL);
    tp = localtime(&curtime);

    va_start(argptr, fmt);
    rv = vsnprintf(message, sizeof (message), fmt, argptr);
    va_end(argptr);

    FILE* fpout = stdout;
    char* prefix = NO_PREFIX;
    if (is_error) {
        fpout = stderr;
        prefix = ERROR_PREFIX;
    }
    fprintf(fpout, "%3.3s %3.3s %2.2d %2.2d:%2.2d:%2.2d %4.4d - %s%s",
            day[tp->tm_wday], month[tp->tm_mon], tp->tm_mday,
            tp->tm_hour, tp->tm_min, tp->tm_sec, tp->tm_year + 1900,
            prefix, message);
    if (message[strlen(message) - 1] != '\n')
        fprintf(fpout, "\n");
    fflush(fpout);

    return rv;
} /* End of lprintf() */


