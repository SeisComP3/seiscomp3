SUBDIRS(share)

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.generated)
	INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.generated)
ENDIF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.generated)


SET(DM_SOURCES
	${CORE_DATAMODEL_GENERATED_SOURCES}
	databasearchive.cpp
	messages.cpp
	notifier.cpp
	object.cpp
	publicobjectcache.cpp
	publicobject.cpp
	diff.cpp
	utils.cpp
)

SET(DM_HEADERS
	databasearchive.h
	messages.h
	metadata.h
	notifier.h
	object.h
	publicobjectcache.h
	publicobject.h
	diff.h
	utils.h
	${CORE_DATAMODEL_GENERATED_HEADERS}
)

SC_ADD_SUBDIR_SOURCES(DM exchange)
SC_SETUP_LIB_SUBDIR(DM)
