/***************************************************************************
 * dsarchive.c
 * Routines to archive Mini-SEED data records.
 *
 * Written by Chad Trabant, ORFEUS/EC-Project MEREDIAN
 *
 * The philosophy: a "DataStream" describes an archive that miniSEED
 * records will be saved to.  Each archive can be separated into
 * "DataStreamGroup"s, each unique group will be saved into a unique
 * file.  The definition of the groups is implied by the format of the
 * archive.
 *
 * modified: 2004.237
 ***************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <glob.h>

#include "dsarchive.h"

/* Functions internal to this source file */
static DataStreamGroup *ds_getstream (DataStream *datastream, SLMSrecord *msr,
				      int reclen, const char *defkey,
				      char *filename, int nondefflags,
				      const char *globmatch);
static int ds_openfile (DataStream *datastream, const char *filename);
static int ds_closeidle (DataStream *datastream, int idletimeout);
static void ds_shutdown (DataStream *datastream);
static double msr_lastsamptime (SLMSrecord *msr);
static char sl_typecode (int type);

int DS_BUFSIZE = 1000 * 512;

static void
ds_freegroup(DataStreamGroup *group)
{
	free (group->defkey);
	if ( group->buf != NULL )
		free (group->buf);
	free (group);
}

/***************************************************************************
 * ds_streamproc:
 *
 * Save MiniSEED records in a custom directory/file structure.  The
 * appropriate directories and files are created if nesecessary.  If
 * files already exist they are appended to.  If both 'pathformat' and
 * 'msr' are NULL then ds_shutdown() will be called to close all open files
 * and free all associated memory.
 *
 * Returns 0 on success, -1 on error.
 ***************************************************************************/
