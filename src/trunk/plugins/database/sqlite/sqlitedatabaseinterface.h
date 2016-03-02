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


#ifndef __SEISOMP_SERVICES_DATABASE_SQLITE_INTERFACE_H__
#define __SEISOMP_SERVICES_DATABASE_SQLITE_INTERFACE_H__

#include <seiscomp3/io/database.h>
#include <sqlite3.h>


namespace Seiscomp {
namespace Database {


class SQLiteDatabase : public Seiscomp::IO::DatabaseInterface {
	DECLARE_SC_CLASS(SQLiteDatabase);

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		SQLiteDatabase();
		~SQLiteDatabase();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		bool connect(const char *con);
		void disconnect();

		bool isConnected() const;

		void start();
		void commit();
		void rollback();

		bool execute(const char* command);
		bool beginQuery(const char* query);
		void endQuery();

		const char* defaultValue() const;
		unsigned long lastInsertId(const char*);

		bool fetchRow();
		int findColumn(const char* name);
		int getRowFieldCount() const;
		const char *getRowFieldName(int index);
		const void* getRowField(int index);
		size_t getRowFieldSize(int index);


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool open();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		sqlite3* _handle;
		sqlite3_stmt* _stmt;
		int _columnCount;
};


}
}


#endif
