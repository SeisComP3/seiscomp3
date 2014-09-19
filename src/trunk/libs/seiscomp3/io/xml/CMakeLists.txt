SET(XML_SOURCES
	importer.cpp
	exporter.cpp
	handler.cpp
)
	
SET(XML_HEADERS
	importer.h
	exporter.h
	handler.h
	handler.ipp
)

SC_SETUP_LIB_SUBDIR(XML)
