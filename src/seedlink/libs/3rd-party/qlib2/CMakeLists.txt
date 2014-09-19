SET(QLIB2_SOURCES
	qlib2_version.c
	drm_utils.c
	ms_utils.c
	ms_pack.c
	ms_unpack.c
	pack.c
	qda_utils.c
	qtime.c
	sdr_utils.c
	unpack.c
	qutils.c
)

ADD_LIBRARY(qlib2 STATIC ${QLIB2_SOURCES})
