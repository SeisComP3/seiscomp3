/***************************************************************************
 * mseedscan.c
 *
 * A SeedLink plugin to recursively scan one or more directory
 * structures and continuously check for file modifications.  The
 * files are presumed to be composed of Mini-SEED records of length
 * RECSIZE (defined below).  As the files are appended or created the
 * plugin will feed the new records to the controlling SeedLink
 * server.
 *
 * The scanning of files uses the concept of active, idle and quiet
 * files.  Files that have not been modified for specified amounts of
 * time are considered idle or quiet.  The idle files will not be
 * checked on every scan, instead a specified number of scans will be
 * skipped for each idle file.  The quiet files will be be checked
 * ever again.
 *
 * If a file is scanned and a valid Mini-SEED record was not read from
 * the file a placeholder will be kept but the file will not be read
 * from again.  This way files which do not contain Mini-SEED will
 * only be reported once.  This placeholder will be retained in a
 * statefile (if used), this means that the file will not be scanned
 * if the program is re-started.  In order to get the plugin to
 * re-scan a file it must be removed from the state file.
 *
 * A balanced binary-tree is used to keep track of the files processed
 * and allows for operation with 100,000s of files.
 *
 * Broken symbolic links are quietly skipped.  If they are eventually
 * re-connected to something the something will be scanned as
 * expected.
 *
 * The directory separator is assumed to be '/'.
 *
 * If the signal SIGUSR1 is recieved the program will print the
 * current file list to standard error.
 *
 * Chad Trabant, IRIS Data Management Center
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>

#ifndef TESTING
#include <plugin.h>
#endif

#include "rbtree.h"
#include "stack.h"
#include "seedutil.h"

#define PACKAGE "mseedscan_plugin"
#define VERSION "2006.292"

#define RECSIZE 512

/* Maximum filename length including path */
#define MAX_FILENAME_LENGTH 512

/* Directory list file will be re-checked at scan intervals */
#define DIRFILEINT 10;

/* The FileKey and FileNode structures form the key and data elements
 * of a balanced tree that is used to keep track of all files being
 * processed.
 */

/* Structure used as the key for B-tree of file entries (FileNode) */
typedef struct filekey {
  ino_t inode;           /* Inode number, assumed to be unsigned */
  char filename[1];      /* File name, sized appropriately */
} FileKey;

/* Structure used as the data for B-tree of file entries */
typedef struct filenode {
  off_t offset;          /* Last file read offset, must be signed */
  time_t modtime;        /* Last file modification time, assumed to be signed */
  time_t scantime;       /* Last file scan time */
  int idledelay;         /* Idle file scan iteration delay */
  int quiet;             /* Quiet files are never scanned again */
} FileNode;

/* Linkable structure used for each specified base directory */
typedef struct dirlink {
  struct dirlink *next;
  char dirname[1];
} DirLink;

/* Structure for directory list file */
typedef struct dirfile {
  char filename[MAX_FILENAME_LENGTH];
  time_t modtime;
} DirFile;


static DirLink *dirlist = 0;    /* Base directories for file scanning */
static DirFile *dirfile = 0;    /* Directory list file */
static RBTree *filetree = 0;    /* Working list of scanned files in a tree */

static char  stopsig    = 0;    /* Stop/termination signal */
static int   verbose    = 0;    /* Verbosity level */
static char  maxrecur   = 3;    /* Maximum level of directory recursion */
static char  nextnew    = 0;    /* Only read next new data in existing files */
static int   netstaid   = 0;    /* Include net code in station id */
static int   scanint    = 5;    /* Scanning interval in seconds */
static int   idledelay  = 60;   /* Multiplier to scanint for idle files */
static int   idlesec    = 7200; /* Files are idle if not modified for idlesec */
static int   quietsec   = 0;    /* Files are quiet if not modified for idlesec */
static int   stateint   = 300;  /* State saving interval in seconds */
static char *statefile  = 0;    /* State file for saving/restoring time stamps */
static regex_t *fnmatch  = 0;   /* Filename match regex */
static regex_t *fnreject = 0;   /* Filename reject regex */

static int scanfiles (char *targetdir, char *basedir, int level, time_t scantime);
static FileNode *findfile (FileKey *fkey);
static FileNode *addfile (ino_t inode, char *filename, time_t modtime);
static off_t processfile (char *filename, FileNode *fnode, off_t newsize, time_t newmodtime);
static void  prunefiles (time_t scantime);
static void  printfilelist (FILE *fd);
static int   savestate (char *statefile);
static int   recoverstate (char *statefile);
static int   processparam (int argcount, char **argvec);
static char *getoptval (int argcount, char **argvec, int argopt);
static void  adddir (char *dirname);
static void  processdirfile (char *filename);
static int   keycompare (const void *a, const void *b);
static void  term_handler();
static void  print_handler();
static int   lprintf (int level, const char *fmt, ...);
static void  usage ();


