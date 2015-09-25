MACRO(SC_BEGIN_PACKAGE ...)
	SET(SC3_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	SET(SC3_PACKAGE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	IF (${ARGC} GREATER 1)
		SET(SC3_PACKAGE_DIR ${ARGV1})
		SET(SC3_PACKAGE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/${ARGV1})
		SET(SC3_PACKAGE_BIN_DIR ${ARGV1}/bin)
		SET(SC3_PACKAGE_SBIN_DIR ${ARGV1}/sbin)
		SET(SC3_PACKAGE_LIB_DIR ${ARGV1}/lib)
		SET(SC3_PACKAGE_PYTHON_LIB_DIR ${ARGV1}/${PYTHON_LIBRARY_PATH})
		SET(SC3_PACKAGE_INCLUDE_DIR ${ARGV1}/include)
		SET(SC3_PACKAGE_SHARE_DIR ${ARGV1}/share)
		SET(SC3_PACKAGE_INIT_DIR ${ARGV1}/etc/init)
		SET(SC3_PACKAGE_CONFIG_DIR ${ARGV1}/etc/defaults)
		SET(SC3_PACKAGE_APP_CONFIG_DIR ${ARGV1}/etc)
		SET(SC3_PACKAGE_APP_DESC_DIR ${ARGV1}/etc/descriptions)
		SET(SC3_PACKAGE_TEMPLATES_DIR ${ARGV1}/share/templates)
	ELSE (${ARGC} GREATER 1)
		SET(SC3_PACKAGE_DIR ".")
		SET(SC3_PACKAGE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
		SET(SC3_PACKAGE_BIN_DIR bin)
		SET(SC3_PACKAGE_SBIN_DIR sbin)
		SET(SC3_PACKAGE_LIB_DIR lib)
		SET(SC3_PACKAGE_PYTHON_LIB_DIR ${PYTHON_LIBRARY_PATH})
		SET(SC3_PACKAGE_INCLUDE_DIR include)
		SET(SC3_PACKAGE_SHARE_DIR share)
		SET(SC3_PACKAGE_INIT_DIR etc/init)
		SET(SC3_PACKAGE_CONFIG_DIR etc/defaults)
		SET(SC3_PACKAGE_APP_CONFIG_DIR etc)
		SET(SC3_PACKAGE_APP_DESC_DIR etc/descriptions)
		SET(SC3_PACKAGE_TEMPLATES_DIR share/templates)
	ENDIF (${ARGC} GREATER 1)
	MESSAGE(STATUS "Adding pkg ${ARGV0}")
	MESSAGE(STATUS "... resides in ${SC3_PACKAGE_SOURCE_DIR}.")
	MESSAGE(STATUS "... will be installed under ${SC3_PACKAGE_INSTALL_PREFIX}.")
ENDMACRO(SC_BEGIN_PACKAGE)

MACRO(SC_ADD_SUBDIRS)
	FILE(GLOB files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "[^.]*")
	SET(dirs "")
	SET(prio_dirs " ")
	IF (${ARGC} GREATER 0)
		SET(prio_dirs ${ARGN})
		LIST(REVERSE prio_dirs)
	ENDIF (${ARGC} GREATER 0)
	FOREACH(dir ${files})
		IF (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
			SET(dirs ${dirs} ${dir})
		ENDIF (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
	ENDFOREACH(dir ${files})
	IF(prio_dirs)
		FOREACH(prio_dir ${prio_dirs})
			LIST(FIND dirs ${prio_dir} dirs_index)
			IF(${dirs_index} GREATER -1)
				LIST(REMOVE_AT dirs ${dirs_index})
				SET(dirs ${prio_dir} ${dirs})
			ENDIF(${dirs_index} GREATER -1)
		ENDFOREACH(prio_dir ${prio_dirs})
	ENDIF(prio_dirs)
	SUBDIRS(${dirs})
ENDMACRO(SC_ADD_SUBDIRS)

MACRO(SC_SETUP_LIB_SUBDIR _package)
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY SOURCES ${${_package}_SOURCES})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY HEADERS ${${_package}_HEADERS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFS ${${_package}_DEFINITIONS})
ENDMACRO(SC_SETUP_LIB_SUBDIR)


# This macro changed in CMake 2.8 it does not work anymore correctly so
# it is included here as _COMPAT version
MACRO (QT4_EXTRACT_OPTIONS_COMPAT _qt4_files _qt4_options)
  SET(${_qt4_files})
  SET(${_qt4_options})
  SET(_QT4_DOING_OPTIONS FALSE)
  FOREACH(_currentArg ${ARGN})
    IF ("${_currentArg}" STREQUAL "OPTIONS")
      SET(_QT4_DOING_OPTIONS TRUE)
    ELSE ("${_currentArg}" STREQUAL "OPTIONS")
      IF(_QT4_DOING_OPTIONS) 
        LIST(APPEND ${_qt4_options} "${_currentArg}")
      ELSE(_QT4_DOING_OPTIONS)
        LIST(APPEND ${_qt4_files} "${_currentArg}")
      ENDIF(_QT4_DOING_OPTIONS)
    ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
  ENDFOREACH(_currentArg) 
ENDMACRO (QT4_EXTRACT_OPTIONS_COMPAT)


MACRO(SC_QT4_WRAP_UI outfiles)
  QT4_EXTRACT_OPTIONS_COMPAT(ui_files ui_options ${ARGN})

  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(_rel ${it} PATH)
    IF (_rel)
      SET(_rel "${_rel}/")
    ENDIF (_rel)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${_rel}ui_${outfile}.h) # Here we set output
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
ENDMACRO (SC_QT4_WRAP_UI)


MACRO(SC_SETUP_GUI_LIB_SUBDIR _package)
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY SOURCES ${${_package}_SOURCES})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY HEADERS ${${_package}_HEADERS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFS ${${_package}_DEFINITIONS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY MOCS ${${_package}_MOC_HEADERS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY UIS ${${_package}_UI})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY RESOURCES ${${_package}_RESOURCES})
ENDMACRO(SC_SETUP_GUI_LIB_SUBDIR)


MACRO(SC_ADD_SUBDIR_SOURCES ...)
        SET(prefix ${ARGV0})
        SET(dir ${ARGV1})
        ADD_SUBDIRECTORY(${dir})
        SET(sources "")
        SET(headers "")
        SET(defs "")
        GET_PROPERTY(sources DIRECTORY ${dir} PROPERTY SOURCES)
        GET_PROPERTY(headers DIRECTORY ${dir} PROPERTY HEADERS)
        GET_PROPERTY(defs DIRECTORY ${dir} PROPERTY DEFS)

        ADD_DEFINITIONS(${defs})
	SET(${prefix}_DEFINITIONS ${${prefix}_DEFINITIONS} ${defs})

        FOREACH (_src ${sources})
                SET(_src ${dir}/${_src})
                SET(${prefix}_SOURCES ${${prefix}_SOURCES} ${_src})
		#IF(NOT "${defs}" STREQUAL "")
		#	SET_SOURCE_FILES_PROPERTIES(${_src} COMPILE_FLAGS ${defs})
		#ENDIF(NOT "${defs}" STREQUAL "")
        ENDFOREACH(_src)

        FILE(RELATIVE_PATH _package_dir ${SC3_PACKAGE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR})

        FOREACH (_head ${headers})
                SET(_head ${dir}/${_head})
                SET(${prefix}_HEADERS ${${prefix}_HEADERS} ${_head})
                IF(NOT ARGV2)
                        INSTALL(FILES ${_head} DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}/${dir})
                ENDIF(NOT ARGV2)
        ENDFOREACH(_head)
ENDMACRO(SC_ADD_SUBDIR_SOURCES)


MACRO(SC_ADD_GUI_SUBDIR_SOURCES ...)
        SET(prefix ${ARGV0})
        SET(dir ${ARGV1})
        ADD_SUBDIRECTORY(${dir})
        SET(sources "")
        SET(headers "")
        SET(defs "")
        SET(mocs "")
        SET(uis "")
        SET(resources "")
        GET_PROPERTY(sources DIRECTORY ${dir} PROPERTY SOURCES)
        GET_PROPERTY(headers DIRECTORY ${dir} PROPERTY HEADERS)
        GET_PROPERTY(defs DIRECTORY ${dir} PROPERTY DEFS)
        GET_PROPERTY(mocs DIRECTORY ${dir} PROPERTY MOCS)
        GET_PROPERTY(uis DIRECTORY ${dir} PROPERTY UIS)
        GET_PROPERTY(resources DIRECTORY ${dir} PROPERTY RESOURCES)

        ADD_DEFINITIONS(${defs})
	SET(${prefix}_DEFINITION ${defs})

        FOREACH (_src ${sources})
                SET(_src ${dir}/${_src})
                SET(${prefix}_SOURCES ${${prefix}_SOURCES} ${_src})
        ENDFOREACH(_src)

        FILE(RELATIVE_PATH _package_dir ${SC3_PACKAGE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR})

        FOREACH (_head ${headers})
                SET(_head ${dir}/${_head})
                SET(${prefix}_HEADERS ${${prefix}_HEADERS} ${_head})
                IF(NOT ARGV2)
                        INSTALL(FILES ${_head} DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}/${dir})
                ENDIF(NOT ARGV2)
        ENDFOREACH(_head)

        FOREACH (_moc ${mocs})
                SET(_moc ${dir}/${_moc})
                SET(${prefix}_MOC_HEADERS ${${prefix}_MOC_HEADERS} ${_moc})
                IF(NOT ARGV2)
                        INSTALL(FILES ${_moc} DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}/${dir})
                ENDIF(NOT ARGV2)
        ENDFOREACH(_moc)

        FOREACH (_res ${resources})
                SET(_res ${dir}/${_res})
                SET(${prefix}_RESOURCES ${${prefix}_RESOURCES} ${_res})
        ENDFOREACH(_res)

        FOREACH (_ui ${uis})
                SET(_ui ${dir}/${_ui})
                SET(${prefix}_UI ${${prefix}_UI} ${_ui})
		# TODO: Add QT4_WRAP_HERE and set the install directory here as well
		# in cmakelists.txt set UI_HEADERS to ""
		SET(_ui_out "")
		SC_QT4_WRAP_UI(_ui_out ${_ui})
		IF(NOT ARGV2)
			INSTALL(FILES ${_ui_out} DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}/${dir})
		ENDIF(NOT ARGV2)
        ENDFOREACH(_ui)
