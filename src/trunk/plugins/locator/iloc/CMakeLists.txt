OPTION(SC_TRUNK_LOCATOR_ILOC "Enable iLoc locator plugin (still experimental)" OFF)

IF(SC_TRUNK_LOCATOR_ILOC)

FIND_PACKAGE(LAPACK REQUIRED)

SET(RSTT_GEOTESS_SOURCES
	RSTT/GeoTessCPP/src/ArrayReuse.cc
	RSTT/GeoTessCPP/src/CPPUtils.cc
	RSTT/GeoTessCPP/src/CpuTimer.cc
	RSTT/GeoTessCPP/src/DataType.cc
	RSTT/GeoTessCPP/src/EathShape.cc
	RSTT/GeoTessCPP/src/EnumType.cc
	RSTT/GeoTessCPP/src/GeoTessDataArray.cc
	RSTT/GeoTessCPP/src/GeoTessData.cc
	RSTT/GeoTessCPP/src/GeoTessDataValue.cc
	RSTT/GeoTessCPP/src/GeoTessException.cc
	RSTT/GeoTessCPP/src/GeoTessGreatCircle.cc
	RSTT/GeoTessCPP/src/GeoTessGrid.cc
	RSTT/GeoTessCPP/src/GeoTessHorizon.cc
	RSTT/GeoTessCPP/src/GeoTessHorizonDepth.cc
	RSTT/GeoTessCPP/src/GeoTessHorizonLayer.cc
	RSTT/GeoTessCPP/src/GeoTessHorizonRadius.cc
	RSTT/GeoTessCPP/src/GeoTessInterpolatorType.cc
	RSTT/GeoTessCPP/src/GeoTessMetaData.cc
	RSTT/GeoTessCPP/src/GeoTessModel.cc
	RSTT/GeoTessCPP/src/GeoTessModelUtils.cc
	RSTT/GeoTessCPP/src/GeoTessOptimizationType.cc
	RSTT/GeoTessCPP/src/GeoTessPointMap.cc
	RSTT/GeoTessCPP/src/GeoTessPolygon3D.cc
	RSTT/GeoTessCPP/src/GeoTessPolygon.cc
	RSTT/GeoTessCPP/src/GeoTessPosition.cc
	RSTT/GeoTessCPP/src/GeoTessPositionLinear.cc
	RSTT/GeoTessCPP/src/GeoTessPositionNaturalNeighbor.cc
	RSTT/GeoTessCPP/src/GeoTessProfile.cc
	RSTT/GeoTessCPP/src/GeoTessProfileConstant.cc
	RSTT/GeoTessCPP/src/GeoTessProfileEmpty.cc
	RSTT/GeoTessCPP/src/GeoTessProfileNPoint.cc
	RSTT/GeoTessCPP/src/GeoTessProfileSurface.cc
	RSTT/GeoTessCPP/src/GeoTessProfileSurfaceEmpty.cc
	RSTT/GeoTessCPP/src/GeoTessProfileThin.cc
	RSTT/GeoTessCPP/src/GeoTessProfileType.cc
	RSTT/GeoTessCPP/src/GeoTessUtils.cc
	RSTT/GeoTessCPP/src/IFStreamAscii.cc
	RSTT/GeoTessCPP/src/IFStreamBinary.cc
)


SET(RSTT_SLBM_SOURCES
	RSTT/SLBM/src/Brents.cc
	RSTT/SLBM/src/CrustalProfile.cc
	RSTT/SLBM/src/CrustalProfileStore.cc
	RSTT/SLBM/src/DataBuffer.cc
	RSTT/SLBM/src/GeoStack.cc
	RSTT/SLBM/src/GeoTessModelSLBM.cc
	RSTT/SLBM/src/GreatCircle.cc
	RSTT/SLBM/src/GreatCircleFactory.cc
	RSTT/SLBM/src/GreatCircle_Xg.cc
	RSTT/SLBM/src/GreatCircle_Xn.cc
	RSTT/SLBM/src/Grid.cc
	RSTT/SLBM/src/GridGeoTess.cc
	RSTT/SLBM/src/GridProfile.cc
	RSTT/SLBM/src/GridProfileGeoTess.cc
	RSTT/SLBM/src/GridProfileSLBM.cc
	RSTT/SLBM/src/GridSLBM.cc
	RSTT/SLBM/src/IntegrateFunction.cc
	RSTT/SLBM/src/InterpolatedProfile.cc
	RSTT/SLBM/src/LayerProfile.cc
	RSTT/SLBM/src/LayerProfileG.cc
	RSTT/SLBM/src/Location.cc
	RSTT/SLBM/src/MD50.cc
	RSTT/SLBM/src/QueryProfile.cc
	RSTT/SLBM/src/SlbmInterface.cc
	RSTT/SLBM/src/SlbmInterfaceToJNI.cc
	RSTT/SLBM/src/TauPSite.cc
	RSTT/SLBM/src/TauPSiteFunctionals.cc
	RSTT/SLBM/src/TPVelocityModels.cc
	RSTT/SLBM/src/Triangle.cc
	RSTT/SLBM/src/Uncertainty.cc
)


SET(RSTT_SLBM_C_SHELL_SOURCES
	RSTT/SLBM_C_shell/src/slbm_C_shell.cc
)

SET(ILOC_SOURCES
	${RSTT_GEOTESS_SOURCES}
	${RSTT_SLBM_SOURCES}
	${RSTT_SLBM_C_SHELL_SOURCES}
	sciLocCluster.c
	sciLocDistAzimuth.c
	sciLocInitializations.c
	sciLocLocalTT.c
	sciLocNA.c
	sciLocReadAuxDataFiles.c
	sciLocDataCovariance.c
	sciLocFreeMemory.c
	sciLocInterface.h
	sciLocLocationQuality.c
	sciLocPhaseIdentification.c
	sciLocSVD.c
	sciLocDepthPhases.c
	sciLocGregion.c
	sciLocInterpolate.c
	sciLocLocator.c
	sciLocPrintEvent.c
	sciLocTravelTimes.c
	sciLocTravelTimeAPI.c
)

INCLUDE_DIRECTORIES(RSTT/GeoTessCPP/include)
INCLUDE_DIRECTORIES(RSTT/SLBM/include)
INCLUDE_DIRECTORIES(RSTT/SLBM_C_shell/include)
INCLUDE_DIRECTORIES(.)

ADD_LIBRARY(iloc STATIC ${ILOC_SOURCES})
SET_TARGET_PROPERTIES(iloc PROPERTIES COMPILE_FLAGS "-fPIC")

SET(ILOC_PLUGIN_SOURCES
	plugin.cpp
	iloc.cpp
)

SC_ADD_PLUGIN_LIBRARY(ILOC_PLUGIN lociloc "")
SC_LINK_LIBRARIES_INTERNAL(lociloc core)
SC_LINK_LIBRARIES(lociloc iloc lapack m)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})

ENDIF(SC_TRUNK_LOCATOR_ILOC)
