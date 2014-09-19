SET(GUI_MAP_SOURCES
	layer.cpp
	legend.cpp
	mapsymbol.cpp
	mapsymbolcollection.cpp
	maptree.cpp
	imagetree.cpp
	canvas.cpp
	mapwidget.cpp
	texturecache.cpp
	projection.cpp
	rectangularprojection.cpp
)

SET(GUI_MAP_HEADERS
	maptree.h
	imagetree.h
	mapsymbol.h
	mapsymbolcollection.h
	decorator.h
	texturecache.h
	texturecache.ipp
	projection.h
	rectangularprojection.h
)

SET(GUI_MAP_MOC_HEADERS
	legend.h
	layer.h
	imagetree.h
	canvas.h
	mapwidget.h
)

SET(GUI_MAP_RESOURCES
	mapicons.qrc
)

SC_ADD_GUI_SUBDIR_SOURCES(GUI_MAP layers)
SC_SETUP_GUI_LIB_SUBDIR(GUI_MAP)
