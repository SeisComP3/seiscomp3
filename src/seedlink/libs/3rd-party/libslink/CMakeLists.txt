SET(SLINK_SOURCES
	gswap.c
	unpack.c
	msrecord.c
	genutils.c
	strutils.c
	logging.c
	network.c
	statefile.c
	config.c
	slplatform.c
	slutils.c
	globmatch.c
)

ADD_LIBRARY(slink STATIC ${SLINK_SOURCES})
