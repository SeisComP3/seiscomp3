# - Find the Boost includes and libraries.
# The following variables are set if Boost is found.  If Boost is not
# found, BOOST_FOUND is set to false.
#  BOOST_FOUND       - True when the Boost include directory is found.

#  BOOST_FILESYSTEM_FOUND - True when Boost::FileSystem is found.
#  BOOST_PROGRAM_OPTIONS_FOUND - True when Boost::ProgramOptions is found.
#  BOOST_THREAD_FOUND - True when Boost::Thread is found.
#  BOOST_IOSTREAMS_FOUND - True when Boost::IOStreams is found.
#  BOOST_SERIALIZATION_FOUND - True when Boost::Serialization is found.
#  BOOST_SIGNALS_FOUND - True when Boost::Signals is found.
#  BOOST_DATE_TIME_FOUND - True when Boost::DateTime is found.
#  BOOST_REGEX_FOUND - True when Boost::Regex is found.
#  BOOST_UNIT_TEST_FRAMEWORK_FOUND - True when Boost::UnitTest is found.

#  BOOST_INCLUDE_DIR - the path to where the boost include files are.

#  BOOST_FILESYSTEM_LIB - The Boost::FileSystem library flag.
#  BOOST_PROGRAM_OPTIONS_LIB - The Boost::ProgramOptions library flag.
#  BOOST_THREAD_LIB - The Boost::Thread library flag.
#  BOOST_IOSTREAMS_LIB - The Boost::IOStreams library flag.
#  BOOST_SERIALIZATION_LIB - The Boost::Serialization library flag.
#  BOOST_SIGNALS_LIB - The Boost::Signals library flag.
#  BOOST_DATE_TIME_LIB - The Boost::DateTime library flag.
#  BOOST_REGEX_LIB - The Boost::Regex library flag.
#  BOOST_UNIT_TEST_FRAMEWORK_LIB - The Boost::UnitTest library flag.

#  BOOST_LIB_DIAGNOSTIC_DEFINITIONS - Only set if using Windows.

# ----------------------------------------------------------------------------
# If you have installed Boost in a non-standard location or you have
# just staged the boost files using bjam then you have three
# options. In the following comments, it is assumed that <Your Path>
# points to the root directory of the include directory of Boost. e.g
# If you have put boost in C:\development\Boost then <Your Path> is
# "C:/development/Boost" and in this directory there will be two
# directories called "include" and "lib".
# 1) After CMake runs, set Boost_INCLUDE_DIR to <Your Path>/include/boost<-version>
# 2) Use CMAKE_INCLUDE_PATH to set a path to <Your Path>/include. This will allow FIND_PATH()
#    to locate Boost_INCLUDE_DIR by utilizing the PATH_SUFFIXES option. e.g.
#    SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "<Your Path>/include")
# 3) Set an environment variable called ${BOOST_ROOT} that points to the root of where you have
#    installed Boost, e.g. <Your Path>. It is assumed that there is at least a subdirectory called
#    include in this path.
#
# Note:
#  1) If you are just using the boost headers, then you do not need to use
#     Boost_LIBRARY_DIRS in your CMakeLists.txt file.
#  2) If Boost has not been installed, then when setting Boost_LIBRARY_DIRS
#     the script will look for /lib first and, if this fails, then for /stage/lib.
#
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Boost
# FIND_PACKAGE(BoostLibs)
# ...
# INCLUDE_DIRECTORIES(${BOOST_INCLUDE_DIRS})
#
# In Windows, we make the assumption that, if the Boost files are installed, the default directory
# will be C:\boost.

