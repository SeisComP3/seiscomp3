/*
 * gnet.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gnet.h,v 1.4 2004/04/15 22:35:45 root Exp $
 */

/*
 * $Log: gnet.h,v $
 * Revision 1.4  2004/04/15 22:35:45  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/06/06 16:10:53  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/06/06 16:03:37  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/06/06 09:38:10  root
 * *** empty log message ***
 *
 */

#ifndef __GNET_H__
#define __GNET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
	struct sockaddr_in sin;				
} G2NetAddr;

#define G2NetAddrEq(a,b) (((a)->sin.sin_port==(b)->sin.sin_port) && ((a)->sin.sin_addr.s_addr=(b)->sin.sin_addr.s_addr))
			

typedef struct {
	int fd;
	int eof;
	void *client_data;
	/*Private below here*/

	struct sockaddr_in sin;
} G2Net;

extern int        G2NetLookup(char *namecolonport,G2NetAddr *a);
extern int	  G2NetPort(G2Net *);
extern G2Net     *G2NetOpenUDP(int port);	
extern G2Net     *G2NetOpenUDPB(int port);	
extern G2Net     *G2NetOpenTCPC(G2NetAddr *remote);
extern G2Net     *G2NetOpenTCPCNB(G2NetAddr *remote);
extern int        G2NetOpenTCPCNBE(G2Net *);
extern G2Net     *G2NetOpenTCPS(int port);
extern int        G2NetOpenUDPTCP(int port,G2Net **tcp,G2Net **udp);
extern void       G2NetClose(G2Net *);
extern int G2NetRecv(G2Net *net, G2NetAddr *from, void *buf, int len);
extern int G2NetSend(G2Net *net, G2NetAddr *to, void *buf, int lne);
extern char *G2NetAddrtoa(G2NetAddr *a);
extern G2Net *G2NetAccept(G2Net *tcps, G2NetAddr *from);




#endif /* __GNET_H__ */
