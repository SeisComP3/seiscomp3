# - Find PostgreSQL
# Find the PostgreSQL includes and client library
# This module defines
#  POSTGRESQL_INCLUDE_DIR, where to find POSTGRESQL.h
#  POSTGRESQL_LIBRARIES, the libraries needed to use POSTGRESQL.
#  POSTGRESQL_FOUND, If false, do not try to use PostgreSQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Modified by Jan Becker, <jabe@gfz-potsdam.de>
#  * added REQUIRED and QUIETLY check

if(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
   set(POSTGRESQL_FOUND TRUE)

else(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)

  find_path(POSTGRESQL_INCLUDE_DIR libpq-fe.h
      /usr/include/pgsql
      /usr/local/include/pgsql
      /usr/include/postgresql
      )

  find_library(POSTGRESQL_LIBRARIES NAMES pq
      PATHS
      /usr/lib
      /usr/local/lib
      /usr/lib/postgresql
      )

  if(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
    set(POSTGRESQL_FOUND TRUE)
    if (NOT PostgreSQL_FIND_QUIETLY)
	message(STATUS "Found PostgreSQL: ${POSTGRESQL_INCLUDE_DIR}, ${POSTGRESQL_LIBRARIES}")
    endif (NOT PostgreSQL_FIND_QUIETLY)
  else(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
    set(POSTGRESQL_FOUND FALSE)
    if (PostgreSQL_FIND_REQUIRED)
	message(FATAL_ERROR "PostgreSQL not found.")
    else (PostgreSQL_FIND_REQUIRED)
	if (NOT PostgreSQL_FIND_QUIETLY)
	    message(STATUS "PostgreSQL not found.")
	endif (NOT PostgreSQL_FIND_QUIETLY)
    endif (PostgreSQL_FIND_REQUIRED)
  endif(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)

  mark_as_advanced(POSTGRESQL_INCLUDE_DIR POSTGRESQL_LIBRARIES)

endif(POSTGRESQL_INCLUDE_DIR AND POSTGRESQL_LIBRARIES)
