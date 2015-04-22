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

#define SEISCOMP_COMPONENT System

#include <seiscomp3/logging/log.h>
#include <seiscomp3/system/model.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/utils/files.h>

#include <boost/version.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <fstream>
#include <set>


using namespace std;
using namespace Seiscomp::Config;

namespace fs = boost::filesystem;


namespace Seiscomp {
namespace System {


IMPLEMENT_RTTI(Parameter, "Parameter", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Parameter)

IMPLEMENT_RTTI(Structure, "Structure", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Structure)

IMPLEMENT_RTTI(Group, "Group", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Group)

IMPLEMENT_RTTI(Section, "Section", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Section)

//IMPLEMENT_RTTI(BindingSection, "BindingSection", Core::BaseObject)
//IMPLEMENT_RTTI_METHODS(BindingSection)

IMPLEMENT_RTTI(Binding, "Binding", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Binding)

IMPLEMENT_RTTI(ModuleBinding, "ModuleBinding", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(ModuleBinding)

IMPLEMENT_RTTI(Module, "Module", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Module)

IMPLEMENT_RTTI(Station, "Station", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Station)

IMPLEMENT_RTTI(Model, "Model", Core::BaseObject)
IMPLEMENT_RTTI_METHODS(Model)


namespace {


class CaseSensitivityCheck : public ModelVisitor {
	public:
		CaseSensitivityCheck(ConfigDelegate *delegate,
		                     Module *target, int stage, Config::Symbol *symbol)
		: _delegate(delegate), _target(target), _stage(stage), _symbol(symbol) {}

	protected:
		virtual bool visit(Module *m) { return m == _target; }
		virtual bool visit(Section*) { return true; }
		virtual bool visit(Group*) { return true; }
		virtual bool visit(Structure*) { return true; }
		virtual void visit(Parameter *p, bool unknown) {
			if ( !unknown && Core::compareNoCase(p->variableName, _symbol->name) == 0 ) {
				ConfigDelegate::CSConflict csc;
				csc.module = _target;
				csc.parameter = p;
				csc.stage = _stage;
				csc.symbol = _symbol;
				if ( _delegate ) _delegate->caseSensitivityConflict(csc);
			}
		}

	private:
		ConfigDelegate *_delegate;
		Module         *_target;
		int             _stage;
		Config::Symbol *_symbol;
};



class DuplicateNameCheck : public ModelVisitor {
	public:
		DuplicateNameCheck()
		: _currentModule(NULL) {}

		bool visit(Module *mod) {
			_currentModule = mod;
			_currentSection = NULL;
			_names.clear();
			return true;
		}

		bool visit(Section *sec) {
			_currentSection = sec;
			return true;
		}

		bool visit(Group*) {
			return true;
		}

		bool visit(Structure*) {
			return true;
		}

		void visit(Parameter *param, bool unknown) {
			// Do not touch unknown parameters
			if ( unknown ) return;

			if ( _names.find(param->variableName) != _names.end() ) {
				if ( _currentSection )
					SEISCOMP_WARNING("C duplicate parameter definition in %s/%s: %s",
					                 _currentModule->definition->name.c_str(),
					                 _currentSection->name.c_str(),
					                 param->variableName.c_str());
				else
					SEISCOMP_WARNING("C duplicate parameter definition in %s: %s",
					                 _currentModule->definition->name.c_str(),
					                 param->variableName.c_str());

				return;
			}

			_names.insert(param->variableName);
		}

	private:
		const Module  *_currentModule;
		const Section *_currentSection;
		set<string>    _names;
};




bool createPath(const std::string &pathname) {
	if ( mkdir(pathname.c_str(), 0755) < 0 ) {
		if ( errno == ENOENT ) {
			size_t slash = pathname.rfind('/');
			if ( slash != std::string::npos ) {
				std::string prefix = pathname.substr(0, slash);
				if ( !createPath(prefix) ) return false;
				if ( slash == pathname.size() -1 ) return true;
				return mkdir(pathname.c_str(), 0755) == 0;
			}
			else
				return false;
		}
		else
			return false;
	}

	return true;
}


string blockComment(const string &input, size_t lineWidth) {
	// Make space for leading "# "
	lineWidth -= 2;

	string txt = string("# ") + input;
	size_t s = 2;
	size_t to = s + lineWidth;

	while ( to < txt.length() ) {
		// find linebreaks and comment each new line
		size_t p = txt.find_first_of('\n', s);
		if ( p != string::npos && (p - s) < lineWidth) {
			s = p + 1;
			txt.insert(p+1, "# ");
			s = p + 3;
		}
		else {
			// insert line break if possible at last space else inside word
			// without hyphenation
			p = txt.find_last_of(' ', to-1);
			if ( p == string::npos || p < s || (p -s) > lineWidth) {
				txt.insert(to, "\n# ");
				s = to + 3;
			}
			else {
				txt[p] = '\n';
				txt.insert(p+1, "# ");
				s = p+3;
			}
		}

		to = s + lineWidth;
	}

	// comment line breaks in last line
	while ( s < txt.length() ) {
		size_t p = txt.find_first_of('\n', s);
		if ( p == string::npos ) break;
		txt.insert(p+1, "# ");
		s = p + 3;
	}

	return txt;
}



bool loadGroup(Container *c, SchemaGroup *group, const std::string &prefix);


Structure *loadStructure(SchemaStructure *struc, const std::string &prefix,
                         const std::string &name) {
	std::string namePrefix = prefix + name;

	Structure *strucParam = new Structure(struc, namePrefix, name);

	for ( size_t i = 0; i < struc->parameterCount(); ++i ) {
		SchemaParameter *pdef = struc->parameter(i);
		std::string paramName = namePrefix;
		if ( !pdef->name.empty() )
			paramName += "." + pdef->name;
		ParameterPtr param = new Parameter(pdef, paramName);
		strucParam->add(param.get());
	}

	for ( size_t i = 0; i < struc->groupCount(); ++i ) {
		SchemaGroup *gdef = struc->group(i);
		loadGroup(strucParam, gdef, namePrefix + ".");
	}

	for ( size_t i = 0; i < struc->structureCount(); ++i ) {
		SchemaStructure *gdef = struc->structure(i);
		strucParam->addType(loadStructure(gdef, namePrefix + ".", ""));
	}

	return strucParam;
}


Binding *loadCategoryBinding(SchemaBinding *def, const std::string &prefix,
                             const std::string &name) {
	std::string namePrefix = prefix + name;

	Binding *binding = new Binding(def->name);
	binding->definition = def;

	SectionPtr sec = new Section(def->name);
	binding->sections.push_back(sec);

	if ( def->parameters ) {
		for ( size_t i = 0; i < def->parameters->parameterCount(); ++i ) {
			SchemaParameter *pdef = def->parameters->parameter(i);
			std::string paramName = namePrefix + "." + pdef->name;
			ParameterPtr param = new Parameter(pdef, paramName);
			sec->add(param.get());
		}

		for ( size_t i = 0; i < def->parameters->groupCount(); ++i ) {
			SchemaGroup *gdef = def->parameters->group(i);
			loadGroup(sec.get(), gdef, namePrefix + ".");
		}

		for ( size_t i = 0; i < def->parameters->structureCount(); ++i ) {
			SchemaStructure *gdef = def->parameters->structure(i);
			sec->addType(loadStructure(gdef, namePrefix + ".", ""));
		}
	}

	return binding;
}


//void createSections();
bool loadGroup(Container *c, SchemaGroup *group, const std::string &prefix) {
	std::string namePrefix = prefix + group->name;

	GroupPtr groupParam;
	Container *container = NULL;

	// try to find the existing group
	for ( size_t i = 0; i < c->groups.size(); ++i ) {
		if ( c->groups[i]->definition->name == group->name ) {
			container = c->groups[i].get();
			break;
		}
	}

	if ( !container ) {
		groupParam = new Group(group, namePrefix);
		container = groupParam.get();
	}

	for ( size_t i = 0; i < group->parameterCount(); ++i ) {
		SchemaParameter *pdef = group->parameter(i);
		std::string paramName = namePrefix + "." + pdef->name;
		ParameterPtr param = new Parameter(pdef, paramName);
		container->add(param.get());
	}

	for ( size_t i = 0; i < group->groupCount(); ++i ) {
		SchemaGroup *gdef = group->group(i);
		loadGroup(container, gdef, namePrefix + ".");
	}

	for ( size_t i = 0; i < group->structureCount(); ++i ) {
		SchemaStructure *gdef = group->structure(i);
		container->addType(loadStructure(gdef, namePrefix + ".", ""));
	}

	if ( groupParam ) c->add(groupParam.get());
	return true;
}


void updateParameter(Parameter *param, Model::SymbolFileMap &symbols, int stage) {
	SymbolMapItemPtr item = symbols[param->variableName];
	if ( item == NULL ) {
		item = new SymbolMapItem();
		//item->symbol.name = param->variableName;
		symbols[param->variableName] = item;
	}

	// Tell the item that it is known and stored
	item->known = true;
	param->symbols[stage] = item;
}


void updateContainer(Container *c, Model::SymbolFileMap &symbols, int stage) {
	for ( size_t i = 0; i < c->parameters.size(); ++i )
		updateParameter(c->parameters[i].get(), symbols, stage);

	for ( size_t i = 0; i < c->groups.size(); ++i )
		updateContainer(c->groups[i].get(), symbols, stage);

	for ( size_t i = 0; i < c->structureTypes.size(); ++i ) {
		const string &xpath = c->structureTypes[i]->path;
		set<string> structs;
		Model::SymbolFileMap::iterator it;
		for ( it = symbols.begin(); it != symbols.end(); ++it ) {
			// Parameter not from file?
			if ( it->second->symbol.uri.empty() ) continue;

			// Check if the symbol name starts with xpath
			if ( it->first.compare(0, xpath.size(), xpath) ) continue;

			size_t pos = it->first.find('.', xpath.size());

			string name;
			if ( pos != string::npos )
				name = it->first.substr(xpath.size(), pos-xpath.size());
			else
				name = it->first.substr(xpath.size());
				// No dot means we are looking at a variable and not a struct
				//continue;

			if ( !name.empty() ) structs.insert(name);
		}

		// Instantiate available structures
		set<string>::iterator sit;
		for ( sit = structs.begin(); sit != structs.end(); ++sit )
			// This may fail if another stage has added this struct
			// already.
			c->instantiate(c->structureTypes[i].get(), sit->c_str());
	}

	for ( size_t i = 0; i < c->structures.size(); ++i )
		updateContainer(c->structures[i].get(), symbols, stage);
}


void updateParameter(Parameter *param, int updateMaxStage) {
	param->updateFinalValue(updateMaxStage);
}


void updateContainer(Container *c, int updateMaxStage) {
	for ( size_t i = 0; i < c->parameters.size(); ++i )
		updateParameter(c->parameters[i].get(), updateMaxStage);

	for ( size_t i = 0; i < c->groups.size(); ++i )
		updateContainer(c->groups[i].get(), updateMaxStage);

	for ( size_t i = 0; i < c->structures.size(); ++i )
		updateContainer(c->structures[i].get(), updateMaxStage);
}


bool write(const Parameter *param, const Section *sec, int stage,
           set<string> &symbols, ofstream &ofs, const std::string &filename,
           bool withComment,
           bool &firstSection, bool &firstSectionParam) {
	if ( !param->symbols[stage] ) return true;

	// Do nothing
	if ( param->symbols[stage]->symbol.stage == Environment::CS_UNDEFINED )
		return true;

	if ( !ofs.is_open() ) {
		SEISCOMP_INFO("Updating %s", filename.c_str());
		ofs.open(filename.c_str());
		// Propagate error
		if ( !ofs.is_open() ) return false;
	}

	// Already saved this symbol?
	if ( symbols.find(param->variableName) != symbols.end() ) return true;

	if ( sec ) {
		if ( firstSectionParam ) {
			/*
			if ( !firstSection )
				ofs << endl << endl;
			else
				firstSection = false;

			ofs << "# --------------------------------------------" << endl;
			ofs << "# " << sec->name << " options" << endl;
			ofs << "# --------------------------------------------" << endl;
			ofs << endl;
			*/
			firstSectionParam = false;
		}
	}
	else
		firstSection = false;

	// Write description as comment.
	if ( withComment ) {
		if ( param->definition && !param->definition->description.empty() ) {
			if ( !symbols.empty() ) ofs << endl;
			ofs << blockComment(param->definition->description, 80) << endl;
		}
		else if ( !param->symbols[stage]->symbol.comment.empty() ) {
			if ( !symbols.empty() ) ofs << endl;
			ofs << param->symbols[stage]->symbol.comment << endl;
		}
	}

	ofs << param->variableName << " = ";
	if ( param->symbols[stage]->symbol.content.empty() )
		ofs << "\"\"";
	else
		ofs << param->symbols[stage]->symbol.content;
	//Config::Config::writeValues(ofs, &param->symbols[stage]->symbol);
	ofs << endl;

	symbols.insert(param->variableName);

	return true;
}


bool write(const Container *cont, const Section *sec, int stage,
           set<string> &symbols, ofstream &ofs, const std::string &filename,
           bool withComment, bool &firstSection, bool &firstSectionParam) {
	for ( size_t p = 0; p < cont->parameters.size(); ++p ) {
		if ( !write(cont->parameters[p].get(), sec, stage,
		            symbols, ofs, filename, withComment, firstSection, firstSectionParam) )
			return false;
	}

	for ( size_t g = 0; g < cont->groups.size(); ++g ) {
		if ( !write(cont->groups[g].get(), sec, stage,
		            symbols, ofs, filename, withComment, firstSection, firstSectionParam) )
			return false;
	}

	for ( size_t s = 0; s < cont->structures.size(); ++s ) {
		if ( !write(cont->structures[s].get(), sec, stage,
		            symbols, ofs, filename, withComment, firstSection, firstSectionParam) )
			return false;
	}

	return true;
}


bool compareName(const BindingPtr &v1, const BindingPtr &v2) {
	return v1->name < v2->name;
}


}


Parameter::Parameter(SchemaParameter *def, const char *n)
: parent(NULL), super(NULL), definition(def), variableName(n) {
	symbol.stage = Environment::CS_UNDEFINED;
}


Parameter::Parameter(SchemaParameter *def, const std::string &n)
: parent(NULL), super(NULL), definition(def), variableName(n) {
	symbol.stage = Environment::CS_UNDEFINED;
}


Parameter *Parameter::copy(bool backImport) {
	Parameter *param = new Parameter(definition, variableName);
	param->symbol = symbol;
	if ( backImport ) {
		param->super = NULL;
		super = param;
	}
	else
		param->super = this;

	return param;
}


Parameter *Parameter::clone() const {
	Parameter *param = new Parameter(definition, variableName);
	param->symbol = symbol;
	param->super = super;
	return param;
}


void Parameter::dump(std::ostream &os) const {
	if ( !symbol.uri.empty() )
		os << "[" << symbol.uri << "]" << endl;
	os << variableName << " = " << symbol.content;

	/*
	Symbol::Values::const_iterator it = symbol.values.begin();
	for ( ; it != symbol.values.end(); ++it ) {
		if ( it != symbol.values.begin() )
			os << ", ";
		os << *it;
	}
	*/

	os << endl;
}


bool Parameter::inherits(const Parameter *param) const {
	const Parameter *p = super;
	while ( p ) {
		if ( p == param ) return true;
		p = p->super;
	}

	return false;
}


void Parameter::updateFinalValue(int maxStage) {
	symbol = Symbol();
	symbol.values.push_back(definition->defaultValue);
	symbol.content = definition->defaultValue;

	if ( maxStage >= Environment::CS_QUANTITY )
		maxStage = Environment::CS_LAST;

	for ( int i = 0; i <= maxStage; ++i ) {
		SymbolMapItem *item = symbols[i].get();
		if ( !item || item->symbol.stage == Environment::CS_UNDEFINED )
			continue;
		symbol = item->symbol;
	}
}

void Container::add(Parameter *param) {
	param->parent = this;
	parameters.push_back(param);
}


void Container::add(Structure *struc) {
	struc->parent = this;
	structures.push_back(struc);
}


void Container::add(Group *group) {
	group->parent = this;
	groups.push_back(group);
}


void Container::addType(Structure *struc) {
	struc->parent = this;
	structureTypes.push_back(struc);
}


Structure *Container::findStructureType(const std::string &type) const {
	for ( size_t i = 0; i < structureTypes.size(); ++i )
		if ( structureTypes[i]->definition->type == type )
			return structureTypes[i].get();
	return NULL;
}


bool Container::hasStructure(const char *name) const {
	for ( size_t i = 0; i < structures.size(); ++i )
		if ( structures[i]->name == name ) return true;
	return false;
}


bool Container::hasStructure(const string &name) const {
	for ( size_t i = 0; i < structures.size(); ++i )
		if ( structures[i]->name == name ) return true;
	return false;
}


Structure *Container::instantiate(const Structure *s, const char *name) {
	if ( hasStructure(name) ) return NULL;

	Structure *ns = s->instantiate(name);
	if ( ns != NULL ) structures.push_back(ns);
	return ns;
}


bool Container::remove(Structure *s) {
	vector<StructurePtr>::iterator it;
	it = find(structures.begin(), structures.end(), s);
	if ( it == structures.end() ) return false;
	structures.erase(it);
	return true;
}


Parameter *Container::findParameter(const std::string &fullName) const {
	for ( size_t i = 0; i < parameters.size(); ++i ) {
		if ( parameters[i]->variableName == fullName )
			return parameters[i].get();
	}

	for ( size_t i = 0; i < groups.size(); ++i ) {
		Parameter *param = groups[i]->findParameter(fullName);
		if ( param != NULL ) return param;
	}

	for ( size_t i = 0; i < structures.size(); ++i ) {
		Parameter *param = structures[i]->findParameter(fullName);
		if ( param != NULL ) return param;
	}

	return NULL;
}


Container *Container::findContainer(const std::string &path) const {
	for ( size_t i = 0; i < groups.size(); ++i ) {
		if ( groups[i]->path == path ) return groups[i].get();
		Container *c = groups[i]->findContainer(path);
		if ( c != NULL ) return c;
	}

	for ( size_t i = 0; i < structures.size(); ++i ) {
		if ( structures[i]->path == path ) return structures[i].get();
		Container *c = structures[i]->findContainer(path);
		if ( c != NULL ) return c;
	}

	return NULL;
}


void Container::accept(ModelVisitor *visitor) const {
	for ( size_t i = 0; i < parameters.size(); ++i )
		visitor->visit(parameters[i].get(), false);

	for ( size_t i = 0; i < groups.size(); ++i ) {
		if ( !visitor->visit(groups[i].get()) ) continue;
		groups[i]->accept(visitor);
	}

	for ( size_t i = 0; i < structures.size(); ++i ) {
		if ( !visitor->visit(structures[i].get()) ) continue;
		structures[i]->accept(visitor);
	}
}


Structure *Structure::copy(bool backImport) {
	Structure *struc = new Structure(definition, path, name);
	if ( backImport ) {
		struc->super = NULL;
		super = struc;
	}
	else
		struc->super = this;

	for ( size_t i = 0; i < parameters.size(); ++i )
		struc->add(parameters[i]->copy(backImport));

	for ( size_t i = 0; i < groups.size(); ++i )
		struc->add(groups[i]->copy(backImport));

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		struc->addType(structureTypes[i]->clone());

	return struc;
}


Structure *Structure::clone() const {
	Structure *struc = new Structure(definition, path, name);
	for ( size_t i = 0; i < parameters.size(); ++i )
		struc->add(parameters[i]->clone());

	for ( size_t i = 0; i < groups.size(); ++i )
		struc->add(groups[i]->clone());

	for ( size_t i = 0; i < structures.size(); ++i )
		struc->add(structures[i]->clone());

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		struc->addType(structureTypes[i]->clone());

	return struc;
}


Structure *Structure::instantiate(const char *n) const {
	if ( !name.empty() ) return NULL;
	Structure *struc = loadStructure(definition, path, n);
	updateContainer(struc, Environment::CS_QUANTITY);

	Model::SymbolFileMap symbols;
	updateContainer(struc, symbols, Environment::CS_CONFIG_APP);
	return struc;
}


void Structure::dump(std::ostream &os) const {
	for ( size_t i = 0; i < parameters.size(); ++i )
		parameters[i]->dump(os);
}


Group *Group::copy(bool backImport) {
	Group *group = new Group(definition, path);
	if ( backImport ) {
		group->super = NULL;
		super = group;
	}
	else
		group->super = this;

	for ( size_t i = 0; i < parameters.size(); ++i )
		group->add(parameters[i]->copy(backImport));

	for ( size_t i = 0; i < groups.size(); ++i )
		group->add(groups[i]->copy(backImport));

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		group->addType(structureTypes[i]->clone());

	return group;
}


Group *Group::clone() const {
	Group *group = new Group(definition, path);
	for ( size_t i = 0; i < parameters.size(); ++i )
		group->add(parameters[i]->clone());

	for ( size_t i = 0; i < groups.size(); ++i )
		group->add(groups[i]->clone());

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		group->addType(structureTypes[i]->clone());

	return group;
}


void Group::dump(std::ostream &os) const {
	for ( size_t i = 0; i < parameters.size(); ++i )
		parameters[i]->dump(os);
}


Section *Section::copy(bool backImport) {
	Section *sec = new Section(name);
	sec->description = description;

	for ( size_t i = 0; i < parameters.size(); ++i )
		sec->add(parameters[i]->copy(backImport));

	for ( size_t i = 0; i < groups.size(); ++i )
		sec->add(groups[i]->copy(backImport));

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		sec->addType(structureTypes[i]->clone());

	return sec;
}


Section *Section::clone() const {
	Section *section = new Section(name);
	section->description = description;

	for ( size_t i = 0; i < parameters.size(); ++i )
		section->add(parameters[i]->clone());

	for ( size_t i = 0; i < groups.size(); ++i )
		section->add(groups[i]->clone());

	for ( size_t i = 0; i < structureTypes.size(); ++i )
		section->addType(structureTypes[i]->clone());

	return section;
}


void Section::dump(std::ostream &os) const {
	for ( size_t i = 0; i < parameters.size(); ++i )
		parameters[i]->dump(os);

	for ( size_t i = 0; i < groups.size(); ++i )
		groups[i]->dump(os);
}


Binding *Binding::clone() const {
	Binding *b = new Binding(*this);

	for ( size_t s = 0; s < b->sections.size(); ++s ) {
		b->sections[s] = b->sections[s]->clone();
		b->sections[s]->parent = b;
	}

	return b;
}


void Binding::dump(std::ostream &os) const {
	for ( size_t i = 0; i < sections.size(); ++i )
		sections[i]->dump(os);
}


Container *Binding::findContainer(const std::string &path) const {
	for ( size_t i = 0; i < sections.size(); ++i ) {
		Container *c = sections[i]->findContainer(path);
		if ( c != NULL ) return c;
	}

	return NULL;
}


Parameter *Binding::findParameter(const std::string &fullName) const {
	for ( size_t i = 0; i < sections.size(); ++i ) {
		Parameter *param = sections[i]->findParameter(fullName);
		if ( param != NULL ) return param;
	}

	return NULL;
}


void Binding::accept(ModelVisitor *visitor) const {
	for ( size_t i = 0; i < sections.size(); ++i ) {
		if ( !visitor->visit(sections[i].get()) ) continue;
		sections[i]->accept(visitor);
	}
}


Binding *BindingCategory::binding(const std::string &name) const {
	for ( size_t i = 0; i < bindingTypes.size(); ++i )
		if ( bindingTypes[i]->name == name )
			return bindingTypes[i].get();

	return NULL;
}


BindingCategory *BindingCategory::clone() const {
	BindingCategory *cat = new BindingCategory(*this);
	cat->parent = NULL;

	for ( size_t i = 0; i < cat->bindingTypes.size(); ++i ) {
		cat->bindingTypes[i] = cat->bindingTypes[i]->clone();
		cat->bindingTypes[i]->parent = (BindingCategory*)this;
	}

	for ( size_t i = 0; i < cat->bindings.size(); ++i ) {
		cat->bindings[i].binding = cat->bindings[i].binding->clone();
		cat->bindings[i].binding->parent = (BindingCategory*)this;
	}

	return cat;
}


void BindingCategory::dump(std::ostream &os) const {
	for ( size_t i = 0; i < bindings.size(); ++i )
		bindings[i].binding->dump(os);
}


Container *BindingCategory::findContainer(const std::string &path) const {
	BindingInstances::const_iterator it;
	for ( it = bindings.begin(); it != bindings.end(); ++it ) {
		Container *c = it->binding->findContainer(path);
		if ( c != NULL ) return c;
	}
	return NULL;
}


Parameter *BindingCategory::findParameter(const std::string &fullName) const {
	BindingInstances::const_iterator it;
	for ( it = bindings.begin(); it != bindings.end(); ++it ) {
		Parameter *p = it->binding->findParameter(fullName);
		if ( p != NULL ) return p;
	}
	return NULL;
}


void BindingCategory::accept(ModelVisitor *visitor) const {
	BindingInstances::const_iterator it;
	for ( it = bindings.begin(); it != bindings.end(); ++it )
		it->binding->accept(visitor);
}



bool BindingCategory::hasBinding(const char *alias) const {
	for ( size_t i = 0; i < bindings.size(); ++i )
		if ( bindings[i].alias == alias ) return true;
	return false;
}


Binding *BindingCategory::instantiate(const Binding *b, const char *alias) {
	string tmp = alias;
	if ( tmp.empty() ) tmp = b->name;

	if ( hasBinding(tmp.c_str()) ) return NULL;

	Binding *nb = loadCategoryBinding(b->definition, name + ".", tmp);
	for ( size_t i = 0; i < nb->sections.size(); ++i )
		updateContainer(nb->sections[i].get(), Environment::CS_QUANTITY);

	Model::SymbolFileMap symbols;
	for ( size_t i = 0; i < nb->sections.size(); ++i )
		updateContainer(nb->sections[i].get(), symbols, Environment::CS_CONFIG_APP);
	nb->parent = this;
	bindings.push_back(BindingInstance(nb, tmp));
	return nb;
}


const char *BindingCategory::alias(const Binding *b) const {
	for ( size_t i = 0; i < bindings.size(); ++i )
		if ( bindings[i].binding.get() == b ) return bindings[i].alias.c_str();

	return NULL;
}


bool BindingCategory::removeInstance(const Binding *b) {
	BindingInstances::iterator it = bindings.begin();
	for ( ; it != bindings.end(); ++it ) {
		if ( it->binding.get() == b ) {
			bindings.erase(it);
			return true;
		}
	}

	return false;
}


bool BindingCategory::removeInstance(const char *alias) {
	BindingInstances::iterator it = bindings.begin();
	for ( ; it != bindings.end(); ++it ) {
		if ( it->alias == alias ) {
			bindings.erase(it);
			return true;
		}
	}

	return false;
}


ModuleBinding *ModuleBinding::clone() const {
	ModuleBinding *b = new ModuleBinding(*this);

	b->parent = NULL;
	b->configFile = "";

	for ( size_t s = 0; s < b->sections.size(); ++s ) {
		b->sections[s] = b->sections[s]->clone();
		b->sections[s]->parent = (ModuleBinding*)this;
	}

	for ( size_t i = 0; i < b->categories.size(); ++i ) {
		b->categories[i] = b->categories[i]->clone();
		b->categories[i]->parent = b;
	}

	return b;
}


void ModuleBinding::add(BindingCategory *cat) {
	cat->parent = this;
	categories.push_back(cat);
}


BindingCategory *ModuleBinding::category(const std::string &name) const {
	for ( size_t i = 0; i < categories.size(); ++i )
		if ( categories[i]->name == name )
			return categories[i].get();

	return NULL;
}


void ModuleBinding::dump(std::ostream &os) const {
	Binding::dump(os);

	for ( size_t i = 0; i < categories.size(); ++i )
		categories[i]->dump(os);
}


//! Returns a container at path @path@.
Container *ModuleBinding::findContainer(const std::string &path) const {
	Container *c = Binding::findContainer(path);
	if ( c != NULL ) return c;

	Categories::const_iterator it;
	for ( it = categories.begin(); it != categories.end(); ++it ) {
		c = (*it)->findContainer(path);
		if ( c != NULL ) return c;
	}

	return NULL;
}


Parameter *ModuleBinding::findParameter(const std::string &fullName) const {
	Parameter *p = Binding::findParameter(fullName);
	if ( p != NULL ) return p;

	Categories::const_iterator it;
	for ( it = categories.begin(); it != categories.end(); ++it ) {
		p = (*it)->findParameter(fullName);
		if ( p != NULL ) return p;
	}

	return NULL;
}


void ModuleBinding::accept(ModelVisitor *visitor) const {
	Binding::accept(visitor);

	Categories::const_iterator it;
	for ( it = categories.begin(); it != categories.end(); ++it )
		(*it)->accept(visitor);
}



bool ModuleBinding::writeConfig(const string &filename) const {
	ofstream ofs(filename.c_str());
	if ( !ofs.is_open() ) return false;

	set<string> symbols;
	int stage = Environment::CS_CONFIG_APP;

	bool first = true;
	bool firstSection = true;

	for ( size_t s = 0; s < sections.size(); ++s ) {
		Section *section = sections[s].get();
		if ( !write(section, NULL, stage, symbols, ofs, filename, true, firstSection, first) )
			return false;
	}

	for ( size_t i = 0; i < categories.size(); ++i ) {
		BindingCategory *cat = categories[i].get();

		if ( cat->bindings.empty() ) continue;

		if ( !symbols.empty() ) ofs << endl;

		ofs << "# Activated plugins for category " << cat->name << endl;
		ofs << cat->name << " = ";
		symbols.insert(cat->name);

		for ( size_t b = 0; b < cat->bindings.size(); ++b ) {
			if ( b > 0 ) ofs << ", ";

			if ( cat->bindings[b].binding->name == cat->bindings[b].alias )
				ofs << cat->bindings[b].binding->name;
			else
				ofs << cat->bindings[b].alias << ":" << cat->bindings[b].binding->name;
		}

		ofs << endl;

		for ( size_t b = 0; b < cat->bindings.size(); ++b ) {
			Binding *curr = cat->bindings[b].binding.get();

			for ( size_t s = 0; s < curr->sections.size(); ++s ) {
				Section *sec = curr->sections[s].get();
				if ( !write(sec, NULL, stage, symbols, ofs, filename, true, firstSection, first) )
					return false;
			}
		}
	}

	return true;
}


void Module::add(Section *sec) {
	sec->parent = this;
	sections.push_back(sec);
}


Parameter *Module::findParameter(const std::string &fullName) const {
	for ( size_t i = 0; i < unknowns.size(); ++i ) {
		if ( unknowns[i]->variableName == fullName ) return unknowns[i].get();
	}

	for ( size_t i = 0; i < sections.size(); ++i ) {
		Parameter *param = sections[i]->findParameter(fullName);
		if ( param != NULL ) return param;
	}

	return NULL;
}


Container *Module::findContainer(const std::string &path) const {
	for ( size_t i = 0; i < sections.size(); ++i ) {
		Container *c = sections[i]->findContainer(path);
		if ( c != NULL ) return c;
	}

	return NULL;
}


bool Module::hasConfiguration() const {
	if ( definition == NULL || !definition->parameters ) return false;
	return definition->parameters->parameterCount() > 0 ||
	       definition->parameters->groupCount() > 0 ||
	       definition->parameters->structureCount() > 0 ||
	       !definition->isStandalone();
}


int Module::loadProfiles(const std::string &keyDir, ConfigDelegate *delegate) {
	if ( !supportsBindings() ) return -1;

	fs::directory_iterator it;
	fs::directory_iterator fsDirEnd;

	try {
		it = fs::directory_iterator(SC_FS_PATH(keyDir));
	}
	catch ( ... ) {
		it = fsDirEnd;
		SEISCOMP_DEBUG("%s not available", keyDir.c_str());
		return 0;
	}

	for ( ; it != fsDirEnd; ++it ) {
		if ( fs::is_directory(*it) ) continue;

		string filename = SC_FS_IT_LEAF(it);
		if ( filename.compare(0, 8, "profile_") != 0 )
			continue;

		ModuleBindingPtr profile = createBinding();
		if ( profile == NULL ) {
			cerr << "ERROR: internal error: unable to create binding" << endl;
			break;
		}

		profile->name = filename.substr(8);
		profile->configFile = SC_FS_IT_STR(it);

		if ( !loadBinding(*profile, profile->configFile, false, delegate) ) {
			cerr << "ERROR: invalid config file" << endl;
			continue;
		}

		addProfile(profile.get());
	}

	return (int)profiles.size();
}


bool Module::addProfile(ModuleBinding *b) {
	if ( b->name.empty() ) return false;

	// Does a profile with this name exist already?
	if ( getProfile(b->name) != NULL )
		return false;

	b->parent = this;
	profiles.push_back(b);
	return true;
}


bool Module::removeProfile(const std::string &profile) {
	ModuleBinding *b = NULL;

	for ( size_t i = 0; i < profiles.size(); ++i ) {
		if ( profiles[i]->name == profile ) {
			b = profiles[i].get();
			profiles.erase(profiles.begin()+i);
			break;
		}
	}

	if ( b == NULL ) return false;

	syncProfileRemoval(b);
	return true;
}


bool Module::removeProfile(ModuleBinding *profile) {
	ModuleBinding *b = NULL;

	for ( size_t i = 0; i < profiles.size(); ++i ) {
		if ( profiles[i] == profile ) {
			b = profiles[i].get();
			profiles.erase(profiles.begin()+i);
			break;
		}
	}

	if ( b == NULL ) return false;

	syncProfileRemoval(b);
	return true;
}


void Module::syncProfileRemoval(Binding *b) {
	// Remove all station bindings that use this binding.
	BindingMap::iterator it;
	for ( it = bindings.begin(); it != bindings.end(); ) {
		if ( it->second.get() == b )
			bindings.erase(it++);
		else
			++it;
	}
}


ModuleBinding *Module::bind(const StationID &id, const std::string &profile) {
	ModuleBinding *b;

	if ( profile.empty() )
		return readBinding(id, profile, true);
	else
		b = getProfile(profile);

	if ( b == NULL ) return NULL;
	if ( !bind(id, b) ) return NULL;
	return b;
}


bool Module::bind(const StationID &id, ModuleBinding *binding) {
	if ( binding->parent && binding->parent != this ) return false;
	binding->parent = this;
	bindings[id] = binding;
	return true;
}


bool Module::removeStation(const StationID &id) {
	BindingMap::iterator it = bindings.find(id);
	if ( it == bindings.end() ) return false;
	bindings.erase(it);
	return true;
}


ModuleBinding *Module::createBinding() const {
	if ( !bindingTemplate ) return NULL;
	return bindingTemplate->clone();
}


ModuleBinding *Module::createProfile(const std::string &name) {
	if ( getProfile(name) != NULL ) return NULL;

	ModuleBindingPtr prof = createBinding();
	prof->name = name;
	prof->configFile = keyDirectory + "/profile_" + prof->name;
	if ( !loadBinding(*prof, prof->configFile, true) ) return NULL;
	if ( !addProfile(prof.get()) ) return NULL;
	return prof.get();
}


ModuleBinding *Module::getProfile(const std::string &name) const {
	for ( size_t i = 0; i < profiles.size(); ++i )
		if ( profiles[i]->name == name )
			return profiles[i].get();

	return NULL;
}


ModuleBinding *Module::getBinding(const StationID &id) const {
	BindingMap::const_iterator it = bindings.find(id);
	if ( it == bindings.end() ) return NULL;
	return it->second.get();
}


bool Module::loadBinding(ModuleBinding &binding,
                         const std::string &filename,
                         bool allowConfigFileErrors,
                         ConfigDelegate *delegate) const {
	Model::SymbolFileMap tmp;
	Model::SymbolFileMap *usedSymbols;
	if ( !filename.empty() )
		usedSymbols = &model->symbols[filename];
	else
		usedSymbols = &tmp;

	Model::SymbolFileMap &symbols = *usedSymbols;

	if ( !filename.empty() ) {
		Config::Config *cfg = new Config::Config;

		if ( Util::fileExists(filename) ) {
			if ( delegate ) delegate->aboutToRead(filename.c_str());

			while ( true ) {
				if ( delegate ) cfg->setLogger(delegate);

				if ( cfg->readConfig(filename, Environment::CS_CONFIG_APP) ) {
					if ( delegate ) delegate->finishedReading(filename.c_str());
					break;
				}

				if ( delegate == NULL || !delegate->handleReadError(filename.c_str()) ) {
					if ( !allowConfigFileErrors ) {
						cerr << "ERROR: read " << filename << " failed" << endl;
						delete cfg;
						return false;
					}

					break;
				}

				delete cfg;
				cfg = new Config::Config;
			}
		}

		SymbolTable *symtab = cfg->symbolTable();
		if ( symtab == NULL ) {
			delete cfg;
			cerr << "ERROR: internal error: symbol table not available" << endl;
			return false;
		}

		for ( SymbolTable::iterator it = symtab->begin(); it != symtab->end(); ++it )
			symbols[(*it)->name] = new SymbolMapItem(**it);

		delete cfg;
	}
	else if ( !allowConfigFileErrors ) {
		cerr << "ERROR: file required" << endl;
		return false;
	}

	for ( size_t s = 0; s < binding.sections.size(); ++s ) {
		updateContainer(binding.sections[s].get(), symbols, Environment::CS_CONFIG_APP);
		updateContainer(binding.sections[s].get(), Environment::CS_QUANTITY);
	}

	for ( size_t c = 0; c < binding.categories.size(); ++c ) {
		BindingCategory *cat = binding.categories[c].get();
		cat->bindings.clear();

		SymbolMapItemPtr item = symbols[cat->name];
		if ( !item ) continue;

		const Symbol::Values &catValues = item->symbol.values;

		for ( size_t i = 0; i < catValues.size(); ++i ) {
			size_t pos = catValues[i].find(':');
			string alias, type;
			if ( pos != string::npos ) {
				alias = catValues[i].substr(0,pos);
				type = catValues[i].substr(pos+1);
			}
			else {
				type = catValues[i];
				alias = type;
			}

			Binding *b = cat->binding(type);
			if ( b == NULL ) {
				cerr << "WARNING: binding " << cat->name << "/" << type
				     << " does not exist: ignored" << endl;
				continue;
			}

			if ( !cat->instantiate(b, alias.c_str()) ) {
				cerr << "WARNING: binding " << cat->name << "/" << type
				     << " could not be added with alias '" << alias
				     << "': ignored" << endl;
				continue;
			}
		}

		for ( size_t b = 0; b < cat->bindings.size(); ++b ) {
			Binding *binding = cat->bindings[b].binding.get();
			for ( size_t s = 0; s < binding->sections.size(); ++s ) {
				updateContainer(binding->sections[s].get(), symbols, Environment::CS_CONFIG_APP);
				updateContainer(binding->sections[s].get(), Environment::CS_QUANTITY);
			}
		}
	}

	/*
	// Find active bindings for categories
	for ( size_t i = 0; i < binding.categories.size(); ++i ) {
		BindingCategory *cat = binding.categories[i].get();
		SymbolMapItemPtr item = symbols[cat->name];
		if ( !item )
			cat->activeBinding = NULL;
		else
			cat->activeBinding = cat->binding(item->symbol.content);
	}
	*/

	return true;
}


ModuleBinding *Module::readBinding(const StationID &id,
                                   const std::string &profile,
                                   bool allowConfigFileErrors,
                                   ConfigDelegate *delegate) {
	if ( !profile.empty() ) {
		ModuleBinding *b = bind(id, profile);
		if ( b ) return b;
	}

	ModuleBindingPtr binding = createBinding();
	if ( binding == NULL ) return NULL;

	binding->name = profile;
	binding->configFile = keyDirectory + "/";

	// Station key file
	if ( profile.empty() ) {
		binding->configFile += "station_";
		binding->configFile += id.networkCode + "_" + id.stationCode;
	}
	// Profile key file
	else {
		binding->configFile += "profile_";
		binding->configFile += profile;
	}

	if ( !loadBinding(*binding, binding->configFile, allowConfigFileErrors, delegate) )
		return NULL;

	if ( !profile.empty() && !addProfile(binding.get()) ) {
		cerr << "ERROR: adding profile '" << profile << "' to " << definition->name
		     << " failed" << endl;
		return NULL;
	}

	//binding->dump(cout);

	if ( !bind(id, binding.get()) )
		return NULL;

	return binding.get();
}


void Module::accept(ModelVisitor *visitor) const {
	for ( size_t i = 0; i < unknowns.size(); ++i )
		visitor->visit(unknowns[i].get(), true);

	for ( size_t i = 0; i < sections.size(); ++i ) {
		if ( !visitor->visit(sections[i].get()) ) continue;
		sections[i]->accept(visitor);
	}
}


bool Station::readConfig(const char *filename) {
	config.clear();

	ifstream ifs(filename);
	if ( !ifs.is_open() ) return false;

	set<string> mods;

	string line;
	while ( getline(ifs, line) ) {
		Core::trim(line);
		// Skip empty lines
		if ( line.empty() ) continue;
		// Skip comments
		if ( line[0] == '#' ) continue;

		size_t pos_colon = line.find(':');
		size_t pos_assign = line.find('=');
		string mod, profile;
		string name, value;

		if ( pos_colon != string::npos ) {
			mod = line.substr(0, pos_colon);
			profile = line.substr(pos_colon+1);
			Core::trim(mod);
			Core::trim(profile);
		}
		else if ( pos_assign != string::npos ) {
			name = line.substr(0, pos_assign);
			value = line.substr(pos_assign+1);
			Core::trim(name);
			Core::trim(value);
		}
		else
			mod = line;

		if ( !mod.empty() ) {
			if ( mods.find(mod) != mods.end() ) {
				cerr << filename << ": duplicate module entry for '" << mod << "': ignoring" << endl;
				continue;
			}

			mods.insert(mod);

			config.push_back(ModuleConfig(mod, profile));
		}
		else if ( !name.empty() ) {
			if ( tags.find(name) != tags.end() ) {
				cerr << filename << ": duplicate tag entry for '" << name << "': ignoring" << endl;
				continue;
			}

			tags[name] = value;
		}
	}

	return true;
}


bool Station::writeConfig(const char *filename) const {
	ofstream ofs(filename, ios::out);
	if ( !ofs.is_open() ) return false;

	if ( !tags.empty() ) {
		ofs << "# Station tags" << endl;
		Tags::const_iterator it;
		for ( it = tags.begin(); it != tags.end(); ++it )
			ofs << it->first << " = " << it->second << endl;
	}

	if ( !config.empty() ) {
		ofs << "# Binding references" << endl;
		ModuleConfigs::const_iterator it;
		for ( it = config.begin(); it != config.end(); ++it ) {
			ofs << it->moduleName;
			if ( !it->profile.empty() )
				ofs << ":" << it->profile;
			ofs << endl;
		}
	}

	return true;
}


void Station::setConfig(const std::string &module, const std::string &profile) {
	ModuleConfigs::iterator it;
	for ( it = config.begin(); it != config.end(); ++it ) {
		if ( it->moduleName == module ) {
			it->profile = profile;
			return;
		}
	}

	config.push_back(ModuleConfig(module, profile));
}


bool Station::compareTag(const std::string &name, const std::string &value) const {
	Tags::const_iterator it = tags.find(name);
	if ( it == tags.end() ) return false;
	return it->second == value;
}


Module *Model::module(const std::string &name) const {
	ModMap::const_iterator it = modMap.find(name);
	if ( it == modMap.end() ) return NULL;
	return it->second;
}


std::string Model::systemConfigFilename(bool, const std::string &name) const {
	return Environment::Instance()->appConfigFileName(name);
}


std::string Model::configFileLocation(bool, const std::string &name, int stage) const {
	return Environment::Instance()->configFileLocation(name, stage);
}

std::string Model::stationConfigDir(bool, const std::string &name) const {
	if ( name.empty() )
		return Environment::Instance()->appConfigDir() + "/key";
	else
		return Environment::Instance()->appConfigDir() + "/key/" + name;
}


Model::Model() : schema(NULL) {}


bool Model::create(SchemaDefinitions *schema) {
	modules.clear();
	modMap.clear();
	symbols.clear();

	this->schema = schema;

	// Build module definitions
	for ( size_t i = 0; i < schema->moduleCount(); ++i ) {
		SchemaModule *def = schema->module(i);
		create(schema, def);
	}

	// Build back imports
	for ( size_t i = 0; i < schema->moduleCount(); ++i ) {
		SchemaModule *def = schema->module(i);
		// We do not backimport aliases
		if ( def->aliasedModule != NULL ) continue;

		ModMap::iterator mit = modMap.find(def->name);
		if ( mit == modMap.end() ) continue;

		Module *mod = mit->second;

		set<string> imports;
		// Default is: standalone = false
		// Then "global" is automatically imported
		if ( def->name != "global" && (!def->standalone || *def->standalone == false) )
			imports.insert("global");

		if ( !def->import.empty() && def->import != def->name )
			imports.insert(def->import);

		Section *modSec = mod->sections.back().get();

		set<string>::iterator it;
		for ( it = imports.begin(); it != imports.end(); ++it ) {
			mit = modMap.find(*it);
			if ( mit == modMap.end() ) continue;

			Module *baseMod = mit->second;
			Section *secCopy = modSec->copy(true);
			secCopy->description = string("Backimport which allows to configure ") + mod->definition->name +
			                              " (including its aliases) parameters in " + baseMod->definition->name + ".";
			baseMod->add(secCopy);
		}
	}

	/*
	DuplicateNameCheck check;
	accept(&check);
	*/

	return true;
}


Module *Model::create(SchemaDefinitions *schema, SchemaModule *def) {
	if ( def == NULL ) return NULL;

	// Loaded already?
	ModMap::iterator mit = modMap.find(def->name);
	if ( mit != modMap.end() ) return mit->second;

	set<string> imports;

	// Default is: standalone = false
	// Then "global" is automatically imported
	if ( def->name != "global" && (!def->standalone || *def->standalone == false) )
		imports.insert("global");

	if ( !def->import.empty() && def->import != def->name )
		imports.insert(def->import);

	ModulePtr mod = new Module(def);

	if ( !def->category.empty() )
		categories.insert(def->category);

	set<string>::iterator it;
	for ( it = imports.begin(); it != imports.end(); ++it ) {
		SchemaModule *baseDef = schema->module(*it);
		if ( baseDef == NULL ) {
			// TODO: set error message
			return NULL;
		}

		Module *base = create(schema, baseDef);
		if ( base == NULL ) {
			// TODO: set error message
			return NULL;
		}

		for ( size_t i = 0; i < base->sections.size(); ++i ) {
			Section *secCopy = base->sections[i]->copy();
			secCopy->description = string("Import of ") + base->definition->name + " parameters "
			                       "which can be overwritten in this configuration.";
			mod->add(secCopy);
		}
	}


	SectionPtr sec = new Section(def->name);
	//sec->description = def->description;

	if ( def->parameters ) {
		for ( size_t j = 0; j < def->parameters->parameterCount(); ++j ) {
			SchemaParameter *pdef = def->parameters->parameter(j);
			ParameterPtr param = new Parameter(pdef, pdef->name);
			sec->add(param.get());
		}

		for ( size_t j = 0; j < def->parameters->groupCount(); ++j )
			loadGroup(sec.get(), def->parameters->group(j), "");

		for ( size_t j = 0; j < def->parameters->structureCount(); ++j )
			sec->addType(loadStructure(def->parameters->structure(j), "", ""));
	}

	// Load plugin definitions
	vector<SchemaPlugin*> plugins = schema->pluginsForModule(def->name);
	for ( size_t p = 0; p < plugins.size(); ++p ) {
		SchemaPlugin *plugin = plugins[p];
		if ( plugin->parameters ) {
			for ( size_t j = 0; j < plugin->parameters->parameterCount(); ++j ) {
				SchemaParameter *pdef = plugin->parameters->parameter(j);
				ParameterPtr param = new Parameter(pdef, pdef->name);
				sec->add(param.get());
			}

			for ( size_t j = 0; j < plugin->parameters->groupCount(); ++j )
				loadGroup(sec.get(), plugin->parameters->group(j), "");

			for ( size_t j = 0; j < plugin->parameters->structureCount(); ++j )
				sec->addType(loadStructure(plugin->parameters->structure(j), "", ""));
		}
	}

	mod->add(sec.get());

	// Create bindings model
	vector<SchemaBinding*> schemaBindings = schema->bindingsForModule(def->name);
	vector<SchemaBinding*> globalBindings;

	if ( !schemaBindings.empty() && imports.find("global") != imports.end() &&
	     def->inheritGlobalBinding && *def->inheritGlobalBinding == true )
		globalBindings = schema->bindingsForModule("global");

	// Import global
	if ( !globalBindings.empty() ) {
		Section *sec = new Section("global");
		sec->description = "The global section allows to override parameters of "
		                   "the global binding. The values do not reflect the "
		                   "currently assigned global binding values but the "
		                   "values given in this binding.";

		if ( mod->bindingTemplate == NULL ) {
			mod->bindingTemplate = new ModuleBinding(def->name);
			mod->bindingTemplate->parent = mod.get();
		}

		mod->bindingTemplate->sections.push_back(sec);

		for ( size_t i = 0; i < globalBindings.size(); ++i ) {
			SchemaBinding *sb = globalBindings[i];

			// Category bindings are not supported for import
			if ( !sb->category.empty() ) continue;

			if ( sb->parameters ) {
				for ( size_t j = 0; j < sb->parameters->parameterCount(); ++j ) {
					SchemaParameter *pdef = sb->parameters->parameter(j);
					ParameterPtr param = new Parameter(pdef, pdef->name);
					sec->add(param.get());
				}

				for ( size_t j = 0; j < sb->parameters->groupCount(); ++j )
					loadGroup(sec, sb->parameters->group(j), "");

				for ( size_t j = 0; j < sb->parameters->structureCount(); ++j )
					sec->addType(loadStructure(sb->parameters->structure(j), "", ""));
			}
		}
	}

	Section *bindingSection = NULL;

	for ( size_t i = 0; i < schemaBindings.size(); ++i ) {
		SchemaBinding *sb = schemaBindings[i];
		if ( mod->bindingTemplate == NULL ) {
			mod->bindingTemplate = new ModuleBinding(def->name);
			mod->bindingTemplate->parent = mod.get();
		}

		if ( bindingSection == NULL ) {
			bindingSection = new Section(def->name);
			mod->bindingTemplate->sections.push_back(bindingSection);
		}

		Section *sec = NULL;
		string prefix;

		if ( sb->category.empty() ) {
			sec = bindingSection;
			// TODO: use definition vector to store multiple
			//       definitions (eg when using plugins)
			mod->bindingTemplate->definition = sb;
		}
		else if ( !sb->name.empty() ) {
			BindingCategoryPtr cat = mod->bindingTemplate->category(sb->category);
			if ( cat == NULL ) {
				cat = new BindingCategory(sb->category.c_str());
				mod->bindingTemplate->add(cat.get());
			}

			BindingPtr b = cat->binding(sb->name);
			if ( b != NULL ) {
				cerr << "ERROR: " << def->name << "/" << sb->category << ": "
				        "duplicate name '" << sb->name << "'" << endl;
				continue;
			}

			b = new Binding(sb->name);
			b->definition = sb;
			b->parent = cat.get();
			b->description = b->definition->description;
			b->sections.push_back(new Section(sb->name));
			cat->bindingTypes.push_back(b);
			sec = b->sections.back().get();

			// Prefix configuration parameters with cat.name.
			prefix = sb->category + "." + sb->name + ".";
		}
		else {
			cerr << "ERROR: " << def->name << "/" << sb->category << " binding: "
			        "no name but a category: ignoring" << endl;
			continue;
		}

		if ( sb->parameters ) {
			for ( size_t j = 0; j < sb->parameters->parameterCount(); ++j ) {
				SchemaParameter *pdef = sb->parameters->parameter(j);
				ParameterPtr param = new Parameter(pdef, prefix + pdef->name);
				sec->add(param.get());
			}

			for ( size_t j = 0; j < sb->parameters->groupCount(); ++j )
				loadGroup(sec, sb->parameters->group(j), prefix);

			for ( size_t j = 0; j < sb->parameters->structureCount(); ++j )
				sec->addType(loadStructure(sb->parameters->structure(j), prefix, ""));
		}
	}

	if ( mod->bindingTemplate ) {
		for ( size_t i = 0; i < mod->bindingTemplate->categories.size(); ++i ) {
			BindingCategory *cat = mod->bindingTemplate->categories[i].get();
			sort(cat->bindingTypes.begin(), cat->bindingTypes.end(), compareName);
		}
	}

	mod->model = this;
	modules.push_back(mod);
	modMap[def->name] = mod.get();

	return mod.get();
}


bool Model::readConfig(int updateMaxStage, ConfigDelegate *delegate) {
	fs::directory_iterator it;
	fs::directory_iterator fsDirEnd;
	string keyDir;

	// Clear configuration of stations
	stations.clear();
	symbols.clear();

	// Collect all symbols at each stage and build global map
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();

		// Clear bindings and profiles
		mod->bindings.clear();
		mod->profiles.clear();

		// Initialize key directory
		mod->keyDirectory = stationConfigDir(true, mod->definition->name);

		// Initialize config file name
		mod->configFile = systemConfigFilename(true, mod->definition->name);

		for ( int stage = Environment::CS_FIRST; stage <= Environment::CS_LAST; ++stage ) {
			string uri = configFileLocation(true, mod->definition->name, stage);
			// File already read
			if ( symbols.find(uri) != symbols.end() ) continue;

			SEISCOMP_DEBUG("reading config %s", uri.c_str());

			if ( delegate ) delegate->aboutToRead(uri.c_str());

			Config::Config *cfg;

			while ( true ) {
				cfg = new Config::Config();

				if ( delegate ) cfg->setLogger(delegate);

				if ( cfg->readConfig(uri, stage, true) ) {
					if ( delegate ) delegate->finishedReading(uri.c_str());
					break;
				}

				if ( delegate == NULL || !delegate->handleReadError(uri.c_str()) )
					break;

				delete cfg;
			}

			symbols[uri] = SymbolFileMap();

			SymbolTable *symtab = cfg->symbolTable();
			if ( symtab == NULL ) {
				delete cfg;
				continue;
			}

			for ( SymbolTable::iterator it = symtab->begin(); it != symtab->end(); ++it )
				symbols[(*it)->uri][(*it)->name] = new SymbolMapItem(**it);

			delete cfg;
		}
	}

	// Update each module parameter with configuration symbols
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();
		bool isGlobal = mod->definition->name == "global";
		map<string, ParameterPtr> unknowns;
		mod->unknowns.clear();

		for ( int stage = Environment::CS_FIRST; stage <= Environment::CS_LAST; ++stage ) {
			SymbolFileMap *fileMap;
			string uri;
			if ( isGlobal && (stage == Environment::CS_DEFAULT_GLOBAL ||
			                  stage == Environment::CS_CONFIG_GLOBAL ||
			                  stage == Environment::CS_USER_GLOBAL) ) {
				uri = "";
				fileMap = &symbols[uri];
				fileMap->clear();
			}
			else {
				uri = configFileLocation(true, mod->definition->name, stage);
				fileMap = &symbols[uri];
			}

			for ( size_t s = 0; s < mod->sections.size(); ++s ) {
				Section *sec = mod->sections[s].get();
				updateContainer(sec, *fileMap, stage);
			}

			// Collect and store unknown symbols
			SymbolFileMap::iterator it;
			for ( it = fileMap->begin(); it != fileMap->end(); ++it ) {
				if ( it->second->known ) continue;
				ParameterPtr param = unknowns[it->first];
				if ( param == NULL ) {
					param = new Parameter(NULL, it->first);
					unknowns[it->first] = param;
					mod->unknowns.push_back(param);
				}

				param->symbols[stage] = it->second;
			}
		}

		for ( size_t s = 0; s < mod->sections.size(); ++s ) {
			Section *sec = mod->sections[s].get();
			updateContainer(sec, updateMaxStage);
		}

		// Read available profiles
		mod->loadProfiles(stationConfigDir(true, mod->definition->name), delegate);

		// Check for case sensitivity conflicts
		for ( size_t p = 0; p < mod->unknowns.size(); ++ p ) {
			ParameterPtr param = mod->unknowns[p];
			for ( int stage = Environment::CS_FIRST; stage <= Environment::CS_LAST; ++stage ) {
				if ( param->symbols[stage] == NULL ) continue;
				CaseSensitivityCheck check(delegate, mod, stage, &param->symbols[stage]->symbol);
				mod->accept(&check);
			}
		}
	}


