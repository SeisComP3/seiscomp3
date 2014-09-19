SET(SLPLUGIN_HEADERS
	plugin.h
	plugin_channel.h
	plugin_exceptions.h
	plugin_module.h
)


SET(SLPLUGIN_SOURCES
	plugin.c
	plugin_channel.cc
)

INCLUDE_DIRECTORIES(../slutils)
INCLUDE_DIRECTORIES(../3rd-party/qlib2)

ADD_LIBRARY(slplugin STATIC ${SLPLUGIN_SOURCES})
