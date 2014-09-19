/*  open2300  - Linux specific library functions
 *  This file contains the common functions that are unique to
 *  Linux. The entire file is ignored in case of Windows
 *  
 *  Version 1.10
 *  
 *  Control WS2300 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen
 *  This program is published under the GNU General Public license
 */

#ifndef WIN32
#define DEBUG 0

#include <errno.h>
#include <sys/file.h>
#include "rw2300.h"

/********************************************************************
 * open_weatherstation, Linux version
 *
 * Input:   devicename (/dev/tty0, /dev/tty1 etc)
 * 
 * Returns: Handle to the weatherstation (type WEATHERSTATION)
 *
 ********************************************************************/
WEATHERSTATION open_weatherstation(char *device)
{
	WEATHERSTATION ws2300;
	struct termios adtio;
	int portstatus;

	//Setup serial port

	if ((ws2300 = open(device, O_RDWR | O_NOCTTY)) < 0)
	{
		printf("\nUnable to open serial device %s\n", device);
		exit(EXIT_FAILURE);
	}
	
	if ( flock(ws2300, LOCK_EX) < 0 ) {
		perror("\nSerial device is locked by other program\n");
		exit(EXIT_FAILURE);
	}
	
	//We want full control of what is set and simply reset the entire adtio struct
	memset(&adtio, 0, sizeof(adtio));
	
	//tcgetattr(ws2300, &adtio);   // Commented out and replaced by the memset above
	
	// Serial control options
	adtio.c_cflag &= ~PARENB;      // No parity
	adtio.c_cflag &= ~CSTOPB;      // One stop bit
	adtio.c_cflag &= ~CSIZE;       // Character size mask
	adtio.c_cflag |= CS8;          // Character size 8 bits
	adtio.c_cflag |= CREAD;        // Enable Receiver
	adtio.c_cflag &= ~HUPCL;       // No "hangup"
	adtio.c_cflag &= ~CRTSCTS;     // No flowcontrol
	adtio.c_cflag |= CLOCAL;       // Ignore modem control lines

	// Baudrate, for newer systems
	cfsetispeed(&adtio, BAUDRATE);
	cfsetospeed(&adtio, BAUDRATE);	
	
	// Serial local options: adtio.c_lflag
	// Raw input = clear ICANON, ECHO, ECHOE, and ISIG
	// Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
	// So we actually clear all flags in adtio.c_lflag
	adtio.c_lflag = 0;

	// Serial input options: adtio.c_iflag
	// Disable parity check = clear INPCK, PARMRK, and ISTRIP 
	// Disable software flow control = clear IXON, IXOFF, and IXANY
	// Disable any translation of CR and LF = clear INLCR, IGNCR, and ICRNL	
	// Ignore break condition on input = set IGNBRK
	// Ignore parity errors just in case = set IGNPAR;
	// So we can clear all flags except IGNBRK and IGNPAR
	adtio.c_iflag = IGNBRK|IGNPAR;
	
	// Serial output options
	// Raw output should disable all other output options
	adtio.c_oflag &= ~OPOST;

	adtio.c_cc[VTIME] = 10;		// timer 1s
	adtio.c_cc[VMIN] = 0;		// blocking read until 1 char
	
	if (tcsetattr(ws2300, TCSANOW, &adtio) < 0)
	{
		printf("Unable to initialize serial device");
		exit(0);
	}

	tcflush(ws2300, TCIOFLUSH);

	// Set DTR low and RTS high and leave other ctrl lines untouched

	ioctl(ws2300, TIOCMGET, &portstatus);	// get current port status
	portstatus &= ~TIOCM_DTR;
	portstatus |= TIOCM_RTS;
	ioctl(ws2300, TIOCMSET, &portstatus);	// set current port status

	return ws2300;
}

/********************************************************************
 * close_weatherstation, Linux version
 *
 * Input: Handle to the weatherstation (type WEATHERSTATION)
 *
 * Returns nothing
 *
 ********************************************************************/
void close_weatherstation(WEATHERSTATION ws)
{
	close(ws);
	return;
}

/********************************************************************
 * reset_06 WS2300 by sending command 06 (Linux version)
 * 
 * Input:   device number of the already open serial port
 *           
 * Returns: nothing, exits progrsm if failing to reset
 *
 ********************************************************************/
void reset_06(WEATHERSTATION serdevice)
{
	unsigned char command = 0x06;
	unsigned char answer;
	int i;

	for (i = 0; i < 100; i++)
	{

		// Discard any garbage in the input buffer
		tcflush(serdevice, TCIFLUSH);

		write_device(serdevice, &command, 1);

		// Occasionally 0, then 2 is returned.  If zero comes back, continue
		// reading as this is more efficient than sending an out-of sync
		// reset and letting the data reads restore synchronization.
		// Occasionally, multiple 2's are returned.  Read with a fast timeout
		// until all data is exhausted, if we got a two back at all, we
		// consider it a success
		
		while (1 == read_device(serdevice, &answer, 1))
		{
			if (answer == 2)
			{
				return;
			}
		}

		usleep(50000 * i);   //we sleep longer and longer for each retry
	}
	fprintf(stderr, "\nCould not reset\n");
	exit(0);
}