extern int
ds_streamproc (DataStream *datastream, char *pathformat, SLMSrecord *msr,
	       int reclen)
{
  DataStreamGroup *foundgroup = NULL;
  SLstrlist *fnlist, *fnptr;
  char *tptr;
  char net[3], sta[6], loc[3], chan[4];
  char filename[MAX_FILENAME_LEN];
  char definition[MAX_FILENAME_LEN];
  char globmatch[MAX_FILENAME_LEN];
  int nondefflags = 0;

  /* Special case for stream shutdown */
  if ( pathformat == NULL && msr == NULL )
    {
      ds_shutdown ( datastream );
      return 0;
    }

  /* Build file path and name from pathformat */
  filename[0] = '\0';
  definition[0] = '\0';
  globmatch[0] = '\0';
  sl_strparse (pathformat, "/", &fnlist);
  
  fnptr = fnlist;
  
  /* Count all of the non-defining flags */
  tptr = pathformat;
  while ( (tptr = strchr(tptr, '#')) )
    {
      if ( *(tptr+1) != '#' )
	nondefflags++;
      tptr++;
    }
  
  /* Special case of an absolute path (first entry is empty) */
  if ( *fnptr->element == '\0' )
    {
      if ( fnptr->next != 0 )
	{
	  strcat (filename, "/");
	  strcat (globmatch, "/");
	  fnptr = fnptr->next;
	}
      else
	{
	  sl_log (2, 0, "ds_streamproc(): empty path format\n");
	  sl_strparse (NULL, NULL, &fnlist);
	  return -1;
	}
    }
  
  while ( fnptr != 0 )
    {
      int fnlen = 0;
      int globlen = 0;
      int tdy;
      char *w, *p, def;
      char tstr[20];

      p = fnptr->element;

      /* Special case of no file given */
      if ( *p == '\0' && fnptr->next == 0 )
	{
	  sl_log (2, 0, "ds_streamproc(): no file name specified, only %s\n",
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
	  
	  if ( nondefflags > 0 )
	    {
	      strncat (globmatch, p, (sizeof(globmatch) - globlen));
	      globlen = strlen (globmatch);
	    }
	  
	  w += 1;

	  switch ( *w )
	    {
	    case 't' :
	      snprintf (tstr, sizeof(tstr), "%c", sl_typecode(datastream->packettype));
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "?", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'n' :
	      sl_strncpclean (net, msr->fsdh.network, 2);
	      strncat (filename, net, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, net, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, net, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "*", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 's' :
	      sl_strncpclean (sta, msr->fsdh.station, 5);
	      strncat (filename, sta, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, sta, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, sta, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "*", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'l' :
	      sl_strncpclean (loc, msr->fsdh.location, 2);
	      strncat (filename, loc, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, loc, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, loc, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "*", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'c' :
	      sl_strncpclean (chan, msr->fsdh.channel, 3);
	      strncat (filename, chan, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, chan, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, chan, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "*", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'Y' :
	      snprintf (tstr, sizeof(tstr), "%04d", (int) msr->fsdh.start_time.year);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9][0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
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
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'j' :
	      snprintf (tstr, sizeof(tstr), "%03d", (int) msr->fsdh.start_time.day);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'H' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.hour);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'M' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.min);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'S' :
	      snprintf (tstr, sizeof(tstr), "%02d", (int) msr->fsdh.start_time.sec);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case 'F' :
	      snprintf (tstr, sizeof(tstr), "%04d", (int) msr->fsdh.start_time.fract);
	      strncat (filename, tstr, (sizeof(filename) - fnlen));
	      if ( def ) strncat (definition, tstr, (sizeof(definition) - fnlen));
	      if ( nondefflags > 0 )
		{
		  if ( def ) strncat (globmatch, tstr, (sizeof(globmatch) - globlen));
		  else strncat (globmatch, "[0-9][0-9][0-9][0-9]", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    case '%' :
	      strncat (filename, "%", (sizeof(filename) - fnlen));
	      strncat (globmatch, "%", (sizeof(globmatch) - globlen));
	      fnlen = strlen (filename);
	      globlen = strlen (globmatch);
	      p = w + 1;
	      break;
	    case '#' :
	      strncat (filename, "#", (sizeof(filename) - fnlen));
	      nondefflags--;
	      if ( nondefflags > 0 )
		{
		  strncat (globmatch, "#", (sizeof(globmatch) - globlen));
		  globlen = strlen (globmatch);
		}
	      fnlen = strlen (filename);
	      p = w + 1;
	      break;
	    default :
	      sl_log (2, 0, "unknown file name format code: %c\n", *w);
	      p = w;
	      break;
	    }
	}
      
      strncat (filename, p, (sizeof(filename) - fnlen));
      fnlen = strlen (filename);
      
      if ( nondefflags > 0 )
	{
	  strncat (globmatch, p, (sizeof(globmatch) - globlen));
	  globlen = strlen (globmatch);
	}
      
      /* If not the last entry then it should be a directory */
      if ( fnptr->next != 0 )
	{
	  strncat (filename, "/", (sizeof(filename) - fnlen));
	  fnlen++;
	  
	  if ( nondefflags > 0 )
	    {
	      strncat (globmatch, "/", (sizeof(globmatch) - globlen));
	      globlen++;
	    }
	}

      fnptr = fnptr->next;
    }

  sl_strparse (NULL, NULL, &fnlist);
  
  /* Check for previously used stream entry, otherwise create it */
  foundgroup = ds_getstream (datastream, msr, reclen, definition, filename,
			     nondefflags, globmatch);
  
  if ( foundgroup != NULL )
    {
      /* Initial check (existing files) for future data, a negative
       * last sample time indicates it was derived from an existing
       * file.
       */
      if ( datastream->packettype == SLDATA &&
	   datastream->futureinitflag &&
	   foundgroup->lastsample < 0 )
	{
	  int overlap = (int) ((-1.0 * foundgroup->lastsample) - sl_msr_depochstime(msr));
	  
	  if ( overlap > datastream->futureinit )
	    {
	      if ( foundgroup->futureinitprint )
		{
		  sl_log (2, 0,
			  "%d sec. overlap of existing archive data in %s, skipping\n",
			  overlap, foundgroup->filename);
		  foundgroup->futureinitprint = 0;  /* Suppress further messages */
		}
	      
	      return 0;
	    }
	}
      
      /* Continuous check for future data, a positive last sample time
       * indicates it was derived from the last packet received.
       */
      if ( datastream->packettype == SLDATA &&
	   datastream->futurecontflag &&
	   foundgroup->lastsample > 0 )
	{
	  int overlap = (int) (foundgroup->lastsample - sl_msr_depochstime(msr));
	  
          if ( overlap > datastream->futurecont )
	    {
	      if ( foundgroup->futurecontprint )
		{
		  sl_log (2, 0,
			  "%d sec. overlap of continuous data for %s, skipping\n",
			  overlap, foundgroup->filename);
		  foundgroup->futurecontprint = 0;  /* Suppress further messages */
		}
	      
	      return 0;
	    }
	  else if ( foundgroup->futurecontprint == 0 )
	    foundgroup->futurecontprint = 1;  /* Reset message printing */
	}

      if ((foundgroup->bp + reclen) > DS_BUFSIZE && foundgroup->bp > 0)
        {
          /*  Write the record to the appropriate file */
          sl_log (1, 3, "Writing data to data stream file %s\n", foundgroup->filename);
      
          if ( write (foundgroup->filed, foundgroup->buf, foundgroup->bp) != foundgroup->bp )
	    {
	      sl_log (2, 1,
		      "ds_streamproc: failed to write record\n");
	      return -1;
	    }

          foundgroup->modtime = time (NULL);
	  foundgroup->bp = 0;
	}

      /* Does the record fit into the buffer */
      if ((foundgroup->bp + reclen) <= DS_BUFSIZE)
        {
          memcpy(&foundgroup->buf[foundgroup->bp], msr->msrecord, reclen);
          foundgroup->bp += reclen;

          if ( datastream->packettype == SLDATA &&
               (datastream->futureinitflag || datastream->futurecontflag) )
            foundgroup->lastsample = msr_lastsamptime (msr);

          sl_log (1, 3, "Queueing record for file %s, current buffer filled with %d byte\n",
		          foundgroup->filename, foundgroup->bp);
          return 0;
        }
      else
        {
          /*  Write the record to the appropriate file */
          sl_log (1, 3, "Writing data to data stream file %s\n", foundgroup->filename);

          if ( !write (foundgroup->filed, msr->msrecord, reclen) )
            {
              sl_log (2, 1,
                      "ds_streamproc: failed to write record\n");
              return -1;
            }
          else
            {
              foundgroup->modtime = time (NULL);

              if ( datastream->packettype == SLDATA &&
                   (datastream->futureinitflag || datastream->futurecontflag) )
                foundgroup->lastsample = msr_lastsamptime (msr);
            }
        }
    }
  
  return -1;
}				/* End of ds_streamproc() */


/***************************************************************************
 * ds_getstream:
 *
 * Find the DataStreamGroup entry that matches the definition key, if
 * no matching entries are found allocate a new entry and open the
 * given file.
 *
 * Resource maintenance is performed here: the modification time of
 * each stream, modtime, is compared to the current time.  If the
 * stream entry has been idle for 'DataStream.idletimeout' seconds
 * then the stream will be closed (file closed and memory freed).
 *
 * Returns a pointer to a DataStreamGroup on success or NULL on error.
 ***************************************************************************/
static DataStreamGroup *
ds_getstream (DataStream *datastream, SLMSrecord *msr, int reclen,
	      const char *defkey, char *filename,
	      int nondefflags, const char *globmatch)
{
  DataStreamGroup *foundgroup = NULL;
  char *matchedfilename = 0;
  static int closeidle_counter = 0;
  
  HASH_FIND_STR(datastream->grouphash, defkey, foundgroup);

  if ( foundgroup )
    {
      sl_log (1, 3, "Found data stream entry for key %s\n", defkey);

      /* Keep ds_closeidle from closing this stream */
      if ( foundgroup->modtime > 0 )
          foundgroup->modtime *= -1;
    }

  /* If no matching stream entry was found but the format included
     non-defining flags, try to use globmatch to find a matching file
     and resurrect a stream entry */

  if ( foundgroup == NULL && nondefflags > 0 )
    {
      glob_t pglob;
      int rval;

      sl_log (1, 3, "No stream entry found, searching for: %s\n", globmatch);
      
      rval = glob (globmatch, 0, NULL, &pglob);

      if ( rval && rval != GLOB_NOMATCH )
	{
	  switch (rval)
	    {
	    case GLOB_ABORTED : sl_log (2, 1, "glob(): Unignored lower-level error\n");
	    case GLOB_NOSPACE : sl_log (2, 1, "glob(): Not enough memory\n");    
	    case GLOB_NOSYS : sl_log (2, 1, "glob(): Function not supported\n");
	    default : sl_log (2, 1, "glob(): %d\n", rval);
	    }
	}
      else if ( rval == 0 && pglob.gl_pathc > 0 )
	{
	  if ( pglob.gl_pathc > 1 )
	    sl_log (1, 3, "Found %d files matching %s, using last match\n",
		    pglob.gl_pathc, globmatch);

	  matchedfilename = pglob.gl_pathv[pglob.gl_pathc-1];
	  sl_log (1, 2, "Found matching file for non-defining flags: %s\n", matchedfilename);
	  
	  /* Now that we have a match use it instead of filename */
	  filename = matchedfilename;
	}
      
      globfree (&pglob);
    }
  
  /* If not found, create a stream entry */
  if ( foundgroup == NULL )
    {
      time_t curtime = time (NULL);

      if ( matchedfilename )
	sl_log (1, 2, "Resurrecting data stream entry for key %s\n", defkey);
      else
	sl_log (1, 2, "Creating data stream entry for key %s\n", defkey);
      
      foundgroup = (DataStreamGroup *) malloc (sizeof (DataStreamGroup));
      if ( DS_BUFSIZE > 0 )
          foundgroup->buf = malloc(sizeof(char)*DS_BUFSIZE);
      else
          foundgroup->buf = NULL;
      foundgroup->defkey = strdup (defkey);
      foundgroup->filed = 0;
      foundgroup->modtime = -curtime;
      foundgroup->lastsample = 0.0;
      foundgroup->futurecontprint = datastream->futurecontflag;
      foundgroup->futureinitprint = datastream->futureinitflag;
      strncpy (foundgroup->filename, filename, sizeof(foundgroup->filename));
      foundgroup->bp = 0;

      HASH_ADD_KEYPTR(hh, datastream->grouphash, foundgroup->defkey, strlen(foundgroup->defkey), foundgroup);
    }
  
  /* Close idle stream files */
  closeidle_counter = ( closeidle_counter + 1 ) % 100;
  if ( ! closeidle_counter )
      ds_closeidle (datastream, datastream->idletimeout);
  
  /* If no file is open, well, open it */
  if ( foundgroup->filed == 0 )
    {
      int filepos;
      
      sl_log (1, 2, "Opening data stream file %s\n", filename);
      
      if ( (foundgroup->filed = ds_openfile (datastream, filename)) == -1 )
	{
	  sl_log (2, 0, "cannot open data stream file %s, %s\n", filename, strerror (errno));
	  foundgroup->filed = 0;
	  return NULL;
	}
      
      if ( (filepos = (int) lseek (foundgroup->filed, (off_t) 0, SEEK_END)) < 0 )
	{
	  sl_log (2, 0, "cannot seek in data stream file %s, %s\n", filename, strerror (errno));
	  return NULL;
	}
      
      /* Initial future data check (existing files) needs the last
       * sample time from the last record.  Only read the last record
       * if this stream has not been used and there is at least one
       * record to read.
       */
      if ( datastream->packettype == SLDATA &&
	   datastream->futureinitflag  &&
	   !foundgroup->lastsample )
	{
	  if ( filepos >= reclen )
	    {
	      SLMSrecord *lmsr = NULL;
	      char *lrecord;
	      
	      sl_log (1, 2, "Reading last record in existing file\n");

	      lrecord = (char *) malloc (reclen);
	      
	      if ( (lseek (foundgroup->filed, (off_t) (reclen * -1), SEEK_END)) < 0 )
		{
		  sl_log (2, 0, "cannot seek in data stream file, %s\n", strerror (errno));
		  free (lrecord);
		  return NULL;
		}
	      
	      if ( (read (foundgroup->filed, lrecord, reclen)) != reclen )
		{
		  sl_log(2, 0, "cannot read the last record of stream file\n");
		  free (lrecord);
		  return NULL;
		}
	      
	      if ( sl_msr_parse (NULL, lrecord, &lmsr, 0, 0) != NULL )
		{
		  /* A negative last sample time means it came from an existing file */
		  foundgroup->lastsample = (-1 * msr_lastsamptime (lmsr));
		}
	      else
		{
		  /* Zero means last sample time is unknown, disabling checks */
		  foundgroup->lastsample = 0.0;
		}

	      sl_msr_free (&lmsr);
	      free (lrecord);
	    }
	}
    }
  
  /* There used to be a further check here, but it shouldn't be reached, just in
     case this is left for the moment until I'm convinced. */
  else if ( strcmp (defkey, foundgroup->defkey) )
    sl_log (2, 0, "Arg! open file for a key that no longer matches\n");
  
  if ( foundgroup->modtime < 0 )
    {
      foundgroup->modtime *= -1;
    }

  return foundgroup;
}				/* End of ds_getstream() */


/***************************************************************************
 * ds_openfile:
 *
 * Open a specified file, if the open file limit has been reach try
 * once to increase the limit, if that fails or has already been done
 * start closing idle files with decreasing idle timeouts until a file
 * can be opened.
 *
 * Return the result of open(2), normally this a the file descriptor
 * on success and -1 on error.
 ***************************************************************************/
static int
ds_openfile (DataStream *datastream, const char *filename)
{
  static char rlimit = 0;
  struct rlimit rlim;
  int idletimeout = datastream->idletimeout;
  int oret = 0;
  
  SLstrlist *fnlist, *fnptr;
  char dirname[MAX_FILENAME_LEN+1];
  sl_strparse (filename, "/", &fnlist);
  
  fnptr = fnlist;
  dirname[0] = '\0';
  
  /* Special case of an absolute path (first entry is empty) */
  if ( *fnptr->element == '\0' )
    {
      strcat (dirname, "/");
      fnptr = fnptr->next;
    }
  
  while ( fnptr != 0 )
    {
      strncat (dirname, fnptr->element, (sizeof(dirname) - strlen(dirname)));

      /* If not the last entry then it should be a directory */
      if ( fnptr->next != 0 )
	{
	  if ( access (dirname, F_OK) )
	    {
	      if ( errno == ENOENT )
		{
		  sl_log (1, 1, "Creating directory: %s\n", dirname);
		  if (mkdir
		      (dirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		    {
		      sl_log (2, 1, "ds_streamproc: mkdir(%s) %s\n", dirname,
			      strerror (errno));
		      sl_strparse (NULL, NULL, &fnlist);
		      return -1;
		    }
		}
	      else
		{
		  sl_log (2, 0, "%s: access denied, %s\n", dirname, strerror(errno));
		  sl_strparse (NULL, NULL, &fnlist);
		  return -1;
		}
	    }
	  
	  strncat (dirname, "/", (sizeof(dirname) - strlen(dirname)));
	}

      fnptr = fnptr->next;
    }

  sl_strparse (NULL, NULL, &fnlist);
  
  if ( (oret = open (filename, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1 )
    {
      
      /* Check if max number of files open */
      if ( errno == EMFILE && rlimit == 0 )
	{
	  rlimit = 1;
	  sl_log (1, 1, "Too many open files, trying to increase limit\n");
	  
	  /* Set the soft open file limit to the hard open file limit */
	  if ( getrlimit (RLIMIT_NOFILE, &rlim) == -1 )
	    {
	      sl_log (2, 0, "getrlimit failed to get open file limit\n");
	    }
	  else
	    {
	      rlim.rlim_cur = rlim.rlim_max;
	      
	      if ( rlim.rlim_cur == RLIM_INFINITY )
		sl_log (1, 3, "Setting open file limit to 'infinity'\n");
	      else
		sl_log (1, 3, "Setting open file limit to %d\n", rlim.rlim_cur);
	      
	      if ( setrlimit (RLIMIT_NOFILE, &rlim) == -1 )
		{
		  sl_log (2, 0, "setrlimit failed to set open file limit\n");
		}
	      else
		{
		  /* Try to open the file again */
		  if ( (oret = open (filename, O_RDWR | O_CREAT | O_APPEND, 0644)) != -1 )
		    return oret;
		}
	    }
	}
      
      if ( errno == EMFILE || errno == ENFILE )
	{
	  sl_log (1, 2, "Too many open files, closing idle stream files\n");
	  
	  /* Close idle streams until we have free descriptors */
	  while ( ds_closeidle (datastream, idletimeout) == 0 && idletimeout >= 0 )
	    {
	      idletimeout = (idletimeout / 2) - 1;
	    }
	  
	  /* Try to open the file again */
	  if ( (oret = open (filename, O_RDWR | O_CREAT | O_APPEND, 0644)) != -1 )
	    return oret;
	}
    }
  
  return oret;
}				/* End of ds_openfile() */


/***************************************************************************
 * ds_closeidle:
 *
 * Close all stream files that have not been active for the specified
 * idletimeout.
 *
 * Return the number of files closed.
 ***************************************************************************/
static int
ds_closeidle (DataStream *datastream, int idletimeout)
{
  DataStreamGroup *curgroup, *tmp;
  int count = 0;
  time_t curtime;

  curtime = time (NULL);

  /* Traverse the stream chain */
  HASH_ITER(hh, datastream->grouphash, curgroup, tmp)
    {
      if ( curgroup->modtime > 0 && (curtime - curgroup->modtime) > idletimeout )
	{
          if ( curgroup->bp )
	    {
              sl_log (1, 3, "Writing data to data stream file %s\n", curgroup->filename);

	      if ( write (curgroup->filed, curgroup->buf, curgroup->bp) != curgroup->bp )
	        {
	          sl_log (2, 1,
		          "ds_closeidle: failed to write record\n");
	        }

	      curgroup->bp = 0;
	    }

	  sl_log (1, 2, "Closing idle stream with key %s\n", curgroup->defkey);
	  
	  /* Close the associated file */
	  if ( close (curgroup->filed) )
	    sl_log (2, 0, "ds_closeidle(), closing data stream file, %s\n",
		    strerror (errno));
	  else
	    count++;

          HASH_DELETE(hh, datastream->grouphash, curgroup);
          
	  ds_freegroup (curgroup);
	}
    }
  
  return count;
}				/* End of ds_closeidle() */


/***************************************************************************
 * ds_shutdown:
 *
 * Close all stream files and release all of the DataStreamGroup memory
 * structures.
 ***************************************************************************/
static void
ds_shutdown (DataStream *datastream)
{
  DataStreamGroup *curgroup, *tmp;

  HASH_ITER(hh, datastream->grouphash, curgroup, tmp)
    {
      if ( curgroup->bp )
        {
          sl_log (1, 3, "Writing data to data stream file %s\n", curgroup->filename);
      
          if ( write (curgroup->filed, curgroup->buf, curgroup->bp) != curgroup->bp )
            {
              sl_log (2, 1,
                      "ds_shutdown: failed to write record\n");
            }

          curgroup->bp = 0;
        }

      sl_log (1, 3, "Shutting down stream with key: %s\n", curgroup->defkey);

      if ( close (curgroup->filed) )
	sl_log (2, 0, "ds_shutdown(), closing data stream file, %s\n",
		strerror (errno));
      
      HASH_DELETE(hh, datastream->grouphash, curgroup);
      
      ds_freegroup (curgroup);
    }
}				/* End of ds_shutdown() */


/***************************************************************************
 * msr_lastsamptime:
 *
 * Calculate the time of the last sample in the record; this is the actual
 * last sample time and *not* the time "covered" by the last sample.
 *
 * Returns the time of the last sample as a double precision epoch time.
 ***************************************************************************/
double
msr_lastsamptime (SLMSrecord *msr)
{
  double startepoch;
  double dsamprate;
  double span = 0.0;
  
  sl_msr_dsamprate (msr, &dsamprate);
  
  if ( dsamprate )
    span = (double) (msr->fsdh.num_samples - 1 ) * (1.0 / dsamprate);
  
  startepoch = sl_msr_depochstime(msr);
  
  return (startepoch + span);
}				/* End of msr_lastsamptime() */


/***************************************************************************
 * sl_typecode:
 * Look up the one character code that corresponds to the packet type.
 *
 * Returns the type character on success and '?' if no matching type.
 ***************************************************************************/
char
sl_typecode (int packettype)
{
  switch (packettype)
    {
    case SLDATA:
      return 'D';
    case SLDET:
      return 'E';
    case SLCAL:
      return 'C';
    case SLTIM:
      return 'T';
    case SLMSG:
      return 'L';
    case SLBLK:
      return 'O';
    case SLCHA:
      return 'U';
    case SLINF:
    case SLINFT:
    case SLKEEP:
      return 'I';

    default:
      return '?';
    }
}
