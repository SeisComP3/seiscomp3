SET(RECORDS_SOURCES
	shrecord.cpp
	sacrecord.cpp
	binaryrecord.cpp
)

SET(RECORDS_HEADERS
	shrecord.h
	sacrecord.h
	binaryrecord.h
)

IF (MSEED_FOUND)
	SET(RECORDS_SOURCES ${RECORDS_SOURCES} mseedrecord.cpp)
	SET(RECORDS_HEADERS ${RECORDS_HEADERS} mseedrecord.h)
ENDIF (MSEED_FOUND)

SC_SETUP_LIB_SUBDIR(RECORDS)
