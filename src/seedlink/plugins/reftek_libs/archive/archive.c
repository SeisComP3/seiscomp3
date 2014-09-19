#pragma ident "$Id: archive.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : archive.c
 Purpose  : General utility functions.
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

-----------------------------------------------------------------------*/

#define _ARCHIVE_C
#define _DECLARE_GLOBALS
#include "archive.h"

/*---------------------------------------------------------------------*/
static BOOL _archive_api_initialized = FALSE;
static VOID(*UserLogFunction) (CHAR * string) = NULL;
static UINT32 _archive_log_level = ARC_LOG_ERRORS;

#ifdef _DEBUG
extern UINT32 _allocated_nodes;
#endif

/*---------------------------------------------------------------------*/
/* General API functions */
/*---------------------------------------------------------------------*/
VOID ArchiveInitAPI(VOID)
{
    H_ARCHIVE harchive;
    H_STREAM hstream;

    if (_archive_api_initialized)
        return;

    _archive_api_initialized = TRUE;

    for (harchive = 0; harchive < MAX_OPEN_ARCHIVES; harchive++)
        InitArchive(harchive);

    for (hstream = 0; hstream < MAX_OPEN_STREAMS; hstream++)
        InitRead(hstream);

    return;
}

/*---------------------------------------------------------------------*/
VOID ArchiveCloseAll(VOID)
{
    INT16 i;

    for (i = 0; i < MAX_OPEN_ARCHIVES; i++) {
        if (_archive[i].access != ARC_CLOSED)
            ArchiveClose(i);
    }

    DestroyStash();

#ifdef _DEBUG
    ArchiveLog(ARC_LOG_MAXIMUM, "Allocated nodes: %lu", _allocated_nodes);
#endif

    return;
}

/*---------------------------------------------------------------------*/
BOOL ArchiveInformation(H_ARCHIVE harchive, ARC_INFO * info)
{

    ASSERT(info != NULL);

    if (!ValidateHandle(harchive))
        return (FALSE);

    strcpy(info->running_version, ARC_VERSION);
    strcpy(info->copyright, ARC_COPYRIGHT);
    strcpy(info->created_version, _archive[harchive].state.version);
    strcpy(info->name, _archive[harchive].state.name);
    info->created = _archive[harchive].state.created;
    info->last_update = _archive[harchive].state.last_update;
    info->open_for_write = _archive[harchive].state.write;
    info->time.earliest = _archive[harchive].state.time.earliest;
    info->time.latest = _archive[harchive].state.time.latest;
    info->n_streams = _archive[harchive].state.n_streams;
    info->max_bytes = _archive[harchive].state.max_bytes;
    info->thres_bytes = _archive[harchive].state.thres_bytes;
    info->bytes = _archive[harchive].state.bytes;

    return (TRUE);
}

/*---------------------------------------------------------------------*/
VOID ArchiveLogInfo(ARC_INFO * info)
{
    CHAR string[32];

    ASSERT(info != NULL);

    ArchiveLog(ARC_LOG_VERBOSE, "Running API:       %s", info->running_version);
    ArchiveLog(ARC_LOG_VERBOSE, "Created by:        %s", info->created_version);
    ArchiveLog(ARC_LOG_VERBOSE, "Archive name:      %s", info->name);
    ArchiveLog(ARC_LOG_VERBOSE, "Created on:        %s", FormatMSTime(string, info->created, 10));
    ArchiveLog(ARC_LOG_VERBOSE, "Last modification: %s", FormatMSTime(string, info->last_update, 10));
    ArchiveLog(ARC_LOG_VERBOSE, "Writes allowed:    %s", (info->open_for_write ? "No" : "Yes"));
    ArchiveLog(ARC_LOG_VERBOSE, "Earliest data:     %s",
        (UndefinedTime(info->time.earliest) ? "Undefined" : FormatMSTime(string, info->time.earliest, 10)));
    ArchiveLog(ARC_LOG_VERBOSE, "Latest data:       %s",
        (UndefinedTime(info->time.latest) ? "Undefined" : FormatMSTime(string, info->time.latest, 10)));
    ArchiveLog(ARC_LOG_VERBOSE, "Streams present:   %lu", info->n_streams);
    ArchiveLog(ARC_LOG_VERBOSE, "Max allowed size:  %s bytes",
        (info->max_bytes == VOID_UINT64 ? "Unlimited" :
            FormatAsSIBinaryUnit(string, (INT64) info->max_bytes, TRUE)));
    ArchiveLog(ARC_LOG_VERBOSE, "Purge threshold:   %s bytes",
        (info->thres_bytes == VOID_UINT64 ? "Unlimited" :
            FormatAsSIBinaryUnit(string, (INT64) info->thres_bytes, TRUE)));
    ArchiveLog(ARC_LOG_VERBOSE, "Current size:      %s bytes",
        FormatAsSIBinaryUnit(string, (INT64) info->bytes, TRUE));

    return;
}

