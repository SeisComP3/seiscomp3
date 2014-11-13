
/*
 * dispatch.c:
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

/*static char rcsid[] = "$Id: dispatch.c 1904 2009-09-30 14:08:31Z andres $";*/

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:53  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.7  2003/03/17 11:21:20  root
 * #
 *
 * Revision 1.6  2003/02/28 17:08:27  root
 * #
 *
 * Revision 1.5  2003/02/28 17:05:37  root
 * #
 *
 */

/* Mar 2004  - Modified by Reinoud Sleeman (ORFEUS/KNMI)  */
/*             for the SCREAM plugin in SeedLink          */


#include "project.h"
#include "plugin.h"
#include "map.h"
#include <math.h>

extern Map *rootmap;
extern struct config_struct config;

#define MAXCHAN 5000

void
dispatch (gcf_block b, int recno)
{
  int chid;
  static double prevend[MAXCHAN];
  Map *mp;

  /*printf("Dispatch record %d\n", recno);*/
/* We can't handle status information*/

  if (!b->sample_rate) {
    return;
  }

  chid = -1;
  for ( mp = rootmap; mp != NULL; mp=mp->next ) {
        if (strncmp (b->strid, mp->stream, strlen(mp->stream)) == 0 ) {
    	     if (mp->sysid != NULL)
	     {
	          if (strncmp(b->sysid, mp->sysid, strlen(mp->sysid)) == 0)
		  {
		       printf("Sending raw epoch: %s.%s @ %d Hz\n", mp->station, mp->channel, b->sample_rate);
                       send_raw_depoch ( mp->station, mp->channel, b->estart, 0, -1, b->data, b->samples);
                       chid=mp->id;
		  }
	     }
	     else
	     {
	         printf("Sending raw epoch: %s.%s @ %d Hz\n", mp->station, mp->channel, b->sample_rate);
                 send_raw_depoch ( mp->station, mp->channel, b->estart, 0, -1, b->data, b->samples);
                 chid=mp->id;
	     }
        }
  }

   if ( chid < 0 || chid >= MAXCHAN )
   {
        /*printf("Unable to handle stream %s\n", b->strid);*/
	return;
   }
   
   /*printf("%d %.6s %d samples at %d Hz\n", b->estart, b->strid, b->samples, b->sample_rate);*/
   /*fflush(stdout);*/
   
   if ( prevend[chid] > 0 && (b->estart - prevend[chid]) > (1.0/b->sample_rate) ) {
        /*printf("GAP detected of %lf seconds for %.6s\n", b->estart - prevend[chid], b->strid);*/
        fflush(stdout);
   }

   prevend[chid] = b->estart + ((double) b->samples) / ((double) b->sample_rate);
}

