# - Find the native HDF5 includes and library
#

# This module defines
#  HDF5_INCLUDE_DIR, where to find fftw3.h, etc.
#  HDF5_LIBRARIES, the libraries to link against to use fftw3.
#  HDF5_DEFINITIONS - You should ADD_DEFINITONS(${HDF5_DEFINITIONS}) before compiling code that includes hdf5 library files.
#  HDF5_FOUND, If false, do not try to use hdf5.

SET(HDF5_FOUND "NO")

FIND_PATH(HDF5_INCLUDE_DIR hdf5.h
    /usr/include/
    /usr/local/include/
)

SET(HDF5_NAMES ${HDF5_NAMES} hdf5)
FIND_LIBRARY(HDF5_LIBRARY
    NAMES ${HDF5_NAMES}
)

IF (HDF5_LIBRARY AND HDF5_INCLUDE_DIR)
    SET(HDF5_LIBRARIES ${HDF5_LIBRARY})
    SET(HDF5_FOUND "YES")
ENDIF (HDF5_LIBRARY AND HDF5_INCLUDE_DIR)

IF (HDF5_FOUND)
    IF (NOT HDF5_FIND_QUIETLY)
	MESSAGE(STATUS "Found hdf5: ${HDF5_LIBRARY}")
    ENDIF (NOT HDF5_FIND_QUIETLY)
    MARK_AS_ADVANCED(HDF5_INCLUDE_DIR HDF5_LIBRARIES HDF5_LIBRARY)
ELSE (HDF5_FOUND)
    IF (HDF5_FIND_REQUIRED)
	MESSAGE(FATAL_ERROR "Could not find hdf5 library")
    ENDIF (HDF5_FIND_REQUIRED)
ENDIF (HDF5_FOUND)
