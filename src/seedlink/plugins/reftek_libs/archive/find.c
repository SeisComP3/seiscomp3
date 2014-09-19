#pragma ident "$Id: find.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : find.c
 Purpose  : Find stream files.
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
   3 Aug 1999  ---- (RLB) Mod's for 1.10, merge capabilities
  15 Nov 1999       (RLB) Fixed search in last event problem.

-----------------------------------------------------------------------*/

#define _FIND_C
#include "archive.h"

/* Local constants ----------------------------------------------------*/
#define EXACT 2
#define LENGTH_FIND_WINDOW  7200.0          /* Two hours */

/* Local prototypes ---------------------------------------------------*/
BOOL FindDayDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event);
BOOL FindUnitDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event);
BOOL FindStreamDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event);
BOOL FindFirstEventFile(H_ARCHIVE harchive, STREAM * criteria, EVENT * event);
BOOL FindNextEventFile(H_ARCHIVE harchive, STREAM * criteria, EVENT * event);

/*---------------------------------------------------------------------*/
BOOL ArchiveFindFirstEvent(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the first event that match matches criteria */

    ArchiveLog(ARC_LOG_MAXIMUM, "ArchiveFindFirstEvent: %s", ArchiveFormatCriteria(string, criteria));

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* New search */
    InitEvent(event);

    /* Find the oldest day directory in specified range... */
    while (FindDayDirectory(harchive, criteria, event)) {
        /* ...that contains the specified unit(s)... */
        while (FindUnitDirectory(harchive, criteria, event)) {
            /* ...that contains the specified stream(s)... */
            while (FindStreamDirectory(harchive, criteria, event)) {
                /* ...that contains the oldest event data in the specified
                   range... */
                if (FindFirstEventFile(harchive, criteria, event)) {
                    return (TRUE);
                }
                if (criteria->stream == ALL_STREAMS) {
                    event->stream++;
                }
            }
            if (criteria->unit == ALL_UNITS) {
                event->unit++;
            }
        }
        event->yd += DAY;
    }

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL ArchiveFindNextEvent(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the next event that match matches criteria */

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    ArchiveLog(ARC_LOG_MAXIMUM, "ArchiveFindNextEvent: %s", ArchiveFormatCriteria(string, criteria));

    do {
        do {
            do {
                /* If we haven't found an event for this stream... */
                if (UndefinedTime(event->time.earliest)) {
                    if (FindFirstEventFile(harchive, criteria, event))
                        return (TRUE);
                }
                /* Otherwise, add a millisecond to time and find next event */
                else {
                    event->time.earliest += SI_MILLI;
                    if (FindNextEventFile(harchive, criteria, event))
                        return (TRUE);
                }
                /* Look for next stream */
                if (criteria->stream == ALL_STREAMS)
                    event->stream++;
            } while (FindStreamDirectory(harchive, criteria, event));
            /* Look for next unit */
            if (criteria->unit == ALL_UNITS)
                event->unit++;
        } while (FindUnitDirectory(harchive, criteria, event));
        /* Look for next day */
        event->yd += DAY;
    } while (FindDayDirectory(harchive, criteria, event));

    return (FALSE);
}

/*---------------------------------------------------------------------*/
BOOL FindDayDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];
    BOOL checking_previous;
    int year, doy;
    INT16 found;
    REAL64 earliest;
    EVENT info;

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the oldest day directory that is in the specified range,
       inclusive */

    if (!ValidateHandle(harchive)) {
        return (FALSE);
    }

    ArchiveLog(ARC_LOG_MAXIMUM, "FindDayDirectory: %s", ArchiveFormatCriteria(string, criteria));

    /* If first time in */
    checking_previous = FALSE;
    if (UndefinedTime(event->yd)) {
        /* Search for specified earliest time, start in previous day (might be
           undefined) */
        earliest = criteria->time.earliest - DAY;
        checking_previous = TRUE;
    }
    else
        earliest = event->yd;

