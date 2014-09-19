/*
 * project.h:
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
 * $Id: project.h 2 2005-07-26 19:28:46Z andres $
 */

/*
 * $Log$
 * Revision 1.1  2005/07/26 19:28:54  andres
 * Initial revision
 *
 * Revision 1.1  2003/03/27 18:07:18  alex
 * Initial revision
 *
 * Revision 1.7  2003/02/19 16:00:18  root
 * #
 *
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <errno.h>

#include "oscompat.h"
#include "util.h"
#include "gcf.h"
#include "config.h"
#include "gcf.h"
#include "scream.h"
#include "gputil.h"
#include "dispatch.h"

#define  ERR_MISSMSG       0    /* message missed in transport ring       */
#define  ERR_TOOBIG        1    /* retreived msg too large for buffer     */
#define  ERR_NOTRACK       2    /* msg retreived; tracking limit exceeded */

#endif /* __PROJECT_H__ */