IF(WIN32)
  # In windows, automatic linking is performed, so you do not have to specify the libraries.
  # If you are linking to a dynamic runtime, then you can choose to link to either a static or a
  # dynamic Boost library, the default is to do a static link.  You can alter this for a specific
  # library "whatever" by defining BOOST_WHATEVER_DYN_LINK to force Boost library "whatever" to
  # be linked dynamically.  Alternatively you can force all Boost libraries to dynamic link by
  # defining BOOST_ALL_DYN_LINK.

  # This feature can be disabled for Boost library "whatever" by defining BOOST_WHATEVER_NO_LIB,
  # or for all of Boost by defining BOOST_ALL_NO_LIB.

  # If you want to observe which libraries are being linked against then defining
  # BOOST_LIB_DIAGNOSTIC will cause the auto-linking code to emit a #pragma message each time
  # a library is selected for linking.
  SET(BOOST_LIB_DIAGNOSTIC_DEFINITIONS "-DBOOST_LIB_DIAGNOSTIC")
ENDIF(WIN32)


SET(BOOST_INCLUDE_PATH_DESCRIPTION "directory containing the boost include files. E.g /usr/local/include/boost-1_33_1 or c:\\boost\\include\\boost-1_33_1")

SET(BOOST_DIR_MESSAGE "Set the Boost_INCLUDE_DIR cmake cache entry to the ${BOOST_INCLUDE_PATH_DESCRIPTION}")

SET(BOOST_DIR_SEARCH $ENV{BOOST_ROOT})
IF(BOOST_DIR_SEARCH)
  FILE(TO_CMAKE_PATH ${BOOST_DIR_SEARCH} BOOST_DIR_SEARCH)
  SET(BOOST_DIR_SEARCH ${BOOST_DIR_SEARCH}/include)
ENDIF(BOOST_DIR_SEARCH)

IF(WIN32)
  SET(BOOST_DIR_SEARCH
    ${BOOST_DIR_SEARCH}
    C:/boost/include
    D:/boost/include
  )
ENDIF(WIN32)

# Add in some path suffixes. These will have to be updated whenever a new Boost version comes out.
SET(SUFFIX_FOR_PATH
 boost-1_34_1
 boost-1_34_0
 boost-1_33_1
 boost-1_33_0
)

#
# Look for an installation.
#
FIND_PATH(BOOST_INCLUDE_DIR NAMES boost/config.hpp PATH_SUFFIXES ${SUFFIX_FOR_PATH} PATHS

  # Look in other places.
  ${BOOST_DIR_SEARCH}

  # Help the user find it if we cannot.
  DOC "The ${BOOST_INCLUDE_PATH_DESCRIPTION}"
)

# Assume we didn't find it.
SET(BOOST_FOUND 0)


MACRO(FIND_BOOST_LIBRARY _target _name)
    FIND_LIBRARY(
	BOOST_${_target}_LIB
	NAMES boost_${_name}
	PATHS ${BOOST_LIBRARY_DIR}
    )

    SET(name_postfixes
	-mt
	-s
	-mt-s
	-mgw
	-mgw-mt
	-mgw-s
	-mgw-mt-s
    )

    FOREACH (name_postfix ${name_postfixes})
	FIND_LIBRARY(
	    BOOST_${_target}_LIB
	    NAMES boost_${_name}${name_postfix}
	    PATHS ${BOOST_LIBRARY_DIR}
	)
    ENDFOREACH (name_postfix ${name_postfixes})

    IF (${BOOST_${_target}_LIB} STREQUAL BOOST_${_target}_LIB-NOTFOUND)
	SET(${FOUND_BOOST_${_target}} "NO")
    ELSE (${BOOST_${_target}_LIB} STREQUAL BOOST_${_target}_LIB-NOTFOUND)
	SET(FOUND_BOOST_${_target} "YES")
    ENDIF (${BOOST_${_target}_LIB} STREQUAL BOOST_${_target}_LIB-NOTFOUND)

    MESSAGE(STATUS "Checking for boost_${_name}...${FOUND_BOOST_${_target}}")
ENDMACRO(FIND_BOOST_LIBRARY)