ENDMACRO(SC_ADD_GUI_SUBDIR_SOURCES)


MACRO(SC_ADD_LIBRARY _library_package _library_name)
	SET(_global_library_package SC_${_library_package})

	IF (SHARED_LIBRARIES)
		SET(${_library_package}_TYPE SHARED)
		SET(${_global_library_package}_SHARED 1)
		IF (WIN32)
			ADD_DEFINITIONS(-D${_global_library_package}_EXPORTS)
		ENDIF (WIN32)
	ENDIF (SHARED_LIBRARIES)

	SET(LIBRARY ${_global_library_package})
	SET(LIBRARY_NAME ${_library_name})

	IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
		CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
		CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config.h
		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
		SET(${_library_package}_HEADERS
			${${_library_package}_HEADERS}
			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H}
			${CMAKE_CURRENT_BINARY_DIR}/config.h)
	ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
		SET(${_library_package}_HEADERS
			${${_library_package}_HEADERS}
			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
	ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)

	ADD_LIBRARY(seiscomp3_${_library_name} ${${_library_package}_TYPE} ${${_library_package}_SOURCES})

	INSTALL(TARGETS seiscomp3_${_library_name}
		RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR}
		ARCHIVE DESTINATION ${SC3_PACKAGE_LIB_DIR}
		LIBRARY DESTINATION ${SC3_PACKAGE_LIB_DIR}
	)
