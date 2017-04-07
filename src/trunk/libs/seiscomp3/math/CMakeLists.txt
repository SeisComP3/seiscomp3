SET(MATH_SOURCES
	math.cpp
	geo.cpp
	mean.cpp
	util.cpp
	coord.cpp
	polygon.cpp
	vector3.cpp
	matrix3.cpp
	tensor.cpp
	conversions.cpp
	filter.cpp
	fft.cpp
	windowfunc.cpp
)

SET(MATH_HEADERS
	decomp.ipp
	math.h
	geo.h
	hilbert.ipp
	minmax.ipp
	misc.ipp
	mean.h
	coord.h
	polygon.h
	vector3.h
	vector3.ipp
	matrix3.h
	matrix3.ipp
	tensor.h
	conversions.h
	conversions.ipp
	filter.h
	fft.h
	windowfunc.h
)

SC_ADD_SUBDIR_SOURCES(MATH filter)
SC_ADD_SUBDIR_SOURCES(MATH restitution)
SC_ADD_SUBDIR_SOURCES(MATH windows)

SET(MATH_DEFINITIONS ${FFTW3_DEFINITIONS})
SC_SETUP_LIB_SUBDIR(MATH)
