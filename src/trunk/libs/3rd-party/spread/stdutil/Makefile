all:
	cd src; $(MAKE) all

clean: 
	rm -f *~ core*
	cd src; $(MAKE) clean

distclean: 
	rm -f *~ core* config.*
	rm -rf autom4te.cache
	cd src; $(MAKE) distclean

uberclean:
	rm -f *~ core* config.*
	rm -rf autom4te.cache
	rm -f configure
	cd src; $(MAKE) uberclean