int
main (int argc, char** argv)
{
  DirLink *dlp;
  time_t scantime, statetime;
  struct timespec treq, trem;
  int dirfilecount = DIRFILEINT;

  /* Signal handling using POSIX routines */
  struct sigaction sa;
  
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);

  sa.sa_handler = print_handler;
  sigaction(SIGUSR1, &sa, NULL);
  
  sa.sa_handler = term_handler;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
  
  sa.sa_handler = SIG_IGN;
  sigaction(SIGHUP, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);
  
  filetree = RBTreeCreate (keycompare, free, free);
  
  /* Process command line parameters */
  if (processparam (argc, argv) < 0)
    return 1;
  
  treq.tv_sec = (time_t) scanint;
  treq.tv_nsec = (long) 0;
  
  statetime = time(NULL);
  
  while ( stopsig == 0 )
    {
      dlp = dirlist;

      scantime = time(NULL);
      
      while ( dlp != 0 && stopsig == 0 )
	{
	  scanfiles (dlp->dirname, dlp->dirname, maxrecur, scantime);
	  
	  dlp = dlp->next;
	}
      
      if ( stopsig == 0 )
	{
	  /* Prune files that were not found from the filelist */
	  prunefiles (scantime);
	  
	  /* Save intermediate state file */
	  if ( statefile && stateint && (scantime - statetime) > stateint )
	    {
	      savestate (statefile);
	      statetime = scantime;
	    }
	  
	  /* Re-read directory list file */
	  if ( dirfile && dirfilecount == 0 )
	    {
	      processdirfile (dirfile->filename);
	      dirfilecount = DIRFILEINT;
	    }
	  else
	    {
	      dirfilecount--;
	    }
	  
	  /* Reset the next new flag, the first scan is now complete */
	  if ( nextnew ) nextnew = 0;
	  
	  /* Sleep for specified interval */
	  nanosleep (&treq, &trem);
	}
    }
  
  if ( statefile )
    savestate (statefile);
  
  return 0;
}  /* End of main() */


/***************************************************************************
 * scanfiles:
 *
 * Scan a directory and recursively drop into sub-directories up to the 
 * maximum recursion level.
 *
 * When initially called the targetdir and basedir should be the same,
 * either an absolute or relative path.  During directory recursion
 * these values are maintained such that they are either absolute
 * paths or targetdir is a relatively path from the current working
 * directory (which changes during recursion) and basedir is a
 * relative path from the initial directory.
 *
 * If the FileNode->offset of an existing entry is -1 skip the file,
 * this happens when there was trouble reading the file, the contents
 * were not Mini-SEED on a previous attempt, etc.
 *
 * Return 0 on success and -1 on error.
 ***************************************************************************/