	// Read station module configuration
	keyDir = stationConfigDir(true);

	try {
		it = fs::directory_iterator(SC_FS_PATH(keyDir));
	}
	catch ( ... ) {
		it = fsDirEnd;
		SEISCOMP_DEBUG("%s not available", keyDir.c_str());
	}

	for ( ; it != fsDirEnd; ++it ) {
		if ( fs::is_directory(*it) ) continue;
		string filename = SC_FS_IT_LEAF(it);
		if ( filename.compare(0, 8, "station_") != 0 )
			continue;

		size_t pos = filename.find('_', 8);
		if ( pos == string::npos ) {
			cerr << filename << ": invalid station id: expected '_' as "
			                    "net-sta separator: ignoring" << endl;
			continue;
		}

		StationID id;
		id.networkCode = filename.substr(8, pos-8);
		id.stationCode = filename.substr(pos+1);

		if ( id.networkCode.empty() ) {
			cerr << filename << ": invalid station id: network code must "
			                    "not be empty: ignoring" << endl;
			continue;
		}

		if ( id.stationCode.empty() ) {
			cerr << filename << ": invalid station id: station code must "
			                    "not be empty: ignoring" << endl;
			continue;
		}

		//SEISCOMP_DEBUG("reading station key %s", it->path().string().c_str());
		StationPtr station = new Station;
		if ( !station->readConfig(SC_FS_IT_STR(it).c_str()) )
			cerr << SC_FS_IT_STR(it) << ": error reading configuration: "
			                               "set empty" << endl;

		stations[id] = station;

		for ( size_t i = 0; i < station->config.size(); ++i ) {
			Module *mod = module(station->config[i].moduleName);
			if ( mod == NULL ) {
				cerr << "ERROR: module '" << station->config[i].moduleName
				     << "' is not registered" << endl;
				continue;
			}

			if ( !mod->bindingTemplate ) {
				cerr << "ERROR: module '" << station->config[i].moduleName
				     << "' has no registered binding definitions: ignoring" << endl;
				continue;
			}

			if ( !mod->readBinding(id, station->config[i].profile, false, delegate) )
				cerr << "ERROR: " << station->config[i].moduleName
				     << ": read binding for " << id.networkCode << "."
				     << id.stationCode << " failed: removed" << endl;
		}
	}

