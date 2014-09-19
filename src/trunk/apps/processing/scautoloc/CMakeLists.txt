SET(LOC_TARGET scautoloc)

SET(
	LOC_SOURCES
		app.cpp
		associator.cpp
		autoloc.cpp
		config.cpp
		datamodel.cpp
		locator.cpp
		nucleator.cpp
		util.cpp
		sc3adapters.cpp
		main.cpp
)

SET(
	LOC_HEADERS
		app.h
		associator.h
		autoloc.h
		datamodel.h
		locator.h
		nucleator.h
		util.h
		sc3adapters.h
)

SET(
	LOC_DATA
		config/grid.conf
		config/station.conf
		config/station-locations.conf
)

INCLUDE_DIRECTORIES(.)

SC_ADD_EXECUTABLE(LOC ${LOC_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${LOC_TARGET} client)

SC_INSTALL_DATA(LOC ${LOC_TARGET})
SC_INSTALL_INIT(${LOC_TARGET} ../../templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
