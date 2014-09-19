SET(PACKAGE_NAME MapView)
SET(APPLICATION_NAME scmv)

SET(${PACKAGE_NAME}_SOURCES
	searchwidget.cpp
	mvmainwindow.cpp
	mvmapwidget.cpp
	stationdata.cpp
	stationdatahandler.cpp
	mvstationsymbol.cpp
	eventdata.cpp
	legend.cpp
	eventtablewidget.cpp
	infowidget.cpp
    main.cpp
)

SET(${PACKAGE_NAME}_HEADERS
	types.h
	stationdata.h
	stationdatahandler.h
	mvstationsymbol.h
	eventdata.h
	legend.h
	debug.h
)

SET(${PACKAGE_NAME}_MOC_HEADERS
	searchwidget.h
	mvmainwindow.h
	mvmapwidget.h
	eventtablewidget.h
	infowidget.h
)

SET(${PACKAGE_NAME}_UI
	mvmainwindow.ui
	searchwidget.ui
)

SC_ADD_GUI_EXECUTABLE(${PACKAGE_NAME} ${APPLICATION_NAME})
SC_LINK_LIBRARIES_INTERNAL(${APPLICATION_NAME} qt4)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