static int
scanfiles (char *targetdir, char *basedir, int level, time_t scantime)
{
  static int dlevel = 0;
  FileNode *fnode;
  FileKey *fkey;
  char filekeybuf[sizeof(FileKey)+MAX_FILENAME_LENGTH]; /* Room for fkey */
  struct stat st;
  struct dirent *de;
  int rrootdir;
  DIR *dir;
  
  fkey = (FileKey *) &filekeybuf;
  
  lprintf (3, "Processing directory '%s'", basedir);
  
  if ( (rrootdir = open (".", O_RDONLY, 0)) == -1)
    {
      lprintf (0, "Error opening current working directory\n");
      return -1;
    }
  
  if ( chdir (targetdir) )
    {
      lprintf (0, "Cannot change to directory %s: %s", targetdir, strerror(errno));
      close (rrootdir);
      return -1;
    }
  
  if ( (dir = opendir (".")) == NULL )
    {
      lprintf (0, "Cannot open directory %s: %s", targetdir, strerror(errno));
      
      if ( fchdir (rrootdir) )
	{
	  lprintf (0, "Cannot change to relative root directory: %s", strerror(errno));
	}
      
      close (rrootdir);
      return -1;
    }
  
  while ( (de = readdir(dir)) != NULL && stopsig == 0 )
    {
      int filenamelen;
      
      if ( !strcmp(de->d_name, ".") || !strcmp(de->d_name, "..") )
	continue;
      
      /* Build a FileKey for this file */
      fkey->inode = de->d_ino;
      filenamelen = snprintf (fkey->filename, sizeof(filekeybuf) - sizeof(FileKey),
			      "%s/%s", basedir, de->d_name);
      
      /* Make sure the filename was not truncated */
      if ( filenamelen >= (sizeof(filekeybuf) - sizeof(FileKey) - 1) )
	{
	  lprintf (0, "Directory entry name beyond maximum of %d characters, skipping:\n",
		   (sizeof(filekeybuf) - sizeof(FileKey) - 1));
	  lprintf (0, "  %s\n", de->d_name);
	  continue;
	}
      
      /* Search for a matching entry in the filetree */
      if ( (fnode = findfile (fkey)) )
	{
	  /* Check if the file is permanently skipped */
	  if ( fnode->offset == -1 )
	    {
	      fnode->scantime = scantime;
	      continue;
	    }
	  
	  /* Check if file has triggered a delayed check */
	  if ( fnode->idledelay > 0 )
	    {
	      fnode->scantime = scantime;
	      fnode->idledelay--;
	      continue;
	    }
	  
	  /* Check if the file has been flagged as quiet */
	  if ( fnode->quiet )
	    {
	      continue;
	    }
	}
      
      if ( lstat (de->d_name, &st) < 0 )
	{
	  lprintf (0, "Cannot stat %s: %s", fkey->filename, strerror(errno));
	  continue;
	}
      
      /* If symbolic link stat the real file, if it's a broken link continue */
      if ( S_ISLNK(st.st_mode) )
	{
	  if ( stat (de->d_name, &st) < 0 )
	    {
	      if ( errno != ENOENT )
		lprintf (0, "Cannot stat %s: %s", fkey->filename, strerror(errno));
	      else
		continue;
	    }
	}
      
      /* If directory recurse up to the limit */
      if ( S_ISDIR(st.st_mode) )
	{
	  if ( dlevel < level )
	    {
	      lprintf (4, "Recursing into %s", fkey->filename);
	      
	      dlevel++;
	      scanfiles (de->d_name, fkey->filename, level, scantime);
	      dlevel--;
	    }
	  continue;
	}
      
      /* Do regex matching if an expression was specified */
      if ( fnmatch != 0 )
	if ( regexec (fnmatch, de->d_name, (size_t) 0, NULL, 0) != 0 )
	  continue;
      
      /* Do regex rejecting if an expression was specified */
      if ( fnreject != 0 )
	if ( regexec (fnreject, de->d_name, (size_t) 0, NULL, 0) == 0 )
	  continue;
      
      /* Sanity check for a regular file */
      if ( !S_ISREG(st.st_mode) )
	{
	  lprintf (0, "%s is not a regular file", fkey->filename);
	  continue;
	}
      
      /* Sanity check that the dirent inode and stat inode are the same */
      if ( st.st_ino != de->d_ino )
	{
	  fprintf (stderr, "Inode numbers from dirent and stat do not match for %s\n", fkey->filename);
	  fprintf (stderr, "  dirent: %llu  VS  stat: %llu\n",
		   (unsigned long long int) de->d_ino, (unsigned long long int) st.st_ino);
	  continue;
	}
      
      lprintf (3, "Checking file %s", fkey->filename);
      
      /* If the file has never been seen add and process it.
	 If the file has been modified recently and grown in size process it.
      */
      if ( ! fnode )
	{
	  if ( ! (fnode = addfile (fkey->inode, fkey->filename, st.st_mtime)) )
	    {
	      lprintf (0, "Error adding %s to file list\n", fkey->filename);
	      continue;
	    }
	  
	  /* Only update the offset if skipping the first scan, otherwise process */
	  if ( nextnew )
	    fnode->offset = st.st_size;
	  else
	    fnode->offset = processfile (fkey->filename, fnode, st.st_size, st.st_mtime);
	}
      else if ( fnode->modtime < st.st_mtime &&
		fnode->offset < st.st_size )
	{
	  fnode->offset = processfile (fkey->filename, fnode, st.st_size, st.st_mtime);
	}
      
      if ( fnode )
	{
	  /* Check if the file is idle and set idledelay appropriately */
	  if ( fnode->idledelay == 0 && st.st_mtime < (scantime - idlesec) )
	    {
	      fnode->idledelay = (idledelay > 0) ? (idledelay-1) : 0;
	    }
	  
	  /* Check if the file is quiet and set the quiet flag appropriately */
	  if ( quietsec && st.st_mtime < (scantime - quietsec) )
	    {
	      fnode->quiet = 1;
	    }
	  
	  /* Update scantime */
	  fnode->scantime = scantime;
	}
    }
  
  closedir (dir);
  
  if ( fchdir (rrootdir) )
    {
      lprintf (0, "Cannot change to relative root directory: %s",
	       strerror(errno));
    }
  
  close (rrootdir);
  
  return 0;
}  /* End of scanfiles() */


