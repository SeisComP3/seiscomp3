SET(QCPLUGIN_HEADERS
		qcplugin.h
		qcconfig.h
		qcmessenger.h
		qcbuffer.h
)
SET(QCPLUGIN_SOURCES
		qcplugin.cpp
		qcconfig.cpp
		qcmessenger.cpp
		qcbuffer.cpp
)

SC_ADD_LIBRARY(QCPLUGIN qcplugin)
SC_LIB_INSTALL_HEADERS(QCPLUGIN seiscomp3/plugins/qc)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
SC_LIB_LINK_LIBRARIES(qcplugin ${Boost_LIBRARIES})
SC_LIB_LINK_LIBRARIES_INTERNAL(qcplugin client)
