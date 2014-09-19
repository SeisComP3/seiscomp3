SET(CONFIG_SOURCES log.cpp config.cpp symboltable.cpp strings.cpp)
SET(CONFIG_HEADERS log.h config.h config.ipp symboltable.h exceptions.h)

SC_ADD_LIBRARY(CONFIG config)
SC_LIB_INSTALL_HEADERS(CONFIG)
