#pragma ident "$Id: auth.c,v 1.1 2003/06/09 23:50:26 dechavez Exp $"
/*======================================================================
 * 
 * Sign and verify stubs.
 *
 *====================================================================*/
#include "iacp.h"

BOOL iacpSignFrame(IACP *iacp, IACP_FRAME *frame)
{
    frame->auth.id  = 0;
    frame->auth.len = 0;

    return TRUE;
}

BOOL iacpVerifyFrame(IACP_FRAME *frame)
{
    frame->auth.verified = TRUE;
    return frame->auth.verified;
}

BOOL iacpSignatureOK(IACP_FRAME *frame)
{
    if (frame->auth.len == 0) return TRUE;
    return frame->auth.verified;
}

BOOL iacpAuthInit(IACP *acp)
{
    return TRUE;
}

/* Revision History
 *
 * $Log: auth.c,v $
 * Revision 1.1  2003/06/09 23:50:26  dechavez
 * initial release
 *
 */
