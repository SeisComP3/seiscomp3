SET(ENVELOPE_TARGET scenvelope)

SET(
	ENVELOPE_SOURCES
		butterworth_c.c
		app.cpp
		envelope.cpp
		filter.cpp
		processor.cpp
		main.cpp
		util.cpp
)

SC_ADD_EXECUTABLE(ENVELOPE ${ENVELOPE_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${ENVELOPE_TARGET} client datamodel_vs)
SC_INSTALL_INIT(${ENVELOPE_TARGET} ../../../trunk/apps/templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})


# Test app
SET(TEST_TARGET testenvelope)

SET(
	TEST_SOURCES
		test.cpp
)

SC_ADD_TEST_EXECUTABLE(TEST ${TEST_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${TEST_TARGET} client datamodel_vs)
