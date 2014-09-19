
#ifndef DSARCHIVE_H
#define DSARCHIVE_H

#include <stdio.h>
#include <time.h>
#include <libslink.h>
#include "uthash.h"

#define MAX_FILENAME_LEN 400
extern int DS_BUFSIZE;

typedef struct DataStreamGroup_s
{
  char   *defkey;
  int     filed;
  time_t  modtime;
  double  lastsample;
  char    futurecontprint;
  char    futureinitprint;
  char    filename[MAX_FILENAME_LEN];
  char   *buf;
  int     bp;
  UT_hash_handle hh;
}
DataStreamGroup;

typedef struct DataStream_s
{
  char   *path;
  char    archivetype;
  char    packettype;
  int     idletimeout;
  char    futurecontflag;
  int     futurecont;
  char    futureinitflag;
  int     futureinit;
  struct  DataStreamGroup_s *grouphash;
}
DataStream;

extern int ds_streamproc (DataStream *datastream, char *pathformat,
			  SLMSrecord *msr, int reclen);

#endif

