SET(SCQUERY_TARGET scquery)

SET(
    SCQUERY_HEADERS
    dbquery.h
    dbconnection.h
)

SET(
    SCQUERY_SOURCES
    dbquery.cpp
    dbconnection.cpp
	scquery.cpp
)

SC_ADD_EXECUTABLE(SCQUERY ${SCQUERY_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${SCQUERY_TARGET} client)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
