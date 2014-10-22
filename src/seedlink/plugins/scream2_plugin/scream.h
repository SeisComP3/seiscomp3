/*
 * scream.h:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 *
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
 * $Id: scream.h 1364 2008-10-24 18:42:33Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.8  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __SCREAM_H__
#define __SCREAM_H__

/* Protocol Definitions for scream */

/* commands to send to scream */

/* Open interactive connection to port n */
#define SCREAM_CMD_OPEN_PORT(n)		n
/* No args */

/* Start transmision of blocks as they are received*/
#define SCREAM_CMD_START_XMIT		0xf9
/* No args */

/* Ask scream to send an identification*/
#define SCREAM_CMD_IDENTIFY		0xfc
/* No args */
/* Returns: 
 *  BYTE length of string
 *  BYTE string[]
 *  BYTE 0
 */

/* Ask scream to send index of oldest block */
#define SCREAM_CMD_OLDEST		0xfe
/* No args */
/* Returns:
 *  WORD (network order) oldest block
 */

/* Ask scream to send block number n */
#define SCREAM_CMD_RESEND		0xff
/* Args:
 *  WORD (network order) block number to send
 */

/* Format of messages on wire (either UDP or TCP) 
 *
 * Version 31 format
 * BYTE[1024] GCFBLOCK 
 * BYTE version of protocol == 31 
 * BYTE Length of string
 * BYTE[32] String
 * WORD Block seq
 * BYTE byte order 	2=intel (should never see this) 1=Moro/network.
 *
 * Version 40 format
 * BYTE[1024] GCFBLOCK
 * BYTE version of protocol == 40 
 * BYTE byte order 	2=intel (should never see this) 1=Moro/network.
 * WORD Block seq
 * BYTE Length of string 
 * BYTE[48] String
 *
 */

/* The following are from the scream source code*/

#define SCREAM_V31_LENGTH	1063
#define SCREAM_V40_LENGTH	1077
#define SCREAM_V45_LENGTH	1089
#define SCREAM_MAX_LENGTH	1316

#define SCREAM_INITIAL_LEN	(GCF_BLOCK_LEN+1)
#define SCREAM_V31_SUBSEQUENT	(SCREAM_V31_LENGTH-SCREAM_INITIAL_LEN)
#define SCREAM_V40_SUBSEQUENT	(SCREAM_V40_LENGTH-SCREAM_INITIAL_LEN)
#define SCREAM_V45_SUBSEQUENT	(SCREAM_V45_LENGTH-SCREAM_INITIAL_LEN)

#define SCM_PROTO_UDP		0
#define SCM_PROTO_TCP		1

extern void scream_init_socket(int _protocol, char *server, int port);
extern void scream_receive (int *thisblocknr, uint8_t *buf, int buflen);

#endif /* __SCREAM_H__ */
