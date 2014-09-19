
/*
 * util.c:
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


//static char rcsid[] = "$Id: util.c 2 2005-07-26 19:28:46Z andres $";

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.6  2003/02/28 17:05:37  root
 * #
 *
 */

/* Mar 2004  - Modified by Reinoud Sleeman           */
/*             for the SCREAM plugin in SeedLink     */

#include <unistd.h>
#include "project.h"

int
complete_read (int fd, char *buf, int n)
{
  int c = 0;
  int r;

  while (n)
    {
      r = read (fd, buf, n);
      if (r < 0)
        return r;
      if (!r)
        return c;

      n -= r;
      buf += r;
      c += r;
    }

  return c;
}

