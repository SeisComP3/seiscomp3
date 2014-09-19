# Find the ncurses includes and library
#
# NCURSES_INCLUDE_DIR - Where to find ncurses.h
# NCURSES_LIBRARY    - Library to link against.
# NCURSES_FOUND      - Do not attempt to use if "no" or undefined.


FIND_PATH(NCURSES_INCLUDE_DIR ncurses.h
	/usr/include 
	/usr/local/include
	/sw/include
)

FIND_LIBRARY(NCURSES_LIBRARY ncurses
	/usr/lib 
	/usr/local/lib
	/sw/lib
)

IF (NCURSES_INCLUDE_DIR AND NCURSES_LIBRARY)
   SET(NCURSES_FOUND TRUE)
ENDIF (NCURSES_INCLUDE_DIR AND NCURSES_LIBRARY)


IF (NCURSES_FOUND)
   IF (NOT Ncurses_FIND_QUIETLY)
      MESSAGE(STATUS "Found ncurses: ${NCURSES_INCLUDE_DIR} ${NCURSES_LIBRARY}")
   ENDIF (NOT Ncurses_FIND_QUIETLY)
ELSE (NCURSES_FOUND)
   IF (Ncurses_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ncurses")
   ENDIF (Ncurses_FIND_REQUIRED)
ENDIF (NCURSES_FOUND)
