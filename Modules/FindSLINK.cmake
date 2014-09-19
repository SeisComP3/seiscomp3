# - Find the native SLINK includes and library
#

# This module defines
#  SLINK_INCLUDE_DIR, where to find libmseed.h, etc.
#  SLINK_LIBRARIES, the libraries to link against to use libmseed.
#  SLINK_DEFINITIONS, You should ADD_DEFINITONS(${SLINK_DEFINITIONS}) before compiling code that includes libmseed library files.
#  SLINK_FOUND, If false, do not try to use fftw3.
#  SLINK_VERSION, The found libmseed version

INCLUDE(CheckFunctionExists)

SET(SLINK_FOUND "NO")
SET(SLINK_VERSION_RESULT -1)

FIND_PATH(SLINK_INCLUDE_DIR libslink.h)

SET(SLINK_NAMES ${SLINK_NAMES} slink)
FIND_LIBRARY(SLINK_LIBRARY
    NAMES ${SLINK_NAMES}
)

SET(SLINK_VERSION_CHECK "
#include <stdio.h>
#include <string.h>
#include <libslink.h>

int main(int argc, char** argv) {
#ifdef SLINK_REQUESTED_VERSION
  int slink_major_version = 0, slink_minor_version = 0;
  int major = 0, minor = 0;

  sscanf(LIBSLINK_VERSION, \"%d.%d\", &slink_major_version, &slink_minor_version);
  
  if ( sscanf(SLINK_REQUESTED_VERSION, \"%d.%d\", &major, &minor) < 1 ) {
    /* Bad version string */
    return 2;
  }
  
  printf(\"@SLINK_VERSION=%d.%d@\", slink_major_version, slink_minor_version);

  /* Test that the library is greater than our minimum version */
  if ( (slink_major_version > major) ||
       ((slink_major_version == major) && (slink_minor_version > minor)) ||
       ((slink_major_version == major) && (slink_minor_version == minor)) )
        /* Correct slink version found */
        return 0;

  /* Version too old */
  return 1;
#endif
  /* SLINK_REQUESTED_VERSION has not been defined */
  return 3;
}
"
)

IF (SLINK_LIBRARY AND SLINK_INCLUDE_DIR)
	SET(SLINK_LIBRARIES ${SLINK_LIBRARY})
	IF (WIN32)
		SET(SLINK_LIBRARIES ${SLINK_LIBRARIES} ws2_32)
	ENDIF (WIN32)

    SET(CMAKE_REQUIRED_LIBRARIES ${SLINK_LIBRARIES})
    SET(CMAKE_REQUIRED_INCLUDES ${SLINK_INCLUDE_DIR})
    CHECK_FUNCTION_EXISTS(sl_addstream SLINK_FUNCTION_SL_ADDSTREAM)

    IF (SLINK_FUNCTION_SL_ADDSTREAM)

	IF (SLINK_REQUESTED_VERSION)
	
	    FILE(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/slink/${CMAKE_FILES_DIRECTORY}/CMakeTmp/slink_version.c" "${SLINK_VERSION_CHECK}")
	
	    TRY_RUN(
		SLINK_VERSION_RESULT
		SLINK_VERSION_COMPILE_RESULT
		${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/slink
		${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/slink/${CMAKE_FILES_DIRECTORY}/CMakeTmp/slink_version.c
		COMPILE_DEFINITIONS "-DSLINK_REQUESTED_VERSION=\\\\\"${SLINK_REQUESTED_VERSION}\\\\\""
		OUTPUT_VARIABLE SLINK_VERSION_OUTPUT
	    )

	    STRING(
		REGEX REPLACE
		".*@SLINK_VERSION=(.*)@.*"
		"\\1"
		SLINK_VERSION
		${SLINK_VERSION_OUTPUT}
	    )

	    IF (NOT SLINK_FIND_QUIETLY)
		MESSAGE(STATUS "Checking for libslink version ${SLINK_REQUESTED_VERSION}, found version ${SLINK_VERSION}")
	    ENDIF (NOT SLINK_FIND_QUIETLY)

	    IF (${SLINK_VERSION_RESULT} EQUAL 0)
		SET(SLINK_FOUND "YES")
	    ENDIF (${SLINK_VERSION_RESULT} EQUAL 0)
	
	ELSE (SLINK_REQUESTED_VERSION)
	    SET(SLINK_FOUND "YES")
	ENDIF (SLINK_REQUESTED_VERSION)

    ENDIF (SLINK_FUNCTION_SL_ADDSTREAM)
    
ENDIF (SLINK_LIBRARY AND SLINK_INCLUDE_DIR)

IF (SLINK_FOUND)
    IF (NOT SLINK_FIND_QUIETLY)
	MESSAGE(STATUS "Found libslink: ${SLINK_LIBRARY}")
    ENDIF (NOT SLINK_FIND_QUIETLY)
    MARK_AS_ADVANCED(SLINK_INCLUDE_DIR SLINK_LIBRARIES SLINK_LIBRARY)
ELSE (SLINK_FOUND)
    IF (SLINK_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find libslink")
    ELSE (SLINK_FIND_REQUIRED)
	IF (NOT SLINK_FIND_QUIETLY)
	    MESSAGE(STATUS "Could not find libslink")
	ENDIF (NOT SLINK_FIND_QUIETLY)
	
    ENDIF (SLINK_FIND_REQUIRED)
ENDIF (SLINK_FOUND)
