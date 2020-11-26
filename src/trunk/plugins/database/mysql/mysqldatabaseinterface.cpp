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


#define SEISCOMP_COMPONENT MYSQL
#include "mysqldatabaseinterface.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/plugin.h>
#include <seiscomp3/core/system.h>
#include <string.h>
#if defined(WIN32)
#include <errmsg.h>
#else
#include <mysql/errmsg.h>
#endif

#if LIBMYSQL_VERSION_ID >= 80000
typedef bool my_bool;
#endif


namespace Seiscomp {
namespace Database {


IMPLEMENT_SC_CLASS_DERIVED(MySQLDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "mysql_database_interface");

REGISTER_DB_INTERFACE(MySQLDatabase, "mysql");
ADD_SC_PLUGIN("MySQL database driver", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 9, 2)


MySQLDatabase::MySQLDatabase()
	: _handle(NULL), _result(NULL), _row(NULL), _debug(false)
	, _fieldCount(0), _lengths(NULL)  {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MySQLDatabase::~MySQLDatabase() {
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::handleURIParameter(const std::string &name,
                                       const std::string &value) {
	if ( !DatabaseInterface::handleURIParameter(name, value) ) return false;

	if ( name == "debug" ) {
		if ( value != "0" && value != "false" )
			_debug = true;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::open() {
	_handle = mysql_init(NULL);
	if ( _handle == NULL )
		return false;

	my_bool reconnectFlag = true;
	mysql_options(_handle, MYSQL_OPT_RECONNECT, (const char*)&reconnectFlag);

	if ( _timeout > 0 ) {
		SEISCOMP_INFO("Apply database read timeout of %d seconds", _timeout);
		mysql_options(_handle, MYSQL_OPT_READ_TIMEOUT, (const char*)&_timeout);
	}
	if ( _host == "localhost" && _port != 3306 ) {
		SEISCOMP_WARNING("You are trying to open a MySQL TCP connection on a "
		                 "non standard port using the host string 'localhost'. "
		                 "The port might be ignored in favor of a Unix socket "
		                 "or shared memory connection. Use 127.0.0.1 or a host "
		                 "name other than 'localhost' to force the creation of "
		                 "a TCP connection.");
	}
	if ( mysql_real_connect(_handle, _host.c_str(), _user.c_str(), _password.c_str(),
	                        _database.c_str(), _port, NULL, 0) == NULL ) {
		SEISCOMP_ERROR("Connect to %s:******@%s:%d/%s failed", _user.c_str(),
		               _host.c_str(), _port, _database.c_str());
		mysql_close(_handle);
		_handle = NULL;
		return false;
	}

	SEISCOMP_DEBUG("Connected to %s:******@%s:%d/%s (%s)", _user.c_str(),
	               _host.c_str(), _port, _database.c_str(),
	               _handle->host_info);

	// Regarding some newsgroup results it is better to set the option AFTER
	// the connection has been established even though the documentation says
	// to do it BEFORE connecting
	mysql_options(_handle, MYSQL_OPT_RECONNECT, (const char*)&reconnectFlag);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::connect(const char *con) {
	_host = "localhost";
	_user = "sysop";
	_password = "sysop";
	_database = "seiscomp3";
	_port = 3306;
	_columnPrefix = "";
	return DatabaseInterface::connect(con);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::disconnect() {
	if ( _handle ) {
		SEISCOMP_INFO("Disconnecting from database");
		if ( _result ) {
			mysql_free_result(_result);
			_result = NULL;
		}
		mysql_close(_handle);
		_handle = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::isConnected() const {
	if ( _handle == NULL ) return false;
	int err = mysql_errno(_handle);
	if ( err < CR_UNKNOWN_ERROR ) return true;

	SEISCOMP_ERROR("connection error %d (%s) -> ping", err, mysql_error(_handle));
	return ping();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::ping() const {
	if ( !mysql_ping(_handle) ) return true;

	SEISCOMP_ERROR("ping() = %d (%s)", mysql_errno(_handle), mysql_error(_handle));
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::reconnect() {
	while ( _handle && !ping() ) {
		if ( _result ) {
			mysql_free_result(_result);
			_result = NULL;
		}
		_row = NULL;
		Seiscomp::Core::msleep(500);
	}

	if ( _handle ) {
		SEISCOMP_INFO("Database connection reestablished");
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::start() {
	execute("start transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::commit() {
	execute("commit");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::rollback() {
	execute("rollback");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::query(const char *c, const char *comp) {
	// No connection yet established or disconnect has been called
	if ( _handle == NULL || c == NULL ) return false;

	unsigned int err;
	const char *err_msg;
	bool firstTry = true;

	do {
		if ( _debug )
			SEISCOMP_DEBUG("[mysql-%s] %s", comp, c);

		int result = mysql_query(_handle, c);
		if ( result ) {
			err = mysql_errno(_handle);
			err_msg = mysql_error(_handle);
			// Client connection error?
			if ( err >= CR_UNKNOWN_ERROR ) {
				/* No automatic reconnect yet
				if ( isReconnectEnabled() ) {
					if ( !reconnect() ) break;
				}
				else*/ if ( firstTry ) {
					firstTry = false;
					if ( !_handle || !ping() ) break;
				}
				else
					break;
			}
			// Break when a query based error occured
			else {
				break;
			}
		}
		else {
			err = 0;
			err_msg = NULL;
			break;
		}
	}
	while ( true );

	if ( err > 0 ) {
		SEISCOMP_ERROR("%s(\"%s\") = %d (%s)", comp, c, err, err_msg?err_msg:"unknown");
		return false;
	}
	else if ( _debug )
		SEISCOMP_DEBUG("[mysql-%s] OK", comp);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::execute(const char* command) {
	return query(command, "execute");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::beginQuery(const char* q) {
	if ( _result ) {
		SEISCOMP_ERROR("beginQuery: nested queries are not supported");
		//SEISCOMP_DEBUG("last successfull query: %s", _lastQuery.c_str());
		return false;
	}

	if ( !query(q, "query") ) return false;

	_result = mysql_use_result(_handle);
	//_result = mysql_store_result(_handle);

	if ( _result == NULL )
		return false;

	_fieldCount = (int)mysql_field_count(_handle);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MySQLDatabase::endQuery() {
	if ( _result ) {
		mysql_free_result(_result);
		_result = NULL;
		_lengths = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface::OID MySQLDatabase::lastInsertId(const char*) {
	my_ulonglong id = mysql_insert_id(_handle);
	return id == 0 ? IO::DatabaseInterface::INVALID_OID : id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t MySQLDatabase::numberOfAffectedRows() {
	my_ulonglong r = mysql_affected_rows(_handle);
	if ( r != (my_ulonglong)~0 )
		return r;

	return (uint64_t)~0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::fetchRow() {
	_row = mysql_fetch_row(_result);
	_lengths = mysql_fetch_lengths(_result);
	return _row != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MySQLDatabase::findColumn(const char* name) {
	MYSQL_FIELD* field;
	for ( int i = 0; i < _fieldCount; ++i ) {
		field = mysql_fetch_field_direct(_result, i);
		if ( !strcmp(field->name, name) )
			return i;
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MySQLDatabase::getRowFieldCount() const {
	return _fieldCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *MySQLDatabase::getRowFieldName(int index) {
	MYSQL_FIELD* field = mysql_fetch_field_direct(_result, index);
	return field != NULL ? field->name : NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* MySQLDatabase::getRowField(int index) {
	return _row[index];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MySQLDatabase::getRowFieldSize(int index) {
	return _lengths[index];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MySQLDatabase::escape(std::string &out, const std::string &in) {
	if ( !_handle ) return false;
	out.resize(in.size()*2);
	size_t l = mysql_real_escape_string(_handle, &out[0], in.c_str(), in.size());
	out[l] = '\0';
	out.resize(l);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
