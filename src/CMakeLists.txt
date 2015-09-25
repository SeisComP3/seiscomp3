# System directory has highest priority, so pass its name to make it the first
# item in the result list if available, then trunk and then gui-qt4
SC_ADD_SUBDIRS(system trunk gui-qt4)
