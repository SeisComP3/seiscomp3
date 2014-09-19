/***************************************************************************** 
 * confparams.h
 *
 * CfgParser generic parameter types
 *
 * (c) 2001 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef CONFPARAMS_H
#define CONFPARAMS_H

#include <string>
#include <list>
#include <new>
#include <cstdlib>
#include <cstring>

#include <stdint.h>
#include <regex.h>

#include "confbase.h"

namespace CfgParser_private {

//*****************************************************************************
// IntAttribute
//*****************************************************************************

class IntAttribute: public CfgAttribute
  {
  private:
    int &valref;
    const int min, max, special;
    const bool mincheck, maxcheck, spcheck;

  public:
    enum lb_check { lower_bound };
    enum ub_check { upper_bound };
    IntAttribute(const string &name, int &valref_init):
      CfgAttribute(name), valref(valref_init),
      min(0), max(0), special(0),
      mincheck(false), maxcheck(false), spcheck(false) {}
      
    IntAttribute(const string &name, int &valref_init, int min_init,
      int max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(0),
      mincheck(true), maxcheck(true), spcheck(false) {}

    IntAttribute(const string &name, int &valref_init, int special_init,
      int min_init, int max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(special_init),
      mincheck(true), maxcheck(true), spcheck(true) {}

    IntAttribute(const string &name, int &valref_init, int min_init,
      enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(0),
      mincheck(true), maxcheck(false), spcheck(false) {}
    
    IntAttribute(const string &name, int &valref_init, int special_init,
      int min_init, enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(special_init),
      mincheck(true), maxcheck(false), spcheck(true) {}
    
    IntAttribute(const string &name, int &valref_init, int max_init,
      enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(0),
      mincheck(false), maxcheck(true), spcheck(false) {}
    
    IntAttribute(const string &name, int &valref_init, int special_init,
      int max_init, enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(special_init),
      mincheck(false), maxcheck(true), spcheck(true) {}
    
    bool assign(ostream &cfglog, const string &value)
      {
        int arg;
        char *tail;

        arg = strtol(value.c_str(), &tail, 10);
        if(*tail) 
          {
            
            cfglog << "[" << item_name << "] " << value << " is not an integer" << endl;
            return false;
          }

        if(spcheck && arg == special)
          {
            valref = arg;
            return true;
          }
        
        if(mincheck && arg < min)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << min << " instead" << endl;
            valref = min;
            return true;
          }

        if(maxcheck && arg > max)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << max << " instead" << endl;
            valref = max;
            return true;
          }

        valref = arg;
        return true;
      }
  };

//*****************************************************************************
// Int64Attribute
//*****************************************************************************

class Int64Attribute: public CfgAttribute
  {
  private:
    int64_t &valref;
    const int64_t min, max, special;
    const bool mincheck, maxcheck, spcheck;

  public:
    enum lb_check { lower_bound };
    enum ub_check { upper_bound };
    Int64Attribute(const string &name, int64_t &valref_init):
      CfgAttribute(name), valref(valref_init),
      min(0), max(0), special(0),
      mincheck(false), maxcheck(false), spcheck(false) {}
      
    Int64Attribute(const string &name, int64_t &valref_init, int64_t min_init,
      int64_t max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(0),
      mincheck(true), maxcheck(true), spcheck(false) {}

    Int64Attribute(const string &name, int64_t &valref_init, int64_t special_init,
      int64_t min_init, int64_t max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(special_init),
      mincheck(true), maxcheck(true), spcheck(true) {}

    Int64Attribute(const string &name, int64_t &valref_init, int64_t min_init,
      enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(0),
      mincheck(true), maxcheck(false), spcheck(false) {}
    
    Int64Attribute(const string &name, int64_t &valref_init, int64_t special_init,
      int64_t min_init, enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(special_init),
      mincheck(true), maxcheck(false), spcheck(true) {}
    
    Int64Attribute(const string &name, int64_t &valref_init, int64_t max_init,
      enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(0),
      mincheck(false), maxcheck(true), spcheck(false) {}
    
    Int64Attribute(const string &name, int64_t &valref_init, int64_t special_init,
      int64_t max_init, enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(special_init),
      mincheck(false), maxcheck(true), spcheck(true) {}
    
    bool assign(ostream &cfglog, const string &value)
      {
        int64_t arg;
        char *tail;

        arg = strtol(value.c_str(), &tail, 10);
        if(*tail) 
          {
            
            cfglog << "[" << item_name << "] " << value << " is not an integer" << endl;
            return false;
          }

        if(spcheck && arg == special)
          {
            valref = arg;
            return true;
          }
        
        if(mincheck && arg < min)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << min << " instead" << endl;
            valref = min;
            return true;
          }

        if(maxcheck && arg > max)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << max << " instead" << endl;
            valref = max;
            return true;
          }

        valref = arg;
        return true;
      }
  };

//*****************************************************************************
// FloatAttribute
//*****************************************************************************

class FloatAttribute: public CfgAttribute
  {
  private:
    double &valref;
    const double min, max, special;
    const bool mincheck, maxcheck, spcheck;

  public:
    enum lb_check { lower_bound };
    enum ub_check { upper_bound };
    FloatAttribute(const string &name, double &valref_init):
      CfgAttribute(name), valref(valref_init),
      min(0), max(0), special(0),
      mincheck(false), maxcheck(false), spcheck(false) {}
      
    FloatAttribute(const string &name, double &valref_init, double min_init,
      double max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(0),
      mincheck(true), maxcheck(true), spcheck(false) {}

    FloatAttribute(const string &name, double &valref_init, double special_init,
      double min_init, double max_init):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(max_init), special(special_init),
      mincheck(true), maxcheck(true), spcheck(true) {}

    FloatAttribute(const string &name, double &valref_init, double min_init,
      enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(0),
      mincheck(true), maxcheck(false), spcheck(false) {}
    
    FloatAttribute(const string &name, double &valref_init, double special_init,
      double min_init, enum lb_check):
      CfgAttribute(name), valref(valref_init),
      min(min_init), max(0), special(special_init),
      mincheck(true), maxcheck(false), spcheck(true) {}
    
    FloatAttribute(const string &name, double &valref_init, double max_init,
      enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(0),
      mincheck(false), maxcheck(true), spcheck(false) {}
    
    FloatAttribute(const string &name, double &valref_init, double special_init,
      double max_init, enum ub_check):
      CfgAttribute(name), valref(valref_init),
      min(0), max(max_init), special(special_init),
      mincheck(false), maxcheck(true), spcheck(true) {}
    
    bool assign(ostream &cfglog, const string &value)
      {
        double arg;
        char *tail;

        arg = strtod(value.c_str(), &tail);
        if(*tail) 
          {
            
            cfglog << "[" << item_name << "] " << value << " is not a floating point number" << endl;
            return false;
          }

        if(spcheck && arg == special)
          {
            valref = arg;
            return true;
          }
        
        if(mincheck && arg < min)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << min << " instead" << endl;
            valref = min;
            return true;
          }

        if(maxcheck && arg > max)
          {
            cfglog << "[" << item_name << "] " << arg <<
              " is out of bounds, using " << max << " instead" << endl;
            valref = max;
            return true;
          }

        valref = arg;
        return true;
      }
  };

//*****************************************************************************
// StringAttribute
//*****************************************************************************

class StringAttribute: public CfgAttribute
  {
  private:
    string &valref;

  public:
    StringAttribute(const string &name, string &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        valref = value;
        return true;
      }
  };
  
//*****************************************************************************
// CstringAttribute
//*****************************************************************************

class CstringAttribute: public CfgAttribute
  {
  private:
    char * &valref;

  public:
    CstringAttribute(const string &name, char * &valref_init):
      CfgAttribute(name), valref(valref_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        if((valref = strdup(value.c_str())) == NULL) throw bad_alloc();
        return true;
      }
  };
  
//*****************************************************************************
// CharArrayAttribute
//*****************************************************************************

class CharArrayAttribute: public CfgAttribute
  {
  private:
    char *valref;
    const int len;

  public:
    CharArrayAttribute(const string &name, char *valref_init, int len_init):
      CfgAttribute(name), valref(valref_init), len(len_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        strncpy(valref, value.c_str(), len - 1);
        valref[len - 1] = 0;
        return true;
      }
  };
  
//*****************************************************************************
// StringListAttribute
//*****************************************************************************

class StringListAttribute: public CfgAttribute
  {
  private:
    list<string> &valref;
    const string sep;

  public:
    StringListAttribute(const string &name, list<string> &valref_init,
      const string &sep_init):
      CfgAttribute(name), valref(valref_init), sep(sep_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        const char *p = value.c_str();
        int len = 0;
        
        while(p += len, p += strspn(p, sep.c_str()), len = strcspn(p, sep.c_str()))
            valref.push_back(string(p, len));

        return true;
      }
  };

//*****************************************************************************
// BoolAttribute
//*****************************************************************************

class BoolAttribute: public CfgAttribute
  {
  private:
    bool &valref;
    const string truename;
    const string falsename;

  public:
    BoolAttribute(const string &name, bool &valref_init,
      const string &truename_init, const string &falsename_init):
      CfgAttribute(name), valref(valref_init),
      truename(truename_init), falsename(falsename_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        if(!strcasecmp(value.c_str(), truename.c_str()))
          {
            valref = true;
            return true;
          }

        if(!strcasecmp(value.c_str(), falsename.c_str()))
          {
            valref = false;
            return true;
          }

        cfglog << "[" << item_name << "] '" << value << "' is neither "
          "'" << truename << "' nor '" << falsename << "'" << endl;
        return false;
      }
  };
 
//*****************************************************************************
// RegexAttribute
//*****************************************************************************

class RegexAttribute: public CfgAttribute
  {
  private:
    regex_t &rx;
    bool &rx_ok;
    const int flags;
    enum { ERR_SIZE = 100 };

  public:
    RegexAttribute(const string &name, regex_t &rx_init, bool &rx_ok_init,
      int flags_init):
      CfgAttribute(name), rx(rx_init), rx_ok(rx_ok_init), flags(flags_init) {}

    bool assign(ostream &cfglog, const string &value)
      {
        int err;
        if((err = regcomp(&rx, value.c_str(), flags)) != 0)
          {
            char errbuf[ERR_SIZE];
            regerror(err, &rx, errbuf, ERR_SIZE);
            cfglog << "[" << item_name << "] '" << value << "': " <<
              errbuf << endl;
            return false;
          }

        rx_ok = true;
        return true;
      }
  };
 
} // namespace CfgParser_private

namespace CfgParser {

using CfgParser_private::IntAttribute;
using CfgParser_private::Int64Attribute;
using CfgParser_private::FloatAttribute;
using CfgParser_private::StringAttribute;
using CfgParser_private::CstringAttribute;
using CfgParser_private::CharArrayAttribute;
using CfgParser_private::StringListAttribute;
using CfgParser_private::BoolAttribute;

} // namespace CfgParser

#endif // CONFPARAMS_H