/***************************************************************************
 * findfile:
 *
 * Search the filetree for a given FileKey.
 *
 * Return a pointer to a FileNode if found or 0 if no match found.
 ***************************************************************************/
static FileNode*
findfile (FileKey *fkey)
{
  FileNode *fnode = 0;
  RBNode *tnode;
  
  /* Search for a matching inode + file name entry */
  if ( (tnode = RBFind (filetree, fkey)) )
    {
      fnode = (FileNode *)tnode->data;
    }
  
  return fnode;
}  /* End of findfile() */


/***************************************************************************
 * addfile:
 *
 * Add a file to the file tree, no checking is done to determine if
 * this entry already exists.
 *
 * Return a pointer to the added FileNode on success and 0 on error.
 ***************************************************************************/
static FileNode*
addfile (ino_t inode, char *filename, time_t modtime)
{
  FileKey *newfkey;
  FileNode *newfnode;
  size_t filelen;
  
  lprintf (1, "Adding %s", filename);
  
  /* Create new tree key */
  filelen = strlen (filename);
  newfkey = (FileKey *) malloc (sizeof(FileKey)+filelen);
  
  /* Create new tree node */
  newfnode = (FileNode *) malloc (sizeof(FileNode));
  
  if ( ! newfkey || ! newfnode )
    return 0;
  
  /* Populate the new key and node */
  newfkey->inode = inode;
  memcpy (newfkey->filename, filename, filelen+1);
  
  newfnode->offset = 0;
  newfnode->modtime = modtime;
  newfnode->idledelay = 0;
  newfnode->quiet = 0;
  
  RBTreeInsert (filetree, newfkey, newfnode);
  
  return newfnode;
}  /* End of addfile() */


/***************************************************************************
 * processfile:
 *
 * Process a file by reading any data after the last offset.
 *
 * Return the new file offset on success and -1 on error.
 ***************************************************************************/
static off_t
processfile (char *filename, FileNode *fnode, off_t newsize, time_t newmodtime)
{
  int fd;
  int nread;
  int reccnt = 0;
  off_t newoffset = fnode->offset;
  char mseedbuf[RECSIZE];
  char staid[10];
  char *tptr;

  lprintf (3, "Processing file %s", filename);

  /* We are already in the local directory so do a cheap basename(1) */
  if ( ! (tptr = strrchr (filename, '/')) )
    tptr = filename;
  else
    tptr++;
  
  if ( (fd = open(tptr, O_RDONLY, 0)) == -1 )
    {
      lprintf (0, "Error opening %s: %s", filename, strerror(errno));
      return -1;
    }
  
  if ( lseek (fd, newoffset, SEEK_SET) != newoffset )
    {
      lprintf (0, "Error seeking in %s: %s", filename, strerror(errno));
      close (fd);
      return -1;
    }
  
  /* Read RECSIZE byte chunks off the end of the file */
  while ( (newsize - newoffset) >= RECSIZE )
    {
      if ( (nread = read (fd, mseedbuf, RECSIZE)) != RECSIZE )
	{
	  lprintf (0, "Error: only read %d bytes from %s",
		   nread, filename);
	  close (fd);
	  return -1;
	}
      
      newoffset += nread;
      
      /* Check record for 1000 blockette and verify SEED structure */
      if ( find_reclen (mseedbuf, RECSIZE) <= 0 )
	{
	  lprintf (0, "%s: Not a valid Mini-SEED record at offset %lld",
		   filename, (long long) (newoffset - nread));
	  close (fd);
	  return -1;
	}
      /* Send the record off to the controlling SeedLink server */
      else
	{
	  /* Create a station ID of either 'NET_STA' or 'STA' */
	  if ( netstaid )
	    {
	      strncpy (staid, mseedbuf+18, 2); staid[2] = '\0';
	      if ( (tptr = strchr (staid, ' ')) != NULL )
		*tptr = '\0';
	      
	      strcat (staid, "_");
	      
	      strncat (staid, mseedbuf+8, 5); staid[8] = '\0';
	      if ( (tptr = strchr (staid, ' ')) != NULL )
		*tptr = '\0';
	    }
	  else
	    {
	      strncpy (staid, mseedbuf+8, 5); staid[5] = '\0';
	      if ( (tptr = strchr (staid, ' ')) != NULL )
		*tptr = '\0';
	    }
	  
	  lprintf (4, "Sending (%s) %20.20s", staid, mseedbuf);
	  
#ifndef TESTING
	  send_mseed (staid, mseedbuf, RECSIZE);
#endif
	}
      
      reccnt++;
    }
  
  close (fd);

  /* Update modtime */
  fnode->modtime = newmodtime;
  
  if ( reccnt )
    lprintf (2, "Read %d %d-byte record(s) from %s",
	     reccnt, RECSIZE, filename);
  
  return newoffset;
}  /* End of processfile() */


