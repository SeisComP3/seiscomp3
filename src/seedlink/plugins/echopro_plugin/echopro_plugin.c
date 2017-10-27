// *****************************************************************
// SeisComP 3 plugin for the Kelunji Echo/EchoPro digitizers.
//
// Sends data to SeisComP using the send_raw3() function defined
// in 'plugin.c'. Program will exit if a fatal error is detected
// at startup. If started with -d (debug) option then all serial
// port data will be written to stdout.
//
// Version: 1.0
//
// Author : Oyvind Natvik (UiB, Department of Earth Science)
//
// License: This code may be used and modified freely by anyone.
//
// *****************************************************************

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdbool.h>
#include "plugin.h"

#define MAX_SMPL 2000
#define MAX_CHNL 12

// Global variables.
int fd;
struct termios PrevSettings;


// Termination handler for 'TERM' and 'INT' signals.
void termination_handler (int signum)
{
   // Restore serial port settings.
   tcsetattr(fd, TCSANOW, &PrevSettings);
   // Close the serial port.
   close(fd);
   // Stop the program.
   exit(EXIT_SUCCESS);
}



// Configure serial port.
bool configure_serial_port()
{
   int result;
   struct termios Settings;

   // Prepare configuration. Raw mode is used.
   tcgetattr(fd, &Settings); 
   PrevSettings = Settings;
   cfsetispeed(&Settings, B57600);
   cfsetospeed(&Settings, B57600);
   Settings.c_cflag |= (CREAD | CLOCAL);
   Settings.c_cflag &= ~(CSTOPB | CRTSCTS);      
   cfmakeraw(&Settings);              
   Settings.c_cc[VMIN] = 1;
   Settings.c_cc[VTIME] = 0;

   // Enable configuration. Exit if configuration cannot be enabled.
   if ((result = tcsetattr(fd, TCSANOW, &Settings)) < 0) return false;

   return true;
}



// Prints correct usage and exits the program.
void print_usage() 
{
   // Print usage information.
   printf("Usage: echopro_plugin -s <station> -p <serial-port> [-d] <instance name>\n");
   printf("Example: echopro_plugin -s BER -p /dev/ttyS0 echpro0\n");
   printf("Use '-d' parameter to run plugin outside of SeisComP in debug mode.\n\n");
   exit(EXIT_FAILURE);
}



// Read a line from the serial port. Function will
// read until a LF charcter is received. Function
// does not return CR/LF characters in 'line'.
void read_line(char* line, int linesize)
{
   int numread;
   char cha[2];
   
   // Initialize.
   *line = cha[1] = 0;
   
   // Read a line from file descriptor. 
   while (true) {
      if ((numread = read(fd, cha, 1)) ==  1) {
         if (cha[0] == '\n') break;
         if (cha[0] != '\r') strcat(line, cha);
      }
   }
}



// Read and dump lines until we reach the start of a
// new block. A new block starts with an empty line.
void skip_to_next_block()
{
   char line[96];
   
   read_line(line, sizeof(line));
   while (strlen(line)) read_line(line, sizeof(line));
}



// Checks the time/date line for errors. 
// Function returns 'true' if line is OK.
bool timeline_ok(char line[])
{
   // Check that all seperator characters
   // are located in the right positions.
   if (strlen(line) != 19) return false;
   if (line[4] != '-') return false;
   if (line[7] != '-') return false;
   if (line[13] != ':') return false;
   if (line[16] != ':') return false;
   
   return true;
}



// Find sample rate and number of channels by
// analyzing a block of data from the digitizer.
void analyse_data_block(int *SampleRate, int *NumChannels)
{
   char line[96];
   
   // Initialize.
   *SampleRate = 0; *NumChannels = 0;
   
   // Skip to start of block.
   skip_to_next_block();
   
   // Read the time/date line.
   read_line(line, sizeof(line));
   if (!timeline_ok(line)) return;
      
   // Read the first data line.
   read_line(line, sizeof(line));
   if (strlen(line) < 20) return;

   // Return number of channels.
   *NumChannels = (strlen(line) + 1) / 7;

   // Read the rest of the data lines in the block.
   // We will read until the blank line is read.
   // The number of data lines equals the sample rate.
   while (strlen(line)) { read_line(line, sizeof(line)); (*SampleRate)++; }

}