	/*
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();
		for ( Module::BindingMap::iterator it = mod->bindings.begin();
		      it != mod->bindings.end(); ++it ) {
			cout << "[" << mod->definition->name << "/" << it->first.stationCode << "]" << endl;
			it->second->dump(cout);
		}
	}
	*/

	/*
	for ( SymbolMap::iterator it1 = symbols.begin(); it1 != symbols.end(); ++it1 ) {
		cout << "<" << it1->first << ">" << endl;
		for ( SymbolFileMap::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ) {
			cout << "  <" << it2->first << ">"
			     << it2->second->symbol.content
			     << "</" << it2->first << ">"
			     << endl;
		}
		cout << "</" << it1->first << ">" << endl;
	}
	*/


	return true;
}


bool Model::writeConfig(int stage) {
	//---------------------------------------------------
	// Save station key files
	//---------------------------------------------------
	//stations.clear();
	for ( Stations::iterator it = stations.begin(); it != stations.end(); ++it )
		it->second->config.clear();

	// Collect all stations keyfiles
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();
		if ( !mod->supportsBindings() ) continue;

		Module::BindingMap::iterator it;

		for ( it = mod->bindings.begin(); it != mod->bindings.end(); ++it ) {
			pair<Stations::iterator, bool> sp;
			sp = stations.insert(Stations::value_type(it->first, NULL));
			if ( sp.second )
				sp.first->second = new Station;

			ModuleBinding *binding = it->second.get();
			sp.first->second->setConfig(mod->definition->name, binding->name);
		}
	}

