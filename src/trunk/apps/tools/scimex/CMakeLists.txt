SET(SCIMEX_TARGET scimex)

SET(
    SCIMEX_HEADERS
    imeximpl.h
    imex.h
    imexmessage.h
    criterion.h
    imexscdm051.h
)

SET(
    SCIMEX_SOURCES
    imex.cpp
    imeximpl.cpp
    imexmessage.cpp
    criterion.cpp
	imexscdm051.cpp
	main.cpp
)

SC_ADD_EXECUTABLE(SCIMEX ${SCIMEX_TARGET})
TARGET_LINK_LIBRARIES(${SCIMEX_TARGET} ${Boost_thread_LIBRARY})
SC_LINK_LIBRARIES_INTERNAL(${SCIMEX_TARGET} client)
SC_INSTALL_INIT(${SCIMEX_TARGET} ../../templates/initd.py)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
