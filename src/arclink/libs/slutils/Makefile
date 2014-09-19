XML_INCL := $(shell xml2-config --cflags)

OBJS = utils.o lex.ini.o conf_xml.o confbase.o

CXX = g++
CXXFLAGS = -Wall -O2
LEX = flex

all: libslutils.a

.PHONY: all depend clean

libslutils.a: $(OBJS)
	ar -cr $@ $+

$(OBJS): %.o: %.cc
	$(CXX) $(CXXFLAGS) $(XML_INCL) -c $< -o $@

lex.ini.cc: conf_ini.l
	$(LEX) -o$@ $<

depend:
	$(CXX) $(XML_INCL) -MM *.cc > .depend

clean:
	$(RM) libslutils.a lex.ini.cc $(OBJS) .depend

-include .depend

