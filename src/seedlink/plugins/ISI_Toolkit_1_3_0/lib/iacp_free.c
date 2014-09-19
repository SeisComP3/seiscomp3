#pragma ident "$Id: free.c,v 1.4 2005/09/30 22:16:00 dechavez Exp $"
/*======================================================================
 * 
 * Free IACP resources
 *
 *====================================================================*/
#include "iacp.h"

IACP *iacpFree(IACP *iacp)
{
    if (iacp == (IACP *) NULL) return (IACP *) NULL;
    if (iacp->attr.at_dbgpath != NULL) free(iacp->attr.at_dbgpath);
    free(iacp);
    return (IACP *) NULL;
}

/* Revision History
 *
 * $Log: free.c,v $
 * Revision 1.4  2005/09/30 22:16:00  dechavez
 * fixed memory leak (free attr.at_dbgpath)
 *
 * Revision 1.3  2005/09/14 23:30:24  dechavez
 * removed dbgpath free, should never have been added in the first place as
 * it is not dynamically allocated by iacp
 *
 * Revision 1.2  2005/01/28 01:58:24  dechavez
 * at_dbgpath support
 *
 * Revision 1.1  2003/06/09 23:50:26  dechavez
 * initial release
 *
 */
