SUBDIRS(plugins)

SET(PACKAGE_NAME SYNC_STAXML)
SET(APP_NAME staxml2inv)

SET(DATAMODEL_SOURCES
	stationxml/types.cpp
	stationxml/xml.cpp
	stationxml/inttype.cpp
	stationxml/floatnounittype.cpp
	stationxml/floattype.cpp
	stationxml/lattype.cpp
	stationxml/lontype.cpp
	stationxml/distancetype.cpp
	stationxml/angletype.cpp
	stationxml/diptype.cpp
	stationxml/azimuthtype.cpp
	stationxml/clockdrifttype.cpp
	stationxml/sampleratetype.cpp
	stationxml/samplerateratiotype.cpp
	stationxml/samplerategroup.cpp
	stationxml/countertype.cpp
	stationxml/frequencytype.cpp
	stationxml/rolltype.cpp
	stationxml/passtype.cpp
	stationxml/epoch.cpp
	stationxml/site.cpp
	stationxml/stringtype.cpp
	stationxml/agencytype.cpp
	stationxml/emailtype.cpp
	stationxml/phonetype.cpp
	stationxml/contact.cpp
	stationxml/operator.cpp
	stationxml/externaldocument.cpp
	stationxml/datetype.cpp
	stationxml/equipment.cpp
	stationxml/offset.cpp
	stationxml/output.cpp
	stationxml/datalogger.cpp
	stationxml/sensitivity.cpp
	stationxml/decimation.cpp
	stationxml/sa.cpp
	stationxml/spectra.cpp
	stationxml/comment.cpp
	stationxml/poleandzero.cpp
	stationxml/polesandzeros.cpp
	stationxml/coefficients.cpp
	stationxml/responselistelement.cpp
	stationxml/responselist.cpp
	stationxml/genericresponse.cpp
	stationxml/numeratorcoefficient.cpp
	stationxml/fir.cpp
	stationxml/polynomialcoefficient.cpp
	stationxml/polynomial.cpp
	stationxml/response.cpp
	stationxml/gaintype.cpp
	stationxml/leastsignificantbittype.cpp
	stationxml/voltagetype.cpp
	stationxml/secondtype.cpp
	stationxml/channelepoch.cpp
	stationxml/channel.cpp
	stationxml/stationepoch.cpp
	stationxml/station.cpp
	stationxml/network.cpp
	stationxml/stamessage.cpp
	stationxml/date.cpp
)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(${PACKAGE_NAME}_SOURCES
	${DATAMODEL_SOURCES}
	convert2staxml.cpp
	convert2sc3.cpp
	keyfile.cpp
	patterntable.cpp
	main.cpp
)

SC_ADD_EXECUTABLE(${PACKAGE_NAME} ${APP_NAME})
SC_LINK_LIBRARIES_INTERNAL(${APP_NAME} client)
