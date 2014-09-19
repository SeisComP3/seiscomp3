/*
 * ./src/version.c:
 *
 * Copyright (c) 2003 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

static char rcsid[] = "$Id: version.c,v 1.2 2003/05/28 22:03:43 root Exp $";

/*
 * $Log: version.c,v $
 * Revision 1.2  2003/05/28 22:03:43  root
 * *** empty log message ***
 *
 */

#include "version.h"

char *
G2GetVersion (void)
{
  return VERSION;
}
