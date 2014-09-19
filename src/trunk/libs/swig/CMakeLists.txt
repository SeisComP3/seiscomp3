INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(../../../system/libs/swig)

IF (CMAKE_COMPILER_IS_GNUCC)
    # Important for O2 compilation
    ADD_DEFINITIONS("-fno-strict-aliasing")
ENDIF (CMAKE_COMPILER_IS_GNUCC)

IF (MSEED_FOUND)
    ADD_DEFINITIONS(-DHAVE_MSEED)
    INCLUDE_DIRECTORIES(${LIBMSEED_INCLUDE_DIR})
ENDIF (MSEED_FOUND)

IF (SC_GLOBAL_PYTHON_WRAPPER_NUMPY)
	MESSAGE(STATUS "Adding support for Python NumPy")
	ADD_DEFINITIONS(-DHAVE_NUMPY)
	INCLUDE_DIRECTORIES(${PYTHON_NUMPY_INCLUDE_DIR})
ENDIF (SC_GLOBAL_PYTHON_WRAPPER_NUMPY)

SET(
	WRAPPER_MODULES
		Core
		Logging
		System
		IO
		Utils
		DataModel
		Math
		Seismology
		Communication
		Client
		helpers
)


SET(WRAPPER_MODULE_Core_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_Logging_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_System_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_IO_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_Communication_EXTRA_SC_LIBS client)
SET(WRAPPER_MODULE_DataModel_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_Math_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_Client_EXTRA_SC_LIBS client)
SET(WRAPPER_MODULE_Utils_EXTRA_SC_LIBS core)
SET(WRAPPER_MODULE_Seismology_EXTRA_SC_LIBS core)

SET(WRAPPER_MODULE_helpers_EXTRA_FILES helpers.cpp)
SET(WRAPPER_MODULE_helpers_EXTRA_SC_LIBS core)


FOREACH (wrapper_module ${WRAPPER_MODULES})
        SET(
                ${wrapper_module}_SOURCES
                        ${wrapper_module}PYTHON_wrap.cxx
                        ${WRAPPER_MODULE_${wrapper_module}_EXTRA_FILES}
        )
        ADD_LIBRARY(_${wrapper_module} MODULE ${${wrapper_module}_SOURCES})
        SET_TARGET_PROPERTIES(_${wrapper_module} PROPERTIES PREFIX "")
        TARGET_LINK_LIBRARIES(_${wrapper_module} ${PYTHON_LIBRARIES})
        TARGET_LINK_LIBRARIES(_${wrapper_module} ${WRAPPER_MODULE_${wrapper_module}_EXTRA_LIBS})
        SC_LINK_LIBRARIES_INTERNAL(_${wrapper_module} ${WRAPPER_MODULE_${wrapper_module}_EXTRA_SC_LIBS})
ENDFOREACH (wrapper_module)

SC_SWIG_GET_MODULE_PATH(_module_path)
SET(_module_path ${SC3_PACKAGE_PYTHON_LIB_DIR}/seiscomp3)

FOREACH (wrapper_module ${WRAPPER_MODULES})
        INSTALL(
                TARGETS _${wrapper_module}
                LIBRARY DESTINATION ${_module_path}
        )

        INSTALL(
                FILES ${CMAKE_CURRENT_SOURCE_DIR}/${wrapper_module}.py
                DESTINATION ${_module_path}
        )

        INSTALL(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${wrapper_module}.pyc
                DESTINATION ${_module_path}
                OPTIONAL
        )

        INSTALL(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${wrapper_module}.pyo
                DESTINATION ${_module_path}
                OPTIONAL
        )

        ADD_CUSTOM_COMMAND(
                TARGET _${wrapper_module}
                POST_BUILD
                COMMAND ${CMAKE_SOURCE_DIR}/py-compile ARGS --destdir ${CMAKE_CURRENT_BINARY_DIR} ${wrapper_module}.py
        )
ENDFOREACH (wrapper_module)