try_again:

    /* Look at all day directories... */
    sprintf(info.filespec, "%s%c*", _archive[harchive].path, PATH_DELIMITER);
    info.attribute = FIND_SUBDIR;

    found = FALSE;
    if (FileFindFirst(&info)) {
        do {
            /* Ignore . and .. and insist on right length! */
            if (info.filespec[0] == '.' || strlen(info.filespec) != 7) {
                continue;
            }
            /* Decode year and doy */
            sscanf(info.filespec, "%4d%3d", &year, &doy);
            /* Set hour, minute, and second to zeros */
            info.yd = EncodeMSTimeDOY((INT32) year, (INT32) doy, 0, 0, 0.0);

            /* If earliest is specified and time is less, go around */
            if (!UndefinedTime(earliest) &&
                CompareYD(info.yd, earliest) < 0) {
                continue;
            }

            /* If latest is specified and time is greater, go around */
            if (!UndefinedTime(criteria->time.latest) &&
                CompareYD(info.yd, criteria->time.latest) > 0) {
                continue;
            }

            /* Otherwise, we'll take the oldest directory that we see */
            if (!found || CompareYD(info.yd, event->yd) < 0) {
                /* Check for an exact match to specified criteria */
                if (!UndefinedTime(earliest) &&
                    CompareYD(info.yd, earliest) == 0) {
                    found = EXACT;
                }
                else {
                    found = TRUE;
                }

                event->yd = info.yd;
                strcpy(event->filespec, info.filespec);

                /* If we have an exact match, don't look further */
                if (found == EXACT) {
                    break;
                }
            }
        } while (FileFindNext(&info));
    }

    FileFindClose(&info);

    if ((BOOL) found) {
        ArchiveLog(ARC_LOG_MAXIMUM, "Day directory: %s", event->filespec);
        _archive[harchive].last_error = ARC_NO_ERROR;
        return (TRUE);
    }
    else {
        if (checking_previous) {
            /* Search for specified earliest time */
            earliest = criteria->time.earliest;
            checking_previous = FALSE;
            goto try_again;
        }
        ArchiveLog(ARC_LOG_MAXIMUM, "Day directory not found");
        /* Clear all searches */
        InitEvent(event);
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }
}

