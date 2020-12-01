# Try to find numarray python package
# Once done this will define
#
# PYTHON_NUMPY_FOUND        - system has numarray development package and it should be used
# PYTHON_NUMPY_INCLUDE_DIR  - directory where the arrayobject.h header file can be found

IF(PYTHON_EXECUTABLE)
	FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/det_npp.py "from __future__ import print_function\ntry: import numpy; print(numpy.get_include())\nexcept: pass\n")
	EXEC_PROGRAM("${PYTHON_EXECUTABLE}"
		ARGS "\"${CMAKE_CURRENT_BINARY_DIR}/det_npp.py\""
		OUTPUT_VARIABLE NUMPY_PATH
	)
	FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/det_npp.py)
ENDIF(PYTHON_EXECUTABLE)

FIND_PATH(PYTHON_NUMPY_INCLUDE_DIR numpy/arrayobject.h
	"${NUMPY_PATH}/"
	DOC "Directory where the numpy/arrayobject.h header file can be found. This file is part of the numpy package"
)

IF(PYTHON_NUMPY_INCLUDE_DIR)
	SET (PYTHON_NUMPY_FOUND 1 CACHE INTERNAL "Python numpy development package is available")
ENDIF(PYTHON_NUMPY_INCLUDE_DIR)
