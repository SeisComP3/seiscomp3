#pragma ident "$Id: find.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * Equivalents to the Win32 functions _findfirst, _findnext, _findclose,
 * but with less general wildcard support.  Here, only the last path
 * element can be wildcarded, and the only wildcard that is recognized
 * is the "match everything" wildcard, ie '*'.
 *
 * MT-safe
 * 
 *====================================================================*/
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "util.h"

#define WILDCARD '*'

/* For internal bookkeeping */

typedef struct {
    MUTEX mutex;
    BOOL  done;
    DIR  *dir;
    char dirname[MAXPATHLEN+1];
} POOL;

static POOL  *pool        = (POOL *) NULL;
static INT32 global_count = 0;
static MUTEX global_mutex = MUTEX_INITIALIZER;

/* Test for a valid handle */

static BOOL valid(INT32 handle)
{
INT32 maxhandle;

    MUTEX_LOCK(&global_mutex);
        maxhandle = global_count - 1;
    MUTEX_UNLOCK(&global_mutex);

    return (handle < 0 || handle > maxhandle) ? FALSE : TRUE;
}

/* Find index of next free slot, realloc a new one if necessary */

static INT32 new_handle(DIR *dir, char *dirname)
{
INT32 i, handle;

    MUTEX_LOCK(&global_mutex);

    /* See if there is an unused slot we can grab */

        for (handle = -1, i = 0; handle == -1 && i < global_count; i++) {
            MUTEX_LOCK(&pool[i].mutex);
                if (pool[i].dir == (DIR *) NULL) handle = i;
            MUTEX_UNLOCK(&pool[i].mutex);
        }

    /* No free slots, so allocate a new one */

        if (handle < 0) {
            handle = global_count++;
            pool = realloc(pool, sizeof(POOL) * global_count);
            if (pool != (POOL *) NULL) MUTEX_INIT(&pool[handle].mutex);
        }

    /* Initialize our slot, if we've got one */

        if (pool != (POOL *) NULL && handle >= 0) {
            MUTEX_LOCK(&pool[handle].mutex);
                pool[handle].dir = dir;
                if (dir == (DIR *) NULL || dirname == (char *) NULL) {
                    pool[handle].done = TRUE;
                    pool[handle].dirname[0] = 0;
                } else {
                    strcpy(pool[handle].dirname, dirname);
                    pool[handle].done = FALSE;
                }
            MUTEX_UNLOCK(&pool[handle].mutex);
        }

    MUTEX_UNLOCK(&global_mutex);

/* Return new handle (-1 on failure) */

    return handle;
}

/* Fill in the file information */

static int fill_finddata(char *path, char *fname, struct _finddata_t *data)
{
FILE *fp;
struct stat sbuf;

    if (stat(path, &sbuf) != 0) return -1;

    data->time_create = sbuf.st_ctime;
    data->time_access = sbuf.st_atime;
    data->time_write  = sbuf.st_mtime;
    strcpy(data->name, fname);
    data->attrib = 0;

    if (fname[0] == '.') data->attrib |= _A_HIDDEN;

    if (sbuf.st_mode & S_IFREG) {
        data->size = sbuf.st_size;
        if ((fp = fopen(path, "r+")) == (FILE *) NULL) {
            data->attrib |= _A_RDONLY;
        } else {
            data->attrib |= _A_NORMAL;
            fclose(fp);
        }
    } else if (sbuf.st_mode & S_IFDIR) {
        data->attrib |= _A_SUBDIR;
    } else {
        data->attrib |= _A_SYSTEM;
    }

    return 0;
}

/* _findclose */

INT32 util_findclose(INT32 handle)
{
int status;

    if (!valid(handle)) {
        errno = EINVAL;
        return -1;
    }

    MUTEX_LOCK(&pool[handle].mutex);
        if (pool[handle].dir != (DIR *) NULL) {
            do {
                status =  closedir(pool[handle].dir);
            } while (status != 0 && errno == EINTR);
            pool[handle].dir = (DIR *) NULL;
        }
    MUTEX_UNLOCK(&pool[handle].mutex);

    return 0;
}

/* _findnext */

