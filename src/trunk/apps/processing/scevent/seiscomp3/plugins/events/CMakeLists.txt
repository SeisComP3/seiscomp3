SET(EVPLUGIN_HEADERS
	eventprocessor.h
	scoreprocessor.h
)

SET(EVPLUGIN_SOURCES
	eventprocessor.cpp
	scoreprocessor.cpp
)

# Install the headers in install/seiscomp3/plugins/events
SC_LIB_INSTALL_HEADERS(EVPLUGIN seiscomp3/plugins/events)
SC_ADD_LIBRARY(EVPLUGIN evplugin)
SC_LIB_LINK_LIBRARIES_INTERNAL(evplugin core)
