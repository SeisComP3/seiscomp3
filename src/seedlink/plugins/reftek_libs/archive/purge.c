#pragma ident "$Id: purge.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
	Program  :	Any
	Task     :	Archive Library API functions.
	File     :	purge.c
	Purpose  :	Purge functions.
	Host     :	CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
	Target   :	Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
	Author	:	Robert Banfill (r.banfill@reftek.com)
	Company  :	Refraction Technology, Inc.
					2626 Lombardy Lane, Suite 105
					Dallas, Texas  75220  USA
					(214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
	Copyright:	(c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
	Notes    :

-----------------------------------------------------------------------------
   $Source$
   $Revision: 165 $
	$Date: 2005-12-23 13:34:58 +0100 (Fri, 23 Dec 2005) $

-----------------------------------------------------------------------------
	Revised  :
		05Oct05	(pld) remove unused code in RemoveEventFile()
		16Dec04	(pld) tweak various log msgs
		15Dec04	(pld) more work on maintaining proper archive stats during purge
		17Aug98  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#define _PURGE_C
#include "archive.h"

/* Local structure types */
typedef struct yd_node 
	{
	REAL64 yd;
	int year;
	int day;
	struct yd_node *next;
	} YD_NODE;

/* Local prototypes ---------------------------------------------------*/
static BOOL		Purge(H_ARCHIVE harchive);
static UINT64	DeleteOldestEvent(H_ARCHIVE harchive, EVENT *event, STREAM *criteria,STREAM **stream);
static BOOL		RemoveEventFile(CHAR *filespec);
static BOOL		PathEmpty(CHAR *path);

static YD_NODE	*GetYDList(H_ARCHIVE harchive,  STREAM *criteria);
static VOID		DeleteYDList(YD_NODE *cur);

/*======================================================================
----------------------------------------------------------------------*/
VOID InitPurge(PURGE *purge)
   {
   ASSERT(purge != NULL);

   purge->active = FALSE;
   purge->stop = FALSE;
   MUTEX_INIT(&purge->mutex);
   SEM_INIT(&purge->semaphore, 0, 1);
   purge->thread_id = 0;

   return ;
   }  /* end InitPurge() */

/*======================================================================
----------------------------------------------------------------------*/
VOID DestroyPurge(PURGE *purge)
   {
   ASSERT(purge != NULL);

   MUTEX_DESTROY(&purge->mutex);
   SEM_DESTROY(&purge->semaphore);

   return ;
   }  /* end DestroyPurge() */

/*======================================================================
----------------------------------------------------------------------*/
BOOL PurgeOldestEvent(H_ARCHIVE harchive)
   {
   ARCHIVE *archive;

   if (!ValidateHandle(harchive))
      {
      ArchiveLog(ARC_LOG_ERRORS, "PurgeOldestEvent: ValidateHandle failed");
      return FALSE;
      }

   archive = &_archive[harchive];

   MUTEX_LOCK(&archive->purge.mutex);
   if (archive->purge.active)
      {
      MUTEX_UNLOCK(&archive->purge.mutex);
      SEM_POST(&archive->purge.semaphore);
      }
   else
      MUTEX_UNLOCK(&archive->purge.mutex);

   return TRUE;
   }  /* end PurgeOldestEvent() */

/*======================================================================
   Purpose:	Purge thread main loop.
   Returns:	
   Notes:	
   Revised:
		15Dec04	(pld) use SEM_TRYWAIT() to clear semiphore after purge to
						prevent purging more than needed (several SEM_POST()
						calls while doing purge)
----------------------------------------------------------------------*/
THREAD_FUNC PurgeThread(VOID *argument)
   {
   PURGE *purge;

   ASSERT(argument != NULL);

   purge = (PURGE *)argument;

   MUTEX_LOCK(&purge->mutex);
      purge->active = TRUE;
      purge->stop = FALSE;
   MUTEX_UNLOCK(&purge->mutex);

   ArchiveLog(ARC_LOG_VERBOSE, "PurgeThread start, tid: %u, pid %u", THREAD_SELF(), getpid());

   while (TRUE)
      {
      SEM_WAIT(&purge->semaphore);
      if (purge->stop)
         break;

      ArchiveLog(ARC_LOG_VERBOSE, "Archive: purge initiated");

      if (!Purge(purge->handle))
         break;

		while (SEM_TRYWAIT(&purge->semaphore) == 0)
			;
      }

   ArchiveLog(ARC_LOG_VERBOSE, "PurgeThread exit");
   MUTEX_LOCK(&purge->mutex);
      purge->active = FALSE;
   MUTEX_UNLOCK(&purge->mutex);
   THREAD_EXIT((VOID*)0);
   }  /* end PurgeThread() */

/*======================================================================
   Purpose:	Perform the purge
   Returns:	TRUE	something purged
				FALSE	nothing purged
   Notes:	
   Revised:
		15Dec04	(pld) move archive size adjustment into DeleteOldestEvent()
----------------------------------------------------------------------*/
static BOOL Purge(H_ARCHIVE harchive)
   {
   VOID		*ptr;
   CHAR		string[32];
   REAL64	earliest;
   STREAM	criteria;
   STREAM	*stream;
   EVENT		event;
   UINT64	bytes_removed;
   
   if (!ValidateHandle(harchive))
      {
      ArchiveLog(ARC_LOG_ERRORS, "Purge: ValidateHandle failed");
      return (FALSE);
      }

   _archive[harchive].last_error = ARC_NO_ERROR;
   _archive_error = ARC_NO_ERROR;

   /* Try and delete a file to make room for another packet */
   /* Note: if no file found, bytes_removed will be 0 */
   InitStream(&criteria);
	if (bytes_removed = DeleteOldestEvent(harchive, &event, &criteria, &stream),
			bytes_removed == 0)
		{
      ArchiveLog(ARC_LOG_ERRORS, "Purge: DeleteOldestEvent failed");
      return (FALSE);
		}

   /* Find the *new* earliest event for this stream */
   InitStream(&criteria);
   criteria.unit = event.unit;
   criteria.stream = event.stream;
   InitEvent(&event);
   if (!ArchiveFindFirstEvent(harchive, &criteria, &event))
      {
      MUTEX_LOCK(&_archive[harchive].mutex);
      stream->time.earliest = VOID_TIME;
      stream->time.latest = VOID_TIME;
      }
   /* This is now the earliest time in the stream record */
   else
      {
      MUTEX_LOCK(&_archive[harchive].mutex);
      stream->time.earliest = event.time.earliest;
      ArchiveLog(ARC_LOG_MAXIMUM, "Purge: stream earliest data: %s", FormatMSTime(string, stream->time.earliest, 10));
      }

   /* Find new archive earliest time */
   earliest = VOID_TIME;
   if ((stream = ArchiveFirstStream(harchive, &ptr)) != NULL)
      {
      do
         {
         if (UndefinedTime(earliest) || stream->time.earliest < earliest)
            earliest = stream->time.earliest;
         } while ((stream = ArchiveNextStream(&ptr)) != NULL);
      }
   if (UndefinedTime(earliest))
      {
      _archive[harchive].state.time.earliest = VOID_TIME;
      _archive[harchive].state.time.latest = VOID_TIME;
      }
   else
      {
      _archive[harchive].state.time.earliest = earliest;
      ArchiveLog(ARC_LOG_MAXIMUM, "Purge: archive earliest data: %s", FormatMSTime(string, _archive[harchive].state.time.earliest, 10));
      }
   MUTEX_UNLOCK(&_archive[harchive].mutex);

   return (TRUE);
   }  /* end Purge() */

/*======================================================================
   Purpose:	Find and delete the oldest file.
   Returns:	Size in bytes of deleted file.
   Notes:	
   Notes:	Try and find the oldest file and delete it. If not deleted
				try next oldest.
   Revised:
		15Dec04	(pld) move archive size adjustment here from Purge()
----------------------------------------------------------------------*/
static UINT64 DeleteOldestEvent(H_ARCHIVE harchive, EVENT *event, STREAM *criteria, STREAM **stream)
   {
   EVENT		info;
   STREAM	day_criteria;
   YD_NODE	*cur_dir;
   YD_NODE	*dir_list;
   INT16		max_tries_n_no_file_removed = 7;
   UINT64	bytes_removed = 0;
   char		filename[MAX_PATH_LEN + 1];
   CHAR		string[32];
  
   ASSERT(event != NULL);

	if (!ValidateHandle(harchive))
      {
      ArchiveLog(ARC_LOG_ERRORS, "DeleteOldestEvent: ValidateHandle failed");
      return (FALSE);
      }

   /* Lets get an ordered list of YYYYDDD directories to search thru */
   dir_list = GetYDList(harchive,criteria);
   cur_dir = dir_list;
   
   /* Now lets proceed thru the YYYYDDD directory list & try and
		find a file to delete */
	while ((max_tries_n_no_file_removed > 0) && cur_dir)
		{
		max_tries_n_no_file_removed--;
		
		/* Set criteria time range to cover this day */
		InitStream(&day_criteria);
    	day_criteria.time.earliest = EncodeMSTimeDOY(cur_dir->year, cur_dir->day, 0, 0, 0.0);
    	day_criteria.time.latest = day_criteria.time.earliest + DAY;
   	ArchiveLog(ARC_LOG_MAXIMUM, "DeleteOldestEvent: Look in Directory %4d%3d",cur_dir->year,cur_dir->day);   	 	
    	
		do 
			{
    		/* Find oldest event file in this day */
    		InitEvent(&info);
   		InitEvent(event);
   		filename[0] = '\0';
    		if (ArchiveFindFirstEvent(harchive, &day_criteria, &info)) 
    			{
    			memcpy(filename,info.filespec,sizeof(filename));

        		do 
        			{
            	if (event->unit == VOID_UNIT || info.time.earliest < event->time.earliest)
            		memcpy(event, &info, sizeof(STREAM));
					} while (ArchiveFindNextEvent(harchive, &day_criteria, &info) &&
								strncmp(filename,info.filespec,MAX_PATH_LEN));
				}
			
			if (event->unit == VOID_UNIT)
				{
				break; /* try next days directory */
				}
				
   		ArchiveLog(ARC_LOG_VERBOSE, "Archive: Purge event: %s", event->filespec);
 
	  		/* Find the stream record */
   		MUTEX_LOCK(&_archive[harchive].mutex);
   		if ((*stream = LookupStream(harchive, event->unit, event->stream)) == NULL)
   	   	{
   	   	ArchiveLog(ARC_LOG_ERRORS, "DeleteOldestEvent: LookupStream failed");
   			MUTEX_UNLOCK(&_archive[harchive].mutex);
   			day_criteria.time.earliest += DAY/24;
   	   	ArchiveLog(ARC_LOG_VERBOSE, "DeleteOldestEvent: Look for next earliest");   	 	
   	   	continue; /* try & find next earliest in directory */
   	   	}
  			MUTEX_UNLOCK(&_archive[harchive].mutex);
   	
   		/* Try to delete the file */
   		bytes_removed = 0;
   		if (RemoveEventFile(event->filespec))
   	   	{
   	   	bytes_removed = event->bytes;
   	   	break;
   	   	}
   		else	/* if we were unable to remove this old event,*/
   	 		/* try and delete the next oldest event */
   	 		{
   	 		day_criteria.time.earliest = event->time.latest + (DAY/24);
   	   	ArchiveLog(ARC_LOG_VERBOSE, "DeleteOldestEvent: Look for next earliest");   
		   	continue;	 	
  	 			}
  	 		} while(event->unit != VOID_UNIT);/* end do while files in days directory */

  	 	if (bytes_removed)
  	 		{
   		/* Adjust size based on data actually removed*/
  			MUTEX_LOCK(&_archive[harchive].mutex);
   		if ((*stream)->bytes > bytes_removed)
      		(*stream)->bytes -= bytes_removed;
   		else
      		(*stream)->bytes = 0;

   		if (_archive[harchive].state.bytes > bytes_removed)
      		_archive[harchive].state.bytes -= bytes_removed;
   		else
      		_archive[harchive].state.bytes = 0;
      
   		ArchiveLog(ARC_LOG_MAXIMUM, "DeleteOldestEvent: stream %04X:%u size: %s bytes",
				event->unit, event->stream, FormatAsSIBinaryUnit(string, (*stream)->bytes, TRUE));
   		ArchiveLog(ARC_LOG_MAXIMUM, "DeleteOldestEvent: Archive size: %s bytes",
				FormatAsSIBinaryUnit(string, _archive[harchive].state.bytes, TRUE));
  			MUTEX_UNLOCK(&_archive[harchive].mutex);

  	 		break;								/* if we deleted a file we are done */
  	 		}  /* end if (bytes_removed) */

  	 	cur_dir = cur_dir->next;
   	}  /* end while no file removed */

	DeleteYDList(dir_list);
	return(bytes_removed);   
   }  /* end DeleteOldestEvent() */

/*======================================================================
----------------------------------------------------------------------*/
static YD_NODE *GetYDList(H_ARCHIVE harchive,  STREAM *criteria)
	{
	EVENT info;
   YD_NODE *head = NULL;
   YD_NODE *tail = NULL;
   YD_NODE *cur,*new_node;
   int	year;
   int	day;

   InitEvent(&info);

   if (!ValidateHandle(harchive))
      {
      ArchiveLog(ARC_LOG_ERRORS, "GetYDList: ValidateHandle failed");
      return (NULL);
      }
   
	/* Look at all day directories and store them */
	sprintf(info.filespec, "%s%c*", _archive[harchive].path, PATH_DELIMITER);
	info.attribute = FIND_SUBDIR;
	if (FileFindFirst(&info)) 
		{
		do 
			{
		   /* Ignore . and .. and insist on right length! */
		   if (info.filespec[0] == '.' || strlen(info.filespec) != 7) 
		   	{
		      continue;
		   	}

		   /* Decode year and doy */
			sscanf(info.filespec, "%4d%3d", &year, &day);
		   /* Set hour, minute, and second to zeros */
		   info.yd = EncodeMSTimeDOY((INT32) year, (INT32) day, 0, 0, 0.0);
		
		   /* If earliest is specified and time is less, go around */
		   if (!UndefinedTime(criteria->time.earliest) &&
		       CompareYD(info.yd, criteria->time.earliest) < 0) 
		   	{
		      continue;
		   	}
		
		   /* If latest is specified and time is greater, go around */
		   if (!UndefinedTime(criteria->time.latest) &&
		       CompareYD(info.yd, criteria->time.latest) > 0) 
		   	{
		      continue;
		   	}
		
		   /* Otherwise, we'll save this year & day, inserting it in ordered list*/
		  	if (head == NULL)	/* if first directory, then start list*/
		  		{
		  		head = (YD_NODE*) malloc(sizeof(YD_NODE));
	  			if (head == NULL)
		  				break;
		  		tail = head;
		  		head->yd = info.yd;
				head->year = year;
				head->day = day;
				head->next = NULL;
		  		}
		  	else						/* find place to insert data */
		  		{
		  		cur = NULL;
		  		if (CompareYD(info.yd, head->yd) <= 0)
		  			{
		  			cur = (YD_NODE*) malloc(sizeof(YD_NODE));
		  			if (cur == NULL)
		  				break;
		  			cur->next = head;
					head = cur;
		  			}
		  		else if (CompareYD(info.yd, tail->yd) >= 0)
		  			{
		  			cur = (YD_NODE*) malloc(sizeof(YD_NODE));
		  			if (cur == NULL)
		  				break;
		  			tail->next = cur;
					cur->next = NULL;
					tail = cur;
		  			}
		  		else
		  			{
		  			for (cur = head; ; cur = cur->next)
		  				{
		  				if (cur->next == NULL)/*we already checked the tail*/
							break;
						if ((CompareYD(info.yd, cur->yd) >= 0) &&
					  	 (CompareYD(info.yd, cur->next->yd) <= 0))
							{
							new_node = (YD_NODE*) malloc(sizeof(YD_NODE));
			  				if (new_node == NULL)
			  					{
			  					cur = NULL;
			  					break;
			  					}
							new_node->next = cur->next;
							cur->next = new_node;
							cur = new_node;
							break;
							}
		  				}
		  			}
		  		/* If no memory to build list quit*/
		  		if (cur != NULL)
		  			{
		  			cur->yd = info.yd;
					cur->year = year;
					cur->day = day;
					}
				else
					{
					break;
					}
		  		}/*end else insert in list*/
			} while (FileFindNext(&info));
		}/* end if found first */
		FileFindClose(&info);
		return(head);
	}  /* end get_yd_list */
	
/*======================================================================
----------------------------------------------------------------------*/
static VOID DeleteYDList(YD_NODE *cur)
	{
	YD_NODE *nxt_node;
	
	while (cur)
		{
		ArchiveLog(ARC_LOG_MAXIMUM, "DeleteYDList: Done with Y%hd D%hd \n",cur->year,cur->day);
		nxt_node = cur->next;
		free(cur);
		cur = nxt_node;
		}
	}  /* end DeleteYDList() */

/*======================================================================
   Purpose:	Remove a file.
   Returns:	TRUE	file removed
				FALSE	unable to remove file
   Notes:	
   Revised:
		15Dec04	PLD	remove check for open file; sopen not *nix-compatible
							after all
		08Dec04	PLD	add check for open file: attempt to open it with
							exclusive access.
----------------------------------------------------------------------*/
static BOOL RemoveEventFile(CHAR *path)
   {

   ASSERT(path != NULL);

   /* Delete the file... */
   if (remove(path) < 0)
      {
      ArchiveLog(ARC_LOG_ERRORS, "Purge: unable to remove file: %s", path);
      return FALSE;
      }

   /* Remove emtpy stream directory */
   if (!TrimPath(path, 1))
      return FALSE;
   if (!PathEmpty(path))
      return TRUE;
   ArchiveLog(ARC_LOG_VERBOSE, "Purge: removing %s", path);
   if (!DestroyPath(path))
      {
      ArchiveLog(ARC_LOG_ERRORS, "Purge: unable to remove directory: %s - %s", path, FileErrorString(FileLastError()));
      return FALSE;
      }

   /* Remove empty unit directory */
   if (!TrimPath(path, 1))
      return FALSE;
   if (!PathEmpty(path))
      return TRUE;
   ArchiveLog(ARC_LOG_VERBOSE, "Purge: removing %s", path);
   if (!DestroyPath(path))
      {
      ArchiveLog(ARC_LOG_ERRORS, "Purge: unable to remove directory: %s - %s", path, FileErrorString(FileLastError()));
      return FALSE;
      }

   /* Remove empty day directory */
   if (!TrimPath(path, 1))
      return FALSE;
   if (!PathEmpty(path))
      return TRUE;
   ArchiveLog(ARC_LOG_VERBOSE, "Purge: removing %s", path);
   if (!DestroyPath(path))
      {
      ArchiveLog(ARC_LOG_ERRORS, "Purge: unable to remove directory: %s - %s", path, FileErrorString(FileLastError()));
      return FALSE;
      }

   return TRUE;
   }  /* end RemoveEventFile() */

/*======================================================================
----------------------------------------------------------------------*/
static BOOL PathEmpty(CHAR *path)
   {
   BOOL result;
   EVENT info;

   ASSERT(path != NULL);

   /* Look for any files... */
   result = TRUE;
   sprintf(info.filespec, "%s/*", path);
   info.attribute = FIND_FILE;
   if (FileFindFirst(&info))
      result = FALSE;
   FileFindClose(&info);

   if (!result)
      return result;

   /* or subdirectories... */
   result = TRUE;
   sprintf(info.filespec, "%s/*", path);
   info.attribute = FIND_SUBDIR;
   if (FileFindFirst(&info))
      {
      do
         {
         if (info.filespec[0] != '.')
            {
            result = FALSE;
            break;
            }
         }
      while (FileFindNext(&info));
      }
   FileFindClose(&info);

   return result;
   }  /* end PathEmpty() */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:13  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.15  2004/12/16 21:57:33  pdavidson
 * Prevent over-purge.  Tweak log messages.
 *
 * Revision 1.14  2004/12/15 23:23:13  pdavidson
 * Modify purge thread to prevent over-purge at startup.
 *
 * Revision 1.13  2004/12/15 20:12:27  pdavidson
 * Bug fix in archive references.
 *
 * Revision 1.12  2004/12/15 19:32:28  pdavidson
 * More work handling archive stats during purge.
 *
 * Revision 1.11  2004/12/14 19:37:33  pdavidson
 * Add rtpaux project.
 *
 * Revision 1.10  2004/11/22 14:57:38  roberta
 * Make more changes to purge to ensure the linked list of YearDay directory names are properly formed.
 *
 * Revision 1.9  2004/11/19 14:08:57  rstavely
 * Misplaced check for duplicate filename in ArchiveFindNextEvent loop.
 * Corrected this to check only after get seconf filename.
 *
 * Revision 1.8  2004/11/18 16:03:19  rstavely
 * Had to use dos2unix to convert file.
 *
 * Revision 1.7  2004/11/18 14:59:56  roberta
 * A line of code which prevents infinite looping in DeleteOldestEvent was not merged properly. COrrected problem.
 *
 * Revision 1.6  2004/11/18 13:35:42  roberta
 * Continued problems with purge on SOlaris & Linux required bug fixes to version 1.10.7 of RTPD
 *
 * Revision 1.5  2004/11/05 14:49:16  rstavely
 * Modify the purge routines to update the number 
 * of bytes in the archive if a file was actually removed, 
 * and have it remove the next oldest file, if it is unable 
 * to remove the oldest file. This change was made when it was 
 * found that a unit connected to a GPS with a bad time, 
 * created an old file time, and the purge tries to remove that 
 * "old" but still open file.
 *
 * Revision 1.4  2002/01/18 17:53:21  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.3  2001/10/04 20:42:19  nobody
 * Fixed Linux build issues
 *
 * Revision 1.2  2001/07/23 18:48:25  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
  */