/*---------------------------------------------------------------------*/
BOOL FindUnitDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];
    int unit;
    INT16 found;
    UINT16 lowest;
    EVENT info;
    MSTIME_COMP time;

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the specified or lowest numbered unit directory */

    if (!ValidateHandle(harchive))
        return (FALSE);

    ArchiveLog(ARC_LOG_MAXIMUM, "FindUnitDirectory: %s", ArchiveFormatCriteria(string, criteria));

    /* Make sure that we have a valid upper level directory */
    if (UndefinedTime(event->yd)) {
        event->unit = VOID_UNIT;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* We'll need time components below */
    DecomposeMSTime(event->yd, &time);

    /* If unit is specified in criteria... */
    if (criteria->unit != ALL_UNITS) {
        /* Have we already done this once? */
        if (event->unit == VOID_UNIT) {
            /* Build pathname */
            sprintf(info.filespec, "%s%c%04d%03d%c%04X",
                _archive[harchive].path, PATH_DELIMITER,
                time.year, time.doy, PATH_DELIMITER,
                criteria->unit);
            /* Does this directory exist? */
            if (IsDirectory(info.filespec)) {
                /* Yes, save the unit number and pathname */
                event->unit = criteria->unit;
                strcpy(event->filespec, info.filespec);
                /* We're done */
                _archive[harchive].last_error = ARC_NO_ERROR;
                return (TRUE);
            }
        }
        event->unit = VOID_UNIT;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    if (event->unit == VOID_UNIT)
        event->unit = 0;

    lowest = event->unit;

    /* Have we reached the max unit number? */
    if (lowest > RF_MAX_UNIT) {
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* Create file search criteria for all units... */
    sprintf(info.filespec, "%s%c%04d%03d%c*",
        _archive[harchive].path, PATH_DELIMITER,
        time.year, time.doy, PATH_DELIMITER);
    info.attribute = FIND_SUBDIR;

    /* These mean that we haven't found anything yet */
    event->unit = VOID_UNIT;
    found = FALSE;

    if (FileFindFirst(&info)) {
        do {
            /* Ignore . and .. and insist on right length! */
            if (info.filespec[0] == '.' || strlen(info.filespec) != 4)
                continue;
            /* Decode unit number */
            sscanf(info.filespec, "%04X", &unit);
            info.unit = (UINT16) unit;

            /* Ignore all units that are less than lowest we're looking for */
            if (info.unit < lowest || info.unit > RF_MAX_UNIT)
                continue;

            /* Pick up lowest numbered unit */
            if (!found || info.unit <= event->unit) {
                /* Check for exact match to any specified criteria */
                if (criteria->unit != ALL_UNITS && info.unit == criteria->unit)
                    found = EXACT;
                else
                    found = TRUE;

                /* Save the unit number and pathname */
                event->unit = info.unit;
                sprintf(event->filespec, "%s%c%04d%03d%c%s",
                    _archive[harchive].path, PATH_DELIMITER,
                    time.year, time.doy, PATH_DELIMITER,
                    info.filespec);

                /* If exact, look no further */
                if (found == EXACT)
                    break;
            }
        } while (FileFindNext(&info));
    }

    FileFindClose(&info);

    if ((BOOL) found) {
        ArchiveLog(ARC_LOG_MAXIMUM, "Unit directory: %s", event->filespec);
        _archive[harchive].last_error = ARC_NO_ERROR;
        return (TRUE);
    }
    else {
        ArchiveLog(ARC_LOG_MAXIMUM, "Unit directory: not found");
        event->unit = VOID_UNIT;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }
}

/*---------------------------------------------------------------------*/
BOOL FindStreamDirectory(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];
    int stream;
    UINT8 lowest;
    INT16 found;
    EVENT info;
    MSTIME_COMP time;

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the specified or lowest numbered stream directory */

    if (!ValidateHandle(harchive))
        return (FALSE);

    ArchiveLog(ARC_LOG_MAXIMUM, "FindStreamDirectory: %s", ArchiveFormatCriteria(string, criteria));

    /* Make sure that we have a valid upper level directories */
    if (UndefinedTime(event->yd) || event->unit == VOID_UNIT) {
        event->stream = VOID_STREAM;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* We'll need time components below */
    DecomposeMSTime(event->yd, &time);

    /* If stream is specified in criteria... */
    if (criteria->stream != ALL_STREAMS) {
        if (event->stream == VOID_STREAM) {
            /* Build pathname */
            sprintf(info.filespec, "%s%c%04d%03d%c%04X%c%01u",
                _archive[harchive].path, PATH_DELIMITER,
                time.year, time.doy, PATH_DELIMITER,
                event->unit, PATH_DELIMITER,
                criteria->stream);
            /* Does this directory exist? */
            if (IsDirectory(info.filespec)) {
                /* Yes, save the stream number and pathname */
                event->stream = criteria->stream;
                strcpy(event->filespec, info.filespec);
                /* We're done */
                _archive[harchive].last_error = ARC_NO_ERROR;
                return (TRUE);
            }
        }
        /* No, we're still done */
        event->stream = VOID_STREAM;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    if (event->stream == VOID_STREAM)
        event->stream = 0;

    lowest = event->stream;

    /* Have we reached the max stream number? */
    if (lowest > RF_MAX_STREAMS) {
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* Create file search criteria for all streams... */
    sprintf(info.filespec, "%s%c%04d%03d%c%04X%c*",
        _archive[harchive].path, PATH_DELIMITER,
        time.year, time.doy, PATH_DELIMITER,
        event->unit, PATH_DELIMITER);
    info.attribute = FIND_SUBDIR;

    /* These mean that we haven't found anything yet */
    event->stream = VOID_STREAM;
    found = FALSE;

    if (FileFindFirst(&info)) {
        do {
            /* Ignore . and .. and insist on right length! */
            if (info.filespec[0] == '.' || strlen(info.filespec) != 1)
                continue;
            /* Decode stream number */
            sscanf(info.filespec, "%1d", &stream);
            info.stream = (UINT8) stream;

            /* Ignore all streams that are less than lowest we're looking for */
            if (info.stream < lowest || info.stream > RF_MAX_STREAMS)
                continue;

            /* Pick up lowest numbered unit */
            if (!found || info.stream <= event->stream) {
                /* Check for exact match to any specified criteria */
                if (criteria->stream != ALL_STREAMS && info.stream == criteria->stream)
                    found = EXACT;
                else
                    found = TRUE;

                /* Save the stream number and pathname */
                event->stream = info.stream;
                sprintf(event->filespec, "%s%c%04d%03d%c%04X%c%s",
                    _archive[harchive].path, PATH_DELIMITER,
                    time.year, time.doy, PATH_DELIMITER,
                    event->unit, PATH_DELIMITER,
                    info.filespec);

                /* If exact, look no further */
                if (found == EXACT)
                    break;
            }
        } while (FileFindNext(&info));
    }

    FileFindClose(&info);

    if ((BOOL) found) {
        ArchiveLog(ARC_LOG_MAXIMUM, "Stream directory: %s", event->filespec);
        _archive[harchive].last_error = ARC_NO_ERROR;
        return (TRUE);
    }
    else {
        ArchiveLog(ARC_LOG_MAXIMUM, "Stream directory not found");
        event->stream = VOID_STREAM;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }
}

/*---------------------------------------------------------------------*/
BOOL FindFirstEventFile(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];
    long hour, minute, millisecond;
    unsigned long length;
    INT16 found;
    EVENT info, saved;
    MSTIME_COMP time;
    STREAM stream, *pstream;

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Look for file that contains criteria.range.earliest */

    ArchiveLog(ARC_LOG_MAXIMUM, "FindFirstEventFile: %s", ArchiveFormatCriteria(string, criteria));

    if (!ValidateHandle(harchive))
        return (FALSE);

    if (UndefinedTime(event->yd) ||
        event->unit == VOID_UNIT ||
        event->stream == VOID_STREAM) {
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* If criteria was not specified, get earliest file and return */
    if (UndefinedTime(criteria->time.earliest)) {
        event->time.earliest = VOID_TIME;
        event->time.latest = VOID_TIME;
        return (FindNextEventFile(harchive, criteria, event));
    }

    /* Save contents of event struct */
    memcpy(&saved, event, sizeof(EVENT));

try_again:

    /* We'll need time components below */
    DecomposeMSTime(event->yd, &time);

    /* Look at all event files for current stream */
    sprintf(info.filespec, "%s%c%04d%03d%c%04X%c%01u%c*",
        _archive[harchive].path, PATH_DELIMITER,
        time.year, time.doy, PATH_DELIMITER,
        event->unit, PATH_DELIMITER,
        event->stream, PATH_DELIMITER);
    info.attribute = FIND_FILE;

    found = FALSE;
    if (FileFindFirst(&info)) {
        do {
            /* Insist on right length and length delimiter in right place! */
            if (strlen(info.filespec) != 18 || info.filespec[9] != '_')
                continue;
            /* Decode time range */
            sscanf(info.filespec, "%2ld%2ld%5ld_%8x", &hour, &minute, &millisecond, &length);
            info.time.earliest = EncodeMSTimeDOY(time.year, time.doy, (INT32) hour,
                (INT32) minute, (REAL64) (millisecond * SI_MILLI));
            info.time.latest = info.time.earliest + ((REAL64) length * SI_MILLI);
            /* Check for zero length */
            if (length == 0) {
                /* See if latest time on this stream makes sense... */
                if ((pstream = LookupStream(harchive, event->unit, event->stream)) != NULL) {
                    if (pstream->time.latest > info.time.earliest &&
                        pstream->time.latest <= info.time.earliest + LENGTH_FIND_WINDOW) {
                        /* Use it instead */
                        info.time.latest = pstream->time.latest;
                    }
                }
            }

            /* We're looking for the file that contains criteria.range.earliest */
            if (criteria->time.earliest >= info.time.earliest &&
                criteria->time.earliest <= info.time.latest) {

                found = TRUE;

                /* Save time range, pathname, and bytes */
                event->time.earliest = info.time.earliest;
                event->time.latest = info.time.latest;
                event->bytes = info.bytes;
                sprintf(event->filespec, "%s%c%04d%03d%c%04X%c%01u%c%s",
                    _archive[harchive].path, PATH_DELIMITER,
                    time.year, time.doy, PATH_DELIMITER,
                    event->unit, PATH_DELIMITER,
                    event->stream, PATH_DELIMITER,
                    info.filespec);
                break;
            }
        } while (FileFindNext(&info));
    }

    FileFindClose(&info);

    /* If we've found the file, we're done */
    if (found) {
        ArchiveLog(ARC_LOG_MAXIMUM, "First event file: %s", event->filespec);
        _archive[harchive].last_error = ARC_NO_ERROR;
        return (TRUE);
    }

    /* Have we looked in the previous day? */
    if (CompareYD(event->yd, criteria->time.earliest) == 0) {
        ArchiveLog(ARC_LOG_MAXIMUM, "Looking in previous day");
        /* Copy current criteria and set times to start of previous day */
        memcpy(&stream, criteria, sizeof(STREAM));
        stream.time.earliest = event->yd - DAY;
        stream.time.latest = stream.time.earliest;
        /* Find, if anything fails, give up */
        if (FindDayDirectory(harchive, &stream, &info)) {
            if (FindUnitDirectory(harchive, &stream, &info)) {
                if (FindStreamDirectory(harchive, &stream, &info)) {
                    memcpy(event, &info, sizeof(EVENT));
                    goto try_again;
                }
            }
        }
    }
    /* Otherwise, just try to get earliest in range file */

    /* Restore event struct */
    memcpy(event, &saved, sizeof(EVENT));

    event->time.earliest = VOID_TIME;
    event->time.latest = VOID_TIME;
    return (FindNextEventFile(harchive, criteria, event));
}

/*---------------------------------------------------------------------*/
BOOL FindNextEventFile(H_ARCHIVE harchive, STREAM * criteria, EVENT * event)
{
    CHAR string[128];
    int hour, minute, millisecond;
    unsigned long length;
    BOOL found;
    REAL64 earliest;
    EVENT info;
    MSTIME_COMP time;
    STREAM *pstream;

    ASSERT(criteria != NULL);
    ASSERT(event != NULL);

    /* Return the oldest event file that is in the specified range, inclusive */

    if (!ValidateHandle(harchive))
        return (FALSE);

    ArchiveLog(ARC_LOG_MAXIMUM, "FindNextEventFile: %s", ArchiveFormatCriteria(string, criteria));

    /* Make sure that we have a valid upper level directories */
    if (UndefinedTime(event->yd) ||
        event->unit == VOID_UNIT ||
        event->stream == VOID_STREAM) {
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }

    /* We'll need time components below */
    DecomposeMSTime(event->yd, &time);

    /* If first time in */
    if (UndefinedTime(event->time.earliest))
        /* Search for specified earliest time (might be undefined) */
        earliest = criteria->time.earliest;
    else
        earliest = event->time.earliest;

    /* Look at all event files... */
    sprintf(info.filespec, "%s%c%04d%03d%c%04X%c%01u%c*",
        _archive[harchive].path, PATH_DELIMITER,
        time.year, time.doy, PATH_DELIMITER,
        event->unit, PATH_DELIMITER,
        event->stream, PATH_DELIMITER);
    info.attribute = FIND_FILE;

    found = FALSE;
    if (FileFindFirst(&info)) {
        do {
            /* Insist on right length and length delimiter in right place! */
            if (strlen(info.filespec) != 18 || info.filespec[9] != '_')
                continue;
            /* Decode time range */
            sscanf(info.filespec, "%2ld%2ld%5ld_%8x", &hour, &minute, &millisecond, &length);
            info.time.earliest = EncodeMSTimeDOY(time.year, time.doy, (INT32) hour,
                (INT32) minute, (REAL64) (millisecond * SI_MILLI));
            info.time.latest = info.time.earliest + ((REAL64) length * SI_MILLI);
            /* Check for zero length */
            if (length == 0) {
                /* See if latest time on this stream makes sense... */
                if ((pstream = LookupStream(harchive, event->unit, event->stream)) != NULL) {
                    if (pstream->time.latest > info.time.earliest &&
                        pstream->time.latest <= info.time.earliest + LENGTH_FIND_WINDOW) {
                        /* Use it instead */
                        info.time.latest = pstream->time.latest;
                    }
                }
            }

            /* If earliest is specified and time is less, go around */
            if (!UndefinedTime(earliest) && info.time.earliest < earliest)
                continue;

            /* If latest is specified and time is greater, go around */
            if (!UndefinedTime(criteria->time.latest) && info.time.earliest > criteria->time.latest)
                continue;

            /* Otherwise, we'll take the oldest file that we see */
            if (!found || info.time.earliest < event->time.earliest) {

                found = TRUE;

                /* Save time, pathname, and bytes */
                event->time.earliest = info.time.earliest;
                event->time.latest = info.time.latest;
                event->bytes = info.bytes;
                sprintf(event->filespec, "%s%c%04d%03d%c%04X%c%01u%c%s",
                    _archive[harchive].path, PATH_DELIMITER,
                    time.year, time.doy, PATH_DELIMITER,
                    event->unit, PATH_DELIMITER,
                    event->stream, PATH_DELIMITER,
                    info.filespec);
            }
        } while (FileFindNext(&info));
    }

    FileFindClose(&info);

    if (found) {
        ArchiveLog(ARC_LOG_MAXIMUM, "Event file: %s", event->filespec);
        _archive[harchive].last_error = ARC_NO_ERROR;
        return (TRUE);
    }
    else {
        ArchiveLog(ARC_LOG_MAXIMUM, "Event file not found");
        event->time.earliest = VOID_TIME;
        event->time.latest = VOID_TIME;
        _archive[harchive].last_error = ARC_NOT_FOUND;
        return (FALSE);
    }
}

/*---------------------------------------------------------------------*/
VOID InitEvent(EVENT * event)
{

    ASSERT(event != NULL);

    event->unit = VOID_UNIT;
    event->stream = VOID_STREAM;
    event->chn_mask = 0;
    event->number = 0;
    event->sequence = 0;
    event->time.earliest = VOID_TIME;
    event->time.latest = VOID_TIME;
    event->bytes = 0;
    event->file = VOID_H_FILE;
    event->filespec[0] = '\0';
    event->find = 0;
    event->attribute = FIND_FILE;
    event->yd = VOID_TIME;
    Timer48Start(&event->last_io, 0);

    return;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:53:21  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
