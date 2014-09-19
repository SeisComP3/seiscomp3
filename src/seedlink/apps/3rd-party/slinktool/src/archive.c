/***************************************************************************
 * archivedata.c
 * Routines to control the archiving data streams.  These are front ends
 * to the data stream archiving routines in dsarchive.c
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified: 2006.355
 ***************************************************************************/

#include <string.h>
#include <libslink.h>

#include "dsarchive.h"


/***************************************************************************
 * arch_streamproc(): 
 * Save MiniSEED records in a custom directory/file structure.  The
 * appropriate directories and files are created if nesecessary.  If
 * files already exist they are appended to.  If both 'archformat' and
 * 'msr' are NULL then ds_shutdown() will be called to close all open files
 * and free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
int
arch_streamproc (const char *archformat, const SLMSrecord *msr, int reclen,
		 int type, int idletimeout)
{
  static DataStream *streamroot = NULL;
  char format[400];

  /* Check if this is a call to shut everything down */
  if (archformat == NULL && msr == NULL)
    {
      sl_log (0, 1, "Shutting down stream archiving\n");
      ds_streamproc (&streamroot, NULL, NULL, 0, 0, 0);
      return 0;
    }

  strncpy (format, archformat, sizeof(format) - 1);

  return ds_streamproc (&streamroot, format, msr, reclen, type, idletimeout);
}				/* End of arch_streamproc() */


/***************************************************************************
 * sds_streamproc(): 
 * Save MiniSEED records in an SDS directory/file structure.  The
 * appropriate directories and files are created if nesecessary.  If
 * files already exist they are appended to.  If both 'basedir' and
 * 'msr' are NULL then ds_shutdown() will be called to close all open files
 * and free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
int
sds_streamproc (const char *basedir, const SLMSrecord *msr, int reclen,
		int type, int idletimeout)
{
  static DataStream *streamroot = NULL;
  char format[400];

  /* Check if this is a call to shut everything down */
  if (basedir == NULL && msr == NULL)
    {
      sl_log (0, 1, "Shutting down SDS archiving\n");
      ds_streamproc (&streamroot, NULL, NULL, 0, 0, 0);
      return 0;
    }

  snprintf (format, sizeof(format),
	    "%s/%%Y/%%n/%%s/%%c.%%t/%%n.%%s.%%l.%%c.%%t.%%Y.%%j",
	    basedir);

  return ds_streamproc (&streamroot, format, msr, reclen, type, idletimeout);
}				/* End of sds_streamproc() */


/***************************************************************************
 * bud_streamproc(): 
 * Save MiniSEED records in a BUD directory/file structure.  The
 * appropriate directories and files are created if nesecessary.  If
 * files already exist they are appended to.  If both 'basedir' and 'msr'
 * are NULL then ds_shutdown() will be called to close all open files and
 * free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
int
bud_streamproc (const char *basedir, const SLMSrecord *msr, int reclen,
	        int idletimeout)
{
  static DataStream *streamroot = NULL;
  char format[400];

  /* Check if this is a call to shut everything down */
  if (basedir == NULL && msr == NULL)
    {
      sl_log (0, 1, "Shutting down BUD archiving\n");
      ds_streamproc (&streamroot, NULL, NULL, 0, 0, 0);
      return 0;
    }

  snprintf (format, sizeof(format),
	    "%s/%%n/%%s/%%s.%%n.%%l.%%c.%%Y.%%j",
	    basedir);

  return ds_streamproc (&streamroot, format, msr, reclen, SLDATA, idletimeout);
}				/* End of bud_streamproc() */


/***************************************************************************
 * dlog_streamproc():
 * Save MiniSEED records in an old style SeisComP/datalog
 * directory/file structure.  The appropriate directories and files
 * are created if nesecessary.  If files already exist they are
 * appended to.  If both 'basedir' and 'msr' are NULL then
 * ds_shutdown() will be called to close all open files and free all
 * associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
int
dlog_streamproc (const char *basedir, const SLMSrecord *msr, int reclen,
		 int type, int idletimeout)
{
  static DataStream *streamroot = NULL;
  char format[400];

  /* Check if this is a call to shut everything down */
  if (basedir == NULL && msr == NULL)
    {
      sl_log (0, 1, "Shutting down SC/datalog archiving\n");
      ds_streamproc (&streamroot, NULL, NULL, 0, 0, 0);
      return 0;
    }

  if ( ! strncmp(msr->fsdh.location, "  ", 2) )
    { /* No location code */
      snprintf (format, sizeof(format),
		"%s/%%s/%%c.%%t/%%s.%%n.%%c.%%t.%%Y.%%j.#H#M",
		basedir);
    }
  else
    { /* Location code present */
      snprintf (format, sizeof(format),
		"%s/%%s/%%l.%%c.%%t/%%s.%%n.%%c.%%t.%%Y.%%j.#H#M",
		basedir);
    }

  return ds_streamproc (&streamroot, format, msr, reclen, type, idletimeout);
}				/* End of dlog_streamproc() */

