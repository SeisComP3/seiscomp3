# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Modified by Jan Becker, <jabe@gfz-potsdam.de>
#  * added REQUIRED and QUIETLY check
#  * search for mysql/mysql.h instead of just mysql.h 

# Set Homebrew path for MYSQL
if(MACOSX)
	SET(MYSQL_INCLUDE_DIR /usr/local/include/mysql/)
endif(MACOSX)
	
if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
   set(MYSQL_FOUND TRUE)

else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

  find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
      $ENV{ProgramFiles}/MySQL/*/include
      $ENV{SystemDrive}/MySQL/*/include
      )

  find_library(MYSQL_LIBRARIES NAMES mysqlclient
      PATHS
      /usr/lib/mysql
      /usr/local/lib/mysql
      $ENV{ProgramFiles}/MySQL/*/lib/opt
      $ENV{SystemDrive}/MySQL/*/include
      )

  if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND TRUE)
    if(NOT MySQL_FIND_QUIETLY)
	message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
    endif(NOT MySQL_FIND_QUIETLY)
  else(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
    set(MYSQL_FOUND FALSE)
    if(MySQL_FIND_REQUIRED)
	message(FATAL_ERROR "MySQL not found.")
    else(MySQL_FIND_REQUIRED)
	if(NOT MySQL_FIND_QUIETLY)
	    message(STATUS "MySQL not found.")
	endif(NOT MySQL_FIND_QUIETLY)
    endif(MySQL_FIND_REQUIRED)
  endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

  mark_as_advanced(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)

endif(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
