SET(UTILS_SOURCES
	timer.cpp
	base64.cpp
	datetime.cpp
	replace.cpp
	files.cpp
	leparser.cpp
	misc.cpp
)

SET(UTILS_HEADERS
	base64.h
	base64.ipp 
	timer.h
	datetime.h
	replace.h
	files.h
	leparser.h
	misc.h
	mutex.h
)

SC_SETUP_LIB_SUBDIR(UTILS)
