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
		virtual bool connect(const char *con);
		virtual void disconnect();

		virtual bool isConnected() const;

		virtual void start();
		virtual void commit();
		virtual void rollback();

		virtual bool execute(const char* command);
		virtual bool beginQuery(const char* query);
		virtual void endQuery();

		virtual const char* defaultValue() const;
		virtual OID lastInsertId(const char*);
		virtual uint64_t numberOfAffectedRows();

		virtual bool fetchRow();
		virtual int findColumn(const char* name);
		virtual int getRowFieldCount() const;
		virtual const char *getRowFieldName(int index);
		virtual const void* getRowField(int index);
		virtual size_t getRowFieldSize(int index);
		virtual bool escape(std::string &out, const std::string &in);


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
