/***************************************************************************** 
 * diag.h
 *
 * Macros for diagnostic purposes
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef DIAG_H
#define DIAG_H

#include <iostream>
#include <cstdlib>

#define internal_check(expr) do { if(!(expr)) { \
  std::clog << "internal error (" __FILE__  ":" << __LINE__ << "); " \
  "failed expression was: " #expr << endl; std::exit(1); } } while(0)

#define N(expr) do { if((expr) < 0) { \
  std::clog << __FILE__ ":" << __LINE__ << ": " << strerror(errno) << endl; \
  std::exit(1); } } while(0)

#define P(expr) do { if((expr) == NULL) { \
  std::clog << __FILE__ ":" << __LINE__ << ": " << strerror(errno) << endl; \
  std::exit(1); } } while(0)

#ifdef DEBUG_MESSAGES
#define DEBUG_MSG(s) do { logs(LOG_DEBUG) << s; } while(0)
#else
#define DEBUG_MSG(s) do { } while(0)
#endif

#endif // DIAG_H

