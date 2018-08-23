SET(TESTS
	amplitudes.cpp
)

ADD_DEFINITIONS("-DTEST_BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"")

SET(test_mn_amplitudes test_core_${testName})
ADD_EXECUTABLE(test_mn_amplitudes amplitudes.cpp ../amplitude.cpp ../magnitude.cpp ../regions.cpp)
SC_LINK_LIBRARIES_INTERNAL(test_mn_amplitudes client)

ADD_TEST(
	NAME test_mn_amplitudes
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND test_mn_amplitudes --inventory-db data/inventory.xml --debug
)

# Set the system and user configuration directories with respect to the
# data directory to avoid reading installed and maybe tuned configurations.
SET_TESTS_PROPERTIES(test_mn_amplitudes PROPERTIES
	ENVIRONMENT "SEISCOMP_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/data;SEISCOMP_LOCAL_CONFIG=${CMAKE_CURRENT_SOURCE_DIR}/.seiscomp3"
)
