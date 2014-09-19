/***************************************************************************** 
 * plugin_module.h
 *
 * Helper classes for dynamic module registration
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef PLUGIN_MODULE_H
#define PLUGIN_MODULE_H

#include <iostream>
#include <string>
#include <list>

#include <cstring>

#include "utils.h"
#include "plugin_exceptions.h"

namespace PluginModule_private {

using namespace std;
using namespace Utilities;

//*****************************************************************************
// ModuleSpec
//*****************************************************************************

template<class T>
class ModuleSpec
  {
  public:
    virtual rc_ptr<T> instance(const string &name) const =0;
    virtual ~ModuleSpec() {}
  };

template<class T, class U>
class ModuleSpecImpl: public ModuleSpec<T>
  {
  public:
    rc_ptr<T> instance(const string &name) const
      {
        return new U(name);
      }
  };

//*****************************************************************************
// RegisteredModule
//*****************************************************************************

template<class T>
class RegisteredModule
  {
  private:
    static RegisteredModule<T> *registered;
    RegisteredModule<T> *next;
    const PluginModule_private::ModuleSpec<T> *const spec;
    
  public:
    const string name;

    RegisteredModule(const string &name_init,
      const PluginModule_private::ModuleSpec<T> *spec_init):
      spec(spec_init), name(name_init)
      {
        next = registered;
        registered = this;
      }
    
    static list<string> list_modules();
    static Utilities::rc_ptr<T> instance(const string &name);
  };

template<class T>
list<string> RegisteredModule<T>::list_modules()
  {
    list<string> module_names;
    RegisteredModule<T> *p;

    for(p = RegisteredModule<T>::registered; p; p = p->next)
        module_names.push_back(p->name);

    return module_names;
  }

template<class T>
Utilities::rc_ptr<T> RegisteredModule<T>::instance(const string &name)
  {
    RegisteredModule<T> *p;
    for(p = RegisteredModule<T>::registered; p; p = p->next)
        if(!strcasecmp(p->name.c_str(), name.c_str()))
            return p->spec->instance(name);

    return NULL;
  }

//*****************************************************************************
// list_modules()
//*****************************************************************************

template<class T>
void list_modules(const string &desc)
  {
    int len = desc.length() + 2;

    cout << desc << ": ";

    list<string> module_names = RegisteredModule<T>::list_modules();
    list<string>::const_iterator p = module_names.begin();
    while(p != module_names.end())
      {
        len += p->length() + 2;

        if(len > 81)
          {
            cout << endl << "  ";
            len = p->length() + 4;
          }

        cout << *p;

        if(++p != module_names.end())
            cout << ", ";
      }

    cout << endl;
  }

} // namespace PluginModule_private

namespace PluginModule {

using PluginModule_private::ModuleSpecImpl;
using PluginModule_private::RegisteredModule;
using PluginModule_private::list_modules;

} // namespace PluginModule

#endif // PLUGIN_MODULE_H

