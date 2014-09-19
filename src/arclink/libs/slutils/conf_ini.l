/***************************************************************************** 
 * conf_ini.l
 *
 * Parser for .ini files
 *
 * (c) 2001 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

%{

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include <cerrno>

#ifndef yyFlexLexer
#define yyFlexLexer INIFlexLexer
#include <FlexLexer.h>
#endif

#include "confbase.h"
#include "conf_ini.h"
#include "cppstreams.h"
#include "utils.h"

namespace CfgParser_private {

using namespace std;
using namespace CPPStreams;
using namespace Utilities;

//*****************************************************************************
// CfgSectionElement
//*****************************************************************************

rc_ptr<CfgAttributeMap> CfgSectionElement::start_attributes(ostream &cfglog,
  const string &name)
  {
    if(strcasecmp(want.c_str(), name.c_str())) return NULL;
    
    if(found)
      {
        cfglog << "duplicate section '" << name << "'" << endl;
        return NULL;
      }

    found = true;
    return attributes;
  }

rc_ptr<CfgElementMap> CfgSectionElement::start_children(ostream &cfglog,
  const string &name)
  {
    if(strcasecmp(want.c_str(), name.c_str())) return NULL;
    
    return children;
  }

//*****************************************************************************
// CfgParserINI
//*****************************************************************************

class CfgParserINI: public INIFlexLexer, public StreambufBasePartner
  {
  private:
    const string file_name;
    StreambufBase *sbuf;
    ostream cfglog;
    CfgStack stack;
    string current_name;
    rc_ptr<CfgElement> current_element;
    rc_ptr<CfgAttributeMap> current_attributes;
    enum { L_File, L_Section, L_Definition};
    int scope_level;
    int lineno;
    
    int yylex();
    string msgprefix();
    void start_element(const string &keyword, const char *name, int level);
    void set(const string &parmname, const char *value);

  public:
    CfgParserINI(const string &file, rc_ptr<CfgElementMap> root,
      istream &yyin);

    ~CfgParserINI();

    void doit();
  };

} // namespace CfgParser_private

using namespace CfgParser_private;

%}

%option noyywrap nounput nolex-compat noreject nointeractive
%option c++ prefix="INI" yyclass="CfgParserINI"
%x SKIP SECTION DATA PARAM EQUALS QVALUE VALUE NAME RECOVERY

%%

%{

#define SKIPTO(state) do { nextstate = (state); BEGIN(SKIP); } while(0)
#define RECOVER(state) do { nextstate = (state); BEGIN(RECOVERY); } while(0)

int nextstate = 0;
string ptoken;

lineno = 1;

%}

[^\[\n]+        /* eat up bytes to '[' or to the end of line */
\[[ \t]*        BEGIN(SECTION);

<INITIAL,SKIP>{
\n              ++lineno;
^\*[^\n]*       /* comment */
^#[^\n]*        /* comment */
^;[^\n]*        /* comment */
<<EOF>>         yyterminate();
}

<SKIP>{
[ \t]+          /* eat up whitespace */
.               BEGIN(nextstate); yyless(0);
}

<SECTION>{
[^ \t\n\[\]]+   start_element("section", yytext, L_Section);
[ \t]*\]        if(current_element != NULL) SKIPTO(DATA); else BEGIN(INITIAL);
}

<DATA>{
\[              BEGIN(INITIAL); yyless(0);  /* end of section */
.               SKIPTO(PARAM); yyless(0);
}

<PARAM>{
[^ \t\n=]+      ptoken = yytext; BEGIN(EQUALS);
}

<EQUALS>{
[ \t]*=[ \t]*\" BEGIN(QVALUE);
[ \t]*=[ \t]*   BEGIN(VALUE);
[ \t]+          BEGIN(NAME);
}

<QVALUE>{
.*[^\\]/\"      set(ptoken, yytext);
\"              SKIPTO(DATA);
}

<VALUE>{
[^ \t\n=]+      set(ptoken, yytext); SKIPTO(DATA);
}

<NAME>{
[^ \t\n=]+      start_element(ptoken, yytext, L_Definition); SKIPTO(DATA);
}

<SECTION>{
.|\n            cfglog << "parse error" << endl; RECOVER(INITIAL); yyless(0);
}

<PARAM,EQUALS,QVALUE,VALUE,NAME>{
.|\n            cfglog << "parse error" << endl; RECOVER(DATA); yyless(0);
}

<RECOVERY>{
[^\n]+          /* eat up bytes to the end of line */
\n              SKIPTO(nextstate); yyless(0); 
}

<<EOF>>         cfglog << "unexpected eof" << endl; yyterminate();

%%

namespace CfgParser_private {

string CfgParserINI::msgprefix()
  {
    ostringstream ss;
    ss << file_name << ":" << lineno << ": ";
    return ss.str();
  }

void CfgParserINI::start_element(const string &keyword, const char *name, int level)
  {
    if(current_element != NULL && current_attributes != NULL)
        current_element->end_attributes(cfglog);

    if(level > scope_level)
      {
        stack.push(cfglog, current_element, current_name);
        ++scope_level;
      }
    else if(level < scope_level)
      {
        stack.pop(cfglog);
        --scope_level;
      }
    
    scope_level = level;
    current_name = name;
    current_element = NULL;
    current_attributes = NULL;
    
    rc_ptr<CfgElementMap> elements = stack.get_elements();

    if(elements == NULL) return;

    if((current_element = elements->find_item(keyword)) == NULL)
      {
        cfglog << "definition of '" << keyword << "' is not used" << endl;
        return;
      }

    current_attributes = current_element->start_attributes(cfglog, name);
  }

void CfgParserINI::set(const string &parmname, const char *value)
  {
    if(current_attributes == NULL) return;

    rc_ptr<CfgAttribute> at = current_attributes->find_item(parmname);
    
    if(at == NULL)
        cfglog << "parameter '" << parmname << "' is not used" << endl;
    else if(!at->set(cfglog, value))
        cfglog << "parameter '" << parmname << "' is assigned more than once" << endl;
  }

void CfgParserINI::doit()
  {
    yylex();
    if(current_element != NULL && current_attributes != NULL)
        current_element->end_attributes(cfglog);

    for(; scope_level > L_Section; --scope_level) stack.pop(cfglog);
    current_element = NULL;
    current_attributes = NULL;
  }

CfgParserINI::CfgParserINI(const string &file, rc_ptr<CfgElementMap> root,
  istream &yyin):
  INIFlexLexer(&yyin, NULL), file_name(file), sbuf(logs.bufbase(*this)),
  cfglog(sbuf), current_element(NULL), current_attributes(NULL),
  scope_level(L_Section), lineno(0)
  {
    sbuf->set_priority(LOG_WARNING);
    stack.init(root);
  }

CfgParserINI::~CfgParserINI()
  {
    delete sbuf;
  }

//*****************************************************************************
// Entry point
//*****************************************************************************

void read_config_ini_helper(const string &file_name,
  rc_ptr<CfgElementMap> root)
  {
    ifstream infile(file_name.c_str());
    if(infile.fail()) throw CfgCannotOpenFile(file_name);
    CfgParserINI parser(file_name, root, infile);
    parser.doit();
  }

} // namespace CfgParser_private

