IF(SC_IPGP_HYPO71)
	IF(DEFINED CMAKE_Fortran_COMPILER AND CMAKE_Fortran_COMPILER MATCHES "^$")
		SET(CMAKE_Fortran_COMPILER CMAKE_Fortran_COMPILER-NOTFOUND)
	ENDIF(DEFINED CMAKE_Fortran_COMPILER AND CMAKE_Fortran_COMPILER MATCHES "^$")
	ENABLE_LANGUAGE(Fortran)

	IF(CMAKE_Fortran_COMPILER_WORKS)
		MESSAGE(STATUS "... Hypo71 is built from source")

		SET(HYPO71_TARGET Hypo71PC)

		IF(Fortran_COMPILER_NAME STREQUAL "gfortran")
			SET(CMAKE_Fortran_FLAGS "-g -Bstatic")
		ENDIF(Fortran_COMPILER_NAME STREQUAL "gfortran")

		SET(
			HYPO71_SOURCES
				main.f
				hypo1m2.f
				hypo2.f
				hypo3.f
				hypo4.f
				single.f
				ytrv.f
				input1.f
				timz3.f
				geo_sp.f
		)

		INCLUDE_DIRECTORIES(.)

		SC_ADD_EXECUTABLE(HYPO71 ${HYPO71_TARGET})
	ELSE(CMAKE_Fortran_COMPILER_WORKS)
		MESSAGE(STATUS "... [WARNING] Hypo71 is not built, Fortran compiler not available")
	ENDIF(CMAKE_Fortran_COMPILER_WORKS)
ELSE(SC_IPGP_HYPO71)
	MESSAGE(STATUS "... Hypo71 is not built, disabled by config")
ENDIF(SC_IPGP_HYPO71)
