SET(SEISMICPI_PLUGIN_SOURCES
	seismicpi_writer.c
	seismicpi_writer.h
	seismicpidevice.c
	seismicpidevice.h
	settings/settings.c
	settings/settings.h
	settings/strmap.c
	settings/strmap.h
)

INCLUDE_DIRECTORIES(../../libs/plugin ${LIBMSEED_INCLUDE_DIR})
ADD_EXECUTABLE(seismicpi_plugin ${SEISMICPI_PLUGIN_SOURCES})
TARGET_LINK_LIBRARIES(seismicpi_plugin rt slplugin ${LIBMSEED_LIBRARY})
INSTALL(TARGETS seismicpi_plugin RUNTIME DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})

