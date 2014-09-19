/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SC_LOGGING_COMMON_H__
#define __SC_LOGGING_COMMON_H__


#include <seiscomp3/core.h>

#define LOG_CONCAT2(A,B) A##B
#define LOG_CONCAT(A,B) LOG_CONCAT2(A,B)
#define LOG_STR(X) #X


/*! @file common.h
  @brief Configuration macros for log.
*/

/*
    Defined by configure if our compiler allows us to specify printf attributes
    on a function pointer..  Newer versions of GCC allow this, but older ones
    do not..
*/
#define HAVE_PRINTF_FP 1

/*
    Defined by configure if our compiler understands VARIADAC macros.
*/
#define C99_VARIADAC_MACROS 1
#define PREC99_VARIADAC_MACROS 1

#define SEISCOMP_TIME_TSC 1

/*
    We use __printf__ attribute to allow gcc to inspect printf style arguments
    and give warnings if the rDebug(), rWarning(), etc macros are misused.

    We use __builtin_expect on GCC 2.96 and above to allow optimization of
    publication activation check.  We tell the compiler that the branch is
    unlikely to occur, allowing GCC to push unecessary code out of the main
    path.
*/
#ifdef __GNUC__

# define PRINTF(FMT,X) __attribute__ (( __format__ ( __printf__, FMT, X)))
# define HAVE_PRINTF_ATTR 1
//# define SEISCOMP_SECTION __attribute__ (( section("SEISCOMP_DATA") ))
# define SEISCOMP_SECTION

#if __GNUC__ >= 3
# define expect(foo, bar) __builtin_expect((foo),bar)
#else
# define expect(foo, bar) (foo)
#endif


# define   likely(x)  __builtin_expect((x),1)
# define unlikely(x)  __builtin_expect((x),0)

#else

// Not using the gcc compiler, make the macros do nothing..  They are
// documented as the last instance of the macros..

/*! @def PRINTF(FMT,X)
  On GCC, this uses the compiler's __printf__ attribute to tell the compiler
  to treat certain arguments as printf formating options, which allows it to
  print warnings at compile time if the arguments do not match the format
  string.
  @internal
*/
# define PRINTF(FMT,X)
# define HAVE_PRINTF_ATTR 0
# define SEISCOMP_SECTION
/*!
*/
# define   likely(x)  (x)
/*! @def unlikely(x)
  Starting with GCC 2.96, we can tell the compiler that an if condition is
  likely or unlikely to occur, which allows the compiler to optimize for the
  normal case.
  @internal
*/
# define unlikely(x)  (x)

#endif

#if HAVE_PRINTF_FP
# define PRINTF_FP(FMT,X) PRINTF(FMT,X)
#else
# define PRINTF_FP(FMT,X)
#endif

/*! @def SEISCOMP_COMPONENT
    @brief Specifies build-time component, eg -DSEISCOMP_COMPONENT="component-name"

    Define SEISCOMP_COMPONENT as the name of the component being built.
    For example, as a compile flag,  -DSEISCOMP_COMPONENT="component-name"

    If SEISCOMP_COMPONENT is not specified, then it will be defined as "[unknown]"
*/
#ifndef SEISCOMP_COMPONENT
#  warning SEISCOMP_COMPONENT not defined - setting to UNKNOWN
#define SEISCOMP_COMPONENT "[unknown]"
#endif // SEISCOMP_COMPONENT not defined

// Use somewhat unique names (doesn't matter if they aren't as they are used in
// a private context, so the compiler will make them unique if it must)
#define _SCLOGID LOG_CONCAT(_rL_, __LINE__)

#endif
