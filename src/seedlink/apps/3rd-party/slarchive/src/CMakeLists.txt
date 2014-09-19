SET(SLARCHIVE_SOURCES
	archive.c
	dsarchive.c
	slarchive.c
)

INCLUDE_DIRECTORIES(../../../../libs/3rd-party/libslink)

ADD_EXECUTABLE(slarchive ${SLARCHIVE_SOURCES})

TARGET_LINK_LIBRARIES(slarchive slink)

INSTALL(TARGETS slarchive
	RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR})
