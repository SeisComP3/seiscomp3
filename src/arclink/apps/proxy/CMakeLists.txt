INSTALL(PROGRAMS arclinkproxy.py RENAME arclinkproxy DESTINATION ${SC3_PACKAGE_SBIN_DIR})

SC_INSTALL_INIT(arclinkproxy config/arclinkproxy.py)

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/arclinkproxy.cfg)
        INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config/arclinkproxy.cfg
                DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/arclinkproxy.cfg)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
