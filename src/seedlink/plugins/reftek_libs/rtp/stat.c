#pragma ident "$Id: stat.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 * 
 * rtp_setstat():
 * Reset the state of health enumeration to the begining of the set
 * of DAS entries.
 *
 * rtp_getstat():
 * Get the next DAS entry from the list.
 *
 *====================================================================*/
#include "rtp.h"

UINT16 rtp_setstat(RTP *rtp)
{
UINT16 ndas;

    if (rtp == (RTP *) NULL) return 0;

    MUTEX_LOCK(&rtp->soh.sh_mutex);
        rtp->soh.sh_index = 0;
        ndas = (UINT16) rtp->soh.sh_ndas;
    MUTEX_UNLOCK(&rtp->soh.sh_mutex);

    return ndas;
}

struct rtp_stat *rtp_getstat(RTP *rtp)
{
struct rtp_stat *entry, *null = (struct rtp_stat *) NULL;

    if (rtp == (RTP *) NULL) return null;

    MUTEX_LOCK(&rtp->soh.sh_mutex);
        if (rtp->soh.sh_index >= rtp->soh.sh_ndas) {
            entry = null;
        } else {
            entry = rtp->soh.sh_stat + rtp->soh.sh_index;
            ++rtp->soh.sh_index;
        }
    MUTEX_UNLOCK(&rtp->soh.sh_mutex);

    return entry;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:57:50  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
