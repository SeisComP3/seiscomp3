#SUBDIRS(plugins)

SET(PACKAGE_NAME MESSAGEMONITOR)
SET(APP_NAME scmm)

SET(
    ${PACKAGE_NAME}_SOURCES
	main.cpp
	mainframe.cpp
	tracewidget.cpp
)

SET(
    ${PACKAGE_NAME}_HEADERS
	plugin.h
)

SET(
    ${PACKAGE_NAME}_MOC_HEADERS
	mainframe.h
	tracewidget.h
)

SET(
    ${PACKAGE_NAME}_UI
	mainframe.ui
)

SC_ADD_GUI_EXECUTABLE(${PACKAGE_NAME} ${APP_NAME})
SC_LINK_LIBRARIES_INTERNAL(${APP_NAME} qt4)
