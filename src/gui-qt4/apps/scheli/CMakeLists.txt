SET(PACKAGE_NAME HELICORDER)
SET(APP_NAME scheli)

SET(
	${PACKAGE_NAME}_SOURCES
		main.cpp
		mainwindow.cpp
		heliwidget.cpp
)

SET(
	${PACKAGE_NAME}_HEADERS
)


SET(
	${PACKAGE_NAME}_MOC_HEADERS
		mainwindow.h
		heliwidget.h
		app.h
)

SET(
	${PACKAGE_NAME}_UI
		mainwindow.ui
)


SET(
	${PACKAGE_NAME}_RESOURCES
)

SC_ADD_GUI_EXECUTABLE(${PACKAGE_NAME} ${APP_NAME})
SC_LINK_LIBRARIES_INTERNAL(${APP_NAME} qt4)
