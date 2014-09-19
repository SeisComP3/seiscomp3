/*
 * gserialcli.h:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*
 * $Id: gserialcli.h,v 1.10 2005/03/01 10:41:42 root Exp $
 */

/*
 * $Log: gserialcli.h,v $
 * Revision 1.10  2005/03/01 10:41:42  root
 * *** empty log message ***
 *
 * Revision 1.9  2004/11/09 09:35:36  root
 * *** empty log message ***
 *
 * Revision 1.8  2004/10/29 10:28:51  root
 * *** empty log message ***
 *
 * Revision 1.7  2004/04/23 00:21:24  root
 * *** empty log message ***
 *
 * Revision 1.6  2004/04/21 09:34:18  root
 * *** empty log message ***
 *
 * Revision 1.5  2004/04/20 13:06:21  root
 * *** empty log message ***
 *
 * Revision 1.4  2004/04/20 13:05:41  root
 * *** empty log message ***
 *
 * Revision 1.3  2004/04/15 11:08:18  root
 * *** empty log message ***
 *
 * Revision 1.2  2004/04/14 15:39:52  root
 * *** empty log message ***
 *
 * Revision 1.1  2004/02/13 15:42:03  root
 * *** empty log message ***
 *
 */

#ifndef __GSERIALCLI_H__
#define __GSERIALCLI_H__

/*Used for two things - getting a command mode prompt for the likes of
 *scream, and for executing commands*/


#include "gserial.h"
#include "gserialp.h"

#define G2SERIALCLI_LLL 0x400

typedef struct G2SerialCLI_struct
{
  void (*callback)(struct G2SerialCLI_struct *,uint8_t *buf,int len);
  void *client_data;

  G2Serial *s;
  G2SerialP *p;

  char lastline[G2SERIALCLI_LLL];
  int llrptr;
  int llwptr;

#define G2SERIALCLI_OKSM_LOST	0
#define G2SERIALCLI_OKSM_SP 	1
#define G2SERIALCLI_OKSM_O	2	
#define G2SERIALCLI_OKSM_K	3
#define G2SERIALCLI_OKSM_US	4
#define G2SERIALCLI_OKSM_CR	5

  int oksm;

#define G2SERIALCLI_OBESM_LOST	0
#define G2SERIALCLI_OBESM_OB	1
#define G2SERIALCLI_OBESM_E	2

  int obesm;

#define G2SERIALCLI_BSSM_LOST	0
  int bssm;

#define G2SERIALCLI_BFSM_LOST	0
  int bfsm;

#define G2SERIALCLI_FSSM_LOST	0
  int fssm;

#define G2SERIALCLI_UDSM_LOST	0
  int udsm;

#define G2SERIALCLI_SY_LOST	0
#define G2SERIALCLI_SY_DRAINED	1
#define G2SERIALCLI_SY_SYNCED	2
  int syncsm;

#define G2SERIALCLI_CMD_NONE	0
#define G2SERIALCLI_CMD_PENDING	1
#define G2SERIALCLI_CMD_SENT	2
#define G2SERIALCLI_CMD_SYNC	3	
#define G2SERIALCLI_CMD_DONE	4
  int cmdsm;

  int oks;
  int undefs;

  int incmdmode;

  struct timeval last_intr;


  char *banner;
  char *command;
  char *response;

#define G2SERIALCLI_CRLF_NONE	0
#define G2SERIALCLI_CRLF_LF	1
#define G2SERIALCLI_CRLF_CR	2

  int crlfsm;

} G2SerialCLI;




void G2SerialCLIParse(G2SerialCLI *c, uint8_t *buf, int len);
G2SerialCLI *G2CreateSerCLI(G2Serial *s);
void G2HushDestroySerCLI(G2SerialCLI *c);
void G2DestroySerCLI(G2SerialCLI *c);
void G2SerCLIDispatch(G2SerialCLI *c);
int G2SerCLIBlock(G2SerialCLI *c, int timeout);
int G2SerCLIInCmdMode(G2SerialCLI *c);
char *G2SerCLIBanner(G2SerialCLI *c);
char *G2SerCLIResponse(G2SerialCLI *c);
void G2SerCLICommand(G2SerialCLI *c, char *cmd);



#endif

