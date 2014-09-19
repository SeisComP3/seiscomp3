SUBDIRS(plugins)

SET(
    MASTER_SOURCES
	clientdb.cpp
	master.cpp
	scmaster.cpp
)

SC_ADD_EXECUTABLE(MASTER scmaster ${SC3_PACKAGE_SBIN_DIR})
SC_LINK_LIBRARIES(scmaster ${Boost_program_options_LIBRARY} ${Boost_thread_LIBRARY})
SC_LINK_LIBRARIES_INTERNAL(scmaster client)

# Install spread and scmaster init scripts
SC_INSTALL_INIT(scmaster config/scmaster.py)
SC_INSTALL_INIT(spread config/spread.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
INSTALL(FILES config/spread.conf.tpl DESTINATION ${SC3_PACKAGE_TEMPLATES_DIR})
