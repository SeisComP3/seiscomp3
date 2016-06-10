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


#define SEISCOMP_COMPONENT Database
#include <seiscomp3/io/database.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/interfacefactory.ipp>
#include <seiscomp3/logging/log.h>

#include <string.h>

IMPLEMENT_INTERFACE_FACTORY(Seiscomp::IO::DatabaseInterface, SC_SYSTEM_CORE_API);

namespace Seiscomp {
namespace IO {


using namespace std;


IMPLEMENT_SC_ABSTRACT_CLASS(DatabaseInterface, "DatabaseInterface");


DatabaseInterface::DatabaseInterface() : _timeout(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface::~DatabaseInterface() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface* DatabaseInterface::Create(const char* service) {
	if ( service == NULL ) return NULL;

	return DatabaseInterfaceFactory::Create(service);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseInterface* DatabaseInterface::Open(const char* uri) {
	const char* tmp;
	std::string service;
	std::string source;
	std::string type;

	tmp = strstr(uri, "://");
	if ( tmp ) {
		std::copy(uri, tmp, std::back_inserter(service));
		uri = tmp + 3;
	}
	else
		service = "mysql";

	source = uri;

	DatabaseInterface* db = Create(service.c_str());
	if ( !db ) {
		SEISCOMP_ERROR("Database driver '%s' is not supported", service.c_str());
		return NULL;
	}

	if ( !db->connect(source.c_str()) ) {
		SEISCOMP_ERROR("Connection to failed to %s://%s:****@%s/%s",
		               service.c_str(), db->_user.c_str(),
		               db->_host.c_str(), db->_database.c_str());
		delete db;
		return NULL;
	}

	return db;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseInterface::connect(const char* con) {
	if ( isConnected() )
		return false;

	_timeout = 0;
	string connection = con;
	string params;
	size_t pos = connection.find('?');
	if ( pos != string::npos ) {
		params = connection.substr(pos+1);
		connection.erase(connection.begin() + pos, connection.end());
	}

	vector<string> tokens;
	string host;
	if ( Core::split(tokens, connection.c_str(), "@") >= 2 ) {
		string login = tokens[0];
		host = tokens[1];

		Seiscomp::Core::split(tokens, login.c_str(), ":");
		if ( tokens.size() > 0 ) _user = tokens[0];
		if ( tokens.size() > 1 ) _password = tokens[1];
	}
	else
		host = tokens[0];

	size_t splitter = host.find_first_of('/');
	if ( splitter != string::npos ) {
		tokens.resize(2);
		tokens[0] = host.substr(0, splitter);
		tokens[1] = host.substr(splitter+1);
	}
	else
		tokens[0] = host;

	if ( !tokens[0].empty() )
		_host = tokens[0];

	if ( tokens.size() >= 2 ) _database = tokens[1];

	pos = _host.find(":");
	if ( pos != string::npos ) {
		Seiscomp::Core::fromString(_port, _host.substr(pos+1));
		_host.erase(pos);
	}

	Core::split(tokens, params.c_str(), "&");
	if ( !tokens.empty() ) {
		for ( size_t i = 0; i < tokens.size(); ++i ) {
			vector<string> param;
			Core::split(param, tokens[i].c_str(), "=");
			if ( !param.empty() ) {
				if ( param.size() == 1 ) {
					if ( !handleURIParameter(param[0], "") )
						return false;
				}
				else if ( param.size() == 2 ) {
					if ( !handleURIParameter(param[0], param[1]) )
						return false;
				}
			}
		}
	}

	return open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseInterface::handleURIParameter(const std::string &name,
                                           const std::string &value) {
	if ( name == "column_prefix" ) {
		if ( !value.empty() )
			_columnPrefix = value;
	}
	else if ( name == "timeout" ) {
		if ( !value.empty() ) {
			if ( !Core::fromString(_timeout, value) ) {
				SEISCOMP_ERROR("Invalid timeout parameter '%s' for database connection",
				               value.c_str());
				return false;
			}
			else
				SEISCOMP_DEBUG("Request database read timeout of %d seconds", _timeout);
		}
		else {
			SEISCOMP_ERROR("Database timeout parameter expects a value");
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* DatabaseInterface::defaultValue() const {
	return "default";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::timeToString(const Seiscomp::Core::Time& t) {
	return t.toString("%Y-%m-%d %H:%M:%S");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time DatabaseInterface::stringToTime(const char* str) {
	Seiscomp::Core::Time t;
	if ( str == NULL ) return t;
	t.fromString(str, "%F %T");
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& DatabaseInterface::columnPrefix() const {
	return _columnPrefix;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::convertColumnName(const std::string& name) const {
	return _columnPrefix + name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseInterface::getRowFieldString(int index) {
	const void *data = getRowField(index);
	if ( !data ) return "";
	return (const char*)data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
