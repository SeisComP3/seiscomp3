/*
 * gserialp.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gserialp.h,v 1.6 2005/07/13 09:19:16 root Exp $
 */

/*
 * $Log: gserialp.h,v $
 * Revision 1.6  2005/07/13 09:19:16  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/06/06 09:41:41  root
 * *** empty log message ***
 *
 * Revision 1.3  2003/05/29 16:40:03  root
 * *** empty log message ***
 *
 * Revision 1.2  2003/05/28 22:03:42  root
 * *** empty log message ***
 *
 * Revision 1.1  2003/05/16 10:40:26  root
 * *** empty log message ***
 *
 * Revision 1.4  2003/05/14 16:31:48  root
 * #
 *
 * Revision 1.3  2003/05/13 15:10:05  root
 * #
 *
 * Revision 1.2  2003/05/13 09:56:31  root
 * #
 *
 * Revision 1.1  2003/05/13 09:16:24  root
 * #
 *
 */

#ifndef __GSERIALP_H__
#define __GSERIALP_H__


typedef enum
{
  G2SS_LOST,
  G2SS_SEQ,
  G2SS_SZ1,
  G2SS_SZ2,
  G2SS_DATA,
  G2SS_CK1,
  G2SS_CK2,
  G2SS_1B
}
G2SerState;

typedef struct
{
  uint8_t data[1024];
  int size;
  int seq;
  int ck;
}
G2SerBlock;

typedef struct
{
  G2SerState state;
#ifdef ENABLE_MONITOR
  int monitor_state;
  struct timeval  monitor_time;
#endif

  G2Serial *s;

  G2SerBlock b;
  int cksum, dred, dtoread;
  int inoob;

  void (*block) (G2Serial *, G2SerBlock *);
  void (*oob_start) (G2Serial *);
  void (*oob_data) (G2Serial *, uint8_t *, int);
  void (*oob_end) (G2Serial *);
  void (*all_data) (G2Serial *, uint8_t *, int);

}
G2SerialP;

extern void G2SerCheckSum (G2SerBlock *);
extern void G2SerDispatch (G2SerialP * p);
extern G2SerialP *G2CreateSerParser (G2Serial * s,
                                     void (*block) (G2Serial *, G2SerBlock *),
                                     void (*oob_start) (G2Serial *),
                                     void (*oob_data) (G2Serial *, uint8_t *,
                                                       int),
                                     void (*oob_end) (G2Serial *),
                                     void (*all_data) (G2Serial *, uint8_t *,
                                                       int));


extern int G2DestroySerParser (G2SerialP *);

void G2SerAckIntr(G2Serial *s,G2SerBlock *b);
void G2SerNack(G2Serial *s,G2SerBlock *b, int seq);
void G2SerAck(G2Serial *s,G2SerBlock *b);
void G2SerIntr(G2Serial *s);

#endif /* __GSERIALP_H__ */
