# the idea comes from here: https://github.com/commontk/PythonQt/issues/24#issuecomment-127639573
# this overwrite is needed to avoid collision if python version an mac if homebrew python is used
find_program(PYTHON_CONFIG_EXECUTABLE python-config)
if (NOT PYTHON_CONFIG_EXECUTABLE)
	message(SEND_ERROR "python-config executable not found, but python is required.")
endif()
# using "python-config --prefix" so that cmake always uses
# the python that is
# in the user's path, this is a fix for homebrew on Mac:
# https://github.com/Homebrew/homebrew/issues/25118
execute_process(COMMAND ${PYTHON_CONFIG_EXECUTABLE} --prefix OUTPUT_VARIABLE python_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
set(PYTHON_INCLUDE_DIR ${python_prefix}/include/python2.7 CACHE PATH "Path to where Python.h is found" FORCE)
set(PYTHON_LIBRARY ${python_prefix}/lib/libpython2.7${CMAKE_SHARED_LIBRARY_SUFFIX} CACHE PATH "Path to where libpython2.7.dylib is found" FORCE)