/*---------------------------------------------------------------------*/
VOID ArchiveLogLevel(UINT32 level)
{

    if (level > ARC_LOG_MAXIMUM)
        level = ARC_LOG_MAXIMUM;

    _archive_log_level = level;

    return;
}

/*---------------------------------------------------------------------*/
VOID ArchiveLogFunction(VOID(*function) (CHAR * string))
{

    ASSERT(function != NULL);

    UserLogFunction = function;

    return;
}

/*---------------------------------------------------------------------*/
BOOL ArchiveParseCriteria(STREAM * criteria, CHAR * string)
{
    CHAR element[CRI_MAX_ELEM][CRI_ELEM_LENGTH + 1], *ptr;
    UINT8 channel;
    INT16 i, j;

    ASSERT(criteria != NULL);
    ASSERT(string != NULL);

    InitStream(criteria);

    /* Wildcard everything */
    criteria->unit = ALL_UNITS;
    criteria->stream = ALL_STREAMS;
    criteria->chn_mask = ALL_CHANNELS;
    criteria->time.earliest = ALL_TIMES;
    criteria->time.latest = ALL_TIMES;

    /* Decompose string into elements */
    for (i = 0; i < CRI_MAX_ELEM; i++)
        memset(element[i], 0, CRI_ELEM_LENGTH + 1);
    i = 0;
    ptr = string;
    while (TRUE) {
        if ((ptr = strrchr(ptr, CRI_DELIMITER)) == NULL) {
            strncpy(element[i], string, CRI_ELEM_LENGTH);
            i++;
            break;
        }
        *ptr = '\0';
        strncpy(element[i], ptr + 1, CRI_ELEM_LENGTH);
        ptr = string;
        i++;
        if (i > CRI_MAX_ELEM)
            break;
    }

    /* Parse elements in reverse order */

    /* Unit ID */
    if (--i < 0)
        return (TRUE);
    if (element[i][0] != CRI_WILDCARD && isdigit(element[i][0])) {
        criteria->unit = (UINT16) strtoul(element[i], (char **) NULL, 16);
        if (criteria->unit > RF_MAX_UNIT) {
            _archive_error = ARC_BAD_CRITERIA;
            return (FALSE);
        }
    }

    /* Stream number */
    if (--i < 0)
        return (TRUE);
    if (element[i][0] != CRI_WILDCARD && isdigit(element[i][0])) {
        criteria->stream = (UINT8) atoi(element[i]);
        if (criteria->stream > RF_MAX_STREAMS) {
            _archive_error = ARC_BAD_CRITERIA;
            return (FALSE);
        }
    }

    /* Channel mask */
    if (--i < 0)
        return (TRUE);
    if (element[i][0] != CRI_WILDCARD && isdigit(element[i][0])) {
        criteria->chn_mask = 0;
        j = 0;
        while (element[i][j]) {
            channel = (UINT8) element[i][j] - '0';
            if (channel > RF_MAX_CHANNELS) {
                _archive_error = ARC_BAD_CRITERIA;
                return (FALSE);
            }
            criteria->chn_mask |= (0x0001 << (channel - 1));
            j++;
            if (j >= CRI_ELEM_LENGTH)
                break;
        }
    }

    /* Earliest time */
    if (--i < 0)
        return (TRUE);
    if (element[i][0] != CRI_WILDCARD && isdigit(element[i][0]))
        criteria->time.earliest = ParseMSTime(element[i], FALSE);

    /* Latest time */
    if (--i < 0)
        return (TRUE);
    if (element[i][0] != CRI_WILDCARD) {
        if (element[i][0] == '+' && !UndefinedTime(criteria->time.earliest))
            criteria->time.latest = criteria->time.earliest + atof(element[i]);
        else if (isdigit(element[i][0]))
            criteria->time.latest = ParseMSTime(element[i], FALSE);
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
CHAR *ArchiveFormatCriteria(CHAR * string, STREAM * criteria)
{
    CHAR buf[32];
    UINT16 channel;
    size_t i;

    ASSERT(string != NULL);
    ASSERT(criteria != NULL);

    if (criteria->unit == ALL_UNITS)
        i = sprintf(string, "%c", CRI_WILDCARD);
    else
        i = sprintf(string, "%04X", criteria->unit);

    if (criteria->stream == ALL_STREAMS)
        i += sprintf(&string[i], "%c%c", CRI_DELIMITER, CRI_WILDCARD);
    else
        i += sprintf(&string[i], "%c%u", CRI_DELIMITER, criteria->stream);

    if (criteria->chn_mask == ALL_CHANNELS || criteria->chn_mask == 0)
        i += sprintf(&string[i], "%c%c", CRI_DELIMITER, CRI_WILDCARD);
    else {
        i += sprintf(&string[i], "%c", CRI_DELIMITER);
        for (channel = 1; channel <= RF_MAX_CHANNELS; channel++) {
            if (criteria->chn_mask & (0x0001 << (channel - 1)))
                i += sprintf(&string[i], "%u", channel);
        }
    }

    if (UndefinedTime(criteria->time.earliest))
        i += sprintf(&string[i], "%c%c", CRI_DELIMITER, CRI_WILDCARD);
    else
        i += sprintf(&string[i], "%c%s", CRI_DELIMITER,
            FormatMSTime(buf, criteria->time.earliest, 10));

    if (UndefinedTime(criteria->time.latest))
        i += sprintf(&string[i], "%c%c", CRI_DELIMITER, CRI_WILDCARD);
    else {
        if (UndefinedTime(criteria->time.earliest)) {
            i += sprintf(&string[i], "%c%s", CRI_DELIMITER,
                FormatMSTime(buf, criteria->time.latest, 10));
        }
        else {
            i += sprintf(&string[i], "%c%+.3lf", CRI_DELIMITER,
                criteria->time.latest - criteria->time.earliest);
        }
    }

    return (string);
}

/*---------------------------------------------------------------------*/
/* Helpers... */
/*---------------------------------------------------------------------*/
VOID UpdateTimeRange(TIME_RANGE * range, REAL64 time)
{

    ASSERT(range != NULL);

    if (UndefinedTime(time))
        return;

    if (UndefinedTime(range->earliest) || time < range->earliest)
        range->earliest = time;

    if (UndefinedTime(range->latest) || time > range->latest)
        range->latest = time;

    return;
}

/*---------------------------------------------------------------------*/
VOID ArchiveLog(UINT32 level, CHAR * format,...)
{
    CHAR string[ARC_MAX_LOG_MESSAGE + 1];
    va_list marker;

    if (UserLogFunction != NULL && level <= _archive_log_level) {

        va_start(marker, format);
        vsnprintf(string, ARC_MAX_LOG_MESSAGE, format, marker);
        va_end(format);

        (*UserLogFunction) (string);
    }

    return;
}

/*---------------------------------------------------------------------*/
BOOL ValidateHandle(H_ARCHIVE harchive)
{

    if (!_archive_api_initialized) {
        _archive_error = ARC_NOT_INITIALIZED;
        return (FALSE);
    }

    if (harchive > MAX_OPEN_ARCHIVES) {
        _archive_error = ARC_INVALID_HANDLE;
        return (FALSE);
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
H_ARCHIVE GetNextHandle(VOID)
{
    H_ARCHIVE harchive;

    harchive = VOID_H_ARCHIVE;
    for (harchive = 0; harchive < MAX_OPEN_ARCHIVES; harchive++) {
        if (_archive[harchive].access == ARC_CLOSED)
            return (harchive);
    }

    _archive_error = ARC_NO_HANDLE;
    return (VOID_H_ARCHIVE);
}

/*---------------------------------------------------------------------*/
BOOL UndefinedTime(REAL64 time)
{

    /* Is time within a millisecond of VOID */
    if (time < VOID_TIME - (0.5 * SI_MILLI) ||
        time > VOID_TIME + (0.5 * SI_MILLI))
        return (FALSE);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
INT32 CompareYD(REAL64 time1, REAL64 time2)
{
    INT32 year1, doy1, year2, doy2, hour, minute, rounded1, rounded2;
    REAL64 second;

    /* Compare year and day portion of time 1 and 2 */
    DecodeMSTimeDOY(time1, &year1, &doy1, &hour, &minute, &second);
    DecodeMSTimeDOY(time2, &year2, &doy2, &hour, &minute, &second);

    rounded1 = (INT32) (EncodeMSTimeDOY(year1, doy1, hour, minute, second) + (REAL64) 0.5);
    rounded2 = (INT32) (EncodeMSTimeDOY(year2, doy2, hour, minute, second) + (REAL64) 0.5);

    /* Return difference as integer seconds */
    return (rounded1 - rounded2);
}

/*---------------------------------------------------------------------*/
VOID InitArchive(H_ARCHIVE harchive)
{

    if (harchive > MAX_OPEN_ARCHIVES)
        return;

    InitState(&_archive[harchive].state);
    CreateList(&_archive[harchive].streams);
    InitPurge(&_archive[harchive].purge);
    _archive[harchive].stream = NULL;
    _archive[harchive].path[0] = '\0';
    _archive[harchive].access = ARC_CLOSED;
    _archive[harchive].dirty = FALSE;
    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive[harchive].file = VOID_H_FILE;
    Timer48Start(&_archive[harchive].update, UPDATE_INTERVAL);
    _archive[harchive].last_error = ARC_NO_ERROR;
    memset(&_archive[harchive].stat, 0, sizeof(FILE_STAT));
    MUTEX_INIT(&_archive[harchive].mutex);

    return;
}

/* Assertion routine --------------------------------------------------*/

#ifdef _DEBUG
VOID _ArchiveAssert(CHAR * file, UINT32 line)
{

    fflush(NULL);
    fprintf(stderr, "\nAssertion failed: %s, line %lu\n", file, line);
    fflush(stderr);

    abort();
}

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:56  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.4  2002/05/14 16:31:05  nobody
 * Batch builds cleanly
 *
 * Revision 1.3  2002/01/18 17:53:21  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
