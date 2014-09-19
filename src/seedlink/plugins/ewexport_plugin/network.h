
#ifndef NETWORK_H
#define NETWORK_H 1

extern int my_connect (char *host, char *port);
extern int my_recv (int sock, char *buf, int buflen, int flags,
		    int timeout_msecs);
extern int my_send (int sock, char *buf, int buflen, int flags,
		    int timeout_msecs);

#endif 