ENDMACRO(SC_ADD_LIBRARY)


MACRO(SC_ADD_PLUGIN_LIBRARY _library_package _library_name _plugin_app)
	SET(_global_library_package SEISCOMP3_PLUGIN_${_library_package})

	SET(${_global_library_package}_SHARED 1)

	ADD_LIBRARY(${_library_name} MODULE ${${_library_package}_SOURCES})
	SET_TARGET_PROPERTIES(${_library_name} PROPERTIES PREFIX "")

	SET(LIBRARY ${_global_library_package})
	SET(LIBRARY_NAME ${_library_name})

	INSTALL(TARGETS ${_library_name}
		DESTINATION ${SC3_PACKAGE_SHARE_DIR}/plugins/${_plugin_app}
	)
ENDMACRO(SC_ADD_PLUGIN_LIBRARY)


MACRO(SC_ADD_GUI_PLUGIN_LIBRARY _library_package _library_name _plugin_app)
	INCLUDE(${QT_USE_FILE})

	SET(_global_library_package SEISCOMP3_PLUGIN_${_library_package})

	SET(${_global_library_package}_SHARED 1)

	# Create MOC Files
	IF (${_library_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_library_package}_MOC_SOURCES ${${_library_package}_MOC_HEADERS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
	ENDIF (${_library_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_library_package}_UI)
		SC_QT4_WRAP_UI(${_library_package}_UI_HEADERS ${${_library_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_library_package}_UI)

    # Add resources
	IF (${_library_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_library_package}_RESOURCE_SOURCES ${${_library_package}_RESOURCES})
	ENDIF (${_library_package}_RESOURCES)

	SET(
		${_library_package}_FILES
			${${_library_package}_SOURCES}
			${${_library_package}_MOC_SOURCES}
			${${_library_package}_UI_HEADERS}
			${${_library_package}_RESOURCE_SOURCES}
	)

	ADD_LIBRARY(${_library_name} MODULE ${${_library_package}_FILES})
	SET_TARGET_PROPERTIES(${_library_name} PROPERTIES PREFIX "")
	TARGET_LINK_LIBRARIES(${_library_name} ${QT_LIBRARIES})

	SET(LIBRARY ${_global_library_package})
	SET(LIBRARY_NAME ${_library_name})

	INSTALL(TARGETS ${_library_name}
		DESTINATION ${SC3_PACKAGE_SHARE_DIR}/plugins/${_plugin_app}
	)
ENDMACRO(SC_ADD_GUI_PLUGIN_LIBRARY)


MACRO(SC_ADD_GUI_LIBRARY_CUSTOM_INSTALL _library_package _library_name)
	INCLUDE(${QT_USE_FILE})

	SET(_global_library_package SC_${_library_package})

	IF (SHARED_LIBRARIES)
		IF (WIN32)
			ADD_DEFINITIONS(-D${_global_library_package}_EXPORTS)
		ENDIF (WIN32)
		SET(${_library_package}_TYPE SHARED)
		SET(${_global_library_package}_SHARED 1)
	ENDIF (SHARED_LIBRARIES)

	# Create MOC Files
	IF (${_library_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_library_package}_MOC_SOURCES ${${_library_package}_MOC_HEADERS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
	ENDIF (${_library_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_library_package}_UI)
		SC_QT4_WRAP_UI(${_library_package}_UI_HEADERS ${${_library_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_library_package}_UI)

	# Add resources
	IF (${_library_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_library_package}_RESOURCE_SOURCES ${${_library_package}_RESOURCES})
	ENDIF (${_library_package}_RESOURCES)

	SET(LIBRARY ${_global_library_package})
	SET(LIBRARY_NAME ${_library_name})

	IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
		CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
		CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config.h
		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
		SET(${_library_package}_HEADERS
			${${_library_package}_HEADERS}
			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H}
			${CMAKE_CURRENT_BINARY_DIR}/config.h)
	ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
		SET(${_library_package}_HEADERS
			${${_library_package}_HEADERS}
			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
	ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)

	SET(
	${_library_package}_FILES_
	    ${${_library_package}_SOURCES}
	    ${${_library_package}_MOC_SOURCES}
	    ${${_library_package}_UI_HEADERS}
	    ${${_library_package}_RESOURCE_SOURCES}
	)

	ADD_LIBRARY(seiscomp3_${_library_name} ${${_library_package}_TYPE} ${${_library_package}_FILES_})
	TARGET_LINK_LIBRARIES(seiscomp3_${_library_name} ${QT_LIBRARIES})
ENDMACRO(SC_ADD_GUI_LIBRARY_CUSTOM_INSTALL)


MACRO(SC_ADD_GUI_LIBRARY _library_package _library_name)
	SC_ADD_GUI_LIBRARY_CUSTOM_INSTALL(${_library_package} ${_library_name})

	INSTALL(TARGETS seiscomp3_${_library_name}
		RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR}
		ARCHIVE DESTINATION ${SC3_PACKAGE_LIB_DIR}
		LIBRARY DESTINATION ${SC3_PACKAGE_LIB_DIR}
	)
ENDMACRO(SC_ADD_GUI_LIBRARY)


MACRO(SC_ADD_EXECUTABLE _package _name)
	IF (${ARGC} GREATER 2)
		SET(bin_dir ${ARGV2})
	ELSE (${ARGC} GREATER 2)
		SET(bin_dir ${SC3_PACKAGE_BIN_DIR})
	ENDIF (${ARGC} GREATER 2)
	ADD_EXECUTABLE(${_name} ${${_package}_SOURCES})
	INSTALL(TARGETS ${_name}
		RUNTIME DESTINATION ${bin_dir}
		ARCHIVE DESTINATION ${SC3_PACKAGE_LIB_DIR}
		LIBRARY DESTINATION ${SC3_PACKAGE_LIB_DIR}
	)

	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)
		INSTALL(FILES config/${_name}.cfg
			DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)

	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
		INSTALL(FILES config/${_name}.xml
			DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)

	# Install all XML files in apps config dir to etc/descriptions
	#FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/config/*.xml")
	#INSTALL(FILES ${files} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
ENDMACRO(SC_ADD_EXECUTABLE)


MACRO(SC_ADD_PYTHON_EXECUTABLE _name)
	FOREACH(file ${${_name}_FILES})
		IF(NOT MAIN_PY)
			SET(MAIN_PY ${file})
		ENDIF(NOT MAIN_PY)
	ENDFOREACH(file)

	# Pop main_py
	LIST(REMOVE_AT ${_name}_FILES 0)

	INSTALL(FILES ${${_name}_FILES} DESTINATION ${SC3_PACKAGE_BIN_DIR})
	INSTALL(PROGRAMS ${MAIN_PY} RENAME ${${_name}_TARGET} DESTINATION ${SC3_PACKAGE_BIN_DIR})

	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${${_name}_TARGET}.cfg)
		INSTALL(FILES config/${${_name}_TARGET}.cfg
			DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${${_name}_TARGET}.cfg)
	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
		INSTALL(FILES config/${_name}.xml
			DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
ENDMACRO(SC_ADD_PYTHON_EXECUTABLE)


MACRO(SC_ADD_PYTHON_PROG _name)
	INSTALL(PROGRAMS ${_name}.py RENAME ${_name} DESTINATION ${SC3_PACKAGE_BIN_DIR})
	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)
		INSTALL(FILES config/${_name}.cfg
			DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)
	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
		INSTALL(FILES config/${_name}.xml
			DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
ENDMACRO(SC_ADD_PYTHON_PROG)

MACRO(SC_ADD_TEST_EXECUTABLE _package _name)
	ADD_DEFINITIONS(-DSEISCOMP_TEST_DATA_DIR=\\\""${PROJECT_TEST_DATA_DIR}"\\\")
	ADD_EXECUTABLE(${_name} ${${_package}_SOURCES})
ENDMACRO(SC_ADD_TEST_EXECUTABLE)



MACRO(SC_ADD_GUI_EXECUTABLE _package _name)
	INCLUDE(${QT_USE_FILE})

	# Create MOC Files
	IF (${_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_package}_MOC_SOURCES ${${_package}_MOC_HEADERS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
	ENDIF (${_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_package}_UI)
		QT4_WRAP_UI(${_package}_UI_HEADERS ${${_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_package}_UI)

	# Add resources
	IF (${_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_package}_RESOURCE_SOURCES ${${_package}_RESOURCES})
	ENDIF (${_package}_RESOURCES)

	SET(
		${_package}_FILES_
			${${_package}_SOURCES}
			${${_package}_MOC_SOURCES}
			${${_package}_UI_HEADERS}
			${${_package}_RESOURCE_SOURCES}
	)

	IF(WIN32)
		ADD_EXECUTABLE(${_name} WIN32 ${${_package}_FILES_})
		TARGET_LINK_LIBRARIES(${_name} ${QT_QTMAIN_LIBRARY})
	ELSE(WIN32)
		ADD_EXECUTABLE(${_name} ${${_package}_FILES_})
	ENDIF(WIN32)
	TARGET_LINK_LIBRARIES(${_name} ${QT_LIBRARIES})
	TARGET_LINK_LIBRARIES(${_name} ${QT_QTOPENGL_LIBRARY})

	INSTALL(TARGETS ${_name}
		RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR}
		ARCHIVE DESTINATION ${SC3_PACKAGE_LIB_DIR}
		LIBRARY DESTINATION ${SC3_PACKAGE_LIB_DIR}
	)

	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)
		INSTALL(FILES config/${_name}.cfg
			DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.cfg)
	IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
		INSTALL(FILES config/${_name}.xml
			DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
	ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/${_name}.xml)
ENDMACRO(SC_ADD_GUI_EXECUTABLE)


MACRO(SC_ADD_GUI_TEST_EXECUTABLE _package _name)
	ADD_DEFINITIONS(-DSEISCOMP_TEST_DATA_DIR=\\\""${PROJECT_TEST_DATA_DIR}"\\\")
	INCLUDE(${QT_USE_FILE})

	# Create MOC Files
	IF (${_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_package}_MOC_SOURCES ${${_package}_MOC_HEADERS} OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
	ENDIF (${_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_package}_UI)
		QT4_WRAP_UI(${_package}_UI_HEADERS ${${_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_package}_UI)

	# Add resources
	IF (${_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_package}_RESOURCE_SOURCES ${${_package}_RESOURCES})
	ENDIF (${_package}_RESOURCES)

	SET(
		${_package}_FILES_
			${${_package}_SOURCES}
			${${_package}_MOC_SOURCES}
			${${_package}_UI_HEADERS}
			${${_package}_RESOURCE_SOURCES}
	)

	IF(WIN32)
		ADD_EXECUTABLE(${_name} WIN32 ${${_package}_FILES_})
		TARGET_LINK_LIBRARIES(${_name} ${QT_QTMAIN_LIBRARY})
	ELSE(WIN32)
		ADD_EXECUTABLE(${_name} ${${_package}_FILES_})
	ENDIF(WIN32)
	TARGET_LINK_LIBRARIES(${_name} ${QT_LIBRARIES})
	TARGET_LINK_LIBRARIES(${_name} ${QT_QTOPENGL_LIBRARY})
ENDMACRO(SC_ADD_GUI_TEST_EXECUTABLE)


MACRO(SC_LINK_LIBRARIES _name)
	TARGET_LINK_LIBRARIES(${_name} ${ARGN})
ENDMACRO(SC_LINK_LIBRARIES)


MACRO(SC_LINK_LIBRARIES_INTERNAL _name)
	FOREACH(_lib ${ARGN})
		TARGET_LINK_LIBRARIES(${_name} seiscomp3_${_lib})
	ENDFOREACH(_lib)
ENDMACRO(SC_LINK_LIBRARIES_INTERNAL)


MACRO(SC_LIB_LINK_LIBRARIES _library_name)
	TARGET_LINK_LIBRARIES(seiscomp3_${_library_name} ${ARGN})
ENDMACRO(SC_LIB_LINK_LIBRARIES)


MACRO(SC_LIB_LINK_LIBRARIES_INTERNAL _library_name)
	FOREACH(_lib ${ARGN})
		TARGET_LINK_LIBRARIES(seiscomp3_${_library_name} seiscomp3_${_lib})
	ENDFOREACH(_lib)
ENDMACRO(SC_LIB_LINK_LIBRARIES_INTERNAL)


MACRO(SC_SWIG_LINK_LIBRARIES_INTERNAL _module)
    FOREACH(_lib ${ARGN})
	SWIG_LINK_LIBRARIES(${_module} seiscomp3_${_lib})
    ENDFOREACH(_lib)
ENDMACRO(SC_SWIG_LINK_LIBRARIES_INTERNAL)


MACRO(SC_SWIG_GET_MODULE_PATH _out)
    FILE(RELATIVE_PATH ${_out} ${SC3_PACKAGE_SOURCE_DIR}/libs/swig ${CMAKE_CURRENT_SOURCE_DIR})
    SET(${_out} ${SC3_PACKAGE_LIB_DIR}${PYTHON_LIBRARY_SUFFIX}/${${_out}})
ENDMACRO(SC_SWIG_GET_MODULE_PATH)


MACRO(SC_LIB_VERSION _library_name _version _soversion)
    SET_TARGET_PROPERTIES(seiscomp3_${_library_name} PROPERTIES VERSION ${_version} SOVERSION ${_soversion})
ENDMACRO(SC_LIB_VERSION)


MACRO(SC_LIB_INSTALL_HEADERS ...)
	IF(${ARGC} GREATER 1)
		SET(_package_dir "${ARGV1}")
	ELSE(${ARGC} GREATER 1)
		FILE(RELATIVE_PATH _package_dir ${SC3_PACKAGE_SOURCE_DIR}/libs ${CMAKE_CURRENT_SOURCE_DIR})
	ENDIF(${ARGC} GREATER 1)

	INSTALL(FILES ${${ARGV0}_HEADERS}
		DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}
	)

	IF (${ARGV0}_MOC_HEADERS)
		INSTALL(FILES ${${ARGV0}_MOC_HEADERS}
			DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}
		)
	ENDIF (${ARGV0}_MOC_HEADERS)

	IF (${ARGV0}_UI_HEADERS)
		INSTALL(FILES ${${ARGV0}_UI_HEADERS}
			DESTINATION ${SC3_PACKAGE_INCLUDE_DIR}/${_package_dir}
		)
	ENDIF (${ARGV0}_UI_HEADERS)
ENDMACRO(SC_LIB_INSTALL_HEADERS)


MACRO(SC_PLUGIN_INSTALL _library_name _plugin_app)
	INSTALL(TARGETS seiscomp3_${_library_name}
		DESTINATION ${SC3_PACKAGE_SHARE_DIR}/plugins/${_plugin_app}
	)
ENDMACRO(SC_PLUGIN_INSTALL)


MACRO(SC_RAW_PLUGIN_INSTALL _library_name _plugin_app)
	INSTALL(TARGETS ${_library_name}
		DESTINATION ${SC3_PACKAGE_SHARE_DIR}/plugins/${_plugin_app}
	)
ENDMACRO(SC_RAW_PLUGIN_INSTALL)


MACRO(SC_INSTALL_DATA _package _path)
	INSTALL(FILES ${${_package}_DATA}
		DESTINATION ${SC3_PACKAGE_SHARE_DIR}/${_path}
	)
ENDMACRO(SC_INSTALL_DATA)


MACRO(SC_INSTALL_CONFIG _package)
	INSTALL(FILES ${${_package}_CONFIG}
		DESTINATION ${SC3_PACKAGE_CONFIG_DIR})
ENDMACRO(SC_INSTALL_CONFIG)

MACRO(SC_INSTALL_INIT _module_name _script)
	INSTALL(FILES ${_script} RENAME ${_module_name}.py DESTINATION ${SC3_PACKAGE_INIT_DIR})
ENDMACRO(SC_INSTALL_INIT)
