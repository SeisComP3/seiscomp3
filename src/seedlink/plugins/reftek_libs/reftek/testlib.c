#pragma ident "$Id: testlib.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * Simple program to test reftek library.  Send it packets from stdin
 * and it will decode them and print header summaries.
 *
 *====================================================================*/
#include <math.h>
#include "reftek.h"

#define INCREMENT_OK(p, c) (((c) == (p) + 1) || ((p) == 9999 && (c) == 0))
#define THRESHOLD 1.0

typedef struct {
    REAL64 tstamp;
    UINT16 nsamp;
    BOOL   set;
} DTCHAN;

typedef struct {
    UINT16 unit;
    UINT16 seqno;
    UINT16 stream;
    REAL64 samprate;
    DTCHAN chan[RTP_MAXCHAN];
} DTPARM;

struct dt_list {
    DTPARM dt;
    struct dt_list *next;
};
    
static struct dt_list head = {
    {0, 0, 0, 0.0,
        {
            {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE},
            {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE},
            {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE},
            {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE}, {0.0, 0, FALSE}
        },
    },
    (struct dt_list *) NULL
};

static CHAR *format_string(UINT16 format)
{
static CHAR *F16 = "16";
static CHAR *F32 = "32";
static CHAR *FC0 = "C0";
static CHAR *BAD = "??";

    switch (format) {
      case REFTEK_FC0: return FC0;
      case REFTEK_F32: return F32;
      case REFTEK_F16: return F16;
      default: return BAD;
    }
}

/* Search the list for a previous entry for this stream */

static DTPARM *PrevParm(struct reftek_dt *dt)
{
struct dt_list *crnt;

    crnt = head.next;
    while (crnt != (struct dt_list *) NULL) {
        if (
            crnt->dt.unit   == dt->unit   &&
            crnt->dt.stream == dt->stream
        ) return &crnt->dt;
        crnt = crnt->next;
    }

    return (DTPARM *) NULL;
}

/* Add a new entry to the list */

static VOID AddStream(struct reftek_dt *dt)
{
UINT16 i;
struct dt_list *new;

    new = (struct dt_list *) malloc(sizeof(struct dt_list));
    if (new == (struct dt_list *) NULL) {
        fprintf(stderr, "FATAL ERROR: malloc: %s\n", strerror(errno));
        exit(1);
    }

    new->dt.unit   = dt->unit;
    new->dt.seqno  = dt->seqno;
    new->dt.stream = dt->stream;
    new->dt.samprate = -1.0;
    for (i = 0; i < RTP_MAXCHAN; i++) new->dt.chan[i].set = FALSE;
    new->dt.chan[dt->chan].tstamp = dt->tstamp;
    new->dt.chan[dt->chan].nsamp  = dt->nsamp;
    new->dt.chan[dt->chan].set    = TRUE;

    new->next = head.next;
    head.next = new;
}

/* Update entry with new values */

static VOID SaveParm(DTPARM *prev, struct reftek_dt *crnt)
{
    prev->seqno  = crnt->seqno;
    prev->chan[crnt->chan].tstamp = crnt->tstamp;
    prev->chan[crnt->chan].nsamp  = crnt->nsamp;
    prev->chan[crnt->chan].set    = TRUE;
}

/* Compare this packet with the previous one and get sample rate */

BOOL GetSamprate(struct reftek_dt *dt, double *output)
{
UINT16 i;
REAL64 newrate, percent_change;
DTPARM *prev;

    *output = -1.0;

/* Make sure we can index off the channel number */

    if (dt->chan >= RTP_MAXCHAN) {
        fprintf(stderr, "FATAL ERROR: illegal chan id: %d\n", dt->chan);
        exit(2);
    }

/* Get the previous parameters for this stream */

    if ((prev = PrevParm(dt)) == (DTPARM *) NULL) {
        AddStream(dt);
        return TRUE;
    }

/* Wipe out prior data if sample numbers fail to increment OK */

    if (!INCREMENT_OK(prev->seqno,dt->seqno)) {
        for (i = 0; i < RTP_MAXCHAN; i++) prev->chan[i].set = FALSE;
    }

/* If we don't have prior data for this channel then use the previous
 * sample rate, if we've got one */

    if (!prev->chan[dt->chan].set) {
        SaveParm(prev, dt);
        if (prev->samprate > 0.0) {
            *output = prev->samprate;
        } else {
            *output = -1.0;
        }
        return TRUE;
    }

/* Should be able to determine a sample rate at this point */

    newrate = (double) prev->chan[dt->chan].nsamp /
              (dt->tstamp - prev->chan[dt->chan].tstamp);
    SaveParm(prev, dt);
    if (prev->samprate < 0.0) prev->samprate = newrate;
    *output = prev->samprate;

/* Note if sample rate changes significantly */

    percent_change = (fabs(newrate - prev->samprate)/newrate) * 100.0;
    if (percent_change > THRESHOLD) {
        fprintf(stderr, "WARNING - %hu:%hu srate change from %.3lf to %.3lf\n",
            prev->unit, prev->stream, prev->samprate, newrate
        );
        prev->samprate = newrate;
        return FALSE;
    } else {
        return TRUE;
    }
}

static VOID PrintSummary(struct reftek_dt *dt, REAL64 samprate, BOOL status)
{
int i;
static int nerr = 0;
CHAR sbuf[UTIL_MAXTIMESTRLEN];

    printf("DT ");
    printf("%03d ", dt->exp);
    printf("%04X ", dt->unit);
    printf("%05d ", dt->seqno);
    printf("%s ",   util_dttostr(dt->tstamp, 0, sbuf));

    printf("%05d ", dt->evtno);
    printf("%02d ", dt->stream);
    printf("%02d ", dt->chan);
    printf("%05d ", dt->nsamp);
    printf("%s ",   format_string(dt->format));

    if (samprate > 0.0) {
        printf("%6.2lf ", samprate);
    } else {
        printf("?????? ");
    }

    if (!status) ++nerr;
    printf("(%d) ", nerr);
    printf("\n");

    if (dt->data != (INT32 *) NULL) {
        for (i = 0; i < dt->nsamp; i++) {
            if (i != 0 && i % 10 == 0) printf("\n");
            printf("%08x ", dt->data[i]);
        }
        printf("\n");
    }
}

main(int argc, char **argv)
{
int i;
BOOL status;
UINT8 raw[1024];
CHAR  buf[1024];
struct reftek_dt dt;
REAL64 samprate;

    i = 0;
    while (fread(raw, 1, 1024, stdin) == 1024) {
        if (reftek_type(raw) == REFTEK_DT) {
            if (!reftek_dt(&dt, raw, TRUE)) {
                fprintf(stderr, "reftek_dt failed\n");
                exit(1);
            }
            status = GetSamprate(&dt, &samprate);
            PrintSummary(&dt, samprate, status);
        } else {
            printf("%s\n", reftek_str(raw, buf));
        }
    }
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:55:58  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