// Calculate day-of-year from given date.
int get_doy(int year, int month, int day)
{
   static const int days[2][13] = {
      {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
      {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
   };
   return days[(year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)][month]+day;
}



// Extract time/date from a time line,
// and save it into a ptime structure.
bool convert_time(char *line, struct ptime *BlockTime)
{
   char *endptr;
   long month, day;
   
   BlockTime->year = strtol(line, &endptr, 10);
   if (line == endptr) return false;
   month = strtol(line+5, &endptr, 10);
   if (line+5 == endptr) return false;
   day = strtol(line+8, &endptr, 10);
   if (line+8 == endptr) return false;
   BlockTime->yday = get_doy(BlockTime->year, month, day);
   BlockTime->hour = strtol(line+11, &endptr, 10);
   if (line+11 == endptr) return false;
   BlockTime->minute = strtol(line+14, &endptr, 10);
   if (line+14 == endptr) return false;
   BlockTime->second = strtol(line+17, &endptr, 10);
   if (line+17 == endptr) return false;
   BlockTime->usec = 0;
      
   return true;
}



// Returns current UTC time in a ptime structure.
void get_utc_time_str(char *timestr)
{
   time_t now = time(NULL);
   struct tm *time = gmtime(&now);
   *timestr = 0;
   strftime(timestr, 32,"%a %b %d %H:%M:%S %Y", time);
}



// Converts a two's complements hex value
// from the data line to a decimal number.
// Function returns 0 if hex value is corrupt.
long to_decimal(char *hexval)
{
   long number;
   bool negative;
   char *endptr;   
   
   number = strtol(hexval, &endptr, 16);
   if (endptr == hexval) return 0;
   negative = (number & (1 << 23)) != 0;
   if (negative) return number | ~((1 << 24)-1);
   
   return number;   
}



// Checks the detected sample rate and number
// of channels. Returns false if these are bad.
bool check_srate_and_numcha(int NC, int SR)
{
   if (NC != 3 && NC != 4 && NC != 6 && NC != 12) return false;
   if (SR != 10 && SR != 20 && SR != 25 && SR != 50 && SR != 100 &&
       SR != 200 && SR != 250 && SR != 500 && SR != 1000 && SR != 2000) return false;

   return true;
}



// ------------------------------------------
// Main program.
// ------------------------------------------
int main(int argc, char *argv[])
{
   bool DEBUG = false;
   int32_t data[MAX_CHNL][MAX_SMPL];
   int CmdLnPar, NumChannels, Index, SampleRate, SmplNum, ChnlNum;
   char ChaNames[12][3] = {"1","2","3","4","5","6","7","8","9","10","11","12"};   
   char Line[96], HexVal[8], StaName[16], DevName[32], ProcName[64], TimeStr[32];
   struct ptime BlockTime; struct sigaction Action;
   
   // Check number of command line arguments.
   if (argc < 6 || argc > 7) print_usage();
   
   // Get and verify command line arguments.
   while ((CmdLnPar = getopt(argc, argv,"s:p:d:")) != -1) {
      switch (CmdLnPar) {
         case 's' : 
         // Get station name.
         strcpy(StaName, optarg);
         break;

         case 'p' : 
         // Get device name of serial port.
         strcpy(DevName, optarg);
         break;

         case 'd' : 
         // Start in debug mode.
         DEBUG = true;
         break;

         case '?' : 
         // Got an unknown parameter.
         print_usage();
         break; 

         default: 
         print_usage(); 
      }
   }

   // Get name of this plugin instance.
   strcpy(ProcName, argv[argc-1]);

   // Set up the termination handler.
   // SeisComP will use SIGTERM to stop us. 
   memset (&Action, '\0', sizeof(Action));
   Action.sa_handler = &termination_handler;
   sigaction(SIGINT, &Action, NULL);    
   sigaction(SIGTERM, &Action, NULL); 

   // Open serial port for reading.
   get_utc_time_str(TimeStr);
   fd = open(DevName, O_RDONLY | O_NOCTTY);
   if (fd == -1) {
      printf("%s - %s: Unable to open serial port. Check permissions on '%s'.\n", TimeStr, ProcName, DevName);
      fflush(stdout); return 1;
   }
   
   // Verify that the given serial port is a tty.
   if (!isatty(fd)) {
      printf("%s - %s: The given serial port is a not a tty.\n", TimeStr, ProcName);
      fflush(stdout); close(fd); return 1;
   }

   // Configure the serial port. Baud rate is fixed to 57600 bps.
   if (!configure_serial_port()) {
      printf("%s - %s: Unable to configure serial port.\n", TimeStr, ProcName);
      fflush(stdout); close(fd); return 1;
   }

   // Analyse incoming data blocks and find sample rate and number of channels.
   // Keep on analysing in a loop until we get valid values.
   SampleRate = NumChannels = 0;
   while (!check_srate_and_numcha(NumChannels, SampleRate)) analyse_data_block(&SampleRate, &NumChannels);

   // Write detected sample rate and number of channels to the seedlink log file.
   get_utc_time_str(TimeStr);
   printf("%s - %s: Detected %d channles and %d samples per second.\n", TimeStr, ProcName, NumChannels, SampleRate); 
   fflush(stdout);

   //
   // Start processing incoming serial data in a loop.
   //
   while (true) {
      // Start reading a new block. First, read the date/time line.
      read_line(Line, sizeof(Line));
      if (DEBUG) printf("Receiving on serial port:\n%s\n", Line);

      // Check if the line is OK.
      if (!timeline_ok(Line)) {
         // The time/date line is corrupt. Report this problem in the seedlink log file.
         get_utc_time_str(TimeStr);
         printf("%s - %s: A bad line was received. Skipping this block.\n", TimeStr, ProcName); 
         fflush(stdout);
         // This block is bad. Skip to the next block.
         skip_to_next_block(); continue; 
      }   
      
      // Save the date/time of block in a ptime structure.
      convert_time(Line, &BlockTime);

      // Read the data lines. The block is done when a blank line is read.
      memset(data, 0, sizeof(data)); SmplNum = 0; 
      while (strlen(Line)) {
         // Read a data line.
         read_line(Line, sizeof(Line)); if (DEBUG) printf("%s\n", Line);
         
         // Line is empty. This block is done. Send block data to SeisComP. 
         if (!strlen(Line)) {
            for (ChnlNum=0;ChnlNum<NumChannels;ChnlNum++) {
               send_raw3(StaName, ChaNames[ChnlNum], &BlockTime, 0, 50, &data[ChnlNum][0], SampleRate);
            }
            if (DEBUG) {
               printf("Sending data to SeisComP:\n");
               printf("%d-%d %d:%d:%d:00\n", BlockTime.year, BlockTime.yday, BlockTime.hour, BlockTime.minute, BlockTime.second);
               for (ChnlNum=0;ChnlNum<NumChannels;ChnlNum++) printf("%s\t", ChaNames[ChnlNum]); printf("\n");
               for (Index=0;Index<SampleRate;Index++) {
                  for (ChnlNum=0;ChnlNum<NumChannels;ChnlNum++) printf("%d\t", data[ChnlNum][Index]);
                  printf("\n");
               }
               printf("\n");
            }  
         }
         
         // Line is not empty. Get data samples from line.
         if (strlen(Line)) {
            // Do a simple line check. Check the data line length.
            if (strlen(Line) == 7*NumChannels-1) {
               // Line length is OK. We assume the line is OK.
               for (ChnlNum=0;ChnlNum<NumChannels;ChnlNum++) {
                  // Get the hexdecimal string for a value. 
                  strncpy(HexVal, &Line[ChnlNum*7], 6);
                  // Convert, and store value in the data table.
                  // If HexVal string is bad then to_decimal() will return 0.
                  data[ChnlNum][SmplNum] = to_decimal(HexVal);
               }
               // Increase sample counter.
               SmplNum++;
            } else {
               // Line has wrong length. Log this incident to the seedlink log file.
               get_utc_time_str(TimeStr);
               printf("%s - %s: A bad line was received. Skipping this block.\n", TimeStr, ProcName); 
               fflush(stdout);
               // This block is bad. Skip to the next block.
               skip_to_next_block(); break;
            } 
         } 
      }
   }   
   
   return 0;
}