# Now try to get the include and library path.
IF(BOOST_INCLUDE_DIR)

  # Look for the boost library path.
  # Note that the user may not have installed any libraries
  # so it is quite possible the BOOST_LIBRARY_PATH may not exist.
  SET(BOOST_LIBRARY_DIR ${BOOST_INCLUDE_DIR})

  IF("${BOOST_LIBRARY_DIR}" MATCHES "boost-[0-9]+")
    GET_FILENAME_COMPONENT(BOOST_LIBRARY_DIR ${BOOST_LIBRARY_DIR} PATH)
  ENDIF ("${BOOST_LIBRARY_DIR}" MATCHES "boost-[0-9]+")

  IF("${BOOST_LIBRARY_DIR}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(BOOST_LIBRARY_DIR ${BOOST_LIBRARY_DIR} PATH)
  ENDIF("${BOOST_LIBRARY_DIR}" MATCHES "/include$")

  IF(EXISTS "${BOOST_LIBRARY_DIR}/lib")
    SET (BOOST_LIBRARY_DIR ${BOOST_LIBRARY_DIR}/lib)
  ELSE(EXISTS "${BOOST_LIBRARY_DIR}/lib")
    IF(EXISTS "${BOOST_LIBRARY_DIR}/stage/lib")
      SET(BOOST_LIBRARY_DIR ${BOOST_LIBRARY_DIR}/stage/lib)
    ELSE(EXISTS "${BOOST_LIBRARY_DIR}/stage/lib")
      SET(BOOST_LIBRARY_DIR "")
    ENDIF(EXISTS "${BOOST_LIBRARY_DIR}/stage/lib")
  ENDIF(EXISTS "${BOOST_LIBRARY_DIR}/lib")

  IF(EXISTS "${BOOST_INCLUDE_DIR}")
    SET(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
    # We have found boost. It is possible that the user has not
    # compiled any libraries so we set BOOST_FOUND to be true here.
    SET(BOOST_FOUND 1)
    
    IF(NOT BOOST_FIND_QUIETLY)
	MESSAGE(STATUS "Checking for boost headers...${BOOST_INCLUDE_DIR}")
    ENDIF(NOT BOOST_FIND_QUIETLY)
  ENDIF(EXISTS "${BOOST_INCLUDE_DIR}")

  IF(BOOST_LIBRARY_DIR AND EXISTS "${BOOST_LIBRARY_DIR}")
    SET(BOOST_LIBRARY_DIRS ${BOOST_LIBRARY_DIR})
    
    FIND_BOOST_LIBRARY(FILESYSTEM filesystem)
    FIND_BOOST_LIBRARY(PROGRAM_OPTIONS program_options)
    FIND_BOOST_LIBRARY(THREAD thread)
    FIND_BOOST_LIBRARY(IOSTREAMS iostreams)
    FIND_BOOST_LIBRARY(SERIALIZATION serialization)
    FIND_BOOST_LIBRARY(SIGNALS signals)
    FIND_BOOST_LIBRARY(DATE_TIME date_time)
    FIND_BOOST_LIBRARY(REGEX regex)
    FIND_BOOST_LIBRARY(UNIT_TEST_FRAMEWORK unit_test_framework)

  ENDIF(BOOST_LIBRARY_DIR AND EXISTS "${BOOST_LIBRARY_DIR}")
ENDIF(BOOST_INCLUDE_DIR)

IF(NOT BOOST_FOUND)
  IF(NOT BOOST_FIND_QUIETLY)
    MESSAGE(STATUS "Boost was not found. ${BOOST_DIR_MESSAGE}")
  ELSE(NOT BOOST_FIND_QUIETLY)
    IF(BOOST_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Boost was not found. ${BOOST_DIR_MESSAGE}")
    ENDIF(BOOST_FIND_REQUIRED)
  ENDIF(NOT BOOST_FIND_QUIETLY)
ENDIF(NOT BOOST_FOUND)
