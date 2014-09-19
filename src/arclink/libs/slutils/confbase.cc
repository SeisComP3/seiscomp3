/***************************************************************************** 
 * confbase.cc
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

#include <iostream>
#include <string>
#include <map>
#include <list>
#include <stack>
#include <cstring>

#include "confbase.h"
#include "utils.h"
#include "diag.h"

namespace CfgParser_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// CfgStack
//*****************************************************************************

void CfgStack::init(rc_ptr<CfgElementMap> root)
  {
    maps.push(root);
  }

void CfgStack::push(ostream &cfglog, rc_ptr<CfgElement> el,
  const string &name)
  {
    if(el == NULL)
      {
        elements.push(NULL);
        maps.push(NULL);
      }
    else
      {
        elements.push(el);
        maps.push(el->start_children(cfglog, name));
      }
  }

void CfgStack::pop(ostream &cfglog)
  {
    internal_check(!elements.empty());
    
    rc_ptr<CfgElement> el = elements.top();
    if(el != NULL && maps.top() != NULL) el->end_children(cfglog);
    maps.pop();
    elements.pop();
  }

rc_ptr<CfgElementMap> CfgStack::get_elements()
  {
    return maps.top();
  }

} // namespace CfgParser_private

