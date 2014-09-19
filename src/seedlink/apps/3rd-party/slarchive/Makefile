DIRS = src

all: $(DIRS)

.PHONY: $(DIRS) $(DIRS:%=depend_%) $(DIRS:%=clean_%) all depend clean

depend: $(DIRS:%=depend_%)

clean: $(DIRS:%=clean_%)

$(DIRS):
	$(MAKE) -C $@

$(DIRS:%=depend_%):
	-$(MAKE) -C $(@:depend_%=%) depend

$(DIRS:%=clean_%):
	-$(MAKE) -C $(@:clean_%=%) clean

