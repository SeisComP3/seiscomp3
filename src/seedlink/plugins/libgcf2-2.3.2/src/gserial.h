/*
 * gserial.h:
 *
 * Copyright (c) 2003 James McKenzie <james@fishsoup.dhs.org>,
 * All rights reserved.
 *
 */

/*
 * $Id: gserial.h,v 1.4 2004/10/29 13:54:10 root Exp $
 */

/*
 * $Log: gserial.h,v $
 * Revision 1.4  2004/10/29 13:54:10  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/14 11:38:30  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:25  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/13 15:10:05  root
 * #
 *
 * Revision 1.2  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.1  2003/05/13 09:16:26  root
 * #
 *
 */

#ifndef __GSERIAL_H__
#define __GSERIAL_H__

#include <termios.h>

typedef struct
{
  int fd;
  int eof;

  void *client_data;

/*Private */
  int isfile;
  char *name;
  struct termios orig_termios;

  char *lock;
}
G2Serial;

#define G2SerFd(s) ((s)->fd)
#define G2SerEof(s) ((s)->eof)

extern void G2SerClose (G2Serial * g);
extern G2Serial *G2SerOpen (char *name, int speed, int flags);
extern G2Serial *G2SerOpenLock (char *name, int speed, int flags, int timeout);
extern int G2SerData (G2Serial * s);
extern int G2SerRead (G2Serial * s, void *buf, int buflen);
extern int G2SerWrite (G2Serial * s, void *buf, int buflen);
extern int G2SerWriteStrSlow(G2Serial *s, char *buf);
extern int G2SerDrain(G2Serial *s,int timeout);

#endif /* __GSERIAL_H__ */
