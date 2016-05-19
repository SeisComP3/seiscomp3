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


#ifndef __SEISCOMP_CONFIGURATION_MODEL_H__
#define __SEISCOMP_CONFIGURATION_MODEL_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/config/log.h>
#include <seiscomp3/config/symboltable.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/system/schema.h>
#include <seiscomp3/core.h>
#include <string>
#include <vector>
#include <map>
#include <set>



namespace Seiscomp {
namespace System {


DEFINE_SMARTPOINTER(Module);
DEFINE_SMARTPOINTER(Parameter);

struct SC_SYSTEM_CORE_API ConfigDelegate : Config::Logger {
	struct CSConflict {
		Module         *module;
		Parameter      *parameter;
		int             stage;
		Config::Symbol *symbol;
	};

	enum Operation {
		Added,
		Removed,
		Updated
	};

	struct Change {
		Change() {}
		Change(Operation op, const std::string &var,
		       const std::string &old_content, const std::string &new_content)
		: operation(op), variable(var)
		, oldContent(old_content), newContent(new_content) {}

		Operation   operation;
		std::string variable;
		std::string oldContent;
		std::string newContent;
	};

	typedef std::vector<Change> ChangeList;

	virtual void aboutToRead(const char *filename) {}
	virtual void finishedReading(const char *filename) {}

	//! Return true to cause the model to re-read the file
	//! or false to go on.
	virtual bool handleReadError(const char *filename) { return false; }

	virtual void aboutToWrite(const char *filename) {}
	virtual void finishedWriting(const char *filename, const ChangeList &changes) {}

	//! Notification about a write error of a file
	virtual void hasWriteError(const char *filename) {}

	//! Return true to cause the model to skip writing to the file
	//! or false to go on.
	virtual bool handleWriteTimeMismatch(const char *filename, const ChangeList &changes) { return false; }
	virtual void caseSensitivityConflict(const CSConflict &) {}
};


DEFINE_SMARTPOINTER(SymbolMapItem);
struct SC_SYSTEM_CORE_API SymbolMapItem : public Core::BaseObject {
	SymbolMapItem() : known(false) {}
	SymbolMapItem(const Config::Symbol &s) : symbol(s), known(false) {}

	Config::Symbol symbol;
	bool           known;
};

class ModelVisitor;

DEFINE_SMARTPOINTER(Group);
DEFINE_SMARTPOINTER(Structure);
DEFINE_SMARTPOINTER(Parameter);
class SC_SYSTEM_CORE_API Container : public Core::BaseObject {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	protected:
		Container(const char *path_, const Container *super_ = NULL)
		: super(super_), parent(NULL), path(path_) {}

		Container(const std::string &path_, const Container *super_ = NULL)
		: super(super_), parent(NULL), path(path_) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void add(Parameter *);
		Parameter *parameter(size_t i) const { return parameters[i].get(); }
		size_t parameterCount() const { return parameters.size(); }

		void add(Group *);
		Group *group(size_t i) const { return groups[i].get(); }
		size_t groupCount() const { return groups.size(); }

		void add(Structure *);
		Structure *structure(size_t i) const { return structures[i].get(); }
		size_t structureCount() const { return structures.size(); }

		void addType(Structure *);

		bool hasStructure(const char *name) const;
		bool hasStructure(const std::string &name) const;

		//! Creates a new structure with name of a certain type wich must
		//! exist. If a valid pointer is returned the structure has been
		//! added to this->structures.
		Structure *instantiate(const Structure *s, const char *name);

		//! Removes an existing structure
		bool remove(Structure *s);

		Structure *findStructureType(const std::string &type) const;

		//! Returns a parameters in the tree where the fully expanded name
		//! matches @fullName@.
		Parameter *findParameter(const std::string &fullName) const;

		//! Returns a container at path @path@.
		Container *findContainer(const std::string &path) const;

		//! Accepts a model visitor and starts to traversing its nodes
		void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		const Container           *super;
		Core::BaseObject          *parent;
		std::string                path;
		std::vector<GroupPtr>      groups;
		std::vector<ParameterPtr>  parameters;
		std::vector<StructurePtr>  structures;
		std::vector<StructurePtr>  structureTypes;
};


class SC_SYSTEM_CORE_API Parameter : public Core::BaseObject {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Parameter(SchemaParameter *def, const char *n);
		Parameter(SchemaParameter *def, const std::string &n);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Parameter *copy(bool backImport = false);
		Parameter *clone() const;

