SET(AR_SOURCES
	binarchive.cpp
	xmlarchive.cpp
	bsonarchive.cpp
	jsonarchive.cpp
)

SET(AR_HEADERS
	binarchive.h
	xmlarchive.h
	bsonarchive.h
	jsonarchive.h
)

SC_SETUP_LIB_SUBDIR(AR)
