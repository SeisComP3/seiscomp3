INCLUDE(flex)
FIND_FLEX()

SET(SLUTILS_HEADERS
	big-endian.h
	byteorder_swap.h
	little-endian.h
	confattr.h
	confbase.h
	conf_ini.h
	conf_xml.h
	cppstreams.h
	descriptor.h
	diag.h
	utils.h
	qtime.h
)

SET(SLUTILS_SOURCES
	utils.cc
	qtime.cc
	conf_xml.cc
	confbase.cc
)

IF (CMAKE_COMPILER_IS_GNUCC)
	REMOVE_DEFINITIONS(-pedantic)
ENDIF (CMAKE_COMPILER_IS_GNUCC)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(.)

ADD_FLEX_FILES(SLUTILS_SOURCES conf_ini.l)
ADD_LIBRARY(slutils STATIC ${SLUTILS_SOURCES})
