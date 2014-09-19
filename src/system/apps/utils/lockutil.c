#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lockutil.h"

#define N(expr) do { if((expr) < 0) { \
  fprintf(stderr, __FILE__ ":%d: %s\n", __LINE__, strerror(errno)); \
  exit(1); } } while(0)

int acquire_lock(const char *lockfile)
  {
    int fd, val;
    char buf[10];
    struct flock lock;
  
    if((fd = open(lockfile, O_WRONLY | O_CREAT,
      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0)
      {
        fprintf(stderr, "could not open file '%s': %s\n",
          lockfile, strerror(errno));
        exit(1);
      }

    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(fd, F_SETLK, &lock) < 0)
      {
        close(fd);
        if(errno == EACCES || errno == EAGAIN) return -1;
        
        fprintf(stderr, "could not lock file '%s': %s\n",
          lockfile, strerror(errno));
        exit(1);
      }
  
    N(ftruncate(fd, 0));
    sprintf(buf, "%d\n", getpid());
    
    if(write(fd, buf, strlen(buf)) != (int) strlen(buf))
      {
        fprintf(stderr, "could not write to file '%s': %s\n",
          lockfile, strerror(errno));
        exit(1);
      }
        
    N((val = fcntl(fd,F_GETFD,0)));

    val |= FD_CLOEXEC;
    N(fcntl(fd, F_SETFD, val));

    return fd;
  }

void release_lock(int fd)
  {
    /* N(ftruncate(fd, 0)); */
    N(close(fd));
  }

