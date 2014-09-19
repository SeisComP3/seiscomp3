SET(AMP_TARGET scamp)

SET(
	AMP_SOURCES
		main.cpp
		amptool.cpp
		util.cpp
)


SET(
	AMP_HEADERS
		amptool.h
		util.h
)


SC_ADD_EXECUTABLE(AMP ${AMP_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${AMP_TARGET} client)
SC_INSTALL_INIT(${AMP_TARGET} ../../templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
