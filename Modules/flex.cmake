# flex a .l file

# search flex
MACRO(FIND_FLEX)
	IF(NOT FLEX_EXECUTABLE)
	    IF (MACOSX)
	        # Check if Homebrew flex binary exists
	        FIND_PROGRAM(FLEX_EXECUTABLE /usr/local/opt/flex/bin/flex)
	    ENDIF (MACOSX)
	    
	    IF (NOT MACOSX)
	        FIND_PROGRAM(FLEX_EXECUTABLE flex)
		ENDIF (NOT MACOSX)
		
		IF (NOT FLEX_EXECUTABLE)
			MESSAGE(FATAL_ERROR "flex not found - aborting")
		ENDIF (NOT FLEX_EXECUTABLE)
	ENDIF(NOT FLEX_EXECUTABLE)

	IF(NOT FLEX_INCLUDE_DIR)
	    IF (MACOSX)
	        # Check if Homebrew flex directory exists
	        IF (EXISTS "/usr/local/opt/flex/")
	        SET(FLEX_INCLUDE_DIR /usr/local/opt/flex/include/)
	        INCLUDE_DIRECTORIES(/usr/local/opt/flex/include/)
	        ENDIF(EXISTS "/usr/local/opt/flex/")
	    ENDIF (MACOSX)
		
		FIND_PATH(FLEX_INCLUDE_DIR FlexLexer.h)
		IF (NOT FLEX_INCLUDE_DIR)
			MESSAGE(FATAL_ERROR "FlexLexer.h not found - aborting")
		ENDIF (NOT FLEX_INCLUDE_DIR)
	ENDIF (NOT FLEX_INCLUDE_DIR)
ENDMACRO(FIND_FLEX)

MACRO(ADD_FLEX_FILES _sources)
	FIND_FLEX()
	FOREACH (_current_FILE ${ARGN})
		GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
		GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)
		SET(_out ${CMAKE_CURRENT_BINARY_DIR}/flex_${_basename}.cc)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${_out}
			COMMAND ${FLEX_EXECUTABLE}
			ARGS -o${_out} ${_in}
			DEPENDS ${_in} ${FLEX_EXECUTABLE} ${FLEX_INCLUDE_DIR}/FlexLexer.h)
		SET(${_sources} ${${_sources}} ${_out})
	ENDFOREACH (_current_FILE)
ENDMACRO(ADD_FLEX_FILES)


