SET(GUI_MAP_LAYER_SOURCES
	citieslayer.cpp
	gridlayer.cpp
	geofeaturelayer.cpp
)

SET(GUI_MAP_LAYER_HEADERS
	citieslayer.h
	gridlayer.h
)

SET(GUI_MAP_LAYER_MOC_HEADERS
	geofeaturelayer.h
)

SC_SETUP_GUI_LIB_SUBDIR(GUI_MAP_LAYER)
