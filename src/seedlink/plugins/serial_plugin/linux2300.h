/* Include file for the open2300 Linux specific functions
 */
 
#ifndef _INCLUDE_LINUX2300_H_
#define _INCLUDE_LINUX2300_H_ 

#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>

#define BAUDRATE B2400
#define DEFAULT_SERIAL_DEVICE "/dev/ttyS0"
typedef int WEATHERSTATION;

#endif /* _INCLUDE_LINUX2300_H_ */

