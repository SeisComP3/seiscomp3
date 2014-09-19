#-------------------------------------------------------------------------
MACRO(MAKE_QCPLUGIN _plugin)
	SET(QCP_HEADERS ${_plugin}.h)
	SET(QCP_SOURCES ${_plugin}.cpp)
	SC_ADD_PLUGIN_LIBRARY(QCP ${_plugin} qc)
	SC_LINK_LIBRARIES_INTERNAL(${_plugin} qcplugin)
ENDMACRO(MAKE_QCPLUGIN)

MAKE_QCPLUGIN(qcplugin_delay)
MAKE_QCPLUGIN(qcplugin_offset)
MAKE_QCPLUGIN(qcplugin_rms)
MAKE_QCPLUGIN(qcplugin_latency)
MAKE_QCPLUGIN(qcplugin_timing)
MAKE_QCPLUGIN(qcplugin_availability)
MAKE_QCPLUGIN(qcplugin_overlap)
MAKE_QCPLUGIN(qcplugin_gap)
MAKE_QCPLUGIN(qcplugin_spike)
MAKE_QCPLUGIN(qcplugin_outage)
#-------------------------------------------------------------------------


#-------------------------------------------------------------------------
INCLUDE_DIRECTORIES(.)
#-------------------------------------------------------------------------
