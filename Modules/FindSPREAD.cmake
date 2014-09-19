# - Find the native SPREAD includes and library
#

# This module defines
#  SPREAD_INCLUDE_DIR, where to find spread.h, etc.
#  SPREAD_LIBRARIES, the libraries to link against to use spread.
#  SPREAD_DEFINITIONS, You should ADD_DEFINITONS(${SPREAD_DEFINITIONS}) before compiling code that includes spread library files.
#  SPREAD_FOUND, If false, do not try to use fftw3.
#  SPREAD_VERSION, The found spread version

INCLUDE(CheckFunctionExists)

SET(SPREAD_FOUND "NO")

FIND_PATH(SPREAD_INCLUDE_DIR sp.h)

SET(SPREAD_NAMES ${SPREAD_NAMES} tspread-core tspread)
FIND_LIBRARY(SPREAD_LIBRARY
	NAMES ${SPREAD_NAMES}
)

SET(SPREAD_VERSION_CHECK "
#include <stdio.h>
#include <string.h>
#include <sp.h>

int main(int argc, char** argv) {
#ifdef SPREAD_REQUESTED_VERSION
  int spread_major_version = 0, spread_minor_version = 0, spread_patch_version = 0;
  int major = 0, minor = 0, patch = 0;

  spread_major_version = SPREAD_VERSION >> 24;
  spread_minor_version = (SPREAD_VERSION >> 16) & ((0x01 << 8)-1);
  spread_patch_version = SPREAD_VERSION & ((0x01 << 16)-1);  
  
  if ( sscanf(SPREAD_REQUESTED_VERSION, \"%d.%d.%d\", &major, &minor, &patch) < 1 ) {
    /* Bad version string */
    return 2;
  }
  
  printf(\"@SPREAD_VERSION=%d.%d.%d@\", spread_major_version, spread_minor_version, spread_patch_version);

  /* Test that the library is greater than our minimum version */
  if ( (spread_major_version > major) ||
       ((spread_major_version == major) && (spread_minor_version > minor)) ||
       ((spread_major_version == major) && (spread_minor_version == minor) && (spread_patch_version >= patch)) )
        /* Correct spread version found */
        return 0;

  /* Version too old */
  return 1;
#endif
  /* SPREAD_REQUESTED_VERSION has not been defined */
  return 3;
}
"
)


IF (SPREAD_LIBRARY AND SPREAD_INCLUDE_DIR)
	SET(SPREAD_LIBRARIES ${SPREAD_LIBRARY})
	IF (WIN32)
		SET(SPREAD_LIBRARIES ${SPREAD_LIBRARIES} ws2_32)
	ENDIF (WIN32)
	
	SET(CMAKE_REQUIRED_LIBRARIES ${SPREAD_LIBRARIES})
	SET(CMAKE_REQUIRED_INCLUDES ${SPREAD_INCLUDE_DIR})
	CHECK_FUNCTION_EXISTS(SP_connect SPREAD_FUNCTION_SP_CONNECT)

	IF (SPREAD_FUNCTION_SP_CONNECT)

		IF (SPREAD_REQUESTED_VERSION)
	
			FILE(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/spread/${CMAKE_FILES_DIRECTORY}/CMakeTmp/spread_version.c" "${SPREAD_VERSION_CHECK}")

			IF (${CMAKE_MAJOR_VERSION} LESS 2 OR ${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} LESS 5)
				SET(SPREAD_REQUESTED_VERSION_DEF "\\\\\"${SPREAD_REQUESTED_VERSION}\\\\\"")
			ELSE (${CMAKE_MAJOR_VERSION} LESS 2 OR ${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} LESS 5)
				SET(SPREAD_REQUESTED_VERSION_DEF "\\\"${SPREAD_REQUESTED_VERSION}\\\"")
			ENDIF (${CMAKE_MAJOR_VERSION} LESS 2 OR ${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} LESS 5)

			TRY_RUN(
				SPREAD_VERSION_RESULT
				SPREAD_VERSION_COMPILE_RESULT
				${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/spread
				${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/spread/${CMAKE_FILES_DIRECTORY}/CMakeTmp/spread_version.c
				CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${SPREAD_INCLUDE_DIR}"
				COMPILE_DEFINITIONS "-DSPREAD_REQUESTED_VERSION=${SPREAD_REQUESTED_VERSION_DEF}"
				OUTPUT_VARIABLE SPREAD_VERSION_OUTPUT
			)

			IF (NOT SPREAD_VERSION_COMPILE_RESULT)
				SET(SPREAD_ERROR "Could not check spread version")
				SET(SPREAD_FOUND "NO")
			ELSE (NOT SPREAD_VERSION_COMPILE_RESULT)
				STRING(
					REGEX REPLACE
					".*@SPREAD_VERSION=(.*)@.*"
					"\\1"
					SPREAD_VERSION
					${SPREAD_VERSION_OUTPUT}
				)
	
				IF (NOT SPREAD_FIND_QUIETLY)
					MESSAGE(STATUS "Checking for spread version ${SPREAD_REQUESTED_VERSION}, found version ${SPREAD_VERSION}")
				ENDIF (NOT SPREAD_FIND_QUIETLY)
	
				IF (${SPREAD_VERSION_RESULT} EQUAL 0)
					SET(SPREAD_FOUND "YES")
				ELSE (${SPREAD_VERSION_RESULT} EQUAL 0)
					SET(SPREAD_ERROR "Could not find required spread version")
				ENDIF (${SPREAD_VERSION_RESULT} EQUAL 0)
			ENDIF (NOT SPREAD_VERSION_COMPILE_RESULT)
	
		ELSE (SPREAD_REQUESTED_VERSION)
			SET(SPREAD_FOUND "YES")
		ENDIF (SPREAD_REQUESTED_VERSION)

	ELSE (SPREAD_FUNCTION_SP_CONNECT)
		SET(SPREAD_ERROR "Could not find needed symbols in spread")
	ENDIF (SPREAD_FUNCTION_SP_CONNECT)

ELSE (SPREAD_LIBRARY AND SPREAD_INCLUDE_DIR)
	IF (NOT SPREAD_LIBRARY)
		SET(SPREAD_ERROR "Could not find spread library")
	ELSE (NOT SPREAD_LIBRARY)
		SET(SPREAD_ERROR "Could not find spread header")
	ENDIF (NOT SPREAD_LIBRARY)
ENDIF (SPREAD_LIBRARY AND SPREAD_INCLUDE_DIR)

IF (SPREAD_FOUND)
	IF (NOT SPREAD_FIND_QUIETLY)
		MESSAGE(STATUS "Found spread: ${SPREAD_LIBRARY}")
	ENDIF (NOT SPREAD_FIND_QUIETLY)
	MARK_AS_ADVANCED(SPREAD_INCLUDE_DIR SPREAD_LIBRARIES SPREAD_LIBRARY)
ELSE (SPREAD_FOUND)
	IF (SPREAD_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR ${SPREAD_ERROR})
	ELSE (SPREAD_FIND_REQUIRED)
		IF (NOT SPREAD_FIND_QUIETLY)
			MESSAGE(STATUS ${SPREAD_ERROR})
		ENDIF (NOT SPREAD_FIND_QUIETLY)
	ENDIF (SPREAD_FIND_REQUIRED)
ENDIF (SPREAD_FOUND)
