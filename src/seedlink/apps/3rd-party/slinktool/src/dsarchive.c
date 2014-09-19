/***************************************************************************
 * Routines to archive Mini-SEED data records.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * modified: 2006.355
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dsarchive.h"

/* Functions internal to this source file */
static DataStream *ds_getstream (DataStream **dstream, const SLMSrecord *msr,
				 const char *defkey, const char *filename,
				 int type, int idletimeout);
static void ds_shutdown (DataStream * dstream);
static char sl_typecode (int type);


/***************************************************************************
 * ds_streamproc(): 
 * Save MiniSEED records in a custom directory/file structure.  The
 * appropriate directories and files are created if nesecessary.  If
 * files already exist they are appended to.  If both 'pathformat' and
 * 'msr' are NULL then ds_shutdown() will be called to close all open files
 * and free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
extern int
ds_streamproc (DataStream **streamroot, char *pathformat, const SLMSrecord *msr,
	       int reclen, int type, int idletimeout)
{
  DataStream *foundstream = NULL;
  SLstrlist    *fnlist, *fnptr;
  char net[3], sta[6], loc[3], chan[4];
  char filename[400];
  char definition[400];

  /* Special case for stream shutdown */
  if ( pathformat == NULL && msr == NULL )
    {
      ds_shutdown ( *streamroot );
      return 0;
    }

  /* Build file path and name from pathformat */
  filename[0] = '\0';
  definition[0] = '\0';
  sl_strparse (pathformat, "/", &fnlist);

  fnptr = fnlist;

  /* Special case of an absolute path (first entry is empty) */
  if ( *fnptr->element == '\0' )
    {
      if ( fnptr->next != 0 )
	{
	  strncat (filename, "/", sizeof(filename));
	  fnptr = fnptr->next;
	}
      else
	{
	  sl_log (1, 0, "ds_streamproc(): empty path format\n");
	  sl_strparse (NULL, NULL, &fnlist);
	  return -1;
	}
    }

  while ( fnptr != 0 )
    {
      int tdy;
      int fnlen = 0;
      char *w, *p, def;
      char tstr[20];

      p = fnptr->element;

      /* Special case of no file given */
      if ( *p == '\0' && fnptr->next == 0 )
	{
	  sl_log (1, 0, "ds_streamproc(): no file name specified, only %s\n",
		  filename);
	  sl_strparse (NULL, NULL, &fnlist);
	  return -1;
	}

      while ( (w = strpbrk (p, "%#")) != NULL )
	{
	  def = ( *w == '%' );
	  *w = '\0';
	  strncat (filename, p, (sizeof(filename) - fnlen));
	  fnlen = strlen (filename);

	  w += 1;

	  switch ( *w )
	    {
	    case 't' :
	      snprintf (tstr, sizeof(tstr), "%c", sl_typecode(type));
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'n' :
	      sl_strncpclean (net, msr->fsdh.network, 2);
	      strncat (filename, net, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, net, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 's' :
	      sl_strncpclean (sta, msr->fsdh.station, 5);
	      strncat (filename, sta, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, sta, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'l' :
	      sl_strncpclean (loc, msr->fsdh.location, 2);
	      strncat (filename, loc, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, loc, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'c' :
	      sl_strncpclean (chan, msr->fsdh.channel, 3);
	      strncat (filename, chan, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, chan, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'Y' :
	      snprintf (tstr, sizeof(tstr), "%04d", (int) msr->fsdh.start_time.year);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'y' :
	      tdy = (int) msr->fsdh.start_time.year;
	      while ( tdy > 100 )
		{
		  tdy -= 100;
		}
	      snprintf (tstr, sizeof(tstr), "%02d", tdy);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'j' :
	      snprintf (tstr, sizeof(tstr), "%03d", (int) msr->fsdh.start_time.day);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'H' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.hour);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'M' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.min);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'S' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.sec);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'F' :
	      snprintf (tstr, sizeof(tstr), "%04d", (int) msr->fsdh.start_time.fract);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case '%' :
	      strncat (filename, "%", (sizeof(filename) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case '#' :
	      strncat (filename, "#", (sizeof(filename) - fnlen));
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    default :
	      sl_log (1, 0, "Unknown file name format code: %c\n", *w);
	      p = w;
	      break;
	    }
	}
      
      strncat (filename, p, (sizeof(filename) - fnlen));
      fnlen = strlen (filename);

      /* If not the last entry then it should be a directory */
      if ( fnptr->next != 0 )
	{
	  if ( access (filename, F_OK) )
	    {
	      if ( errno == ENOENT )
		{
		  sl_log (0, 1, "Creating directory: %s\n", filename);
#if defined(SLP_WIN32)
		  if (mkdir (filename))
		    {
		      sl_log (0, 1, "ds_streamproc: mkdir(%s) %s\n", filename,
			      strerror (errno));
		      sl_strparse (NULL, NULL, &fnlist);
		      return -1;
		    }
#else
		  if (mkdir
		      (filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		    {
		      sl_log (0, 1, "ds_streamproc: mkdir(%s) %s\n", filename,
			      strerror (errno));
		      sl_strparse (NULL, NULL, &fnlist);
		      return -1;
		    }
#endif
		}
	      else
		{
		  sl_log (1, 0, "%d: access denied, %s\n", filename, strerror(errno));
		  sl_strparse (NULL, NULL, &fnlist);
		  return -1;
		}
	    }

	  strncat (filename, "/", (sizeof(filename) - fnlen));
	  fnlen++;
	}

      fnptr = fnptr->next;
    }

  sl_strparse (NULL, NULL, &fnlist);

  /* Check for previously used stream entry, otherwise create it */
  foundstream = ds_getstream (streamroot, msr, definition, filename,
			      type, idletimeout);

  if (foundstream != NULL)
    {
      /*  Write the record to the appropriate file */
      if ( !fwrite (msr->msrecord, reclen, 1, foundstream->filep) )
	{
	  sl_log (0, 1,
		  "ds_streamproc: failed to write record\n");
	  return -1;
	}
      else
	{
	  foundstream->modtime = time (NULL);
	}
      return 0;
    }

  return -1;
}				/* End of ds_streamproc() */


/***************************************************************************
 * ds_getstream():
 * Find the DataStream entry that matches the definition key, if no matching
 * entries are found allocate a new entry and open the given file.
 *
 * The modification time of each stream, modtime, is compared to the current
 * time.  If the stream entry has been idle for 'idletimeout' seconds
 * (default 120) then the stream will be closed.  This will keep us from 
 * having many "hanging" open files.
 *
 * Returns a pointer to DataStream on success or NULL on error.
 ***************************************************************************/
DataStream *
ds_getstream (DataStream **streamroot, const SLMSrecord *msr,
	      const char *defkey, const char *filename, int type,
	      int idletimeout)
{
  DataStream *foundstream = NULL;
  DataStream *searchstream = NULL;
  DataStream *prevstream = NULL;
  time_t curtime;

  searchstream = *streamroot;
  curtime = time (NULL);

  /* Traverse the stream chain */
  while (searchstream != NULL)
    {
      DataStream *nextstream  = (struct DataStream_s *) searchstream->next;

      if ( foundstream == NULL && !strcmp (searchstream->defkey, defkey) )
	{
	  sl_log (0, 3, "Found data stream entry for key %s\n", defkey);

	  prevstream = searchstream;
	  foundstream = searchstream;
	}
      else if ( (curtime - searchstream->modtime) > idletimeout )
	{
	  sl_log (0, 2, "Closing stream with key %s\n", searchstream->defkey);

	  /* Re-link the stream chain */
	  if ( prevstream != NULL )
	    {
	      if ( searchstream->next != NULL )
		prevstream->next = searchstream->next;
	      else
		prevstream->next = NULL;
	    }
	  else
	    {
	      if ( searchstream->next != NULL )
		*streamroot = searchstream->next;
	      else
		*streamroot = NULL;
	    }

	  if (fclose (searchstream->filep))
	    sl_log (1, 0, "ds_getstream(), closing data stream file, %s\n",
		    strerror (errno));

	  free (searchstream->defkey); 
	  free (searchstream);
	}
      else
	{
	  prevstream = searchstream;
	}

      searchstream = nextstream;
    }

  /* If not found, create a stream entry */
  if ( foundstream == NULL )
    {
      sl_log (0, 2, "Creating data stream entry for key %s\n", defkey);

      foundstream = (DataStream *) malloc (sizeof (DataStream));

      foundstream->defkey = strdup (defkey);
      foundstream->filep = NULL;
      foundstream->modtime = curtime;
      foundstream->next = NULL;

      /* Set the stream root if this is the first entry */
      if (*streamroot == NULL)
	{
	  *streamroot = (struct DataStream_s *) foundstream;
	}
      else if (prevstream != NULL)
	{
	  prevstream->next = (struct DataStream_s *) foundstream;
	}
      else
	{
	  sl_log (1, 0, "stream chain is broken!\n");
	  return NULL;
	}
    }

  /* If no file is open, well, open it */
  if ( foundstream->filep == NULL )
    {
      sl_log (0, 2, "Creating new data stream file\n");

      if ((foundstream->filep = fopen (filename, "ab")) == NULL)
	{
	  sl_log (1, 0, "opening new data stream file, %s\n", strerror (errno));
	  return NULL;
	}

      setvbuf(foundstream->filep, NULL, _IONBF, 0);
    }

  /* If a file is open check that the definition key still matches,
     if not close the previous file and open a new one */
  else if ( strcmp (defkey, foundstream->defkey) )
    {
      sl_log (0, 2, "Opening new data stream file\n");

      if (fclose (foundstream->filep))
	{
	  sl_log (1, 0, "closing data stream file, %s\n", strerror (errno));
	  return NULL;
	}

      if ((foundstream->filep = fopen (filename, "ab")) == NULL)
	{
	  sl_log (1, 0, "opening data stream file, %s\n", strerror (errno));
	  return NULL;
	}

      setvbuf(foundstream->filep, NULL, _IONBF, 0);
    }
  
  return foundstream;
}				/* End of ds_getstream() */


/***************************************************************************
 * ds_shutdown():
 * Close all stream files and release all of the DataStream memory
 * structures.
 ***************************************************************************/
void
ds_shutdown (DataStream *streamroot)
{
  DataStream *curstream = NULL;
  DataStream *prevstream = NULL;

  curstream = streamroot;

  if (curstream != NULL)
    sl_log (0, 2, "Flushing and closing data stream structures\n");

  while (curstream != NULL)
    {
      prevstream = curstream;
      curstream = (struct DataStream_s *) curstream->next;

      sl_log (0,3, "Shutting down stream with key: %s\n", prevstream->defkey);

      if (fclose (prevstream->filep))
	sl_log (1, 0, "ds_shutdown(), closing data stream file, %s\n",
		strerror (errno));

      free (prevstream->defkey);
      free (prevstream);
    }
}				/* End of ds_shutdown() */


/***************************************************************************
 * sl_typecode():
 * Look up the one character code that corresponds to the packet type.
 *
 * Returns the type character on success and '?' if no matching type.
 ***************************************************************************/
char
sl_typecode (int type)
{
  switch (type)
    {
    case 0:
      return 'D';
    case 1:
      return 'E';
    case 2:
      return 'C';
    case 3:
      return 'T';
    case 4:
      return 'L';
    case 5:
      return 'O';
    case 6:
      return 'U';
    case 7:
    case 8:
    case 9:
      return 'I';

    default:
      return '?';
    }
}