INT32 util_findnext(INT32 handle, struct _finddata_t *fileinfo)
{ 
BOOL done;
int status, errno;
struct dirent *result;
struct stat buf;
char path[MAXPATHLEN+1];

    if (!valid(handle)) {
        errno = EINVAL;
        return -1;
    }

    MUTEX_LOCK(&pool[handle].mutex);

    if (pool[handle].done) {
        errno = ENOENT;
        MUTEX_UNLOCK(&pool[handle].mutex);
        return -1;
    }

    done = FALSE;
    do {
        errno = 0;
        result = readdir(pool[handle].dir);
        if (result == (struct dirent *) NULL) {
            MUTEX_UNLOCK(&pool[handle].mutex);
            if (errno == 0) errno = ENOENT;
            pool[handle].done = TRUE;
            return -1;
        }
        if (
            strcmp(result->d_name, ".")  != 0 &&
            strcmp(result->d_name, "..") != 0
        ) done = TRUE;
    } while (!done);

    sprintf(path, "%s/%s", pool[handle].dirname, result->d_name);
    status = fill_finddata(path, result->d_name, fileinfo);

    MUTEX_UNLOCK(&pool[handle].mutex);
    return status;
}

/* _findfirst */

INT32 util_findfirst(CHAR *filespec, struct _finddata_t *fileinfo)
{
char *dirname;
DIR *dir;
INT32 handle;
struct stat buf;
char path[MAXPATHLEN+1];

/* See if this is a fully specified file or directory name.  If it is
 * fully specified file name then load the parameters directly.  If
 * it is not a fully specified file or directory then the only other
 * thing it is allowed to be is a directory specification with a wildcard
 * at the end.  That is equivalent to just specifying a directory and is
 * supported explicilty because _findfirst allows such behavior (and more,
 * but we aren't interested in a full implementation of wildcards).
 * Anyway, we want to leave this block of code with a valid directory
 * name that we'll walk.
 */

    dirname = (char *) NULL;
    if (stat(filespec, &buf) == 0) {
        if (buf.st_mode & S_IFREG) {
            handle = new_handle((DIR *) NULL, (char *) NULL);
            if (handle < 0) {
                return -1;
            }
            if (fill_finddata(filespec, filespec, fileinfo) == 0) {
                return handle;
            } else {
                return -1;
            }
        } else if (buf.st_mode & S_IFDIR) {
            dirname = filespec;
        }
    } else if (filespec[strlen(filespec)-1] != WILDCARD) {
        errno = ENOENT;
        return -1;
    } else {
        strcpy(path, filespec);
        path[strlen(path)-1] = 0;
        dirname = path;
    }

/* Open the directory */

    if ((dir = opendir((char *) dirname)) == (DIR *) NULL) {
        return -1;
    }

/* Associate it with a handle */

    if ((handle = new_handle(dir, dirname)) < 0) {
        return -1;
    }

/* Find the first entry */

    if (util_findnext(handle, fileinfo) != 0) {
        util_findclose(handle);
        return -1;
    }

    return handle;
    
}

#ifdef DEBUG_TEST

void pinfo(struct _finddata_t *info)
{
CHAR buf[UTIL_MAXTIMESTRLEN];

    printf("name = %s\n",  info->name);
    printf("size = %ld\n", info->size);
    printf("time_create = %s\n", util_dttostr(info->time_create, 0, buf));
    printf("time_access = %s\n", util_dttostr(info->time_access, 0, buf));
    printf("time_write  = %s\n", util_dttostr(info->time_write,  0, buf));
    printf("attrib      = 0x%08x = ", info->attrib);
    if (info->attrib == _A_NORMAL) {
        printf("_A_NORMAL");
    } else {
        if (info->attrib & _A_RDONLY) printf("_A_RDONLY ");
        if (info->attrib & _A_SYSTEM) printf("_A_SYSTEM ");
        if (info->attrib & _A_HIDDEN) printf("_A_HIDDEN ");
        if (info->attrib & _A_SUBDIR) printf("_A_SUBDIR ");
        if (info->attrib & _A_ARCH  ) printf("_A_ARCH   ");
    }
    printf("\n");
}

void test(CHAR *filespec)
{
INT32 handle;
struct _finddata_t data;

    printf("filespec = '%s'\n", filespec);
    if ((handle = util_findfirst(filespec, &data)) < 0) {
        perror("util_findfirst");
        exit(1);
    }
    printf("handle = %d\n", handle);

    do {
        pinfo(&data);
    } while (util_findnext(handle, &data) == 0);
}

main(int argc, char **argv)
{
int i;
    for (i = 1; i < argc; i++) test((CHAR *) argv[i]);
}
#endif /* DEBUG_TEST */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
