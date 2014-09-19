SET(WFPARAM_TARGET scwfparam)

SET(
	WFPARAM_SOURCES
		main.cpp
		app.cpp
		wfparam.cpp
		util.cpp
		msg.cpp
		processors/pgav.cpp
)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

SC_ADD_EXECUTABLE(WFPARAM ${WFPARAM_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${WFPARAM_TARGET} client datamodel_sm)
SC_INSTALL_INIT(${WFPARAM_TARGET} ../../../trunk/apps/templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