/***************************************************************************
 * prunefiles:
 *
 * Prune file entries from the filetree that do not have a check time
 * later than that specified.
 *
 ***************************************************************************/
static void
prunefiles (time_t scantime)
{
  FileNode *fnode;
  FileKey  *fkey;
  RBNode   *tnode;
  Stack    *stack;
  
  stack = StackCreate();
  RBBuildStack (filetree, stack);
  
  while ( (tnode = (RBNode *) StackPop (stack)) )
    {
      fkey = (FileKey *) tnode->key;
      fnode = (FileNode *) tnode->data;
      
      if ( fnode->scantime < scantime )
      	{
	  lprintf (1, "Removing %s from file list", fkey->filename);
	  
	  RBDelete (filetree, tnode);
	}
    }
  
  StackDestroy (stack, free);
}  /* End of prunefiles() */


/***************************************************************************
 * printfilelist:
 *
 * Print file tree to the specified descriptor.
 ***************************************************************************/
static void
printfilelist (FILE *fp)
{
  FileKey  *fkey;
  FileNode *fnode;
  RBNode   *tnode;
  Stack    *stack;
  
  stack = StackCreate();
  RBBuildStack (filetree, stack);
  
  while ( (tnode = (RBNode *) StackPop (stack)) )
    {
      fkey = (FileKey *) tnode->key;
      fnode = (FileNode *) tnode->data;
      
      fprintf (fp, "%s\t%llu\t%lld\t%lld\n",
	       fkey->filename,
	       (unsigned long long int) fkey->inode,
	       (signed long long int) fnode->offset,
	       (signed long long int) fnode->modtime);
    }
  
  StackDestroy (stack, free);
}  /* End of printfilelist() */


/***************************************************************************
 * savestate:
 *
 * Save state information to a specified file.
 *
 * Returns 0 on success and -1 on error.
 ***************************************************************************/
static int
savestate (char *statefile)
{
  FILE *fp;
  
  lprintf (2, "Saving state file");
  
  if ( (fp = fopen(statefile, "w")) == NULL )
    {
      lprintf (0, "Error opening statefile %s: %s", statefile, strerror(errno));
      return -1;
    }
  
  printfilelist (fp);
  
  fclose (fp);
  
  return 0;
}  /* End of savestate() */


/***************************************************************************
 * recoverstate:
 *
 * Recover the state information from the state file.
 *
 * Returns 0 on success and -1 on error.
 ***************************************************************************/
static int
recoverstate (char *statefile)
{
  FileNode *fnode;
  char line[600];
  int fields, count;
  FILE *fp;
  
  char filename[MAX_FILENAME_LENGTH];
  unsigned long long int inode;
  signed long long int offset, modtime;
  
  if ( (fp=fopen(statefile, "r")) == NULL )
    {
      lprintf (0, "Error opening statefile %s: %s", statefile, strerror(errno));
      return -1;
    }
  
  lprintf (1, "Recovering state");
  
  count = 1;
  
  while ( (fgets (line, sizeof(line), fp)) !=  NULL)
    {
      fields = sscanf (line, "%s %llu %lld %lld\n",
		       filename, &inode, &offset, &modtime);
      
      if ( fields < 0 )
        continue;
      
      if ( fields < 4 )
        {
          lprintf (0, "Could not parse line %d of state file", count);
	  continue;
        }
      
      fnode = addfile ((ino_t) inode, filename, (time_t) modtime);
      
      if ( fnode )
	{
	  fnode->offset = (off_t) offset;
	  fnode->scantime = 0;
	}
      
      count++;
    }
  
  fclose (fp);
  
  return 0;
}  /* End of recoverstate() */


/***************************************************************************
 * processparam:
 *
 * Process the command line parameters.
 *
 * Returns 0 on success, and -1 on failure
 ***************************************************************************/
