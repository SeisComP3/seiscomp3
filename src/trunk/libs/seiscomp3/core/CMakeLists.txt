CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/platform.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/platform/platform.h
               @ONLY)

SET(CORE_SOURCES
	rtti.cpp
	metaobject.cpp
	datetime.cpp
	timewindow.cpp
	optional.cpp
	strings.cpp
	arrayfactory.cpp
	typedarray.cpp
	record.cpp
	array.cpp
	genericrecord.cpp
	greensfunction.cpp
	status.cpp
	recordsequence.cpp
	interruptible.cpp
	message.cpp
	genericmessage.cpp
	datamessage.cpp
	exceptions.cpp
	plugin.cpp
	system.cpp
	baseobject.cpp
	version.cpp
)

SET(CORE_HEADERS
	archive.h
	archive.inl
	archive.ipp
	baseobject.h
	baseobject.inl
	defs.h
	factory.h
	factory.inl
	factory.ipp
	interfacefactory.h
	interfacefactory.ipp
	io.h
	rtti.h
	metaobject.h
	metaproperty.h
	serialization.h
	datetime.h
	timewindow.h
	optional.h
	optional.inl
	enumeration.h
	enumeration.inl
	strings.h
	strings.ipp
	arrayfactory.h
	array.h
	typedarray.h
	record.h
	genericrecord.h
	greensfunction.h
	exceptions.h
	status.h
	recordsequence.h
	interruptible.h
	message.h
	genericmessage.h
	genericmessage.ipp
	datamessage.h
	plugin.h
	system.h
	version.h
)


IF(WIN32)
	SET(CORE_SOURCES ${CORE_SOURCES} strptime.c)
	SET(CORE_HEADERS ${CORE_HEADERS} strptime.h)
ENDIF(WIN32)

SC_ADD_SUBDIR_SOURCES(CORE platform)

SC_SETUP_LIB_SUBDIR(CORE)
