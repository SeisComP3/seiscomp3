DOCS = arclink.pdf

all: $(DOCS)

.PHONY: all clean .depend

$(DOCS): %.pdf: %.tex dmacros.sty
	@echo Generating TOC and references...
	-@latex $< < /dev/null > /dev/null 2>&1
	-@latex $< < /dev/null > /dev/null 2>&1
	latex $< < /dev/null
	dvips -t a4 $(basename $@).dvi -o $(basename $@).ps
	ps2pdf -sPAPERSIZE=a4 $(basename $@).ps

clean:
	$(RM) $(foreach doc, $(DOCS), $(addprefix $(basename $(doc)), .aux .toc .log .dvi .ps .pdf))