static int
processparam (int argcount, char **argvec)
{
  char *dirfilename = 0;
  char *matchstr = 0;
  char *rejectstr = 0;
  char *tptr;
  int optind;
  
  /* Process all command line arguments */
  for (optind = 1; optind < argcount; optind++)
    {
      if (strcmp (argvec[optind], "-V") == 0)
        {
          fprintf (stderr, "%s version: %s\n", PACKAGE, VERSION);
          exit (0);
        }
      else if (strcmp (argvec[optind], "-h") == 0)
        {
          usage();
        }
      else if (strncmp (argvec[optind], "-v", 2) == 0)
        {
          verbose += strspn (&argvec[optind][1], "v");
        }
      else if (strcmp (argvec[optind], "-n") == 0)
        {
          nextnew = 1;
        }
      else if (strcmp (argvec[optind], "-z") == 0)
        {
          nextnew = 2;
        }
      else if (strcmp (argvec[optind], "-s") == 0)
        {
          scanint = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
        }
      else if (strcmp (argvec[optind], "-i") == 0)
        {
          idledelay = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
        }
      else if (strcmp (argvec[optind], "-I") == 0)
        {
          idlesec = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
        }
      else if (strcmp (argvec[optind], "-Q") == 0)
        {
          quietsec = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
        }
      else if (strcmp (argvec[optind], "-r") == 0)
        {
          maxrecur = strtol (getoptval(argcount, argvec, optind++), NULL, 10);
        }
      else if (strcmp (argvec[optind], "-M") == 0)
        {
          matchstr = getoptval(argcount, argvec, optind++);
        }
      else if (strcmp (argvec[optind], "-R") == 0)
        {
          rejectstr = getoptval(argcount, argvec, optind++);
        }
      else if (strcmp (argvec[optind], "-N") == 0)
        {
          netstaid = 1;
        }
      else if (strcmp (argvec[optind], "-x") == 0)
        {
          statefile = getoptval(argcount, argvec, optind++);
        }
      else if (strcmp (argvec[optind], "-d") == 0)
        {
          adddir (getoptval(argcount, argvec, optind++));
        }
      else if (strcmp (argvec[optind], "-D") == 0)
        {
	  if ( dirfilename )
	    {
	      lprintf (0, "Only one -D option is allowed\n");
	      exit (1);
	    }
	  
	  dirfilename = getoptval(argcount, argvec, optind++);
        }
      else if (strncmp (argvec[optind], "-", 1) == 0)
        {
          lprintf (0, "Unknown option: %s", argvec[optind]);
          exit (1);
        }
    }
  
  /* Report the program version */
  lprintf (1, "%s version: %s", PACKAGE, VERSION);
  
  /* Load dirlist from dirfile */
  if ( dirfilename )
    {
      /* Make sure no directories were already added to the list */
      if ( dirlist )
	{
	  lprintf (0, "Cannot specify both -d and -D options\n");
	  exit (1);
	}
      
      /* Allocate new DirFile */
      dirfile = (DirFile *) malloc (sizeof(DirFile));
      
      if ( dirfile == NULL )
	{
	  lprintf (0, "Error allocating memory for dir file list\n");
	  exit (1);
	}
      
      /* Copy directory list file name */
      strncpy (dirfile->filename, dirfilename, sizeof(dirfile->filename));
      
      /* Set dirfile modification time to 0 */
      dirfile->modtime = 0;
      
      processdirfile (dirfile->filename);
    }
  
  /* Make sure base dir(s) specified */
  if ( dirlist == 0 )
    {
      lprintf (0, "No base directories were specified");
      exit (1);
    }
  
  /* Compile the match regex if specified */
  if ( matchstr )
    {
      fnmatch = (regex_t *) malloc (sizeof (regex_t));
      
      if ( regcomp(fnmatch, matchstr, REG_EXTENDED|REG_NOSUB ) != 0 )
	{
	  lprintf (0, "Cannot compile regular expression '%s'", matchstr);
	  exit (1);
	}
    }
  
  /* Compile the reject regex if specified */
  if ( rejectstr )
    {
      fnreject = (regex_t *) malloc (sizeof (regex_t));
      
      if ( regcomp(fnreject, rejectstr, REG_EXTENDED|REG_NOSUB ) != 0 )
	{
	  lprintf (0, "Cannot compile regular expression '%s'", rejectstr);
	  exit (1);
	}
    }
  
  /* Attempt to recover sequence numbers from state file */
  if ( statefile )
    {
      /* Check if interval was specified for state saving */
      if ( (tptr = strchr (statefile, ':')) != NULL )
        {
          char *tail;
          
          *tptr++ = '\0';
	  
          stateint = (int) strtol (tptr, &tail, 10);
	  
          if ( *tail || (stateint < 0 || stateint > 1e9) )
            {
              lprintf (0, "State saving interval specified incorrectly");
              exit (1);
            }
        }
      
      if ( recoverstate (statefile) < 0 )
        {
          lprintf (0, "state recovery failed");
        }
      /* If nextnew (-z) was specified but state file read, turn it off */
      else if ( nextnew == 2 )
	{
	  nextnew = 0;
	}
    }
  
  return 0;
}  /* End of processparam() */


