/***************************************************************************** 
 * my_printf.cc
 *
 * A hack to redirect messages from Titan routines to our log streams
 * Titan code must be compiled with -Dprintf=my_printf -Dfprintf=my_fprintf
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <cstdio>
#include <cstdarg>
#include <cstring>

#include <string>
#include <iostream>

#include "fs_plugin.h"
#include "cppstreams.h"

extern "C" {
int my_printf(const char *fmt, ...);
int my_fprintf(FILE *stream, const char *fmt, ...);
}

namespace {

using namespace std;
using namespace CPPStreams;

const int LOGMSGLEN = 150;

int my_vprintf(ostream &ostr, const char *fmt, va_list ap)
  {
    char buf[LOGMSGLEN];
    const char* p = buf;
    int len = 0;
    int r = vsnprintf(buf, LOGMSGLEN, fmt, ap);

    while(*p)
      {
        if((len = strcspn(p, "\n")) > 0)
            ostr << string(p, len);

        if(p[len] == '\n')
          {
            ostr << endl;
            ++p;
          }

        p += len;
      }
    
    return r;
  }

} // unnamed namespace

using namespace SeedlinkPlugin;

int my_printf(const char *fmt, ...)
  {
    if(verbosity == 0) return 0;

    va_list argptr;
    va_start(argptr, fmt);
    int r = my_vprintf(logs(LOG_NOTICE), fmt, argptr);
    va_end(argptr);
    return r;
  }

int my_fprintf(FILE *stream, const char *fmt, ...)
  {
    if(verbosity == 0 && stream == stdout) return 0;
    
    int r;
    va_list argptr;
    va_start(argptr, fmt);
    
    if(stream == stdout) r = my_vprintf(logs(LOG_NOTICE), fmt, argptr);
    else if(stream == stderr) r = my_vprintf(logs(LOG_WARNING), fmt, argptr);
    else r = vfprintf(stream, fmt, argptr);
      
    va_end(argptr);
    return r;
  }

