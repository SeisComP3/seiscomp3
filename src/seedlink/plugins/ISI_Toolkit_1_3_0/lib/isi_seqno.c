#pragma ident "$Id: seqno.c,v 1.13 2007/10/31 23:10:27 dechavez Exp $"
/*======================================================================
 *
 *  ISI sequence number convenience functions
 *
 *====================================================================*/
#define INCLUDE_ISI_STATIC_SEQNOS
#include "isi.h"
#include "util.h"

/* Convert strings to sequence numbers */

BOOL isiStringToSeqno(char *string, ISI_SEQNO *seqno)
{
int ntoken;
#define NUMTOKEN 2
char *token[NUMTOKEN];
char teename[ISI_SEQNO_STRING_LEN+1] = "SSSSSSSSCCCCCCCCCCCCCCCC"; /* 32-bit sig, 64-bit counter */
                                     /* 012345678 offsets to each part */

    if (seqno == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (string == NULL) {
        *seqno = ISI_UNDEFINED_SEQNO;
        return TRUE;
    }

    if (strlen(string) == strlen(teename)) {
        strlcpy(teename, string, ISI_SEQNO_STRING_LEN+1);
        teename[8] = 0;
        seqno->signature = (UINT32) strtol(teename, NULL, 16);
        seqno->counter = (UINT64) strtoll(&string[8], NULL, 16);
        return TRUE;
    }

    if (strcasecmp(string, ISI_NEWEST_SEQNO_STRING) == 0) {
        *seqno = ISI_NEWEST_SEQNO;
        return TRUE;
    }

    if (strcasecmp(string, ISI_OLDEST_SEQNO_STRING) == 0) {
        *seqno = ISI_OLDEST_SEQNO;
        return TRUE;
    }

    if (strcasecmp(string, ISI_KEEPUP_SEQNO_STRING) == 0) {
        *seqno = ISI_KEEPUP_SEQNO;
        return TRUE;
    }

    if (strcasecmp(string, ISI_NEVER_SEQNO_STRING) == 0) {
        *seqno = ISI_KEEPUP_SEQNO;  /* same as keepup */
        return TRUE;
    }

    ntoken = utilParse(string, token, ":;.,/+-\\", NUMTOKEN, 0);

    if (ntoken == 1) {
        seqno->signature = ISI_CURRENT_SEQNO_SIG;
        seqno->counter = (UINT64) atol(token[0]);
        return TRUE;
    }

    if (ntoken != 2) {
        *seqno = ISI_UNDEFINED_SEQNO;
        return FALSE;
    }

    if (strcasecmp(token[0], "beg") == 0) {
        seqno->signature = ISI_BEG_RELATIVE_SEQNO_SIG;
        seqno->counter = (UINT64) atol(token[1]);
        return TRUE;
    }

    if (strcasecmp(token[0], "end") == 0) {
        seqno->signature = ISI_END_RELATIVE_SEQNO_SIG;
        seqno->counter = (UINT64) atol(token[1]);
        return TRUE;
    }

    seqno->signature = (UINT32) atol(token[0]);
    seqno->counter = (UINT64) atol(token[1]);

    return TRUE;
}

VOID isiPrintSeqno(FILE *fp, ISI_SEQNO *seqno)
{
char buf[256];

    fprintf(fp, "%s", isiSeqnoString(seqno, buf));
}

BOOL isiSeqnoEQ(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (a->signature == b->signature && a->counter == b->counter) return TRUE;
    return FALSE;
}

BOOL isiSeqnoLT(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (a->signature < b->signature) return TRUE;
    if (a->signature > b->signature) return FALSE;
    return (a->counter < b->counter) ? TRUE : FALSE;
}

BOOL isiSeqnoLE(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (isiSeqnoEQ(a, b) || isiSeqnoLT(a, b)) return TRUE;
    return FALSE;
}

BOOL isiSeqnoGT(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (a->signature > b->signature) return TRUE;
    if (a->signature < b->signature) return FALSE;
    return (a->counter > b->counter) ? TRUE : FALSE;
}

BOOL isiSeqnoGE(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (isiSeqnoEQ(a, b) || isiSeqnoGT(a, b)) return TRUE;
    return FALSE;
}

int isiCompareSeqno(ISI_SEQNO *a, ISI_SEQNO *b)
{
    if (isiSeqnoLT(a, b)) return -1;
    if (isiSeqnoGT(a, b)) return  1;
    return 0;
}

ISI_SEQNO isiIncrSeqno(ISI_SEQNO *seqno)
{
    ++seqno->counter;
    return *seqno;
}

/* FILE I/O */

BOOL isiWriteSeqno(FILE *fp, ISI_SEQNO *seqno)
{
    if (fp == NULL || seqno == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (fwrite(&seqno->signature, sizeof(UINT32), 1, fp) != 1) return FALSE;
    if (fwrite(&seqno->counter, sizeof(UINT64), 1, fp) != 1) return FALSE;

    return TRUE;
}

BOOL isiReadSeqno(FILE *fp, ISI_SEQNO *seqno)
{
    if (fp == NULL || seqno == NULL) {
        errno = EINVAL;
        return FALSE;
    }

    if (fread(&seqno->signature, sizeof(UINT32), 1, fp) != 1) return FALSE;
    if (fread(&seqno->counter, sizeof(UINT64), 1, fp) != 1) return FALSE;

    return TRUE;
}

/* Revision History
 *
 * $Log: seqno.c,v $
 * Revision 1.13  2007/10/31 23:10:27  dechavez
 * fixed bug in testing fwrite return values
 *
 * Revision 1.12  2007/10/31 16:45:09  dechavez
 * replaced tabs with spaces
 *
 * Revision 1.11  2007/04/18 22:57:50  dechavez
 * added isiWriteSeqno() and isiReadSeqno()
 *
 * Revision 1.10  2007/01/08 15:58:18  dechavez
 * switch to size-bounded string operations
 *
 * Revision 1.9  2006/12/12 22:43:51  dechavez
 * use new ISI_x_SEQNO_STRING macros for abstract sequence numbers
 *
 * Revision 1.8  2006/06/26 22:38:01  dechavez
 * include util.h for prototypes
 *
 * Revision 1.7  2006/03/13 22:22:02  dechavez
 * added support for "tee name" splitting, fixed some counter casts
 *
 * Revision 1.6  2005/07/26 00:16:11  dechavez
 * fixed errors in isiSeqnoLT() and isiSeqnoGT()
 *
 * Revision 1.5  2005/06/30 01:28:20  dechavez
 * added ISI_CURRENT_SEQNO_SIG support to isiStringToSeqno(), moved isiSeqnoString() to string.c
 *
 * Revision 1.4  2005/06/24 21:34:28  dechavez
 * mods, additions, subtractions as part of getting isidl to work
 *
 * Revision 1.3  2005/06/10 15:48:02  dechavez
 * added isiSeqnoString(), isiStringToSeqno(), isiUpdateSeqno()
 *
 * Revision 1.2  2005/05/25 00:28:23  dechavez
 * fixed bug printing seqno counters
 *
 * Revision 1.1  2005/05/06 00:48:54  dechavez
 * initial release
 *
 */
