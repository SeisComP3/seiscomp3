SET(RECORDFILTER_SOURCES
	iirfilter.cpp
	resample.cpp
	demux.cpp
	spectralizer.cpp
)

SET(RECORDFILTER_HEADERS
	iirfilter.h
	resample.h
	demux.h
	spectralizer.h
)

SC_SETUP_LIB_SUBDIR(RECORDFILTER)
