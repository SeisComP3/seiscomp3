SET(AMPS_SOURCES
	a5_2.cpp
	idc_utils.cpp
	mBc.cpp
	mBc_measure.cpp
	mb.cpp
	m_B.cpp
	Mjma.cpp
	ML.cpp
	MLh.cpp
	MLv.cpp
	msbb.cpp
	Ms20.cpp
	Mwp.cpp
	mwp_utils.c
	sbsnr.cpp
)

SET(AMPS_HEADERS
	mBc.h
	mBc_measure.h
	mb.h
	m_B.h
	Mjma.h
	ML.h
	MLh.h
	MLv.h
	msbb.h
	Ms20.h
	Mwp.h
	mwp_utils.h
)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

SC_SETUP_LIB_SUBDIR(AMPS)
