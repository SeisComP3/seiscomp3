# ------------------------------------------------------
# ------ CORE LIBRARY ----------------------------------
# ------------------------------------------------------

SET(SYSTEM_CORE_SOURCES "")
SET(SYSTEM_CORE_HEADERS "")

INCLUDE_DIRECTORIES(${LIBMSEED_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${Boost_filesystem_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Boost_regex_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Boost_thread_INCLUDE_DIRS})

SET(GEN_VERSION ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

SET_SOURCE_FILES_PROPERTIES(${GEN_VERSION} PROPERTIES GENERATED true)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
        ADD_DEFINITIONS(-DWITH_SVN_REVISION -DWITH_BUILD_INFOS)
        ADD_CUSTOM_COMMAND(
                OUTPUT  ${GEN_VERSION}
                COMMAND ${CMAKE_COMMAND} -E echo_append 'const char* git_revision\(void\)  {const char* GIT_REVISION= \"'  > ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND bash -c 'echo -n `git --git-dir="${CMAKE_SOURCE_DIR}/.git"  --work-tree="${CMAKE_SOURCE_DIR}" rev-parse HEAD`' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo '\"\; return GIT_REVISION\; }' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append 'char const* build_system\(void\) {char const* BUILD_SYSTEM = \"' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append `uname -sr` >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo '\"\; return BUILD_SYSTEM\;}' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp

                COMMAND ${CMAKE_COMMAND} -E echo_append 'char const* compiler_version\(void\) {char const* COMPILER_VERSION = \"' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append `${CMAKE_CXX_COMPILER} --version | head -1` >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo '\"\; return COMPILER_VERSION\;}' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp

                COMMAND ${CMAKE_COMMAND} -E echo_append 'char const* os_version\(void\) {char const* OS_VERSION = \"' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append `lsb_release -sd | sed 's/\"//g'` >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append '/' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo_append '${BUILD_ARCH}' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo '\"\; return OS_VERSION\;}' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp

                COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp ${GEN_VERSION}
                COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMENT "create ${GEN_VERSION} for debug-mode"
        )
else(CMAKE_BUILD_TYPE STREQUAL Debug)
        ADD_DEFINITIONS(-DWITH_SVN_REVISION)
        ADD_CUSTOM_COMMAND(
                OUTPUT ${GEN_VERSION}
                COMMAND ${CMAKE_COMMAND} -E echo_append 'const char* git_revision\(void\)  {const char* GIT_REVISION= \"'  > ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND bash -c 'echo -n `git --git-dir="${CMAKE_SOURCE_DIR}/.git"  --work-tree="${CMAKE_SOURCE_DIR}" rev-parse HEAD`' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E echo '\"\; return GIT_REVISION\; }' >> ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMAND ${CMAKE_COMMAND} -E copy_if_different  ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp ${GEN_VERSION}
                COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/version_pre.cpp
                COMMENT "create ${GEN_VERSION} for release"
        )
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

SET(SYSTEM_CORE_SOURCES ${SYSTEM_CORE_SOURCES} ${GEN_VERSION})


SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE logging)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE core)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE system)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE io)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE utils)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE datamodel)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE math)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE geo)

SC_ADD_SUBDIR_SOURCES(SYSTEM_CORE seismology)

SET(THIRD_PARTY_DIRECTORY ../3rd-party)
SET(LIBLOCSAT_PATH ${THIRD_PARTY_DIRECTORY}/locsat/lib)

SET(HAVE_LOCSAT 1)

INCLUDE_DIRECTORIES(${THIRD_PARTY_DIRECTORY}/tau)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include/db3)
INCLUDE_DIRECTORIES(${LIBLOCSAT_PATH}/include/css)

# Add libtau and LOCSAT sources
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libgeog LIBGEOG_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libinterp LIBINTERP_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libloc LIBLOC_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/librdwrt LIBRDWRT_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libtime LIBTIME_SOURCES)
AUX_SOURCE_DIRECTORY(${LIBLOCSAT_PATH}/libf2c LIBF2C_SOURCES)

SET(SYSTEM_CORE_SOURCES ${SYSTEM_CORE_SOURCES}
	${LIBGEOG_SOURCES}
	${LIBINTERP_SOURCES}
	${LIBLOC_SOURCES}
	${LIBRDWRT_SOURCES}
	${LIBTIME_SOURCES}
	${LIBXPRT2_SOURCES}
	${LIBF2C_SOURCES}
)


SET(PROJECT_API_H core.h)
SET(SYSTEM_CORE_HEADERS "")
SC_ADD_LIBRARY(SYSTEM_CORE core)

SC_LIB_INSTALL_HEADERS(SYSTEM_CORE)

SC_LIB_LINK_LIBRARIES(
	core
		${Boost_thread_LIBRARY}
		${Boost_iostreams_LIBRARY}
		${Boost_filesystem_LIBRARY}
		${Boost_regex_LIBRARY}
		${Boost_LIBRARIES}
		tau_internal
		${LIBXML2_LIBRARIES}
		${OPENSSL_LIBRARIES}
)

SC_LIB_LINK_LIBRARIES_INTERNAL(core config)

