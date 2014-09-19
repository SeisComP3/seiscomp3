# - Find the GNU Compact Disc Input and Control Library 'cdio' includes and library
#

# This module defines
#  CDIO_INCLUDE_DIR, where to find iso9660.hpp, etc.
#  CDIO_LIBRARIES, the libraries to link against.
#  CDIO_FOUND, If false, do not try to use cdio.

SET(CDIO_FOUND FALSE)

FIND_PATH(CDIO_INCLUDE_DIR iso9660.hpp
    /usr/include/cdio++
    /usr/local/include/cdio++
)

FIND_LIBRARY(CDIO_C_LIB iso9660
    /usr/lib
    /usr/local/lib
)

FIND_LIBRARY(CDIO_C++_LIB iso9660++
    /usr/lib
    /usr/local/lib
)

SET(CDIO_LIBRARIES ${CDIO_C_LIB} ${CDIO_C++_LIB})

IF (CDIO_INCLUDE_DIR AND CDIO_LIBRARIES)
    SET(CDIO_FOUND TRUE)
ENDIF (CDIO_INCLUDE_DIR AND CDIO_LIBRARIES)

IF (CDIO_FOUND)
    IF (NOT Cdio_FIND_QUIETLY)
	MESSAGE(STATUS "Found cdio: ${CDIO_INCLUDE_DIR} ${CDIO_LIBRARIES}")
    ENDIF (NOT Cdio_FIND_QUIETLY)
ELSE (CDIO_FOUND)
    IF (Cdio_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find cdio library")
    ENDIF (Cdio_FIND_REQUIRED)
ENDIF (CDIO_FOUND)
