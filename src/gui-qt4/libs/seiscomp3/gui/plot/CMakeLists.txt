SET(
	GUI_PLOT_SOURCES
		abstractdataset.cpp
		abstractlegend.cpp
		range.cpp
		axis.cpp
		graph.cpp
		plot.cpp
		datay.cpp
		dataxy.cpp
		legend.cpp
)

SET(
	GUI_PLOT_HEADERS
		abstractdataset.h
		abstractlegend.h
		range.h
		axis.h
		graph.h
		plot.h
		datay.h
		dataxy.h
		legend.h
)

SET(
	GUI_PLOT_MOC_HEADERS
)

SC_SETUP_GUI_LIB_SUBDIR(GUI_PLOT)
