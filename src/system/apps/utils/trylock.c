#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define N(expr) do { if((expr) < 0) { \
  fprintf(stderr, __FILE__ ":%d: %s\n", __LINE__, strerror(errno)); \
  exit(1); } } while(0)

const char *const usage =
  "Usage: trylock <lock_file>\n";

int main(int argc, char **argv)
  {
    int fd;
    struct flock lock;
    struct stat stbuf;
  
    if(argc != 2)
      {
        printf("%s", usage);
        return 1;
      }

    if((fd = open(argv[1], O_WRONLY)) < 0)
      {
        if(stat(argv[1], &stbuf) < 0 && errno != ENOENT)
          {
            fprintf(stderr, "could not open file '%s': %s\n",
              argv[1], strerror(errno));
            return 1;
          }

        return 0;
      }

    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(fd, F_SETLK, &lock) < 0)
      {
        if(errno == EACCES || errno == EAGAIN) return 1;
        
        fprintf(stderr, "could not lock file '%s': %s\n",
          argv[1], strerror(errno));
        return 1;
      }

    return 0;
  }

