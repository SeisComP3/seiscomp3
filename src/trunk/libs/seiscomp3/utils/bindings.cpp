/***************************************************************************
 * Copyright (C) 2015 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/utils/bindings.h>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Util {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

Core::MetaValue empty;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::Bindings() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::init(const DataModel::ConfigModule *cfg, const std::string &setupName,
                    bool allowGlobal) {
	_bindings.clear();

	if ( cfg == NULL )
		return false;

	for ( size_t i = 0; i < cfg->configStationCount(); ++i ) {
		DataModel::ConfigStation *sta_cfg = cfg->configStation(i);
		if ( !cfg->enabled() ) continue;

		DataModel::Setup *setup = DataModel::findSetup(sta_cfg, setupName, allowGlobal);
		if ( (setup == NULL) || !setup->enabled() ) continue;

		DataModel::ParameterSet* ps = DataModel::ParameterSet::Find(setup->parameterSetID());
		if ( ps == NULL ) {
			/*
			SEISCOMP_WARNING("%s.%s: parameter set '%s' not found",
			                 sta_cfg->networkCode().c_str(),
			                 sta_cfg->stationCode().c_str(),
			                 setup->parameterSetID().c_str());
			*/
			continue;
		}

		KeyValuesPtr params = new KeyValues;
#if SC_API_VERSION < 0x012000
		params->readFrom(ps);
#else
		params->init(ps);
#endif

		_bindings[sta_cfg->networkCode()][sta_cfg->stationCode()].keys = params;
	}

	return !_bindings.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const KeyValues *Bindings::getKeys(const std::string &networkCode,
                                   const std::string &stationCode) const {
	NetworkMap::const_iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return NULL;

	StationMap::const_iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return NULL;

	return it2->second.keys.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const KeyValues *Bindings::getKeys(const DataModel::Station *station) const {
	if ( station->network() == NULL )
		return NULL;

	return getKeys(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::remove(const std::string &networkCode,
                      const std::string &stationCode) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return false;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return false;

	it->second.erase(it2);
	if ( it->second.empty() )
		_bindings.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::remove(const DataModel::Station *station) {
	if ( station->network() == NULL )
		return false;

	return remove(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::setData(const std::string &networkCode,
                       const std::string &stationCode,
                       const Core::MetaValue &value) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return false;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return false;

	it2->second.data = value;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Bindings::setData(const DataModel::Station *station,
                       const Core::MetaValue &value) {
	if ( station->network() == NULL )
		return false;

	return setData(station->network()->code(), station->code(), value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::MetaValue &Bindings::data(const std::string &networkCode,
                                      const std::string &stationCode) {
	NetworkMap::iterator it = _bindings.find(networkCode);
	if ( it == _bindings.end() )
		return empty;

	StationMap::iterator it2 = it->second.find(stationCode);
	if ( it2 == it->second.end() )
		return empty;

	return it2->second.data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Core::MetaValue &Bindings::data(const DataModel::Station *station) {
	if ( station->network() == NULL )
		return empty;

	return data(station->network()->code(), station->code());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::const_iterator Bindings::begin() const {
	const_iterator it;
	it._networks = &_bindings;
	it._stations = NULL;
	it._nIt = _bindings.begin();

	if ( it._nIt != _bindings.end() ) {
		it._stations = &it._nIt->second;
		it._sIt = it._nIt->second.begin();
	}
	else
		it = end();

	return it;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Bindings::const_iterator Bindings::end() const {
	return const_iterator();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
