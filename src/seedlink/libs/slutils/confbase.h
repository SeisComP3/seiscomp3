/***************************************************************************** 
 * confbase.h
 *
 * CfgParser language-independent classes
 *
 * (c) 2001 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef CONFBASE_H
#define CONFBASE_H

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <stack>
#include <cstring>
#include <cerrno>

#include "utils.h"

namespace CfgParser_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// Exceptions
//*****************************************************************************

class CfgError: public GenericException
  {
  public:
    CfgError(const string &message):
      GenericException("CfgParser", message) {}
  };

class CfgParseError: public CfgError
  {
  public:
    CfgParseError(const string &file, int line, const string &message):
      CfgError(file + ":" + to_string(line) + ": " + message) {}
  };

class CfgLibraryError: public CfgError
  {
  public:
    CfgLibraryError(const string &message):
      CfgError(message + " (" + strerror(errno) + ")") {}
  };

class CfgCannotOpenFile: public CfgLibraryError
  {
  public:
    CfgCannotOpenFile(const string &name):
      CfgLibraryError("cannot open file '" + name + "'") {}
  };
      
//*****************************************************************************
// CfgAttribute
//*****************************************************************************

class CfgAttribute
  {
  private:
    bool initialized;
    
  protected:
    CfgAttribute(const string &name): initialized(false), item_name(name) {}

  public:
    const string item_name;

    virtual ~CfgAttribute() {}
    
    virtual bool assign(ostream &cfglog, const string &value)
      {
        return true;
      }

    bool set(ostream &cfglog, const string &value)
      {
        if(initialized) return false;
        if(assign(cfglog, value)) initialized = true;
        return true;
      }
  };
    
//*****************************************************************************
// CfgMap
//*****************************************************************************

template<class T>
class CfgMap
  {
  private:
    class ci_less
      {
      public:
        bool operator()(const string &s1, const string &s2) const
          {
            return (strcasecmp(s1.c_str(), s2.c_str()) < 0);
          }
      };
    
    map<string, rc_ptr<T>, ci_less> items;

  public:
    template<class U> void add_item(const U &item)
      {
        rc_ptr<T> p = new U(item);
        items.insert(make_pair(p->item_name, p));
      }

    void add_item(rc_ptr<T> p)
      {
        items.insert(make_pair(p->item_name, p));
      }

    rc_ptr<T> find_item(const string &name)
      {
        typename map<string, rc_ptr<T> >::iterator p;

        if((p = items.find(name)) == items.end())
            return NULL;

        return p->second;
      }
  };

//*****************************************************************************
// CfgElement
//*****************************************************************************

class CfgElement
  {
  protected:
    CfgElement(const string &name): item_name(name) {}

  public:
    const string item_name;

    virtual ~CfgElement() {}
    
    virtual rc_ptr<CfgMap<CfgAttribute> > start_attributes(ostream &cfglog,
      const string &name = "")
      {
        return new CfgMap<CfgAttribute>;
      }
    
    virtual void end_attributes(ostream &cfglog) {}

    virtual rc_ptr<CfgMap<CfgElement> > start_children(ostream &cfglog,
      const string &name = "")
      {
        return new CfgMap<CfgElement>;
      }

    virtual void end_children(ostream &cfglog) {}
  };

typedef CfgMap<CfgAttribute> CfgAttributeMap;
typedef CfgMap<CfgElement> CfgElementMap;

//*****************************************************************************
// CfgStack
//*****************************************************************************

class CfgStack
  {
  private:
    stack<rc_ptr<CfgElement>, list<rc_ptr<CfgElement> > > elements;
    stack<rc_ptr<CfgElementMap>, list<rc_ptr<CfgElementMap> > > maps;

  public:
    void init(rc_ptr<CfgElementMap> root);
    void push(ostream &cfglog, rc_ptr<CfgElement> el, const string &name = "");
    void pop(ostream &cfglog);
    rc_ptr<CfgElementMap> get_elements();
  };

} // namespace CfgParser_private

namespace CfgParser {

using CfgParser_private::CfgError;
using CfgParser_private::CfgParseError;
using CfgParser_private::CfgLibraryError;
using CfgParser_private::CfgCannotOpenFile;
using CfgParser_private::CfgAttribute;
using CfgParser_private::CfgElement;
using CfgParser_private::CfgAttributeMap;
using CfgParser_private::CfgElementMap;

} // namespace CfgParser

#endif // CONFBASE_H

