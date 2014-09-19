#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define N(expr) do { if((expr) < 0) { \
  fprintf(stderr, __FILE__ ":%d: %s\n", __LINE__, strerror(errno)); \
  exit(1); } } while(0)

const char *const usage =
  "Usage: waitlock <timeout> <lock_file>\n";

static void alarm_handler(int sig)
  {
    exit(2);
  }

int main(int argc, char **argv)
  {
    int fd, to;
    char *tail;
    struct sigaction sa;
    struct flock lock;
    struct stat stbuf;
  
    if(argc != 3)
      {
        printf("%s", usage);
        return 1;
      }

    to = strtoul(argv[1], &tail, 10);
    if(*tail)
      {
        printf("%s", usage);
        return 1;
      }
        
    if((fd = open(argv[2], O_WRONLY)) < 0)
      {
        if(stat(argv[2], &stbuf) < 0 && errno != ENOENT)
          {
            fprintf(stderr, "could not open file '%s': %s\n",
              argv[2], strerror(errno));
            return 1;
          }

        return 0;
      }

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));
    alarm(to);
    
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
  
    if(fcntl(fd, F_SETLKW, &lock) < 0)
      {
        if(errno == EACCES || errno == EAGAIN) return 1;
        
        fprintf(stderr, "could not lock file '%s': %s\n",
          argv[1], strerror(errno));
        return 1;
      }

    return 0;
  }