/***************************************************************************
 * getoptval:
 * Return the value to a command line option; checking that the value is 
 * itself not an option (starting with '-') and is not past the end of
 * the argument list.
 *
 * argcount: total arguments in argvec
 * argvec: argument list
 * argopt: index of option to process, value is expected to be at argopt+1
 *
 * Returns value on success and exits with error message on failure
 ***************************************************************************/
static char *
getoptval (int argcount, char **argvec, int argopt)
{
  if ( argvec == NULL || argvec[argopt] == NULL )
    {
      lprintf (0, "getoptval(): NULL option requested");
      exit (1);
    }
  
  if ( (argopt+1) < argcount && *argvec[argopt+1] != '-' )
    return argvec[argopt+1];
  
  lprintf (0, "Option %s requires a value", argvec[argopt]);
  exit (1);
}  /* End of getoptval() */


/***************************************************************************
 * adddir:
 *
 * Add directory to end of the global directory list (dirlist).
 ***************************************************************************/
static void
adddir (char *dirname)
{
  struct dirlink *lastlp, *newlp;
  struct stat st;
  int dirlen;
  
  if ( ! dirname )
    {
      lprintf (0, "adddir(): No directory name specified");
      return;
    }
  
  dirlen = strlen (dirname);
  
  /* Remove trailing slash if included */
  if ( dirname[dirlen-1] == '/' )
    dirname[dirlen-1] = '\0';
  
  /* Verify that a directory exists, if not log a warning */
  if ( stat (dirname, &st) )
    {
      lprintf (0, "WARNING: Could not find '%s'", dirname);
    }
  else if ( ! S_ISDIR(st.st_mode) )
    {
      lprintf (0, "WARNING '%s' is not a directory, skipping", dirname);
      return;
    }
  
  /* Create the new DirLink */
  newlp = (DirLink *) malloc (sizeof(DirLink)+dirlen+1);
  
  if ( !newlp )
    return;
  
  memcpy (newlp->dirname, dirname, dirlen+1);
  newlp->next = 0;
  
  /* Find the last DirLink in the directory list */
  lastlp = dirlist;
  while ( lastlp != 0 )
    {
      if ( lastlp->next == 0 )
        break;
      
      lastlp = lastlp->next;
    }
  
  /* Insert the new DirLink at the end of the directory list */
  if ( lastlp == 0 )
    dirlist = newlp;
  else
    lastlp->next = newlp;
  
}  /* End of adddir() */


/***************************************************************************
 * processdirfile:
 *
 * Clear out global directory list (dirlist) and add all directories
 * listed in the specified file.
 ***************************************************************************/
static void
processdirfile (char *filename) 
{
  DirLink *dlp;
  struct stat st;
  FILE *fp;
  char dirlistent[MAX_FILENAME_LENGTH];

  lprintf (3, "Checking directory list file '%s'", filename);

  if ( stat (filename, &st) )
    {
      lprintf (0, "Cannot stat dir list file %s: %s", filename, strerror(errno));
      return;
    }

  if ( !S_ISREG(st.st_mode) )
    {
      lprintf (0, "%s is not a regular file", filename);
      return;
    }
  
  /* Check that the file has been modified since the last check */
  if ( st.st_mtime <= dirfile->modtime )
    {
      return;  /* No modification = no (re)loading of dir list */
    }
  
  /* Store new modification time */
  dirfile->modtime = st.st_mtime;
  
  /* Clear out directory list (dirlist) */
  if ( dirlist )
    {
      dlp = dirlist->next;      
      free (dirlist);
      dirlist = dlp;
    }
  dirlist = 0;
  
  if ( ! (fp = fopen(filename, "r")) )
    {
      lprintf (0, "Cannot open dir list file %s: %s", filename, strerror(errno));
      return;
    }
  
  lprintf (2, "Reading list of directories from %s", filename);
  
  while ( fgets (dirlistent, sizeof(dirlistent), fp) )
    {
      char *cp;
      
      /* End string at first newline character */
      if ( (cp = strchr(dirlistent, '\n')) )
	*cp = '\0';
      
      /* Skip empty lines */
      if ( ! strlen (dirlistent) )
	continue;
      
      /* Skip comment lines */
      if ( *dirlistent == '#' )
	continue;
      
      lprintf (2, "Adding directory '%s' from list file", dirlistent);
      
      adddir (dirlistent);
    }
  
  fclose (fp);
  
}  /* End of processdirfile() */


