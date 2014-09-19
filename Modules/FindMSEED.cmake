# - Find the native MSEED includes and library
#

# This module defines
#  MSEED_INCLUDE_DIR, where to find libmseed.h, etc.
#  MSEED_LIBRARIES, the libraries to link against to use libmseed.
#  MSEED_DEFINITIONS, You should ADD_DEFINITONS(${MSEED_DEFINITIONS}) before compiling code that includes libmseed library files.
#  MSEED_FOUND, If false, do not try to use fftw3.
#  MSEED_VERSION, The found libmseed version

INCLUDE(CheckFunctionExists)

SET(MSEED_FOUND "NO")
SET(MSEED_VERSION_RESULT -1)

FIND_PATH(MSEED_INCLUDE_DIR libmseed.h)

SET(MSEED_NAMES ${MSEED_NAMES} mseed)
FIND_LIBRARY(MSEED_LIBRARY
    NAMES ${MSEED_NAMES}
)

SET(MSEED_VERSION_CHECK "
#include <stdio.h>
#include <string.h>
#include <libmseed.h>

int main(int argc, char** argv) {
#ifdef MSEED_REQUESTED_VERSION
  int mseed_major_version = 0, mseed_minor_version = 0;
  int major = 0, minor = 0;

  sscanf(LIBMSEED_VERSION, \"%d.%d\", &mseed_major_version, &mseed_minor_version);
  
  if ( sscanf(MSEED_REQUESTED_VERSION, \"%d.%d\", &major, &minor) < 1 ) {
    /* Bad version string */
    return 2;
  }
  
  printf(\"@MSEED_VERSION=%d.%d@\", mseed_major_version, mseed_minor_version);

  /* Test that the library is greater than our minimum version */
  if ( (mseed_major_version > major) ||
       ((mseed_major_version == major) && (mseed_minor_version > minor)) ||
       ((mseed_major_version == major) && (mseed_minor_version == minor)) )
        /* Correct mseed version found */
        return 0;

  /* Version too old */
  return 1;
#endif
  /* MSEED_REQUESTED_VERSION has not been defined */
  return 3;
}
"
)

IF (MSEED_LIBRARY AND MSEED_INCLUDE_DIR)
    SET(MSEED_LIBRARIES ${MSEED_LIBRARY} -lm)
    
    SET(CMAKE_REQUIRED_LIBRARIES ${MSEED_LIBRARIES})
    SET(CMAKE_REQUIRED_INCLUDES ${MSEED_INCLUDE_DIR})
    CHECK_FUNCTION_EXISTS(msr_unpack MSEED_FUNCTION_MSR_UNPACK)

    IF (MSEED_FUNCTION_MSR_UNPACK)

	IF (MSEED_REQUESTED_VERSION)
	
	    FILE(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mseed/${CMAKE_FILES_DIRECTORY}/CMakeTmp/mseed_version.c" "${MSEED_VERSION_CHECK}")
	
	    INCLUDE_DIRECTORIES(${MSEED_INCLUDE_DIR})	    
	    
	    TRY_RUN(
		MSEED_VERSION_RESULT
		MSEED_VERSION_COMPILE_RESULT
		${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mseed
		${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/mseed/${CMAKE_FILES_DIRECTORY}/CMakeTmp/mseed_version.c
		COMPILE_DEFINITIONS "-DMSEED_REQUESTED_VERSION=\\\\\"${MSEED_REQUESTED_VERSION}\\\\\""
		OUTPUT_VARIABLE MSEED_VERSION_OUTPUT
	    )

	    STRING(
		REGEX REPLACE
		".*@MSEED_VERSION=(.*)@.*"
		"\\1"
		MSEED_VERSION
		${MSEED_VERSION_OUTPUT}
	    )

	    IF (NOT MSEED_FIND_QUIETLY)
		MESSAGE(STATUS "Checking for libmseed version ${MSEED_REQUESTED_VERSION}, found version ${MSEED_VERSION}")
	    ENDIF (NOT MSEED_FIND_QUIETLY)

	    IF (${MSEED_VERSION_RESULT} EQUAL 0)
		SET(MSEED_FOUND "YES")
	    ENDIF (${MSEED_VERSION_RESULT} EQUAL 0)
	
	ELSE (MSEED_REQUESTED_VERSION)
	    SET(MSEED_FOUND "YES")
	ENDIF (MSEED_REQUESTED_VERSION)

    ENDIF (MSEED_FUNCTION_MSR_UNPACK)
ENDIF (MSEED_LIBRARY AND MSEED_INCLUDE_DIR)

IF (MSEED_FOUND)
    IF (NOT MSEED_FIND_QUIETLY)
	MESSAGE(STATUS "Found libmseed: ${MSEED_LIBRARY}")
    ENDIF (NOT MSEED_FIND_QUIETLY)
    MARK_AS_ADVANCED(MSEED_INCLUDE_DIR MSEED_LIBRARIES MSEED_LIBRARY)
ELSE (MSEED_FOUND)
    IF (MSEED_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find libmseed")
    ELSE (MSEED_FIND_REQUIRED)
	IF (NOT MSEED_FIND_QUIETLY)
	    MESSAGE(STATUS "Could not find libmseed")
	ENDIF (NOT MSEED_FIND_QUIETLY)
	
    ENDIF (MSEED_FIND_REQUIRED)
ENDIF (MSEED_FOUND)
