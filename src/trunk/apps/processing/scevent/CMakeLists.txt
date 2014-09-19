SUBDIRS(seiscomp3 plugins)

SET(EVENT_TARGET scevent)

SET(
	EVENT_SOURCES
		main.cpp
		eventtool.cpp
		eventinfo.cpp
		util.cpp
		constraints.cpp
)


SET(
	EVENT_HEADERS
		eventtool.h
		eventinfo.h
		config.h
		constraints.h
		util.h
)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SC_ADD_EXECUTABLE(EVENT ${EVENT_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${EVENT_TARGET} client evplugin)
SC_INSTALL_INIT(${EVENT_TARGET} ../../templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
