SET(LOG_SOURCES
	log.cpp
	channel.cpp
	node.cpp
	publisher.cpp
	location.cpp
	init.cpp
	fd.cpp
	file.cpp
	filerotator.cpp
	output.cpp
)

SET(LOG_HEADERS
	common.h
	defs.h
	log.h
	channel.h
	node.h
	publisher.h
	publishloc.h
	location.h
	init.h
	output.h
	fd.h
	file.h
	filerotator.h
)

IF(NOT WIN32)
	SET(LOG_SOURCES ${LOG_SOURCES} syslog.cpp)
	SET(LOG_HEADERS ${LOG_HEADERS} syslog.h
	)
ENDIF(NOT WIN32)

SC_SETUP_LIB_SUBDIR(LOG)
