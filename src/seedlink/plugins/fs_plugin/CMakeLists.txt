SET(FS_PLUGIN_SOURCES
	fs_plugin.cc
	fs_input_irae.cc
	fs_input_edr.cc
	fs_input_ddb.cc
	fs_decode_mseed.cc
	../plugin.c
)


INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(../../libs/plugin)
INCLUDE_DIRECTORIES(../../libs/3rd-party/libslink)
INCLUDE_DIRECTORIES(../../libs/3rd-party/qlib2)


SET(CONFIG_FILE ${CMAKE_INSTALL_PREFIX}/acquisition/config/plugins.ini)
SET(OPTIONS -DSYSLOG_FACILITY=LOG_LOCAL0 -DCONFIG_FILE=\\\"${CONFIG_FILE}\\\")

ADD_DEFINITIONS(${OPTIONS})

ADD_EXECUTABLE(fs_plugin ${FS_PLUGIN_SOURCES})

TARGET_LINK_LIBRARIES(
	fs_plugin
		slink
		qlib2
		slplugin
		slutils
)

INSTALL(TARGETS fs_plugin RUNTIME DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})
