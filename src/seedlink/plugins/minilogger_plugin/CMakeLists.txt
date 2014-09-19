SET(MINILOGGER_PLUGIN_SOURCES
	mini_logger_writer.c
	mini_logger_writer.h
	minisepdevice.c
	minisepdevice.h
	settings/settings.c
	settings/settings.h
	settings/strmap.c
	settings/strmap.h
)

INCLUDE_DIRECTORIES(../../libs/plugin ${LIBMSEED_INCLUDE_DIR})
ADD_EXECUTABLE(minilogger_plugin ${MINILOGGER_PLUGIN_SOURCES})
TARGET_LINK_LIBRARIES(minilogger_plugin rt slplugin ${LIBMSEED_LIBRARY})
INSTALL(TARGETS minilogger_plugin RUNTIME DESTINATION ${SEEDLINK_PLUGIN_OUTPUT_DIR})