	string keyDir = stationConfigDir(false);
	fs::directory_iterator it;
	fs::directory_iterator fsDirEnd;

	SEISCOMP_INFO("Updating bindings in %s", keyDir.c_str());

	// Clean up old key directory
	try {
		it = fs::directory_iterator(SC_FS_PATH(keyDir));
	}
	catch ( ... ) {
		it = fsDirEnd;
	}

	for ( ; it != fsDirEnd; ++it ) {
		string filename = SC_FS_IT_LEAF(it);
		if ( filename.compare(0, 8, "station_") != 0 )
			continue;

		size_t pos = filename.find('_', 8);
		if ( pos == string::npos ) continue;

		StationID id;
		id.networkCode = filename.substr(8, pos-8);
		id.stationCode = filename.substr(pos+1);

		if ( id.networkCode.empty() ) continue;
		if ( id.stationCode.empty() ) continue;

		if ( stations.find(id) == stations.end() ) {
			try { fs::remove(*it); } catch ( ... ) {}
		}
	}

	// Save configuration
	createPath(keyDir);

	for ( Stations::iterator it = stations.begin(); it != stations.end(); ++it ) {
		string filename = keyDir + "/station_" + it->first.networkCode +
		                  "_" + it->first.stationCode;
		if ( !it->second->writeConfig(filename.c_str()) )
			cerr << "[ERROR] writing " << filename << " failed" << endl;
	}


