SET(MPLUGIN_HEADERS
	monitorplugininterface.h
	monitoroutplugininterface.h
	types.h
)
SET(MPLUGIN_SOURCES
	monitorplugininterface.cpp
	monitoroutplugininterface.cpp
)

# Install the headers in install/seiscomp3/plugins/monitor
SC_ADD_LIBRARY(MPLUGIN mplugin)
SC_LIB_INSTALL_HEADERS(MPLUGIN seiscomp3/plugins/monitor)
SC_LIB_LINK_LIBRARIES_INTERNAL(mplugin core)
