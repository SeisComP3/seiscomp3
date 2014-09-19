#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lockutil.h"

#define N(arg) do { if((arg) < 0) { perror(#arg); exit(1); } } while(0)

static const char *const usage_string = 
    "Usage: run_with_lock [-q] <lock_file> <command> <arg1>...\n";

static int pid;

static void pass_signal(int sig)
  {
    kill(pid, sig);
  }

static void nexec(const char *file, char *const argv[])
  {
    do
      {
        if(execvp(file,argv) < 0)
        {
          perror(file); exit(1);
        }
      }
     while(0);
  }


int main(int argc, char **argv)
  {
    int quiet_mode;
    int fd;
    struct sigaction sa, sa_int_save, sa_term_save, sa_hup_save;
    sigset_t oldmask, newmask;

    if(argc < 3)
      {
        fputs(usage_string, stderr);
        return 1;
      }

    quiet_mode = 0;
    if(strcmp("-q", argv[1]) == 0)
      {
        if(argc < 4)
          {
            fputs(usage_string, stderr);
            return 1;
          }
        quiet_mode = 1;
      }

    N(sigemptyset(&newmask));
    N(sigaddset(&newmask, SIGINT));
    N(sigaddset(&newmask, SIGTERM));
    N(sigaddset(&newmask, SIGHUP));
    N(sigprocmask(SIG_BLOCK, &newmask, &oldmask));
    
    if((fd = acquire_lock(argv[quiet_mode+1])) < 0) return -1;

    N(pid = fork());
    if(pid > 0) 
      {
        int wstat;
        
        N(sigemptyset(&sa.sa_mask));
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = pass_signal;
        N(sigaction(SIGINT, &sa, &sa_int_save));
        N(sigaction(SIGTERM, &sa, &sa_term_save));
        N(sigaction(SIGHUP, &sa, &sa_hup_save));
        N(sigprocmask(SIG_UNBLOCK, &newmask, NULL));
        
        N(waitpid(pid, &wstat, 0));

        N(sigprocmask(SIG_BLOCK, &newmask, NULL));
        N(sigaction(SIGINT, &sa_int_save, NULL));
        N(sigaction(SIGTERM, &sa_term_save, NULL));
        N(sigaction(SIGHUP, &sa_hup_save, NULL));
        N(sigprocmask(SIG_SETMASK, &oldmask, NULL));
        
        release_lock(fd);
        
        if(WIFSIGNALED(wstat))
          {
            if (!quiet_mode)
              fprintf(stderr, "run_with_lock: process \"%s\" (%d) was terminated by signal %d\n",
                argv[quiet_mode+2], pid, WTERMSIG(wstat));

            kill(getpid(), WTERMSIG(wstat));
            return 1;
          }
        else if(WIFEXITED(wstat) && WEXITSTATUS(wstat) != 0)
          {
            if (!quiet_mode)
              fprintf(stderr, "run_with_lock: process \"%s\" (%d) exited with error status %d\n",
                argv[quiet_mode+2], pid, WEXITSTATUS(wstat));

            return WEXITSTATUS(wstat);
          }

        return 0;
      }

    N(sigprocmask(SIG_SETMASK, &oldmask, NULL));
    nexec(argv[quiet_mode+2], argv + quiet_mode + 2);

    return 0; /* not reached */
  }

