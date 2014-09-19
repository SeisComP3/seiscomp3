/*
 * config.h:
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
 * $Id: config.h 2 2005-07-26 19:28:46Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:53  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.6  2003/02/26 15:24:56  root
 * #
 *
 * Revision 1.5  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

struct config_struct
{
  char *server;
  char *modulename;
  char *ringname;
  int port;               // must be UDP port
  int protocol;           // must be UDP
  uint16_t reqport;            // is the TCP port used for requesting missing data from dropped UDP 
  int verbose;
  int writelog;
  double heartbeatinterval;
};

//extern struct config_struct config;
extern void parse_config (const char *filename);

#endif /* __CONFIG_H__ */
