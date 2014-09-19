#pragma ident "$Id: streams.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : streams.c
 Purpose  : Stream record management functions.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#define _STREAMS_C
#include "archive.h"

/* Local prototypes ---------------------------------------------------*/

/*---------------------------------------------------------------------*/
BOOL UpdateStreamTimeInfo(H_ARCHIVE harchive, STREAM * stream, TIME_RANGE * range)
{

    ASSERT(stream != NULL);
    ASSERT(range != NULL);

    if (!ValidateHandle(harchive))
        return (FALSE);

    /* Event range */
    UpdateTimeRange(&stream->event.time, range->earliest);
    UpdateTimeRange(&stream->event.time, range->latest);

    /* Stream range */
    UpdateTimeRange(&stream->time, range->earliest);
    UpdateTimeRange(&stream->time, range->latest);

    /* Archive range */
    UpdateTimeRange(&_archive[harchive].state.time, stream->time.earliest);
    UpdateTimeRange(&_archive[harchive].state.time, stream->time.latest);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
STREAM *LookupStream(H_ARCHIVE harchive, UINT16 unit, UINT8 stream)
{
    NODE *node;

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* If the current stream is not undefined (poor man's cache) */
    if (_archive[harchive].stream != NULL) {
        /* Is it what we're looking up */
        if (unit == _archive[harchive].stream->unit &&
            stream == _archive[harchive].stream->stream) {

            /* Yes, return the current stream */
            ArchiveLog(ARC_LOG_MAXIMUM, "Lookup stream %04X:%u - current stream", unit, stream);
            return (_archive[harchive].stream);
        }
    }

    /* Otherwise, walk the stream list from head to tail looking for the
       specified stream */
    if ((node = FirstNode(&_archive[harchive].streams))) {
        do {
            _archive[harchive].stream = (STREAM *) node->data;
            if (unit == _archive[harchive].stream->unit &&
                stream == _archive[harchive].stream->stream) {

                ArchiveLog(ARC_LOG_MAXIMUM, "Lookup stream %04X:%u - found in list", unit, stream);
                return (_archive[harchive].stream);
            }
        } while ((node = NextNode(node)) != NULL);
    }

    ArchiveLog(ARC_LOG_MAXIMUM, "Lookup stream %04X:%u - not found", unit, stream);

    /* Current stream is undefined */
    _archive[harchive].stream = NULL;

    return (_archive[harchive].stream);
}

/*---------------------------------------------------------------------*/
STREAM *CreateStream(H_ARCHIVE harchive, UINT16 unit, UINT8 stream)
{
    STREAM data;
    NODE *node;

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* Create empty stream record */
    InitStream(&data);
    data.unit = unit;
    data.stream = stream;

    /* Current stream is undefined */
    _archive[harchive].stream = NULL;

    /* Insert before tail of stream list */
    node = &_archive[harchive].streams.tail;
    if ((node = InsertNodeBefore(node, &data, sizeof(STREAM))) == NULL) {
        _archive[harchive].last_error = ARC_NO_MEMORY;
        return (NULL);
    }

    /* This is now the current stream */
    _archive[harchive].stream = (STREAM *) node->data;
    _archive[harchive].state.n_streams++;

    ArchiveLog(ARC_LOG_MAXIMUM, "Create stream record %04X:%u", unit, stream);

    return (_archive[harchive].stream);
}

/*---------------------------------------------------------------------*/
VOID InitStream(STREAM * stream)
{

    /* Initialize stream elements */
    stream->unit = ALL_UNITS;
    stream->stream = ALL_STREAMS;
    stream->chn_mask = 0;
    stream->time.earliest = VOID_TIME;
    stream->time.latest = VOID_TIME;
    stream->bytes = 0;
    stream->rate = VOID_RATE;

    InitEvent(&stream->event);

    return;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:53:22  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
