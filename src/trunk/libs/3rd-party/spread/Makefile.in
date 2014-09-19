SUBDIRS=stdutil libspread-util daemon docs libspread examples 
INSTALLSUBDIRS=daemon docs libspread examples
BINSUBDIRS=daemon libspread examples

.PHONY: all clean distclean mrproper veryclean binrelease install uninstallall uninstall
all: $(SUBDIRS) 
	for dir in $(SUBDIRS); do \
		( $(MAKE) -C $$dir); \
	done

clean:
	for dir in $(SUBDIRS); do \
		( $(MAKE) -C $$dir clean); \
	done

	rm -f core
	rm -rf autom4te.cache

distclean: 
	for dir in $(SUBDIRS); do \
		( $(MAKE) -C $$dir distclean); \
	done
	rm -f config.cache config.log core
	rm -rf autom4te.cache
	rm -f Makefile config.h config.status *~

mrproper: distclean

veryclean: distclean
	rm -f configure

binrelease: $(BINSUBDIRS) 
	for dir in $(BINSUBDIRS); do \
		( $(MAKE) -C $$dir binrelease); \
	done

install: $(INSTALLSUBDIRS) 
	for dir in $(INSTALLSUBDIRS); do \
		( $(MAKE) -C $$dir install); \
	done

uninstallall:	uninstall
	for dir in $(INSTALLSUBDIRS); do \
		( $(MAKE) -C $$dir uninstallall); \
	done

uninstall: 
	for dir in $(INSTALLSUBDIRS); do \
		( $(MAKE) -C $$dir uninstall); \
	done
