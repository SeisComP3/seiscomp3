IF(WIN32)
	SET(TYPE STATIC)
ELSE(WIN32)
	SET(TYPE SHARED)
ENDIF(WIN32)

ADD_LIBRARY(mseed ${TYPE}
	fileutils.c
	genutils.c
	gswap.c
	lmplatform.c
	logging.c
	lookup.c
	msrutils.c
	pack.c
	packdata.c
	traceutils.c
	unpack.c
	unpackdata.c
	selection.c
	tracelist.c
	parseutils.c
)

SET_TARGET_PROPERTIES(mseed PROPERTIES VERSION 2.17)

IF(NOT WIN32)
	INSTALL(TARGETS mseed
		DESTINATION ${SC3_PACKAGE_LIB_DIR}
	)
ENDIF(NOT WIN32)

INSTALL(
	FILES lmplatform.h libmseed.h
	DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}
)
