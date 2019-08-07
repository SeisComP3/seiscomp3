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


#ifndef __SEISOMP_SERVICES_DATABASE_POSTGRESQL_INTERFACE_H__
#define __SEISOMP_SERVICES_DATABASE_POSTGRESQL_INTERFACE_H__

#include <seiscomp3/io/database.h>
#include <libpq-fe.h>


namespace Seiscomp {
namespace Database {


class PostgreSQLDatabase : public Seiscomp::IO::DatabaseInterface {
	DECLARE_SC_CLASS(PostgreSQLDatabase);

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		PostgreSQLDatabase();
		~PostgreSQLDatabase();


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

		virtual unsigned long lastInsertId(const char* table);
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
		bool handleURIParameter(const std::string &name,
		                        const std::string &value);

		bool open();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		PGconn        *_handle;
		PGresult      *_result;
		bool           _debug;
		int            _row;
		int            _nRows;
		int            _fieldCount;
		void          *_unescapeBuffer;
		size_t         _unescapeBufferSize;
};


}
}


#endif