	//---------------------------------------------------
	// Save module configurations
	//---------------------------------------------------
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();
		string filename = configFileLocation(false, mod->definition->name, stage);
		if ( !writeConfig(mod, filename, stage) )
			cerr << "[ERROR] writing " << filename << " failed" << endl;
	}


	//---------------------------------------------------
	// Save bindings
	//---------------------------------------------------
	for ( size_t i = 0; i < modules.size(); ++i ) {
		Module *mod = modules[i].get();
		if ( !mod->supportsBindings() ) continue;

		string keyDir = stationConfigDir(false, mod->definition->name);
		createPath(keyDir);

		set<string> files;

		// Save profiles
		Module::Profiles::iterator pit;
		for ( pit = mod->profiles.begin(); pit != mod->profiles.end(); ++pit) {
			ModuleBinding *prof = pit->get();
			// Safety check, but this should never happen
			if ( prof->name.empty() ) continue;
			files.insert(string("profile_") + prof->name);
			if ( !prof->writeConfig(keyDir + "/profile_" + prof->name) )
				cerr << "[ERROR] writing profile " << keyDir << "/profile"
				     << prof->name << " failed" << endl;
		}

		// Save station keys
		Module::BindingMap::iterator bit;
		for ( bit = mod->bindings.begin(); bit != mod->bindings.end(); ++bit ) {
			ModuleBinding *binding = bit->second.get();
			// Profiles have been written already
			if ( !binding->name.empty() ) continue;
			files.insert(string("station_") + bit->first.networkCode +
			             "_" + bit->first.stationCode);
			if ( !binding->writeConfig(keyDir + "/station_" +
			                           bit->first.networkCode + "_" +
			                           bit->first.stationCode) )
				cerr << "[ERROR] writing binding " << keyDir << "/profile"
				     << binding->name << " failed" << endl;
		}

		// Remove unused files
		try {
			it = fs::directory_iterator(SC_FS_PATH(keyDir));
		}
		catch ( ... ) {
			it = fsDirEnd;
		}

		for ( ; it != fsDirEnd; ++it ) {
			if ( files.find(SC_FS_IT_LEAF(it)) == files.end() ) {
				try { fs::remove(*it); } catch ( ... ) {}
			}
		}
	}

	return true;
}


