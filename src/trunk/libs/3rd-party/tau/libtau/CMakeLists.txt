SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
ADD_DEFINITIONS("-fPIC")
ADD_LIBRARY(tau_internal STATIC emdlv.c libtau.c tauint.c tauspl.c)
LINK_LIBRARIES(tau_internal -lm)
