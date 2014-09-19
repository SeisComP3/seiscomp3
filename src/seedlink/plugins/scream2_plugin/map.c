/*
 * map.c:
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

static char rcsid[] = "$Id: map.c 2 2005-07-26 19:28:46Z andres $";

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.5  2003/02/28 17:05:37  root
 * #
 *
 */

#include "project.h"

#define TRACE_STA_LEN   7
#define TRACE_CHAN_LEN  9
#define TRACE_NET_LEN   9
#define TRACE_LOC_LEN   3

typedef struct {
        int     pinno;          /* Pin number */
        int     nsamp;          /* Number of samples in packet */
        double  starttime;      /* time of first sample in epoch seconds
                                   (seconds since midnight 1/1/1970) */
        double  endtime;        /* Time of last sample in epoch seconds */
        double  samprate;       /* Sample rate; nominal */
        char    sta[TRACE_STA_LEN];         /* Site name */
        char    net[TRACE_NET_LEN];         /* Network name */
        char    chan[TRACE_CHAN_LEN];        /* Component/channel code */
        char    datatype[3];    /* Data format code */
        char    quality[2];     /* Data-quality field */
        char    pad[2];         /* padding */
} TRACE_HEADER;



typedef struct map_struct
{
  struct map_struct *next;
  char *system;
  char *stream;
  char *network;
  char *station;
  char *channel;
  int pinno;
  int ignore;                   /*Use to generate warnings */
}
 *Map;

static Map map = NULL;

void
map_add (char *mapping)
{
/* FIXME: don't use static strings here*/
  Map new;
  char system[128];
  char stream[128];
  char network[128];
  char station[128];
  char channel[128];
  char pinno[128];
  int j;



  if ((j = sscanf (mapping, "%[^ \t]" "%*[ \t]" "%[^ \t]" "%*[ \t]" "%[^ \t]"
                   "%*[ \t]" "%[^ \t]" "%*[ \t]" "%[^ \t]" "%*[ \t]"
                   "%[^ \t]", &system, &stream, &network, &station, &channel,
                   &pinno)) != 6)
    fatal (("Failed to parse ChanInfo %s (got %d fields expected %d)\n",
            mapping, j, 6));


  new = malloc (sizeof (struct map_struct));

  new->system = strdup(gp_base36_to_a(gp_a_to_base36(system)));
  new->stream = strdup(gp_base36_to_a(gp_a_to_base36(stream)));
  new->network = strdup (network);
  new->station = strdup (station);
  new->channel = strdup (channel);
  new->pinno = atoi (pinno);
  new->ignore = 0;
  new->next = map;
  map = new;

}

int
map_lookup (gcf_block b, TRACE_HEADER * h)
{
  Map m = map;

  while (m)
    {
      if ((!strcmp (b->sysid, m->system)) && (!strcmp (b->strid, m->stream)))
        {

          if (m->ignore)
            return 1;

          strncpy (h->net, m->network, sizeof (h->net));
          strncpy (h->sta, m->station, sizeof (h->sta));
          strncpy (h->chan, m->channel, sizeof (h->chan));
          h->pinno = m->pinno;

          return 0;

        }
      m = m->next;
    }

  m = malloc (sizeof (struct map_struct));

  m->system = strdup (b->sysid);
  m->stream = strdup (b->strid);
  m->ignore = 1;

  warning (("block from unmapped channel %s %s", b->sysid, b->strid));

  m->next = map;

  map = m;

  return 1;
}


