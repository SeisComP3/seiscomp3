#SUBDIRS(archive records recordstream gfarchive xml)

SET(IO_SOURCES
	database.cpp
	importer.cpp
	exporter.cpp
	recordinput.cpp
	recordfilter.cpp
	recordstream.cpp
	recordoutputstream.cpp
	gfarchive.cpp
	socket.cpp
)

SET(IO_HEADERS
	database.h
	importer.h
	exporter.h
	recordinput.h
	recordfilter.h
	recordstreamexceptions.h
	recordstream.h
	recordoutputstream.h
	gfarchive.h
	socket.h
	httpsocket.h
	httpsocket.ipp
)

SC_ADD_SUBDIR_SOURCES(IO archive)
SC_ADD_SUBDIR_SOURCES(IO gfarchive)
SC_ADD_SUBDIR_SOURCES(IO quakelink)
SC_ADD_SUBDIR_SOURCES(IO records)
SC_ADD_SUBDIR_SOURCES(IO recordstream)
SC_ADD_SUBDIR_SOURCES(IO recordfilter)
SC_ADD_SUBDIR_SOURCES(IO xml)

SC_SETUP_LIB_SUBDIR(IO)
