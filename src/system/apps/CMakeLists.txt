SUBDIRS(seiscomp utils stationconf)

SC_INSTALL_INIT(diskmon config/diskmon.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
