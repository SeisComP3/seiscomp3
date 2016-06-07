SET(
	GUI_PLOT_SOURCES
		range.cpp
		axis.cpp
		abstractdataset.cpp
		graph.cpp
		plot.cpp
		datay.cpp
		dataxy.cpp
)

SET(
	GUI_PLOT_HEADERS
		range.h
		axis.h
		abstractdataset.h
		graph.h
		plot.h
		datay.h
		dataxy.h
)

SET(
	GUI_PLOT_MOC_HEADERS
)

SC_SETUP_GUI_LIB_SUBDIR(GUI_PLOT)
