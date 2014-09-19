SET(DLSV_TARGET dlsv2inv)

SET(DLSV_SOURCES
	dataless.cpp
	inventory.cpp
	log.cpp
	main.cpp
	mystring.cpp
	seed.cpp
	converter.cpp
	tmanip.cpp
)


INCLUDE_DIRECTORIES(.)

SC_ADD_EXECUTABLE(DLSV ${DLSV_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${DLSV_TARGET} client)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
