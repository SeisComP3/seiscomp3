SET(PROG_TARGET scvsmag)

SET(
	PROG_SOURCES
		scvsmag.cpp
		util.cpp
		timeline.cpp
		equations.cpp
		qualitycontrol.cpp
		Vs30Mapping.cpp
		VsSiteCondition.cpp
		main.cpp
)

ADD_DEFINITIONS(-DVSSC)
SC_ADD_EXECUTABLE(PROG ${PROG_TARGET})
SC_LINK_LIBRARIES_INTERNAL(${PROG_TARGET} client datamodel_vs)
SC_INSTALL_INIT(${PROG_TARGET} ../../../trunk/apps/templates/initd.py)

# scvsmaglog
SET(MAIN_PY scvsmaglog.py)
SET(VSMAGLOG_TARGET scvsmaglog)

SET(VSMAGLOG_FILES scvsmaglog.py)

SC_ADD_PYTHON_EXECUTABLE(VSMAGLOG)
SC_INSTALL_INIT(${VSMAGLOG_TARGET} ../../../trunk/apps/templates/initd.py)

IF(SC_GLOBAL_PYTHON_WRAPPER)
# UD interface
INSTALL(FILES ud_interface.py
        DESTINATION ${PYTHON_LIBRARY_PATH}
)
ENDIF(SC_GLOBAL_PYTHON_WRAPPER)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