IF(WIN32)
	SC_LIB_LINK_LIBRARIES(core ws2_32)
ENDIF(WIN32)

IF(WIN32)
	SC_LIB_LINK_LIBRARIES(core zlib)
ENDIF(WIN32)

IF ( MACOSX )
    SC_LIB_LINK_LIBRARIES(core iconv)
ENDIF ( MACOSX )

IF (CMAKE_SYSTEM_NAME STREQUAL SunOS)
	SC_LIB_LINK_LIBRARIES(core iconv)
ENDIF (CMAKE_SYSTEM_NAME STREQUAL SunOS)

IF (MSEED_FOUND)
	SC_LIB_LINK_LIBRARIES(core ${LIBMSEED_LIBRARY})
ENDIF (MSEED_FOUND)


# ------------------------------------------------------
# ------ CLIENT LIBRARY --------------------------------
# ------------------------------------------------------

SET(SYSTEM_CLIENT_SOURCES "")
SET(SYSTEM_CLIENT_HEADERS "")

SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT communication)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT client)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT processing)
SC_ADD_SUBDIR_SOURCES(SYSTEM_CLIENT qc)

SET(PROJECT_API_H client.h)
SET(SYSTEM_CLIENT_HEADERS "")


IF(NOT WIN32)
        SET(SPREAD_SOURCE_DIR ${SC3_PACKAGE_SOURCE_DIR}/libs/3rd-party/spread)
        SET(SPREAD_BINARY_DIR ${SC3_PACKAGE_BINARY_DIR}/libs/3rd-party/spread)
        SET(SPREAD_LIB_DIR ${SPREAD_BINARY_DIR}/lib)
        SET(SPREAD_CONF_DIR ${SPREAD_BINARY_DIR}/share/doc/spread)
        SET(SPREAD_LIBRARIES ${SPREAD_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}tspread-core${CMAKE_SHARED_LIBRARY_SUFFIX})
        SET(SPREAD_INC ${SPREAD_SOURCE_DIR}/include)
        INCLUDE_DIRECTORIES(${SPREAD_INC})

        SET_SOURCE_FILES_PROPERTIES(
                communication/spread/spreaddriver.cpp
                PROPERTIES OBJECT_DEPENDS ${SPREAD_LIBRARIES}
        )

        SET_DIRECTORY_PROPERTIES(
                PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${SPREAD_LIBRARIES}"
        )

        IF(CMAKE_HOST_ARCHITECTURE)
                ADD_CUSTOM_COMMAND(
                        OUTPUT ${SPREAD_LIBRARIES}
                        COMMAND ./configure ARGS --prefix=${SPREAD_BINARY_DIR} --libdir=${SPREAD_LIB_DIR} --sysconfdir=${SPREAD_CONF_DIR} --host ${CMAKE_HOST_ARCHITECTURE}
                        COMMAND ${CMAKE_MAKE_PROGRAM}
                        COMMAND ${CMAKE_MAKE_PROGRAM} ARGS install
                        WORKING_DIRECTORY ${SPREAD_SOURCE_DIR}
                )

        ELSE(CMAKE_HOST_ARCHITECTURE)
                ADD_CUSTOM_COMMAND(
                        OUTPUT ${SPREAD_LIBRARIES}
                        COMMAND ./configure ARGS --prefix=${SPREAD_BINARY_DIR} --libdir=${SPREAD_LIB_DIR} --sysconfdir=${SPREAD_CONF_DIR}
                        COMMAND ${CMAKE_MAKE_PROGRAM}
                        COMMAND ${CMAKE_MAKE_PROGRAM} ARGS install
                        WORKING_DIRECTORY ${SPREAD_SOURCE_DIR}
                )
        ENDIF(CMAKE_HOST_ARCHITECTURE)
ENDIF(NOT WIN32)

INCLUDE_DIRECTORIES(${SPREAD_INCLUDE_DIR})

IF(NOT WIN32)
        MAKE_DIRECTORY(${SPREAD_BINARY_DIR})
        INSTALL(
                DIRECTORY
                        ${SPREAD_BINARY_DIR}/
                DESTINATION
                        ${SC3_PACKAGE_DIR}
                USE_SOURCE_PERMISSIONS
                PATTERN "CMakeFiles" EXCLUDE
                PATTERN "*.cmake" EXCLUDE
                PATTERN "Makefile" EXCLUDE
        )
ENDIF(NOT WIN32)

SC_ADD_LIBRARY(SYSTEM_CLIENT client)
SC_LIB_INSTALL_HEADERS(SYSTEM_CLIENT)

SET(SYSTEM_CLIENT_DATA client/data/cities.xml)
SC_INSTALL_DATA(SYSTEM_CLIENT "")

SC_LIB_LINK_LIBRARIES_INTERNAL(client core)
SC_LIB_LINK_LIBRARIES(client
	${Boost_iostreams_LIBRARY}
	${Boost_thread_LIBRARY}
	${Boost_program_options_LIBRARY}
	${Boost_LIBRARIES}
	${SPREAD_LIBRARIES}
)

IF(WIN32)
        SC_LIB_LINK_LIBRARIES(client zlib psapi)
ENDIF(WIN32)