bool Model::writeConfig(Module *mod, const std::string &filename, int stage) {
	ofstream ofs;
	bool firstSection = true;
	bool first;

	set<string> symbols;

	for ( size_t s = 0; s < mod->sections.size(); ++s ) {
		Section *sec = mod->sections[s].get();
		Section *paramSection = sec;

		//if ( sec->name == mod->definition->name )
		//	paramSection = NULL;

		first = true;

		if ( !write(sec, paramSection, stage, symbols, ofs, filename, true, firstSection, first) )
			return false;
	}

	first = true;

	for ( size_t p = 0; p < mod->unknowns.size(); ++ p ) {
		Parameter *param = mod->unknowns[p].get();
		if ( !write(param, NULL, stage, symbols, ofs, filename, true, firstSection, first) )
			return false;
	}

	// Nothing written, remove the file
	if ( symbols.empty() ) {
		try {
			fs::remove(SC_FS_PATH(filename));
		}
		catch ( ... ) {}
	}

	return true;
}


void Model::update(const Module *mod, Container *container) const {
	bool isGlobal = mod->definition->name == "global";

	for ( int stage = Environment::CS_FIRST; stage <= Environment::CS_LAST; ++stage ) {
		SymbolFileMap *fileMap;
		string uri;
		if ( isGlobal && (stage == Environment::CS_DEFAULT_GLOBAL ||
		                  stage == Environment::CS_CONFIG_GLOBAL ||
		                  stage == Environment::CS_USER_GLOBAL) ) {
			uri = "";
			fileMap = &symbols[uri];
		}
		else {
			uri = configFileLocation(true, mod->definition->name, stage);
			fileMap = &symbols[uri];
		}

		updateContainer(container, *fileMap, stage);
	}

	updateContainer(container, Environment::CS_QUANTITY);
}