		void dump(std::ostream &os) const;

		bool inherits(const Parameter *param) const;

		void updateFinalValue(int maxStage = Environment::CS_LAST);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Core::BaseObject  *parent;
		const Parameter   *super;
		SchemaParameter   *definition;
		SymbolMapItemPtr   symbols[Environment::CS_QUANTITY];
		Config::Symbol     symbol;
		std::string        variableName;
};


class SC_SYSTEM_CORE_API Structure : public Container {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Structure(SchemaStructure *def, const char *xpth, const char *n)
		: Container(xpth), definition(def), name(n) {}
		Structure(SchemaStructure *def, const std::string &xpth, const std::string &n)
		: Container(xpth), definition(def), name(n) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Structure *copy(bool backImport = false);
		Structure *clone() const;
		Structure *instantiate(const char *name) const;

		void dump(std::ostream &os) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		SchemaStructure  *definition;
		std::string       name;
};


class SC_SYSTEM_CORE_API Group : public Container {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Group(SchemaGroup *def, const char *path_)
		: Container(path_), definition(def) {}
		Group(SchemaGroup *def, const std::string &path_)
		: Container(path_), definition(def) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Group *copy(bool backImport = false);
		Group *clone() const;

		void dump(std::ostream &os) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Core::BaseObject  *parent;
		SchemaGroup       *definition;
};


DEFINE_SMARTPOINTER(Section);
class SC_SYSTEM_CORE_API Section : public Container {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Section(const char *n) : Container(""), parent(NULL), name(n) {}
		Section(const std::string &n) : Container(""), parent(NULL), name(n) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Section *copy(bool backImport = false);
		Section *clone() const;

		void dump(std::ostream &os) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Core::BaseObject               *parent;
		std::string                     name;
		std::string                     description;
};



DEFINE_SMARTPOINTER(Module);
DEFINE_SMARTPOINTER(Model);


DEFINE_SMARTPOINTER(Binding);
class SC_SYSTEM_CORE_API Binding : public Core::BaseObject {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Binding(const std::string &n) : parent(NULL), definition(NULL), name(n) {}
		Binding *clone() const;

		void dump(std::ostream &os) const;

		Section *section(size_t i) const { return sections[i].get(); }
		size_t sectionCount() const { return sections.size(); }

		//! Returns a container at path @path@.
		virtual Container *findContainer(const std::string &path) const;

		//! Returns a parameters in the tree where the fully expanded name
		//! matches fullName.
		virtual Parameter *findParameter(const std::string &fullName) const;

		virtual void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Core::BaseObject         *parent;
		SchemaBinding            *definition;
		std::string               name;
		std::string               description;
		std::vector<SectionPtr>   sections;
};


DEFINE_SMARTPOINTER(BindingCategory);
class SC_SYSTEM_CORE_API BindingCategory : public Core::BaseObject {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		BindingCategory(const char *n) : name(n) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		// Returns a binding (template) with given name
		Binding *binding(const std::string &name) const;
		BindingCategory *clone() const;

		void dump(std::ostream &os) const;

		//! Returns if a binding is instantiated with a given alias.
		bool hasBinding(const char *alias) const;

		//! Creates a new binding as alias of a certain binding wich must
		//! exist. If a valid pointer is returned the binding has been
		//! added to this->bindings.
		Binding *instantiate(const Binding *b, const char *alias);

		//! Returns the alias for an instantiated binding.
		//! Returns NULL if the binding is not instantiated in this
		//! category.
		const char *alias(const Binding *b) const;

		bool removeInstance(const Binding *b);
		bool removeInstance(const char *alias);

		//! Returns a container at path @path@.
		Container *findContainer(const std::string &path) const;

		//! Returns a parameters in the tree where the fully expanded name
		//! matches fullName.
		Parameter *findParameter(const std::string &fullName) const;

		void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		struct BindingInstance {
			BindingInstance(Binding *b, const std::string &a)
			: binding(b), alias(a) {}
			BindingPtr  binding;
			std::string alias;
		};

