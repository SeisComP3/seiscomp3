SET(SCREAM2PLUGIN_SOURCES
	dispatch_ring.c
	gcf.c
	gputil.c
	main.c
	map.c
	mapping.c
	scream.c
	util.c
	../plugin.c
)

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(..)

ADD_EXECUTABLE(scream_plugin_ring ${SCREAM2PLUGIN_SOURCES})
TARGET_LINK_LIBRARIES(scream_plugin_ring m pthread)

INSTALL(TARGETS scream_plugin_ring RUNTIME DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
