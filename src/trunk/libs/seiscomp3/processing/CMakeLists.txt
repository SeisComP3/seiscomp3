SET(PROC_SOURCES
	amplitudeprocessor.cpp
	application.cpp
	detector.cpp
	fx.cpp
	magnitudeprocessor.cpp
	picker.cpp
	processor.cpp
	response.cpp
	secondarypicker.cpp
	sensor.cpp
	stream.cpp
	streambuffer.cpp
	timewindowprocessor.cpp
	waveformoperator.cpp
	waveformprocessor.cpp
)

SET(PROC_HEADERS
	amplitudeprocessor.h
	application.h
	detector.h
	fx.h
	magnitudeprocessor.h
	picker.h
	processor.h
	response.h
	secondarypicker.h
	sensor.h
	stream.h
	streambuffer.h
	timewindowprocessor.h
	waveformoperator.h
	waveformprocessor.h
)

SC_ADD_SUBDIR_SOURCES(PROC amplitudes)
SC_ADD_SUBDIR_SOURCES(PROC fx)
SC_ADD_SUBDIR_SOURCES(PROC magnitudes)
SC_ADD_SUBDIR_SOURCES(PROC operator)
SC_ADD_SUBDIR_SOURCES(PROC picker)
SC_ADD_SUBDIR_SOURCES(PROC secondarypicker)
SC_SETUP_LIB_SUBDIR(PROC)
