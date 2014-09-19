#pragma ident "$Id: state.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : state.c
 Purpose  : State file maintenance routines.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  05 Oct 2005  ---- (pld) Add/modify error messages.
  17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#define _STATE_C
#include "archive.h"

/* Local prototypes ---------------------------------------------------*/

/*---------------------------------------------------------------------*/
BOOL ArchiveSync(H_ARCHIVE harchive)
{
    FILE_STAT stat;

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* If we're open for write and the update timer has expired... */
    if (_archive[harchive].access == ARC_WRITE) {
        if (_archive[harchive].dirty) {
            if (!Timer48Expired(&_archive[harchive].update))
                return (TRUE);
            ArchiveLog(ARC_LOG_MAXIMUM, "Synchronize state file for write");
            if (!WriteState(harchive))
                return (FALSE);
            _archive[harchive].dirty = FALSE;
        }
        /* Restart the update timer */
        Timer48Restart(&_archive[harchive].update);
    }
    /* If we're open for read... */
    else if (_archive[harchive].access == ARC_READ) {
        /* If the state file has changed since we last looked... */
        if (!GetFileStat(_archive[harchive].filespec, &stat))
            return (FALSE);
        if (memcmp(&_archive[harchive].stat, &stat, sizeof(FILE_STAT)) != 0) {
            ArchiveLog(ARC_LOG_VERBOSE, "State file has changed, synchronizing");
            if (!ReadState(harchive))
                return (FALSE);
        }
    }
    else
        return (FALSE);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
VOID InitState(STATE * state)
{

    memset(state, 0, sizeof(STATE));

    /* Initialize state elements */
    state->write = FALSE;
    strncpy(state->version, ARC_VERSION, VERSION_LEN);
    strcpy(state->name, "Unnamed Archive");
    state->created = SystemMSTime();
    state->last_update = state->created;
    state->time.earliest = VOID_TIME;
    state->time.latest = VOID_TIME;
    state->n_streams = 0;
    state->max_bytes = VOID_UINT64;
    state->thres_bytes = VOID_UINT64;
    state->bytes = 0;

    return;
}

/*---------------------------------------------------------------------*/
BOOL CreateState(CHAR * filespec, STATE * state)
{
    H_FILE file;

    ArchiveLog(ARC_LOG_VERBOSE, "Creating state file: %s", filespec);

    if ((file = FileOpenForWrite(filespec)) == VOID_H_FILE) {
        _archive_error = ARC_FILE_IO_ERROR;
        ArchiveLog(ARC_LOG_ERRORS, "CreateState: FileOpenForWrite failed");
        return (FALSE);
    }

    if (SerializedWrite(state, _state_template, file) == VOID_UINT32) {
        ArchiveLog(ARC_LOG_ERRORS, "CreateState: SerializedWrite failed");
        return (FALSE);
    }

    if (!FileClose(file)) {
        ArchiveLog(ARC_LOG_ERRORS, "CreateState: FileClose failed");
        _archive_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL OpenStateForWrite(H_ARCHIVE harchive)
{

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* Create filespec for state file and open for read */
    sprintf(_archive[harchive].filespec, "%s%c%s", _archive[harchive].path,
        PATH_DELIMITER, STATE_FILENAME);
    if ((_archive[harchive].file = FileOpenForRead(_archive[harchive].filespec)) == VOID_H_FILE) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForWrite: Error opening state file: %s", _archive[harchive].filespec);
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }
    /* Read the current state */
    if (!ReadState(harchive))
        return (FALSE);
    if (_archive[harchive].state.write) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForWrite: Archive is open for write by another process: %s", _archive[harchive].path);
        _archive[harchive].last_error = ARC_PERMISSION_DENIED;
        return (FALSE);
    }
    if (!FileClose(_archive[harchive].file)) {
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    /* Reopen for write */
    if ((_archive[harchive].file = FileOpenForWrite(_archive[harchive].filespec)) == VOID_H_FILE) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForWrite: Error opening state file: %s", _archive[harchive].filespec);
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }
    /* Mark as open for write */
    _archive[harchive].state.write = TRUE;
    /* Archive is open for read-write access */
    _archive[harchive].access = ARC_WRITE;
    /* Start the update timer */
    Timer48Start(&_archive[harchive].update, 0);
    if (!WriteState(harchive))
        return (FALSE);

    /* Start the update timer */
    Timer48Start(&_archive[harchive].update, UPDATE_INTERVAL);

    ArchiveLog(ARC_LOG_MAXIMUM, "State file opened for write: %s", _archive[harchive].filespec);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL OpenStateForRead(H_ARCHIVE harchive)
{

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* Build the state filespec and open */
    sprintf(_archive[harchive].filespec, "%s%c%s", _archive[harchive].path,
        PATH_DELIMITER, STATE_FILENAME);
    if ((_archive[harchive].file = FileOpenForRead(_archive[harchive].filespec)) == VOID_H_FILE) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForRead: Error opening state file: %s", _archive[harchive].filespec);
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    /* Read the current state */
    if (!ReadState(harchive)) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForRead: Error trying to retrieve state: %s", _archive[harchive].filespec);
        FileClose(_archive[harchive].file);
        return (FALSE);
    }

    /* File is open for read access */
    _archive[harchive].access = ARC_READ;

    /* Save file status so we can detect changes to the file */
    if (!GetFileStat(_archive[harchive].filespec, &_archive[harchive].stat)) {
        ArchiveLog(ARC_LOG_ERRORS, "OpenStateForRead: Error retrieving file state: %s", _archive[harchive].filespec);
        FileClose(_archive[harchive].file);
        return (FALSE);
        }

    ArchiveLog(ARC_LOG_MAXIMUM, "State file opened for read: %s", _archive[harchive].filespec);

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL ReadState(H_ARCHIVE harchive)
{
    UINT32 i;
    STREAM stream;
    NODE *node;

    if (!ValidateHandle(harchive)) {
        ArchiveLog(ARC_LOG_ERRORS, "ReadState: Invalid archive handle");
        return (FALSE);
     }

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    /* Go to top of file */
    if (!FileRewind(_archive[harchive].file)) {
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        ArchiveLog(ARC_LOG_ERRORS, "ReadState: Error rewinding state file (state): %s", _archive[harchive].filespec);
        return (FALSE);
    }

    /* Read state structure */
    InitState(&_archive[harchive].state);
    if (SerializedRead(&_archive[harchive].state, _state_template,
            _archive[harchive].file) == VOID_UINT32) {
        ArchiveLog(ARC_LOG_ERRORS, "ReadState: Error reading state file (state): %s", _archive[harchive].filespec);
        return (FALSE);
    }

    /* If we don't have streams... */
    if (_archive[harchive].state.n_streams == 0)
        return (TRUE);

    /* Read stream structures into stream list */

    /* Destroy any existing list */
    DestroyList(&_archive[harchive].streams);

    /* Set node to head */
    node = &_archive[harchive].streams.head;
    for (i = 0; i < _archive[harchive].state.n_streams; i++) {
        /* Clean stream record */
        InitStream(&stream);
        if (SerializedRead(&stream, _stream_template,
                _archive[harchive].file) == VOID_UINT32) {
            ArchiveLog(ARC_LOG_ERRORS, "ReadState: Error reading state file (stream): %s", _archive[harchive].filespec);
            return (FALSE);
        }

        /* Insert new node after the current one */
        if ((node = InsertNodeAfter(node, &stream, sizeof(STREAM))) == NULL) {
            _archive_error = ARC_NO_MEMORY;
            ArchiveLog(ARC_LOG_ERRORS, "ReadState: Error allocating stream node (stream): %s", _archive[harchive].filespec);
            return (FALSE);
        }
    }

    return (TRUE);
}

/*---------------------------------------------------------------------*/
BOOL WriteState(H_ARCHIVE harchive)
{
    UINT32 i;
    STREAM *stream;
    NODE *node;

    if (!ValidateHandle(harchive))
        return (FALSE);

    _archive[harchive].last_error = ARC_NO_ERROR;
    _archive_error = ARC_NO_ERROR;

    if (_archive[harchive].access != ARC_WRITE) {
        _archive[harchive].last_error = ARC_PERMISSION_DENIED;
        return (FALSE);
    }

    /* Go to top of file */
    if (!FileRewind(_archive[harchive].file)) {
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    ArchiveLog(ARC_LOG_MAXIMUM, "Writing state file: %s", _archive[harchive].filespec);

    /* Write state structure */
    if (SerializedWrite(&_archive[harchive].state, _state_template,
            _archive[harchive].file) == VOID_UINT32) {
        ArchiveLog(ARC_LOG_ERRORS, "WriteState: Error writing state file (state): %s",
            _archive[harchive].filespec);
        return (FALSE);
    }

    /* If we have streams... */
    if (_archive[harchive].state.n_streams == 0)
        return (TRUE);

    /* Write stream list structures */
    if ((node = FirstNode(&_archive[harchive].streams)) == NULL) {
        printf("\nERROR: WriteState: Unexpected empty stream list!");
        _archive[harchive].last_error = ARC_INTERNAL_ERROR;
        return (FALSE);
    }
    i = 0;
    while (TRUE) {
        stream = (STREAM *) node->data;

        if (SerializedWrite(stream, _stream_template,
                _archive[harchive].file) == VOID_UINT32) {
            ArchiveLog(ARC_LOG_ERRORS, "WriteState: Error writing state file (stream): %s",
                _archive[harchive].filespec);
            return (FALSE);
        }

        if (++i >= _archive[harchive].state.n_streams)
            break;

        if ((node = NextNode(node)) == NULL) {
            printf("\nERROR: WriteState: Unexpected end of stream list!");
            _archive[harchive].last_error = ARC_INTERNAL_ERROR;
            return (FALSE);
        }
    }

    /* Flush buffers to disk */
    if (!FileFlush(_archive[harchive].file)) {
        _archive[harchive].last_error = ARC_FILE_IO_ERROR;
        return (FALSE);
    }

    return (TRUE);
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:23  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
