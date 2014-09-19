/***************************************************************************** 
 * mseedfifo_plugin.c
 *
 * SeedLink MSEED-FIFO plugin
 *
 * (c) 2006 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
#include <getopt.h>
#endif

#include "libslink.h"
#include "plugin.h"

#define MYVERSION "1.0 (2010.256)"

#ifndef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_LOCAL0
#endif

#define STATLEN   5
#define LOGMSGLEN 150

static const char *const ident_str = "SeedLink MSEED-FIFO Plugin v" MYVERSION;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
static const char *const opterr_message = "Try `%s --help' for more information\n";
static const char *const help_message = 
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is used as a signature in log messages\n"
    "\n"
    "-d, --fifo=PATH               Path to FIFO\n"
    "-v                            Increase verbosity level\n"
    "    --verbosity=LEVEL         Set verbosity level\n"
    "-D, --daemon                  Daemon mode\n"
    "-V, --version                 Show version information\n"
    "-h, --help                    Show this help message\n";
#else
static const char *const opterr_message = "Try `%s -h' for more information\n";
static const char *const help_message =
    "Usage: %s [options] plugin_name\n"
    "\n"
    "'plugin_name' is used as a signature in log messages\n"
    "\n"
    "-d PATH        Path to FIFO\n"
    "-v             Increase verbosity level\n"
    "-D             Daemon mode\n"
    "-V             Show version information\n"
    "-h             Show this help message\n";
#endif

static const char *plugin_name = NULL;
static int daemon_mode = 0, daemon_init = 0;
static int verbosity = 0;

static void log_print(char *msg)
  {
    if(daemon_init)
      {
        syslog(LOG_INFO, "%s", msg);
      }
    else
      {
        time_t t = time(NULL);
        char *p = asctime(localtime(&t));
        printf("%.*s - %s: %s\n", (int)(strlen(p) - 1), p, plugin_name, msg);
        fflush(stdout);
      }
  }

static int log_printf(const char *fmt, ...)
  {
    char buf[LOGMSGLEN];
    int r;
    va_list ap;

    va_start(ap, fmt);
    r = vsnprintf(buf, LOGMSGLEN, fmt, ap);
    va_end(ap);

    log_print(buf);

    return r;
  }

static ssize_t readn(int fd, void *vptr, size_t n)
  {
    ssize_t nread;
    size_t nleft = n;
    char* ptr = (char *)(vptr);
    
    while(nleft > 0)
      {
        if((nread = read(fd, ptr, nleft)) <= 0)
            return nread;

        nleft -= nread;
        ptr += nread;
      }

    return(n);
  }

static const char *get_progname(const char *argv0)
  {
    const char *p;

    if((p = strrchr(argv0, '/')) != NULL)
        return (p + 1);

    return argv0;
  }

int main(int argc, char **argv)
  {
    char *fifo_path = NULL;
    struct sl_fsdh_s *fsdh = NULL;
    struct stat st;
    char station[STATLEN+1];
    char buf[512];
    int fd, n;

#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    struct option ops[] = 
      {
        { "fifo",           required_argument, NULL, 'd' },
        { "verbosity",      required_argument, NULL, 'X' },
        { "daemon",         no_argument,       NULL, 'D' },
        { "version",        no_argument,       NULL, 'V' },
        { "help",           no_argument,       NULL, 'h' },
        { NULL }
      };
#endif

    int c;
#if defined(__GNU_LIBRARY__) || defined(__GLIBC__)
    while((c = getopt_long(argc, argv, "d:vDVh", ops, NULL)) != EOF)
#else
    while((c = getopt(argc, argv, "d:vDVh")) != EOF)
#endif
      {
        switch(c)
          {
          case 'd':
            if((fifo_path = strdup(optarg)) == NULL)
              {
                perror("strdup");
                exit(1);
              }
            
            break;
            
          case 'v': ++verbosity; break;
          case 'X': verbosity = atoi(optarg); break;
          case 'D': daemon_mode = 1; break;
          case 'V': fprintf(stdout, "%s\n", ident_str);
                    exit(0);
          case 'h': fprintf(stdout, help_message, get_progname(argv[0]));
                    exit(0);
          case '?': fprintf(stderr, opterr_message, get_progname(argv[0]));
                    exit(1);
          }
      }

    if(optind != argc - 1 || fifo_path == NULL)
      {
        fprintf(stderr, help_message, get_progname(argv[0]));
        exit(1);
      }

    if((plugin_name = strdup(argv[optind])) == NULL)
      {
        perror("strdup");
        exit(1);
      }
    
    if(daemon_mode)
      {
        log_printf("%s started", ident_str);
        log_printf("take a look into syslog files for more messages");
        openlog(plugin_name, 0, SYSLOG_FACILITY);
        daemon_init = 1;
      }
    
    log_printf("%s started", ident_str);

    if(stat(fifo_path, &st) < 0)
      {
        if(mkfifo(fifo_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0)
          {
            log_printf("cannot create %s: %s", fifo_path, strerror(errno));
            exit(1);
          }
      }
    else if(!S_ISFIFO(st.st_mode))
      {
        log_printf("%s exists, but is not a fifo", fifo_path);
        exit(1);
      }

    if((fd = open(fifo_path, O_RDONLY)) < 0)
      {
        log_printf("cannot open %s: %s", fifo_path, strerror(errno));
        exit(1);
      }

    while(1)
      {
        n = readn(fd, buf, 512);
        if(n < 0)
          {
            log_printf("error reading %s: %s", fifo_path, strerror(errno));
            exit(1);
          }
        else if(n == 0)
          {
            log_printf("EOF reading %s: %s", fifo_path, strerror(errno));
            break;
          }

        fsdh = (struct sl_fsdh_s *)buf;
        
        for(n = STATLEN; n > 0; --n)
            if(fsdh->station[n - 1] != ' ')
              {
                strncpy(station, fsdh->station, n);
                station[n] = 0;
                break;
              }

        int r = send_mseed(station, buf, 512);
        if(r < 0)
          {
            log_printf("error sending data to seedlink: %s", strerror(errno));
            exit(1);
          }
        else if(r == 0)
          {
            log_printf("error sending data to seedlink");
            exit(1);
          }
      }
    
    return 0;
  }

