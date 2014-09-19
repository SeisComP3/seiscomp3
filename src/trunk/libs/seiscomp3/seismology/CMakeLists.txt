SET(SEISMOLOGY_SOURCES
	mb.cpp
	ML.cpp
	Mwp.cpp
	Mjma.cpp
	ttt.cpp

	eigv.cpp
	chi2.cpp
	locsat.cpp
	locatorinterface.cpp
	locsat_internal.cpp
)

SET(SEISMOLOGY_HEADERS
	ttt.h
	regions.h
	magnitudes.h
	locsat.h
	locatorinterface.h
)

SC_ADD_SUBDIR_SOURCES(SEISMOLOGY regions)
SC_ADD_SUBDIR_SOURCES(SEISMOLOGY ttt)
SC_SETUP_LIB_SUBDIR(SEISMOLOGY)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
