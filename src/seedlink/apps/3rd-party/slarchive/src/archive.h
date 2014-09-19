
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <libslink.h>

#include "dsarchive.h"

#define ARCH  1
#define SDS   2
#define BUD   3
#define DLOG  4

extern int  archstream_proc (DataStream *datastream, SLMSrecord *msr,
			     int reclen);

#endif
