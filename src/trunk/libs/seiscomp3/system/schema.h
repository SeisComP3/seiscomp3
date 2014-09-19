/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_CONFIGURATION_SCHEMA_H__
#define __SEISCOMP_CONFIGURATION_SCHEMA_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core.h>

#include <string>
#include <iostream>


namespace Seiscomp {
namespace System {


DEFINE_SMARTPOINTER(SchemaParameter);
class SC_SYSTEM_CORE_API SchemaParameter : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaParameter);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaParameter() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::string type;
		std::string unit;
		std::string defaultValue;
		std::string description;
		OPT(bool) readOnly;
};


DEFINE_SMARTPOINTER(SchemaGroup);
DEFINE_SMARTPOINTER(SchemaStructure);


DEFINE_SMARTPOINTER(SchemaParameters);
class SC_SYSTEM_CORE_API SchemaParameters : public Core::BaseObject {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaParameters() {}


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		size_t parameterCount() const;
		SchemaParameter *parameter(size_t i);
		bool add(SchemaParameter *param);

		size_t groupCount() const;
		SchemaGroup *group(size_t i);
		bool add(SchemaGroup *group);

		size_t structureCount() const;
		SchemaStructure *structure(size_t i);
		bool add(SchemaStructure *structure);


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		std::vector<SchemaParameterPtr> _parameters;
		std::vector<SchemaGroupPtr>     _groups;
		std::vector<SchemaStructurePtr> _structs;
};


class SC_SYSTEM_CORE_API SchemaGroup : public SchemaParameters {
	DECLARE_SC_CLASS(SchemaGroup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaGroup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::string description;
};


class SC_SYSTEM_CORE_API SchemaStructure : public SchemaParameters {
	DECLARE_SC_CLASS(SchemaStructure);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaStructure() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string type;
		std::string link;
		std::string description;
};


DEFINE_SMARTPOINTER(SchemaSetupInput);
DEFINE_SMARTPOINTER(SchemaSetupInputOption);
class SC_SYSTEM_CORE_API SchemaSetupInputOption : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetupInputOption);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupInputOption() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string value;
		std::string description;
		std::vector<SchemaSetupInputPtr> inputs;
};


class SC_SYSTEM_CORE_API SchemaSetupInput : public SchemaParameter {
	DECLARE_SC_CLASS(SchemaSetupInput);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupInput() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string                            text;
		std::string                            echo;
		std::vector<SchemaSetupInputOptionPtr> options;
};


DEFINE_SMARTPOINTER(SchemaSetupGroup);
class SC_SYSTEM_CORE_API SchemaSetupGroup : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetupGroup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetupGroup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::vector<SchemaSetupInputPtr> inputs;
};



DEFINE_SMARTPOINTER(SchemaSetup);
class SC_SYSTEM_CORE_API SchemaSetup : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaSetup);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaSetup() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::vector<SchemaSetupGroupPtr> groups;
};


DEFINE_SMARTPOINTER(SchemaModule);
class SC_SYSTEM_CORE_API SchemaModule : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaModule);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaModule() : aliasedModule(NULL) {}

		bool isStandalone() const {
			return standalone && *standalone;
		}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		SchemaModule    *aliasedModule;
		std::string      name;
		std::string      category;
		std::string      import;
		std::string      description;
		OPT(bool)        standalone;
		OPT(bool)        inheritGlobalBinding;
		SchemaParameters parameters;
		SchemaSetupPtr   setup;
};


DEFINE_SMARTPOINTER(SchemaPlugin);
class SC_SYSTEM_CORE_API SchemaPlugin : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaPlugin);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaPlugin() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string              name;
		std::vector<std::string> extends;
		std::string              description;
		SchemaParameters         parameters;
		SchemaSetupPtr           setup;
};


DEFINE_SMARTPOINTER(SchemaBinding);
class SC_SYSTEM_CORE_API SchemaBinding : public Core::BaseObject {
	DECLARE_SC_CLASS(SchemaBinding);

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaBinding() {}


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string              name;
		std::string              module;
		std::string              category;
		std::string              description;
		SchemaParameters         parameters;
};


DEFINE_SMARTPOINTER(SchemaDefinitions);
class SC_SYSTEM_CORE_API SchemaDefinitions : public Core::BaseObject {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef std::vector<SchemaPlugin*>  PluginList;
		typedef std::vector<SchemaBinding*> BindingList;


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		SchemaDefinitions() {}


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		SchemaModule *createAlias(const char *existingModule, const char *newModule);
		bool removeAlias(const char *existingModule);

		size_t moduleCount() const;
		SchemaModule *module(size_t i);
		SchemaModule *module(const char *name);
		SchemaModule *module(const std::string &name);
		bool add(SchemaModule *module);

		size_t pluginCount() const;
		SchemaPlugin *plugin(size_t i);
		SchemaPlugin *plugin(const char *name);
		SchemaPlugin *plugin(const std::string &name);
		bool add(SchemaPlugin *plugin);

		size_t bindingCount() const;
		SchemaBinding *binding(size_t i);
		SchemaBinding *binding(const char *name);
		SchemaBinding *binding(const std::string &name);
		bool add(SchemaBinding *binding);

		//! Returns all plugins for a certain module
		//! The plugin pointers are managed by the Definition instance
		//! and must not be deleted.
		PluginList pluginsForModule(const char *name) const;
		PluginList pluginsForModule(const std::string &name) const;

		//! Returns all bindings for a certain module
		//! The binding pointers are managed by the Definition instance
		//! and must not be deleted.
		BindingList bindingsForModule(const char *name) const;
		BindingList bindingsForModule(const std::string &name) const;


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Read methods
	// ------------------------------------------------------------------
	public:
		bool load(const char *path);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		std::vector<SchemaModulePtr>  _modules;
		std::vector<SchemaPluginPtr>  _plugins;
		std::vector<SchemaBindingPtr> _bindings;
};


}
}


#endif
