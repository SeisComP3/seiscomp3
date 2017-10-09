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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/config.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/configmodule.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(Config, PublicObject, "Config");


Config::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("parameterSet", "ParameterSet", &Config::parameterSetCount, &Config::parameterSet, static_cast<bool (Config::*)(ParameterSet*)>(&Config::add), &Config::removeParameterSet, static_cast<bool (Config::*)(ParameterSet*)>(&Config::remove)));
	addProperty(arrayObjectProperty("module", "ConfigModule", &Config::configModuleCount, &Config::configModule, static_cast<bool (Config::*)(ConfigModule*)>(&Config::add), &Config::removeConfigModule, static_cast<bool (Config::*)(ConfigModule*)>(&Config::remove)));
}


IMPLEMENT_METAOBJECT(Config)


Config::Config() : PublicObject("Config") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config::Config(const Config& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config::~Config() {
	std::for_each(_parameterSets.begin(), _parameterSets.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ParameterSet::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ParameterSetPtr::get)));
	std::for_each(_configModules.begin(), _configModules.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&ConfigModule::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&ConfigModulePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::operator==(const Config& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::operator!=(const Config& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::equal(const Config& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Config& Config::operator=(const Config& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::assign(Object* other) {
	Config* otherConfig = Config::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherConfig;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Config::clone() const {
	Config* clonee = new Config();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::updateChild(Object* child) {
	ParameterSet* parameterSetChild = ParameterSet::Cast(child);
	if ( parameterSetChild != NULL ) {
		ParameterSet* parameterSetElement
			= ParameterSet::Cast(PublicObject::Find(parameterSetChild->publicID()));
		if ( parameterSetElement && parameterSetElement->parent() == this ) {
			*parameterSetElement = *parameterSetChild;
			return true;
		}
		return false;
	}

	ConfigModule* configModuleChild = ConfigModule::Cast(child);
	if ( configModuleChild != NULL ) {
		ConfigModule* configModuleElement
			= ConfigModule::Cast(PublicObject::Find(configModuleChild->publicID()));
		if ( configModuleElement && configModuleElement->parent() == this ) {
			*configModuleElement = *configModuleChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Config::accept(Visitor* visitor) {
	for ( std::vector<ParameterSetPtr>::iterator it = _parameterSets.begin(); it != _parameterSets.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<ConfigModulePtr>::iterator it = _configModules.begin(); it != _configModules.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Config::parameterSetCount() const {
	return _parameterSets.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParameterSet* Config::parameterSet(size_t i) const {
	return _parameterSets[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ParameterSet* Config::findParameterSet(const std::string& publicID) const {
	for ( std::vector<ParameterSetPtr>::const_iterator it = _parameterSets.begin(); it != _parameterSets.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::add(ParameterSet* parameterSet) {
	if ( parameterSet == NULL )
		return false;

	// Element has already a parent
	if ( parameterSet->parent() != NULL ) {
		SEISCOMP_ERROR("Config::add(ParameterSet*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ParameterSet* parameterSetCached = ParameterSet::Find(parameterSet->publicID());
		if ( parameterSetCached ) {
			if ( parameterSetCached->parent() ) {
				if ( parameterSetCached->parent() == this )
					SEISCOMP_ERROR("Config::add(ParameterSet*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Config::add(ParameterSet*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				parameterSet = parameterSetCached;
		}
	}

	// Add the element
	_parameterSets.push_back(parameterSet);
	parameterSet->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		parameterSet->accept(&nc);
	}

	// Notify registered observers
	childAdded(parameterSet);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::remove(ParameterSet* parameterSet) {
	if ( parameterSet == NULL )
		return false;

	if ( parameterSet->parent() != this ) {
		SEISCOMP_ERROR("Config::remove(ParameterSet*) -> element has another parent");
		return false;
	}

	std::vector<ParameterSetPtr>::iterator it;
	it = std::find(_parameterSets.begin(), _parameterSets.end(), parameterSet);
	// Element has not been found
	if ( it == _parameterSets.end() ) {
		SEISCOMP_ERROR("Config::remove(ParameterSet*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_parameterSets.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::removeParameterSet(size_t i) {
	// index out of bounds
	if ( i >= _parameterSets.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_parameterSets[i]->accept(&nc);
	}

	_parameterSets[i]->setParent(NULL);
	childRemoved(_parameterSets[i].get());
	
	_parameterSets.erase(_parameterSets.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Config::configModuleCount() const {
	return _configModules.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfigModule* Config::configModule(size_t i) const {
	return _configModules[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConfigModule* Config::findConfigModule(const std::string& publicID) const {
	for ( std::vector<ConfigModulePtr>::const_iterator it = _configModules.begin(); it != _configModules.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::add(ConfigModule* configModule) {
	if ( configModule == NULL )
		return false;

	// Element has already a parent
	if ( configModule->parent() != NULL ) {
		SEISCOMP_ERROR("Config::add(ConfigModule*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		ConfigModule* configModuleCached = ConfigModule::Find(configModule->publicID());
		if ( configModuleCached ) {
			if ( configModuleCached->parent() ) {
				if ( configModuleCached->parent() == this )
					SEISCOMP_ERROR("Config::add(ConfigModule*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("Config::add(ConfigModule*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				configModule = configModuleCached;
		}
	}

	// Add the element
	_configModules.push_back(configModule);
	configModule->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		configModule->accept(&nc);
	}

	// Notify registered observers
	childAdded(configModule);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::remove(ConfigModule* configModule) {
	if ( configModule == NULL )
		return false;

	if ( configModule->parent() != this ) {
		SEISCOMP_ERROR("Config::remove(ConfigModule*) -> element has another parent");
		return false;
	}

	std::vector<ConfigModulePtr>::iterator it;
	it = std::find(_configModules.begin(), _configModules.end(), configModule);
	// Element has not been found
	if ( it == _configModules.end() ) {
		SEISCOMP_ERROR("Config::remove(ConfigModule*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_configModules.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Config::removeConfigModule(size_t i) {
	// index out of bounds
	if ( i >= _configModules.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_configModules[i]->accept(&nc);
	}

	_configModules[i]->setParent(NULL);
	childRemoved(_configModules[i].get());
	
	_configModules.erase(_configModules.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Config::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Config skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("parameterSet",
	                       Seiscomp::Core::Generic::containerMember(_parameterSets,
	                       Seiscomp::Core::Generic::bindMemberFunction<ParameterSet>(static_cast<bool (Config::*)(ParameterSet*)>(&Config::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("module",
	                       Seiscomp::Core::Generic::containerMember(_configModules,
	                       Seiscomp::Core::Generic::bindMemberFunction<ConfigModule>(static_cast<bool (Config::*)(ConfigModule*)>(&Config::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