		typedef std::vector<BindingPtr> Bindings;
		typedef std::vector<BindingInstance> BindingInstances;

		Binding         *parent;
		std::string      name;
		BindingInstances bindings;
		Bindings         bindingTypes;
};


DEFINE_SMARTPOINTER(ModuleBinding);
class SC_SYSTEM_CORE_API ModuleBinding : public Binding {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		ModuleBinding(const std::string &n) : Binding(n) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ModuleBinding *clone() const;

		void add(BindingCategory *);
		BindingCategory *category(const std::string &name) const;

		bool writeConfig(const std::string &filename,
		                 ConfigDelegate *delegate = NULL) const;
		void dump(std::ostream &os) const;

		//! Returns a container at path @path@.
		Container *findContainer(const std::string &path) const;

		//! Returns a parameters in the tree where the fully expanded name
		//! matches fullName.
		Parameter *findParameter(const std::string &fullName) const;

		void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		typedef std::vector<BindingCategoryPtr> Categories;
		std::string      configFile;
		Categories       categories;
};


struct SC_SYSTEM_CORE_API StationID {
	StationID() {}
	StationID(const std::string &net, const std::string &sta)
		: networkCode(net), stationCode(sta) {}

	bool operator==(const StationID &other) const {
		return networkCode == other.networkCode &&
		       stationCode == other.stationCode;
	}

	bool operator<(const StationID &other) const {
		if ( networkCode == other.networkCode )
			return stationCode < other.stationCode;
		return networkCode < other.networkCode;
	}

	std::string networkCode;
	std::string stationCode;
};


class SC_SYSTEM_CORE_API Module : public Core::BaseObject {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Module(SchemaModule *def)
		: definition(def) {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Returns if the module has any parameter to configure.
		bool hasConfiguration() const;

		void add(Section *);
		Section *section(size_t i) const { return sections[i].get(); }
		size_t sectionCount() const { return sections.size(); }

		//! Returns a parameters in the tree where the fully expanded name
		//! matches fullName.
		Parameter *findParameter(const std::string &fullName) const;

		//! Returns a container at path @path@.
		Container *findContainer(const std::string &path) const;

		bool supportsBindings() const { return bindingTemplate.get() != NULL; }

		int loadProfiles(const std::string &dir, ConfigDelegate *delegate = NULL);

		//! Adds a profile. Returns false if a profile with the same
		//! name exists already or the binding belongs to another
		//! module.
		bool addProfile(ModuleBinding *);

		//! Removes a profile. Returns false if the profile does not exist.
		//! All station bindings that refer to that profile are removed.
		bool removeProfile(const std::string &profile);

		//! Removes a profile. Returns false if the profile does not exist.
		//! All station bindings that refer to that profile are removed.
		bool removeProfile(ModuleBinding *);

		//! Binds a station to a profile. If the profile does not
		//! exist or if the binding belongs to another module,
		//! false is returned.
		ModuleBinding *bind(const StationID &, const std::string &profile);

		//! Binds the station to station binding. If the binding belongs
		//! to another module, false is returned.
		bool bind(const StationID &, ModuleBinding *binding);

		//! Removes a binding for a station
		bool removeStation(const StationID &);

		//! Creates an empty binding based on the templateBinding.
		//! Returns NULL if no template binding is available.
		ModuleBinding *createBinding() const;

		//! Creates an empty profile with name.
		ModuleBinding *createProfile(const std::string &name);

		ModuleBinding *getProfile(const std::string &profile) const;

		//! Returns a station binding if available
		ModuleBinding *getBinding(const StationID &) const;

		ModuleBinding *readBinding(const StationID &,
		                           const std::string &profile = "",
		                           bool allowConfigFileErrors = false,
		                           ConfigDelegate *delegate = NULL);

		//! Accepts a model visitor and starts to traversing its nodes
		void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	private:
		void syncProfileRemoval(Binding *);
		bool loadBinding(ModuleBinding &, const std::string &filename,
		                 bool allowConfigFileErrors = false,
		                 ConfigDelegate *delegate = NULL) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		typedef std::map<StationID, ModuleBindingPtr> BindingMap;
		typedef std::set<std::string>                 BindingCategories;
		typedef std::vector<ModuleBindingPtr>         Profiles;

		Model                    *model;
		std::string               keyDirectory;
		std::string               configFile;
		SchemaModule             *definition;
		ModuleBindingPtr          bindingTemplate;
		std::vector<SectionPtr>   sections;
		std::vector<ParameterPtr> unknowns;
		BindingMap                bindings;
		Profiles                  profiles;
};


DEFINE_SMARTPOINTER(Station);
class SC_SYSTEM_CORE_API Station : public Core::BaseObject {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Station() {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		bool readConfig(const char *filename);
		bool writeConfig(const char *filename,
		                 ConfigDelegate *delegate = NULL) const;

