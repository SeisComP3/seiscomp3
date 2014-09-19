SET(COM_SOURCES
	systemconnection.cpp
	connection.cpp
	protocol.cpp
	systemmessages.cpp
	networkinterface.cpp
	servicemessage.cpp
	connectioninfo.cpp
	clientstatus.cpp
	masterplugininterface.cpp
	spread/spreaddriver.cpp
)

SET(COM_HEADERS
	systemconnection.h
	connection.h
	protocol.h
	systemmessages.h
	networkinterface.h
	servicemessage.h
	masterplugininterface.h
	connectioninfo.h
	clientstatus.h
)

SC_SETUP_LIB_SUBDIR(COM)
