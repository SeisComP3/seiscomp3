DIRS = archive msgq reftek rtp si util

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
        PLATFORM = -DLINUX -D_REENTRANT
endif

ifeq ($(UNAME_S),Darwin)
        PLATFORM = -DMACOSX -D_REENTRANT
endif
    
all: $(DIRS)

.PHONY: $(DIRS) $(DIRS:%=clean_%) all clean

clean: $(DIRS:%=clean_%)

$(DIRS):
	$(MAKE) -C $@ PLATFORM="$(PLATFORM)"

$(DIRS:%=clean_%):
	-$(MAKE) -C $(@:clean_%=%) clean

