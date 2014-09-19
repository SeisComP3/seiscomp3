ADD_EXECUTABLE(timeout timeout.c)
ADD_EXECUTABLE(trylock trylock.c)
ADD_EXECUTABLE(run_with_lock run_with_lock.c lockutil.c)
ADD_EXECUTABLE(waitlock waitlock.c)

INSTALL(
	TARGETS
		timeout
		trylock
		waitlock
		run_with_lock
	RUNTIME DESTINATION ${SC3_PACKAGE_BIN_DIR}
)