/********************************************************************
 * read_device in the Linux version is identical
 * to the standard Linux read() 
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to read into (unsigned char)
 *          size - number of bytes to read
 *
 * Output:  *buffer - modified on success (pointer to unsigned char)
 * 
 * Returns: number of bytes read
 *
 ********************************************************************/
int read_device(WEATHERSTATION serdevice, unsigned char *buffer, int size)
{
	int ret;

	for (;;) {
		ret = read(serdevice, buffer, size);
		if (ret == 0 && errno == EINTR)
			continue;
		return ret;
	}
}

/********************************************************************
 * write_device in the Linux version is identical
 * to the standard Linux write()
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to write from
 *          size - number of bytes to write
 *
 * Returns: number of bytes written
 *
 ********************************************************************/
int write_device(WEATHERSTATION serdevice, unsigned char *buffer, int size)
{
	int ret = write(serdevice, buffer, size);
	tcdrain(serdevice);	// wait for all output written
	return ret;
}

/********************************************************************
 * sleep_short - Linux version
 * 
 * Inputs: Time in milliseconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_short(int milliseconds)
{
	usleep(milliseconds/1000);
}

/********************************************************************
 * sleep_long - Linux version
 * 
 * Inputs: Time in seconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_long(int seconds)
{
	sleep(seconds);
}


/********************************************************************
 * http_request_url - Linux version
 * 
 * Inputs: urlline - URL to Weather Underground with path and data
 *                   as a pointer to char array (string)
 *
 * Returns: 0 on success and -1 if fail.
 *
 * Action: Send a http request to Weather Underground
 *
 ********************************************************************/
int http_request_url(char *urlline)
{
	int sockfd;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	int bytes_read;
	
	if ( (hostinfo = gethostbyname(WEATHER_UNDERGROUND_BASEURL)) == NULL )
	{
		perror("Host not known by DNS server or DNS server not working");
		return(-1);
	}
	
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		perror("Cannot open socket");
		return(-1);
	}

	memset(&urladdress, 0, sizeof(urladdress));
	urladdress.sin_family = AF_INET;
	urladdress.sin_port = htons(80); /*default HTTP Server port */

	urladdress.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

	if (connect(sockfd,(struct sockaddr*)&urladdress,sizeof(urladdress)) != 0)
	{
		perror("Cannot connect to host");
		return(-1);
	}
	sprintf(buffer, "GET %s\nHTTP/1.0\n\n", urlline);
	send(sockfd, buffer, strlen(buffer), 0);

	/* While there's data, read and print it */
	do
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if ( bytes_read > 0 )
			if (DEBUG) printf("%s", buffer);
	}
	while ( bytes_read > 0 );

	/* Close socket and clean up winsock */
	close(sockfd);
	
	return(0);
}


/********************************************************************
 * citizen_weather_send - Linux version
 * 
 * Inputs: config structure (pointer to) - containing CW ID
 *         datastring (pointer to) - containing all the data
 *
 * Returns: 0 on success and -1 if fail.
 *
 * Action: Send data to Citizen Weather
 *
 ********************************************************************/
int citizen_weather_send(struct config_type *config, char *aprsline)
{
	int sockfd = -1; // just to eliminate a warning we'll set this
	int bytes_read;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];          //Enough to hold a response
	int hostnum;
	
	// Connect to server and send the record
	// loop trying all of the defined servers
	for (hostnum = 0; hostnum <= config->num_hosts; hostnum++)
	{
		if ( hostnum == config->num_hosts )
			return(-1);          // tried 'em all, fail exit

		if ( (hostinfo = gethostbyname(config->aprs_host[hostnum].name) ) == NULL )
		{
			sprintf(buffer,"Host, %s, not known ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
			
		if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		{
			sprintf(buffer,"Cannot open socket on %s ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
	
		memset(&urladdress, 0, sizeof(urladdress)); // clear the structure
		urladdress.sin_family = AF_INET;
		urladdress.sin_port = htons(config->aprs_host[hostnum].port);
		urladdress.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

		if ( connect(sockfd, (struct sockaddr*)&urladdress, sizeof(urladdress)) != 0 )
		{
			sprintf(buffer,"Cannot connect to host: %s ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
		else
		{
			break;   // success
		}
	}

	if (DEBUG) printf("%d: %s: ",hostnum, config->aprs_host[hostnum].name);

	memset(buffer, 0, sizeof(buffer));
	
	if ( (recv(sockfd, buffer, sizeof(buffer), 0) > 0) && (DEBUG != 0) )                 // read login prompt
	{
		printf("%s", buffer);	// display prompt - if debug
	}

	// The login/header line
	sprintf(buffer,"user %s pass -1 vers open2300 %s\n",
	        config->citizen_weather_id, VERSION);
	send(sockfd, buffer, strlen(buffer), 0);
	if (DEBUG)
		printf("%s\n", buffer);

	// now the data
	sprintf(buffer,"%s\n", aprsline);
	send(sockfd, buffer, strlen(buffer), 0);
	if (DEBUG)
		printf("%s\n", buffer);

	/* While there's data, read and print it - Not sure it is needed */
	do
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if ( bytes_read > 0 )
		{
			if (DEBUG)
				printf("Data returned from server\n%s\n", buffer);
			break;
		}
	}
	while ( bytes_read > 0 );

	/* Close socket*/
	close(sockfd);

	return(0);
}

#endif
