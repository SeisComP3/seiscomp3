/*
 * gdeclut.c:
 *
 * Copyright (c) 2004 Guralp Systems Limited
 * Author James McKenzie, contact <software@guralp.com>
 * All rights reserved.
 *
 */

/*
 * $Id: gdeclut.c,v 1.2 2006/09/14 14:50:09 lwithers Exp $
 */

/*
 * $Log: gdeclut.c,v $
 * Revision 1.2  2006/09/14 14:50:09  lwithers
 * Fix up mk3 declut stuff
 *
 * Revision 1.1  2006/09/14 14:28:11  lwithers
 * Add G2GetDecLut() and the machinery for the new G2PBlock.dig_type field.
 *
 *
 */

#include "gcf2.h"

/* G2GetDecLut()
 *
 *  Gets the decimation lookup table associated with `pbh'. Returns 0 if it doesn't know about that
 *  particular lut.
 */
G2DecLut* G2GetDecLut(const G2PBlockH* pbh)
{
  switch(pbh->dig_type) {
  case G2DIGTYPEMK2:
    return &G2DM24Mk2DecLut;
  case G2DIGTYPEMK3:
    return &G2DM24Mk3DecLut;
  }

  return 0;
}
