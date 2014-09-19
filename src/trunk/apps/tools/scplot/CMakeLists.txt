SET(SCPLOT_TARGET scplot)

SET(
    SCPLOT_HEADERS
    plotter.h
    gnuplot_i.h
)

SET(
    SCPLOT_SOURCES
    gnuplot_i.c
    plotter.cpp
	scplot.cpp
)

SC_ADD_EXECUTABLE(SCPLOT ${SCPLOT_TARGET})
TARGET_LINK_LIBRARIES(${SCPLOT_TARGET} ${Boost_program_options_LIBRARY} )
SC_LINK_LIBRARIES_INTERNAL(${SCPLOT_TARGET} core)
