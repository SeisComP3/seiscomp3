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


#define SEISCOMP_COMPONENT SQLITE3
#include "sqlitedatabaseinterface.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/plugin.h>
#include <seiscomp3/system/environment.h>

#include <cstdio>
#include <cstring>

namespace Seiscomp {
namespace Database {


IMPLEMENT_SC_CLASS_DERIVED(SQLiteDatabase,
                           Seiscomp::IO::DatabaseInterface,
                           "sqlite3_database_interface");

REGISTER_DB_INTERFACE(SQLiteDatabase, "sqlite3");
ADD_SC_PLUGIN("SQLite3 database driver", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 9, 0)

SQLiteDatabase::SQLiteDatabase()
: _handle(NULL), _stmt(NULL), _columnCount(0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SQLiteDatabase::~SQLiteDatabase() {
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::open() {
	std::string filename = Environment::Instance()->absolutePath(_host);

	FILE *fp = fopen(filename.c_str(), "rb");
	if ( fp == NULL ) {
		SEISCOMP_ERROR("databasefile '%s' not found", filename.c_str());
		return false;
	}

	fclose(fp);

	int res = sqlite3_open(filename.c_str(), &_handle);
	if ( res != SQLITE_OK ) {
		SEISCOMP_ERROR("sqlite3 open error: %d", res);
		sqlite3_close(_handle);
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::connect(const char *con) {
	_host = con;
	_columnPrefix = "";
	return open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::disconnect() {
	if ( _handle != NULL ) {
		sqlite3_close(_handle);
		_handle = NULL;
	}
	
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::isConnected() const {
	return _handle != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::start() {
	execute("begin transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::commit() {
	execute("commit transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::rollback() {
	execute("rollback transaction");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::execute(const char* command) {
	if ( !isConnected() || command == NULL ) return false;

	char* errmsg = NULL;
	int result = sqlite3_exec(_handle, command, NULL, NULL, &errmsg);
	if ( errmsg != NULL ) {
		SEISCOMP_ERROR("sqlite3 execute: %s", errmsg);
		sqlite3_free(errmsg);
	}

	return result == SQLITE_OK;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::beginQuery(const char* query) {
	if ( !isConnected() || query == NULL ) return false;
	if ( _stmt ) {
		SEISCOMP_ERROR("beginQuery: nested queries are not supported");
		return false;
	}

	const char* tail;
	int res = sqlite3_prepare(_handle, query, -1, &_stmt, &tail);
	if ( res != SQLITE_OK )
		return false;

	if ( _stmt == NULL )
		return false;

	_columnCount = sqlite3_column_count(_stmt);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SQLiteDatabase::endQuery() {
	if ( _stmt ) {
		sqlite3_finalize(_stmt);
		_stmt = NULL;
		_columnCount = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* SQLiteDatabase::defaultValue() const {
	return "null";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long SQLiteDatabase::lastInsertId(const char*) {
	return (unsigned long)sqlite3_last_insert_rowid(_handle);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SQLiteDatabase::fetchRow() {
	return sqlite3_step(_stmt) == SQLITE_ROW;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SQLiteDatabase::findColumn(const char* name) {
	for ( int i = 0; i < _columnCount; ++i )
		if ( !strcmp(sqlite3_column_name(_stmt, i), name) )
			return i;

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SQLiteDatabase::getRowFieldCount() const {
	return _columnCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const void* SQLiteDatabase::getRowField(int index) {
	return sqlite3_column_blob(_stmt, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t SQLiteDatabase::getRowFieldSize(int index) {
	return sqlite3_column_bytes(_stmt, index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
