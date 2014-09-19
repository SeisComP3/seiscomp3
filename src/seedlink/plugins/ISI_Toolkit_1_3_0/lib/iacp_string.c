#pragma ident "$Id: string.c,v 1.1 2003/10/16 16:33:47 dechavez Exp $"
/*======================================================================
 * 
 * Code to string conversions.
 *
 *====================================================================*/
#include "iacp.h"

typedef struct {
    UINT32 code;
    char *string;
} STRING_MAP;

char *iacpAlertString(UINT32 code)
{
static char UnknownError[] = "alert code 4,294,967,296";
static STRING_MAP map[] = {
    { IACP_ALERT_NONE,             "no error" },
    { IACP_ALERT_DISCONNECT,       "normal disconnect" },
    { IACP_ALERT_REQUEST_COMPLETE, "request complete" },
    { IACP_ALERT_IO_ERROR,         "I/O error" },
    { IACP_ALERT_SERVER_FAULT,     "server fault" },
    { IACP_ALERT_SERVER_BUSY,      "too many active connections" },
    { IACP_ALERT_FAILED_AUTH,      "frame signature failed to verify" },
    { IACP_ALERT_ACCESS_DENIED,    "access to server refused" },
    { IACP_ALERT_REQUEST_DENIED,   "client request refused" },
    { IACP_ALERT_SHUTDOWN,         "shutdown in progress" },
    { IACP_ALERT_PROTOCOL_ERROR,   "illegal frame received" },
    { IACP_ALERT_ILLEGAL_DATA,     "unexpected frame data" },
    { 0, NULL }
};

int i;

    for (i = 0; map[i].string != NULL; i++) if (map[i].code == code) return map[i].string;
    sprintf(UnknownError, "alert code %lu", code); /* not MT safe, but should never occur */
    return UnknownError; 
}

/* Revision History
 *
 * $Log: string.c,v $
 * Revision 1.1  2003/10/16 16:33:47  dechavez
 * initial release
 *
 */