void Model::updateBinding(const ModuleBinding *mod, Binding *binding) const {
	SymbolFileMap *fileMap = &symbols[mod->configFile];
	for ( size_t s = 0; s < binding->sections.size(); ++s ) {
		updateContainer(binding->sections[s].get(), *fileMap, Environment::CS_CONFIG_APP);
		updateContainer(binding->sections[s].get(), Environment::CS_QUANTITY);
	}
}


bool Model::addStation(const StationID &id) {
	Stations::iterator it = stations.find(id);
	if ( it != stations.end() ) return false;
	stations[id] = new Station;
	return true;
}


bool Model::removeStation(const StationID &id) {
	Stations::iterator it = stations.find(id);
	if ( it == stations.end() ) return false;
	stations.erase(it);

	for ( size_t i = 0; i < modules.size(); ++i )
		modules[i]->removeStation(id);

	return true;
}


bool Model::removeNetwork(const std::string &net) {
	bool found = false;
	Stations::iterator it = stations.begin();
	for ( ; it != stations.end(); ) {
		if ( it->first.networkCode != net )
			++it;
		else {
			for ( size_t i = 0; i < modules.size(); ++i )
				modules[i]->removeStation(it->first);
			stations.erase(it++);
			found = true;
		}
	}

	return found;
}


bool Model::removeStationModule(const StationID &id, Module *mod) {
	Stations::iterator it = stations.find(id);
	if ( it == stations.end() ) return false;

	Station *sta = it->second.get();
	Station::ModuleConfigs::iterator cit = sta->config.begin();
	for ( ; cit != sta->config.end(); ++cit ) {
		if ( cit->moduleName == mod->definition->name ) {
			sta->config.erase(cit);
			return true;
		}
	}

	return mod->removeStation(id);
}


void Model::accept(ModelVisitor *visitor) const {
	for ( size_t i = 0; i < modules.size(); ++i ) {
		if ( !visitor->visit(modules[i].get()) ) continue;
		modules[i]->accept(visitor);
	}
}


}
}
