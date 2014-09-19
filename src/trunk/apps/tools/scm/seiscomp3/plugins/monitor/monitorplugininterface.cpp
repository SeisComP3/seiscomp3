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

#define SEISCOMP_COMPONENT ScMonitor
#include <seiscomp3/logging/log.h>

#include "monitorplugininterface.h"
#include "monitorfilter.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Applications::MonitorPluginInterface, SC_MPLUGIN_API);


namespace Seiscomp {
namespace Applications {

IMPLEMENT_SC_ABSTRACT_CLASS(MonitorPluginInterface, "MonitorPluginInterface");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool findName(ClientInfoData clientData, std::string name) {
	ClientInfoData::const_iterator it = clientData.find(Communication::CLIENTNAME_TAG);
	if ( it == clientData.end() )
		return false;
	if ( name != it->second )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MonitorPluginInterface::MonitorPluginInterface(const std::string& name)
	: _filterMeanInterval(10*60),
	  _filterMeanTimeMark(Core::Time::GMT()),
	  _name(name),
	  _operational(false),
	  _isFilteringEnabled(false),
	  _mFilterParser(NULL),
	  _filter(NULL) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MonitorPluginInterface::~MonitorPluginInterface() {
	if ( _mFilterParser ) delete _mFilterParser;
	if ( _filter ) delete _filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MonitorPluginInterface* MonitorPluginInterface::Create(const std::string& service) {
	return MonitorPluginInterfaceFactory::Create(service.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MonitorPluginInterface::init(const Config::Config &cfg) {
	setOperational(initFilter(cfg));
	return operational();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MonitorPluginInterface::initFilter(const Config::Config &cfg) {
	try {
		_filterStr = cfg.getString(_name + ".filter");
		SEISCOMP_DEBUG("Filter expression: %s", _filterStr.c_str());
		_mFilterParser = new MFilterParser;
		bs::tree_parse_info<> info = bs::ast_parse(_filterStr.c_str(), *_mFilterParser, bs::space_p);
		if ( info.full ) {
			SEISCOMP_DEBUG("Parsing filter expression succeed");
			MOperatorFactory factory;
			_filter = evalParseTree(info.trees.begin(), factory);
		}
		else
			SEISCOMP_ERROR("Parsing filter expression: %s failed at token: %c", _filterStr.c_str(), *info.stop);

		if ( !_filter ) {
			SEISCOMP_ERROR("Message Filter could not be instantiated.");
			return false;
		}
		_isFilteringEnabled = true;
	}
	catch ( Config::OptionNotFoundException& ) {
		SEISCOMP_DEBUG("Filter option for %s not set. Message filtering disabled.", _name.c_str());
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MonitorPluginInterface::operational() const {
	return _operational;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MonitorPluginInterface::isFilteringEnabled() const {
	return _isFilteringEnabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MonitorPluginInterface::setOperational(bool val) {
	_operational = val;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MonitorPluginInterface::filterString() const {
	return _filterStr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClientTable* MonitorPluginInterface::filter(const ClientTable& clientTable) {
	if ( !_filter )
		return NULL;

	_match.clear();
	for ( ClientTable::const_iterator it = clientTable.begin(); it != clientTable.end(); ++it ) {
		if ( _filter->eval(*it) )
			_match.push_back(*it);
	}
	return match();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClientTable* MonitorPluginInterface::filterMean(const ClientTable& clientTable) {
	// Delete already disconnected clients
	ClientTable::iterator fmIt = _filterMeanClientTable.begin();
	while ( fmIt != _filterMeanClientTable.end() ) {
		ClientTable::const_iterator found = std::find_if(
				clientTable.begin(),
				clientTable.end(),
				std::bind2nd(std::ptr_fun(findName), fmIt->info[Communication::CLIENTNAME_TAG])
		);
		if ( found == clientTable.end() ) {
			_filterMeanMessageCount.erase(fmIt->info[Communication::CLIENTNAME_TAG]);
			fmIt = _filterMeanClientTable.erase(fmIt);
		}
		else {
			++fmIt;
		}
	}

	// Add newly connected clients
	ClientTable::const_iterator ctIt = clientTable.begin();
	for ( ; ctIt != clientTable.end(); ++ctIt ) {
		ClientTable::iterator found = std::find_if(
				_filterMeanClientTable.begin(),
				_filterMeanClientTable.end(),
				std::bind2nd(std::ptr_fun(findName), ctIt->info.find(Communication::CLIENTNAME_TAG)->second)
		);

		if ( found != _filterMeanClientTable.end() ) {
			// Update data
			sumData<Communication::TOTAL_MEMORY_TAG>(*found, *ctIt);
			sumData<Communication::CLIENT_MEMORY_USAGE_TAG>(*found, *ctIt);
			sumData<Communication::MEMORY_USAGE_TAG>(*found, *ctIt);
			sumData<Communication::CPU_USAGE_TAG>(*found, *ctIt);
			sumData<Communication::MESSAGE_QUEUE_SIZE_TAG>(*found, *ctIt);
			sumData<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>(*found, *ctIt);
			found->info[Communication::UPTIME_TAG] = ctIt->info.find(Communication::UPTIME_TAG)->second;
			found->info[Communication::RESPONSE_TIME_TAG] = ctIt->info.find(Communication::RESPONSE_TIME_TAG)->second;

			++(_filterMeanMessageCount[ctIt->info.find(Communication::CLIENTNAME_TAG)->second]);
		}
		else {
			_filterMeanClientTable.push_back(*ctIt);
			_filterMeanMessageCount[ctIt->info.find(Communication::CLIENTNAME_TAG)->second] = 1;
		}
	}

	if ( Core::Time::GMT() - _filterMeanTimeMark >= _filterMeanInterval ) {
		_filterMeanTimeMark = Core::Time::GMT();
		// Calculate mean
		for ( ClientTable::iterator it = _filterMeanClientTable.begin();
			 it != _filterMeanClientTable.end(); ++it ) {
			size_t count = _filterMeanMessageCount[it->info[Communication::CLIENTNAME_TAG]];
			calculateMean<Communication::TOTAL_MEMORY_TAG>(*it, count);
			calculateMean<Communication::CLIENT_MEMORY_USAGE_TAG>(*it, count);
			calculateMean<Communication::MEMORY_USAGE_TAG>(*it, count);
			calculateMean<Communication::CPU_USAGE_TAG>(*it, count);
			calculateMean<Communication::MESSAGE_QUEUE_SIZE_TAG>(*it, count);
			calculateMean<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>(*it, count);
		}
		return filter(_filterMeanClientTable);
	}
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MonitorPluginInterface::setFilterMeanInterval(double interval) {
	_filterMeanInterval = interval * 60;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClientTable* MonitorPluginInterface::match() const {
	return _match.size() > 0 ? &_match : NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MonitorPluginInterface::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <Communication::EConnectionInfoTag tag>
void MonitorPluginInterface::sumData(ClientInfoData& lhs, const ClientInfoData& rhs) {
	typedef typename Communication::ConnectionInfoT<tag>::Type Type;
	ClientInfoData::const_iterator found = rhs.find(tag);
	if ( found == rhs.end() ) {
		SEISCOMP_ERROR("Incompatible data found. Tag %s could not be found in ClientInfoData",
				Communication::ConnectionInfoTag(tag).toString());
	}

	try {
		Type result = boost::lexical_cast<Type>(lhs[tag]) +
			          boost::lexical_cast<Type>(found->second);
		lhs[tag] = boost::lexical_cast<std::string>(result);
	}
	catch ( boost::bad_lexical_cast& e ) {
		SEISCOMP_ERROR("%s", e.what());
		SEISCOMP_ERROR(
				"[%s, %s@%s] Datatypes could not be converted",
				rhs.find(Communication::CLIENTNAME_TAG)->second.c_str(),
				rhs.find(Communication::PROGRAMNAME_TAG)->second.c_str(),
				rhs.find(Communication::HOSTNAME_TAG)->second.c_str()
		);
		SEISCOMP_ERROR(
				"tag = %s, lhs[tag] = %s, found->second = %s",
				Communication::ConnectionInfoTag(tag).toString(),
				lhs[tag].c_str(),
				found->second.c_str()
		);
		// throw e;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <Communication::EConnectionInfoTag tag>
void MonitorPluginInterface::calculateMean(ClientInfoData& lhs, size_t count) {
	typedef typename Communication::ConnectionInfoT<tag>::Type Type;
	try {
		Type result = boost::lexical_cast<Type>(lhs[tag]) / count;
		lhs[tag] = boost::lexical_cast<std::string>(result);
	}
	catch ( boost::bad_lexical_cast& e ) {
		SEISCOMP_ERROR("%s", e.what());
		SEISCOMP_ERROR(
				"[%s, %s@%s] Datatypes could not be converted",
				lhs.find(Communication::CLIENTNAME_TAG)->second.c_str(),
				lhs.find(Communication::PROGRAMNAME_TAG)->second.c_str(),
				lhs.find(Communication::HOSTNAME_TAG)->second.c_str()
		);
		SEISCOMP_ERROR(
				"tag = %s, lhs[tag] = %s",
				Communication::ConnectionInfoTag(tag).toString(),
				lhs[tag].c_str()
		);
		// throw e;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Applications
} // namespace Seiscomp
