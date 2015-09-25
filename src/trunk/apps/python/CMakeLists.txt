SC_ADD_PYTHON_PROG(sc2pa)
SC_ADD_PYTHON_PROG(scbulletin)
SC_ADD_PYTHON_PROG(sceplog)
SC_ADD_PYTHON_PROG(scevtls)
SC_ADD_PYTHON_PROG(scorgls)
SC_ADD_PYTHON_PROG(scproclat)
SC_INSTALL_INIT(scproclat ../templates/initd.py)
SC_ADD_PYTHON_PROG(scsohlog)
SC_INSTALL_INIT(scsohlog ../templates/initd.py)
SC_ADD_PYTHON_PROG(scvoice)
SC_INSTALL_INIT(scvoice ../templates/initd.py)
SC_ADD_PYTHON_PROG(scalert)
SC_INSTALL_INIT(scalert ../templates/initd.py)
SC_ADD_PYTHON_PROG(scart)
SC_ADD_PYTHON_PROG(scmssort)
SC_ADD_PYTHON_PROG(scdbstrip)
SC_ADD_PYTHON_PROG(scevtlog)
SC_INSTALL_INIT(scevtlog ../templates/initd.py)
SC_ADD_PYTHON_PROG(scevtstreams)
SC_ADD_PYTHON_PROG(scsendorigin)
SC_ADD_PYTHON_PROG(scsendjournal)
SC_ADD_PYTHON_PROG(bindings2cfg)
SC_ADD_PYTHON_PROG(key2inv)
SC_ADD_PYTHON_PROG(import_inv)
SC_ADD_PYTHON_PROG(sc32inv)
SC_ADD_PYTHON_PROG(scdumpcfg)
SC_ADD_PYTHON_PROG(arclink2inv)
SC_ADD_PYTHON_PROG(inv2dlsv)
SC_ADD_PYTHON_PROG(scqcquery)

SET(PYTHON_PACKAGE_PATH ${SC3_PACKAGE_PYTHON_LIB_DIR}/seiscomp3)

INSTALL(FILES
	# Python file to be installed
	scbulletin-lib.py
	DESTINATION ${PYTHON_PACKAGE_PATH}
	RENAME scbulletin.py)

INSTALL(FILES
	# Python file to be installed
	bindings2cfg-lib.py
	DESTINATION ${PYTHON_PACKAGE_PATH}
	RENAME bindings2cfg.py)

# Copy data files for scvoice and scalert
INSTALL(DIRECTORY data/scvoice DESTINATION ${SC3_PACKAGE_SHARE_DIR} USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN "*~" EXCLUDE)

INSTALL(DIRECTORY data/scalert DESTINATION ${SC3_PACKAGE_SHARE_DIR} USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN "*~" EXCLUDE)

FILE(GLOB descs "${CMAKE_CURRENT_SOURCE_DIR}/descriptions/*.xml")
INSTALL(FILES ${descs} DESTINATION ${SC3_PACKAGE_APP_DESC_DIR})
