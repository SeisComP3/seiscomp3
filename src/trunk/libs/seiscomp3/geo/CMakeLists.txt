SET (GEO_SOURCES
	coordinate.cpp
	boundingbox.cpp
	feature.cpp
	featureset.cpp
)

SET(GEO_HEADERS
	coordinate.h
	coordinate.ipp
	boundingbox.h
	boundingbox.ipp
	feature.h
	featureset.h
)

SC_ADD_SUBDIR_SOURCES(GEO index)
SC_SETUP_LIB_SUBDIR(GEO)
