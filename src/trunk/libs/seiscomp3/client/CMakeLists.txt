SET(CLIENT_SOURCES
	application.cpp
	streamapplication.cpp
	commandline.cpp
	inventory.cpp
	configdb.cpp
	queue.cpp
	daemon.cpp
	pluginregistry.cpp
	monitor.cpp
)

SET(CLIENT_HEADERS
	application.h
	streamapplication.h
	commandline.h
	commandline.ipp
	queue.h
	queue.ipp
	inventory.h
	configdb.h
	daemon.h
	pluginregistry.h
	monitor.h
)

SC_SETUP_LIB_SUBDIR(CLIENT)
