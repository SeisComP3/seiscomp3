/***************************************************************************
 * archivedata.c
 * Routine to control the archiving of data streams.  In reality a
 * front end to the data stream archiving routines in dsarchive.c
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified: 2004.197
 ***************************************************************************/

#include <string.h>
#include <libslink.h>

#include "archive.h"

/***************************************************************************
 * archstream_proc():
 * Save MiniSEED records into a pre-defined archive.  The appropriate
 * directories and files are created if nesecessary.  If files already
 * exist they are appended to.  If both 'msr' is NULL and 'reclen' is
 * zero then ds_shutdown() will be called to close all open files and
 * free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
int
archstream_proc (DataStream *datastream, SLMSrecord *msr, int reclen)
{
  char pathformat[400];

  /* Check if this is a call to shut everything down */
  if (msr == NULL && reclen == 0)
    {
      sl_log (1, 1, "Shutting down stream archiving for: %s\n",
	      datastream->path );
      ds_streamproc (datastream, NULL, NULL, 0);
      return 0;
    }

  if (datastream->archivetype == ARCH)
    strncpy (pathformat, datastream->path, sizeof(pathformat) - 1);

  else if (datastream->archivetype == SDS)
    snprintf (pathformat, sizeof(pathformat),
	      "%s/%%Y/%%n/%%s/%%c.%%t/%%n.%%s.%%l.%%c.%%t.%%Y.%%j",
	      datastream->path);

  else if (datastream->archivetype == BUD)
    snprintf (pathformat, sizeof(pathformat),
	      "%s/%%n/%%s/%%s.%%n.%%l.%%c.%%Y.%%j",
	      datastream->path);

  else if (datastream->archivetype == DLOG) {

    if ( ! strncmp(msr->fsdh.location, "  ", 2) )
      { /* No location code */
	snprintf (pathformat, sizeof(pathformat),
		  "%s/%%s/%%c.%%t/%%s.%%n.%%c.%%t.%%Y.%%j.#H#M",
		  datastream->path);
      }
    else
      { /* Location code present */
	snprintf (pathformat, sizeof(pathformat),
		  "%s/%%s/%%l.%%c.%%t/%%s.%%n.%%c.%%t.%%Y.%%j.#H#M",
		  datastream->path);
      }
  }

  return ds_streamproc (datastream, pathformat, msr, reclen);
}				/* End of gen_streamproc() */
