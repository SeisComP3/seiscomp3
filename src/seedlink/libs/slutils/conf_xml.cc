/***************************************************************************** 
 * conf_xml.cc
 *
 * Parser for .xml files (based on libxml2)
 *
 * (c) 2001 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <new>
#include <list>
#include <stack>

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libxml/parser.h>

#include "cppstreams.h"
#include "utils.h"
#include "conf_xml.h"
#include "diag.h"

namespace CfgParser_private {

using namespace std;
using namespace CPPStreams;
using namespace Utilities;

const int  LOGMSGLEN  = 150;

//*****************************************************************************
// CfgInput
//*****************************************************************************

class CfgInput
  {
  private:
    int fd;

  public:
    CfgInput(): fd(-1) {}

    ~CfgInput()
      {
        if(fd != -1) ::close(fd);
      }

    void open(const string &file_name)
      {
        if((fd = ::open(file_name.c_str(), O_RDONLY)) < 0)
          throw CfgCannotOpenFile(file_name);
      }

    int close()
      {
        int retval = ::close(fd);
        fd = -1;
        return retval;
      }
    
    int read(char *buffer, int len)
      {
        return ::read(fd, buffer, len);
      }
  };

//*****************************************************************************
// CfgParserXML
//*****************************************************************************

class CfgParserXML: public StreambufBasePartner
  {
  private:
    xmlParserCtxtPtr ctxt;
    const string file_name;
    StreambufBase *sbuf;
    ostream cfglog;
    CfgStack stack;
    CfgInput input;

    string msgprefix();
    void start_element(const char *ename, const char **atts);
    void end_element();
    void character_data(const char *s, int len);
    void parser_msg(const char *fmt, va_list ap, int err);
    
    static int ioread(void *ctx, char *buffer, int len);
    static int ioclose(void *ctx);
    static void start_element_proxy(void *ctx, const xmlChar *name,
      const xmlChar **atts);
    static void end_element_proxy(void *ctx, const xmlChar *name);
    static void character_data_proxy(void *ctx, const xmlChar *s, int len);
    static void warning_proxy(void *ctx, const char *fmt, ...);
    static void error_proxy(void *ctx, const char *fmt, ...);

  public:
    CfgParserXML(const string &file, rc_ptr<CfgElementMap> root);
    
    ~CfgParserXML();

    void doit();
  };

string CfgParserXML::msgprefix()
  {
    ostringstream ss;
    ss << file_name << ":" << ctxt->input->line << ": ";
    return ss.str();
  }

int CfgParserXML::ioread(void *ctx, char *buffer, int len)
  {
    return static_cast<CfgInput *>(ctx)->read(buffer, len);
  }

int CfgParserXML::ioclose(void *ctx)
  {
    return static_cast<CfgInput *>(ctx)->close();
  }

void CfgParserXML::start_element_proxy(void *ctx, const xmlChar *name,
  const xmlChar **atts)
  {
    const char* name1 = reinterpret_cast<const char *>(name);
    const char** atts1 = reinterpret_cast<const char **>(atts);
    static_cast<CfgParserXML *>(ctx)->start_element(name1, atts1);
  }

void CfgParserXML::end_element_proxy(void *ctx, const xmlChar *name)
  {
    static_cast<CfgParserXML *>(ctx)->end_element();
  }

void CfgParserXML::character_data_proxy(void *ctx, const xmlChar *s, int len)
  {
    const char* s1 = reinterpret_cast<const char *>(s);
    static_cast<CfgParserXML *>(ctx)->character_data(s1, len);
  }

void CfgParserXML::warning_proxy(void *ctx, const char *fmt, ...)
  {
    va_list argptr;
    va_start(argptr, fmt);
    static_cast<CfgParserXML *>(ctx)->parser_msg(fmt, argptr, 0);
    va_end(argptr);
  }

void CfgParserXML::error_proxy(void *ctx, const char *fmt, ...)
  {
    va_list argptr;
    va_start(argptr, fmt);
    static_cast<CfgParserXML *>(ctx)->parser_msg(fmt, argptr, 1);
    va_end(argptr);
  }

void CfgParserXML::start_element(const char *ename, const char **atts)
  {
    rc_ptr<CfgElementMap> elements = stack.get_elements();

    if(elements == NULL)
      {
        stack.push(cfglog, NULL);
        return;
      }

    rc_ptr<CfgElement> el = elements->find_item(ename);

    if(el == NULL)
      {
        cfglog << "element '" << ename << "' is not used" << endl;
        stack.push(cfglog, NULL);
        return;
      }
    
    rc_ptr<CfgAttributeMap> attributes = el->start_attributes(cfglog);
    
    if(attributes != NULL)
      {
        for(int i = 0; atts && atts[i]; i += 2)
          {
            rc_ptr<CfgAttribute> at = attributes->find_item(atts[i]);

            if(at == NULL)
                cfglog << "attribute '" << atts[i] << "' is not used" << endl;
            else
                at->set(cfglog, atts[i + 1]);
          }

        el->end_attributes(cfglog);
      }
    
    stack.push(cfglog, el);
  }

void CfgParserXML::end_element()
  {
    stack.pop(cfglog);
  }

void CfgParserXML::character_data(const char *s, int len)
  {
    for(int i = 0; i < len; ++i)
      {
        if(!isspace(s[i]))
          {
            cfglog << "unused character data" << endl;
            return;
          }
      }
  }

void CfgParserXML::parser_msg(const char *fmt, va_list ap, int err)
  {
    char buf[LOGMSGLEN];
    vsnprintf(buf, LOGMSGLEN, fmt, ap);

    if(err) cfglog << "XML parser error: ";
    else cfglog << "XML parser warning: ";
    
    const char* p = buf;
    while(*p)
      {
        int len;
        if((len = strcspn(p, "\n")) > 0)
            cfglog << string(p, len);

        if(p[len] == '\n')
          {
            cfglog << endl;
            ++p;
          }

        p += len;
      }
  }

void CfgParserXML::doit()
  {
    input.open(file_name);
    
    xmlSAXHandlerPtr sax;
    if((sax = (xmlSAXHandlerPtr) xmlMalloc(sizeof(xmlSAXHandler))) == NULL)
        throw bad_alloc();

    memset(sax, 0, sizeof(xmlSAXHandler));
    sax->startElement = start_element_proxy;
    sax->endElement = end_element_proxy;
    sax->characters = character_data_proxy;
    sax->warning = warning_proxy;
    sax->error = error_proxy;
    sax->fatalError = error_proxy;
    
    xmlSetGenericErrorFunc(static_cast<void *>(this), error_proxy);
    
    if((ctxt = xmlCreateIOParserCtxt(sax, this, ioread, ioclose, &input,
      XML_CHAR_ENCODING_NONE)) == NULL)
      {
        xmlFree(sax);
        throw bad_alloc();
      }
    
    xmlParseDocument(ctxt);
    xmlFreeParserCtxt(ctxt);
    ctxt = NULL;
  }

CfgParserXML::CfgParserXML(const string &file, rc_ptr<CfgElementMap> root):
  ctxt(NULL), file_name(file), sbuf(logs.bufbase(*this)), cfglog(sbuf)
  {
    sbuf->set_priority(LOG_WARNING);
    stack.init(root);
  }

CfgParserXML::~CfgParserXML()
  {
    if(ctxt != NULL) xmlFreeParserCtxt(ctxt);
    xmlSetGenericErrorFunc(NULL, NULL);
    delete sbuf;
  }

//*****************************************************************************
// Entry point
//*****************************************************************************

void read_config_xml_helper(const string &file_name,
  rc_ptr<CfgElementMap> root)
  {
    CfgParserXML parser(file_name, root);
    parser.doit();
  }

} // namespace CfgParser_private

