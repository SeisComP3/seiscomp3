SET(SEEDLINK_SOURCES
	seedlink.cc
	iosystem.cc
	monitor.cc
	sproc.cc
	mseed.cc
	steim1.cc
	steim2.cc
)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(../../libs/slutils)
INCLUDE_DIRECTORIES(../../libs/plugin)
INCLUDE_DIRECTORIES(../../libs/3rd-party/libslink)
INCLUDE_DIRECTORIES(../../libs/3rd-party/qlib2)


SET(CONFIG_FILE ${SC3_PACKAGE_INSTALL_PREFIX}/acquisition/config/seedlink.ini)

SET(OPTIONS -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE="${CONFIG_FILE}" -DMSEED_ROUND_TENTH_MILLISEC)

# Uncomment the following to enable file descriptor reallocation.
# This is needed for large servers (number of stations multiplied by
# the number of simultaneous connections is larger than 1000).
SET(OPTIONS ${OPTIONS} -DFD_REALLOC)

ADD_DEFINITIONS(${OPTIONS})

ADD_EXECUTABLE(seedlink ${SEEDLINK_SOURCES})

TARGET_LINK_LIBRARIES(
	seedlink
		${LIBXML2_LIBRARIES}
		slutils
		slplugin
		qlib2
)

SC_INSTALL_INIT(seedlink config/seedlink.py)

INSTALL(TARGETS seedlink
	RUNTIME DESTINATION ${SC3_PACKAGE_SBIN_DIR}
	ARCHIVE DESTINATION ${SC3_PACKAGE_LIB_DIR}
	LIBRARY DESTINATION ${SC3_PACKAGE_LIB_DIR}
)

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/seedlink.cfg)
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/seedlink.cfg
		DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/seedlink.cfg)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
