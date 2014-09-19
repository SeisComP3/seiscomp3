# - Find the native F2C includes and library
#

# This module defines
#  F2C_INCLUDE_DIR, where to find f2c.h, etc.
#  F2C_LIBRARIES, the libraries to link against to use F2C.
#  F2C_DEFINITIONS - You should ADD_DEFINITONS(${F2C_DEFINITIONS}) before compiling code that includes F2C library files.
#  F2C_FOUND, If false, do not try to use f2c.
# also defined, but not for general use are
#  F2C_LIBRARY, where to find the f2c library.

SET(F2C_FOUND "NO")

FIND_PATH(F2C_INCLUDE_DIR f2c.h /sw/include )

SET(F2C_NAMES ${F2C_NAMES} f2c.a libf2c.a f2c.lib)
# Use FIND_FILE instead of FIND_LIBRARY because we need the
# static version of libf2c. The shared library produces linker
# errors.
FIND_FILE(
	F2C_LIBRARY
	NAMES ${F2C_NAMES}
	PATHS /usr/lib /usr/local/lib /opt /sw/lib/
)

# Look for libg2c as fallback. libg2c is part of the
# compat-g77 package.
IF (NOT F2C_LIBRARY)
	SET(G2C_NAMES ${G2C_NAMES} g2c libg2c)
	FIND_LIBRARY(F2C_LIBRARY NAMES ${G2C_NAMES})
ENDIF (NOT F2C_LIBRARY)

IF (F2C_LIBRARY AND F2C_INCLUDE_DIR)
	SET(F2C_LIBRARIES ${F2C_LIBRARY})
	SET(F2C_FOUND "YES")
ENDIF (F2C_LIBRARY AND F2C_INCLUDE_DIR)

IF (F2C_FOUND)
	IF (NOT F2C_FIND_QUIETLY)
		MESSAGE(STATUS "Found F2C: ${F2C_LIBRARY}")
	ENDIF (NOT F2C_FIND_QUIETLY)
	MARK_AS_ADVANCED(F2C_INCLUDE_DIR F2C_LIBRARIES F2C_LIBRARY)
ELSE (F2C_FOUND)
	IF (F2C_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find F2C library")
	ENDIF (F2C_FIND_REQUIRED)
ENDIF (F2C_FOUND)
