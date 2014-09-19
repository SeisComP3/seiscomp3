OPTION(SC_TRUNK_MASTER "Compile the messaging master" ON)

IF (SC_TRUNK_MASTER)
	SET(MASTER_DIR scmaster)
ENDIF (SC_TRUNK_MASTER)

SUBDIRS(${MASTER_DIR} scadmin sctestclient)