		void setConfig(const std::string &module, const std::string &profile);

		//! Returns whether a tag with name has a certain value
		bool compareTag(const std::string &name, const std::string &value) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		struct ModuleConfig {
			ModuleConfig() {}
			ModuleConfig(const std::string &name, const std::string &prof = "")
			: moduleName(name), profile(prof) {}

			std::string  moduleName;
			std::string  profile;
		};

		typedef std::vector<ModuleConfig> ModuleConfigs;
		typedef std::map<std::string, std::string> Tags;
		ModuleConfigs config;
		Tags          tags;
};


class SC_SYSTEM_CORE_API Model : public Core::BaseObject {
	DECLARE_RTTI;

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		Model();


	// ------------------------------------------------------------------
	//  Generators
	// ------------------------------------------------------------------
	public:
		//! Creates the tree from schema definitions
		bool create(SchemaDefinitions *def);
		bool readConfig(int updateMaxStage = Environment::CS_LAST,
		                ConfigDelegate *delegate = NULL);
		bool writeConfig(int stage = Environment::CS_CONFIG_APP,
		                 ConfigDelegate *delegate = NULL);
		bool writeConfig(Module *, const std::string &filename, int stage,
		                 ConfigDelegate *delegate = NULL);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Module *module(const std::string &name) const;

		/* By default the SeisComP environment paths are used. But for testing
		 * or alternative locations this methods can be reimplemented to
		 * use different directories.
		 */
		virtual std::string systemConfigFilename(bool read, const std::string &name) const;
		virtual std::string configFileLocation(bool read, const std::string &name, int stage) const;
		virtual std::string stationConfigDir(bool read, const std::string &name = "") const;

		//! Updates parameter values of a container
		void update(const Module *mod, Container *container) const;

		//! Updates parameter values of a binding
		void updateBinding(const ModuleBinding *mod, Binding *binding) const;

		//! Adds a global empty station configuration.
		bool addStation(const StationID &);

		//! Removes a global station configuration. The station is also
		//! removed from all available modules.
		bool removeStation(const StationID &);

		//! Removes all global station configurations that are part of
		//! the given network.
		bool removeNetwork(const std::string &);

		//! Removes a station binding from a module.
		bool removeStationModule(const StationID &, Module *);

		//! Accepts a model visitor and starts to traversing its nodes
		void accept(ModelVisitor *) const;


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		Module *create(SchemaDefinitions *schema, SchemaModule *def);


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		struct SymbolFileMap : std::map<std::string, SymbolMapItemPtr> {
			time_t lastModified;
		};

		typedef std::map<std::string, SymbolFileMap>    SymbolMap;
		typedef std::map<std::string, Module*>          ModMap;
		typedef std::set<std::string>                   Categories;
		typedef std::map<StationID, StationPtr>         Stations;

		SchemaDefinitions      *schema;
		std::vector<ModulePtr>  modules;
		Categories              categories;
		Stations                stations;
		mutable SymbolMap       symbols;
		ModMap                  modMap;
		std::string             keyDirOverride;
};



class SC_SYSTEM_CORE_API ModelVisitor {
	protected:
		ModelVisitor() {}
		virtual ~ModelVisitor() {}

	protected:
		virtual bool visit(Module*) = 0;
		virtual bool visit(Section*) = 0;
		virtual bool visit(Group*) = 0;
		virtual bool visit(Structure*) = 0;
		virtual void visit(Parameter*, bool unknown = false) = 0;

	friend class Container;
	friend class Module;
	friend class Model;
	friend class Binding;
};



}
}


#endif
