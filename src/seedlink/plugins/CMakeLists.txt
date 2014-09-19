INCLUDE_DIRECTORIES(../libs/slutils)

SET(
	_DIR_LIST
	mseedfifo_plugin
	chain_plugin
)

IF ( NOT MACOSX )
	SET(
		_DIR_LIST 
		${_DIR_LIST}
		serial_plugin
		sockplugin
		naqsplugin
		nrts_plugin
		q330plugin
		reftek_plugin
		ewexport_plugin
		ewexport_pasv_plugin
		scream_plugin
		scream2_plugin
		wave24_plugin
		minilogger_plugin
		mseedscan_plugin
		fs_plugin
		win_plugin
	) 
ENDIF ( NOT MACOSX )

SET(SEEDLINK_PLUGIN_OUTPUT_DIR ${SC3_PACKAGE_SHARE_DIR}/plugins/seedlink)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SUBDIRS(${_DIR_LIST})
