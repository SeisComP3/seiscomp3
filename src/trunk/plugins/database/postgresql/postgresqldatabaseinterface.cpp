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


#define SEISCOMP_COMPONENT POSTGRESQL
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/plugin.h>
#include "postgresqldatabaseinterface.h"

#include <stdlib.h>
#include <iostream>


namespace Seiscomp {
namespace Database {


IMPLEMENT_SC_CLASS_DERIVED(PostgreSQLDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "postgresql_database_interface");

REGISTER_DB_INTERFACE(PostgreSQLDatabase, "postgresql");
ADD_SC_PLUGIN("PostgreSQL database driver", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 11, 0)


#define XFREE(ptr) \
	do {\
		if ( ptr ) {\
			PQfreemem(ptr);\
			ptr = NULL;\
			ptr##Size = 0;\
		}\
	} while (0)


PostgreSQLDatabase::PostgreSQLDatabase()
: _handle(NULL)
, _result(NULL)
, _debug(false)
, _unescapeBuffer(NULL)
, _unescapeBufferSize(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PostgreSQLDatabase::~PostgreSQLDatabase() {
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::handleURIParameter(const std::string &name,
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
bool PostgreSQLDatabase::open() {
	std::stringstream ss;
	if ( _port )
		ss << _port;

	_handle = PQsetdbLogin(_host.c_str(), ss.str().c_str(),
	                       NULL,
	                       NULL,
	                       _database.c_str(),
	                       _user.c_str(),
	                       _password.c_str());

	/* Check to see that the backend connection was successfully made */
	if ( PQstatus(_handle) != CONNECTION_OK ) {
		SEISCOMP_ERROR("Connect to %s:******@%s:%d/%s failed: %s", _user.c_str(),
		               _host.c_str(), _port, _database.c_str(),
		               PQerrorMessage(_handle));

		disconnect();
		return false;
	}

	SEISCOMP_DEBUG("Connected to %s:******@%s:%d/%s", _user.c_str(),
	               _host.c_str(), _port, _database.c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::connect(const char *con) {
	_host = "localhost";
	_user = "sysop";
	_password = "sysop";
	_database = "seiscomp3";
	_port = 0;
	_columnPrefix = "m_";
	return DatabaseInterface::connect(con);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::disconnect() {
	if ( _result ) {
		PQclear(_result);
		_result = NULL;
	}

	PQfinish(_handle);
	_handle = NULL;

	XFREE(_unescapeBuffer);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::isConnected() const {
	if ( _handle == NULL ) return false;
	ConnStatusType stat = PQstatus(_handle);
	if ( stat == CONNECTION_OK ) return true;

	SEISCOMP_ERROR("connection bad (%d) -> reconnect", stat);
	PQreset(_handle);
	return PQstatus(_handle) == CONNECTION_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::start() {
	execute("start transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::commit() {
	execute("commit");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::rollback() {
	execute("rollback");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::execute(const char* command) {
	if ( !isConnected() || command == NULL ) return false;

	if ( _debug )
		SEISCOMP_DEBUG("[postgresql-execute] %s", command);

	PGresult *result = PQexec(_handle, command);
	if ( result == NULL ) {
		SEISCOMP_ERROR("execute(\"%s\"): %s", command, PQerrorMessage(_handle));
		return false;
	}

	ExecStatusType stat = PQresultStatus(result);
	if ( stat != PGRES_TUPLES_OK && stat != PGRES_COMMAND_OK ) {
		SEISCOMP_ERROR("QUERY/COMMAND failed");
		SEISCOMP_ERROR("  %s", command);
		SEISCOMP_ERROR("  %s", PQerrorMessage(_handle));
		PQclear(result);
		return false;
	}

	PQclear(result);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::beginQuery(const char* query) {
	if ( !isConnected() || query == NULL ) return false;
	if ( _result ) {
		SEISCOMP_ERROR("beginQuery: nested queries are not supported");
		//SEISCOMP_DEBUG("last successfull query: %s", _lastQuery.c_str());
		return false;
	}

	endQuery();

	if ( _debug )
		SEISCOMP_DEBUG("[postgresql-query] %s", query);

	_result = PQexec(_handle, query);
	if ( _result == NULL ) {
		SEISCOMP_ERROR("query(\"%s\"): %s", query, PQerrorMessage(_handle));
		return false;
	}

	ExecStatusType stat = PQresultStatus(_result);
	if ( stat != PGRES_TUPLES_OK && stat != PGRES_COMMAND_OK ) {
		SEISCOMP_ERROR("QUERY/COMMAND failed");
		SEISCOMP_ERROR("  %s", query);
		SEISCOMP_ERROR("  %s", PQerrorMessage(_handle));
		PQclear(_result);
		_result = NULL;
		return false;
	}

	_nRows = PQntuples(_result);
	_fieldCount = PQnfields(_result);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PostgreSQLDatabase::endQuery() {
	_row = -1;
	_nRows = -1;
	if ( _result ) {
		PQclear(_result);
		_result = NULL;
		XFREE(_unescapeBuffer);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface::OID PostgreSQLDatabase::lastInsertId(const char* table) {
	if ( !beginQuery((std::string("select currval('") + table + "_seq')").c_str()) )
		return 0;

	char* value = PQgetvalue(_result, 0, 0);

	endQuery();

	return OID(value?atoll(value):IO::DatabaseInterface::INVALID_OID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
uint64_t PostgreSQLDatabase::numberOfAffectedRows() {
	char *number = PQcmdTuples(_result);
	if ( number == NULL || *number == '\0' )
		return (uint64_t)~0;

	uint64_t count;
	if ( sscanf(number, "%lud", &count) == 1 )
		return count;

	return (uint64_t)~0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::fetchRow() {
	XFREE(_unescapeBuffer);

	++_row;

	if ( _row < _nRows ) return true;

	_row = _nRows;
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PostgreSQLDatabase::findColumn(const char* name) {
	return PQfnumber(_result, name);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PostgreSQLDatabase::getRowFieldCount() const {
	return PQnfields(_result);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *PostgreSQLDatabase::getRowFieldName(int index) {
	return PQfname(_result, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* PostgreSQLDatabase::getRowField(int index) {
	const void *value;

	if ( PQgetisnull(_result, _row, index) )
		return NULL;

	value = PQgetvalue(_result, _row, index);

	if ( PQftype(_result, index) == 17 ) {
		// bytea
		XFREE(_unescapeBuffer);

		_unescapeBuffer = PQunescapeBytea(reinterpret_cast<const unsigned char *>(value), &_unescapeBufferSize);
		value = _unescapeBuffer;
	}

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t PostgreSQLDatabase::getRowFieldSize(int index) {
	if ( PQftype(_result, index) == 17 ) {
		// bytea
		return _unescapeBufferSize;
	}

	return PQgetlength(_result, _row, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PostgreSQLDatabase::escape(std::string &out, const std::string &in) {
	if ( !_handle ) return false;
	int error;
	out.resize(in.size()*2);
	size_t l = PQescapeStringConn(_handle, &out[0], in.c_str(), in.size(), &error);
	out[l] = '\0';
	out.resize(l);
	return !error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
