DIRS = serial_plugin chain_plugin sockplugin naqsplugin nrts_plugin \
       q330plugin reftek_plugin ewexport_plugin scream_plugin wave24_plugin \
       mseedscan_plugin mseedfifo_plugin fs_plugin win_plugin
       
LIBS = reftek_libs/archive reftek_libs/msgq reftek_libs/reftek \
       reftek_libs/rtp reftek_libs/si reftek_libs/util lib330

SPECIAL_LIBS = ISI_Toolkit_1_3_0 libgcf2-2.3.2

all: $(DIRS)

.PHONY: $(DIRS) $(DIRS:%=depend_%) $(DIRS:%=clean_%) $(LIBS:%=clean_%) \
    $(SPECIAL_LIBS:%=clean_%) all depend clean

depend: $(DIRS:%=depend_%)

clean: $(DIRS:%=clean_%) $(LIBS:%=clean_%) $(SPECIAL_LIBS:%=clean_%)

$(DIRS):
	$(MAKE) -C $@

$(DIRS:%=depend_%):
	-$(MAKE) -C $(@:depend_%=%) depend

$(DIRS:%=clean_%) $(LIBS:%=clean_%):
	-$(MAKE) -C $(@:clean_%=%) clean

clean_ISI_Toolkit_1_3_0:
	$(RM) ISI_Toolkit_1_3_0/lib/libisi.a
	$(RM) ISI_Toolkit_1_3_0/lib/makestamp

clean_libgcf2-2.3.2:
	-$(MAKE) -C libgcf2-2.3.2 distclean

