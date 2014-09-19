SET(PROC_SOURCES
	parameters.cpp
	response.cpp
	sensor.cpp
	stream.cpp
	processor.cpp
	waveformprocessor.cpp
	timewindowprocessor.cpp
	waveformoperator.cpp
	application.cpp
	streambuffer.cpp

	detector.cpp
	picker.cpp
	secondarypicker.cpp
	amplitudeprocessor.cpp
	magnitudeprocessor.cpp
)

SET(PROC_HEADERS
	parameters.h
	response.h
	sensor.h
	stream.h
	processor.h
	waveformprocessor.h
	timewindowprocessor.h
	waveformoperator.h
	application.h
	streambuffer.h

	detector.h
	picker.h
	secondarypicker.h
	amplitudeprocessor.h
	magnitudeprocessor.h
)

SC_ADD_SUBDIR_SOURCES(PROC picker)
SC_ADD_SUBDIR_SOURCES(PROC secondarypicker)
SC_ADD_SUBDIR_SOURCES(PROC amplitudes)
SC_ADD_SUBDIR_SOURCES(PROC magnitudes)
SC_ADD_SUBDIR_SOURCES(PROC operator)
SC_SETUP_LIB_SUBDIR(PROC)
