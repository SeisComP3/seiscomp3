INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
INCLUDE_DIRECTORIES(../../libs/3rd-party/libslink)
INCLUDE_DIRECTORIES(../../libs/3rd-party/qlib2)

ADD_EXECUTABLE(load_timetable load_timetable.c)
TARGET_LINK_LIBRARIES(load_timetable ${LIBXML2_LIBRARIES} slink qlib2 m)

INSTALL(TARGETS	load_timetable
	RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR})
