SET(THIRD_PARTY_LIB_DIR ${SC3_PACKAGE_SOURCE_DIR}/libs/3rd-party)
INCLUDE_DIRECTORIES(THIRD_PARTY_LIB_DIR)

SET(
	LOCATOR_HEADERS
		locsat.h
		fixed-hypocenter.h
		utils.h
)

SET(
	LOCATOR_SOURCES
		locsat.cpp
		locsat_internal.cpp
		fixed-hypocenter.cpp
		chi2.cpp
		eigv.cpp
		utils.cpp
)

SC_SETUP_LIB_SUBDIR(LOCATOR)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