/***************************************************************************
 * keycompare:
 *
 * Compare two FileKeys passed as void pointers.
 *
 * Return 1 if a > b, -1 if a < b and 0 otherwise (e.g. equality).
 ***************************************************************************/
static int
keycompare (const void *a, const void *b)
{
  int cmpval;
  
  /* Compare Inode values */
  if ( ((FileKey*)a)->inode > ((FileKey*)b)->inode )
    return 1;
  
  else if ( ((FileKey*)a)->inode < ((FileKey*)b)->inode )
    return -1;
  
  /* Compare filename values */
  cmpval = strcmp ( ((FileKey*)a)->filename, ((FileKey*)b)->filename );
  
  if ( cmpval > 0 )
    return 1;
  else if ( cmpval < 0 )
    return -1;
  
  return 0;
}  /* End of keycompare() */


/***************************************************************************
 * term_handler and print_handler:
 * Signal handler routines.
 ***************************************************************************/
static void
term_handler (int sig)
{
  stopsig = 1;
}

static void
print_handler (int sig)
{
  fprintf (stderr, "Filename\tInode\tOffset\tModtime\n");
  printfilelist (stderr);
}


/***************************************************************************
 * lprintf:
 *
 * A generic log message handler, pre-pends a current date/time string
 * to each message.  This routine add a newline to the final output
 * message so it should not be included with the message.
 *
 * Returns the number of characters in the formatted message.
 ***************************************************************************/
static int
lprintf (int level, const char *fmt, ...)
{
  int rv = 0;
  char message[1024];
  va_list argptr;
  struct tm *tp;
  time_t curtime;
  
  char *day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
		   "Aug", "Sep", "Oct", "Nov", "Dec"};

  if ( level <= verbose ) {
    
    /* Build local time string and generate final output */
    curtime = time(NULL);
    tp = localtime (&curtime);
    
    va_start (argptr, fmt);
    rv = vsnprintf (message, sizeof(message), fmt, argptr);
    va_end (argptr);
    
    printf ("%3.3s %3.3s %2.2d %2.2d:%2.2d:%2.2d %4.4d - %s: %s\n",
	    day[tp->tm_wday], month[tp->tm_mon], tp->tm_mday,
	    tp->tm_hour, tp->tm_min, tp->tm_sec, tp->tm_year + 1900,
	    PACKAGE, message);
    
    fflush (stdout);
  }
  
  return rv;
}  /* End of lprintf() */


/***************************************************************************
 * usage:
 *
 * Print usage message and exit.
 ***************************************************************************/
static void
usage()
{
  fprintf(stderr,"%s version %s\n\n", PACKAGE, VERSION);
  fprintf(stderr,"Scan directory tree(s) for Mini-SEED files\n\n");
  fprintf(stderr,"Usage: %s [options] -d directory1 [-d directory2] ...\n\n", PACKAGE);
  fprintf(stderr," ## Options ##\n"
	  " -V             Report program version\n"
	  " -h             Show this usage message\n"
	  " -v             Be more verbose, multiple flags can be used\n"
	  " -n             During initial scan of files do not pass data to the server\n"
	  "                  from files not found in the state file\n"
	  " -z             During initial scan of files do not pass data to the server\n"
	  "                  if a state file is specified but not present.\n"
	  " -s secs        Specify the sleep interval between scans (%d seconds)\n"
	  " -i delay       Check idle files every 'delay' intervals (%d intervals)\n"
	  " -I secs        Files idle when unmodified this many seconds (%d seconds)\n"
	  " -Q secs        Files quiet when unmodified this many seconds (default off)\n"
	  " -r level       Maximum directory levels to recurse (%d levels)\n"
	  " -M match       Only process filenames that match this regular expression\n"
	  " -R reject      Only process filenames that do not match this regular expression\n"
	  " -N             Include the network code in the station ID sent to the\n"
	  "                  server in the format: 'NET_STA', default: 'STA'\n"
	  " -x file[:int]  State file to save/restore file time stamps, optionally\n"
	  "                  an interval, in seconds, can be specified to save the\n"
	  "                  state file (default interval: %d seconds)\n\n"
	  " -d directory#  Base directories to scan, may be specified multiple times\n"
	  "    or\n"
	  " -D listfile    File containing a list of directories to scan\n\n",
	  scanint, idledelay, idlesec, maxrecur, stateint);
  exit (1);
}  /* End of usage() */
