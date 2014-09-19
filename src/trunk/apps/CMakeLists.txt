SET(GLOBAL_CONFIG config/global.cfg)
SC_INSTALL_CONFIG(GLOBAL)
SC_INSTALL_INIT(trunk config/global.py)

INSTALL(FILES templates/initd.py
	DESTINATION ${SC3_PACKAGE_SHARE_DIR}/templates)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SUBDIRS(messaging tools python processing fdsnws)
