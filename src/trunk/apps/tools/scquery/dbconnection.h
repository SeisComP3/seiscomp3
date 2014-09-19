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



#ifndef __DBCONNECTION_H__
#define __DBCONNECTION_H__

#include <string>
#include <vector>

#include <seiscomp3/io/database.h>

#include "dbquery.h"


class DBConnection
{
	
	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		DBConnection(Seiscomp::IO::DatabaseInterface*);
		~DBConnection();


	// ------------------------------------------------------------------
	// Public interface
	// ------------------------------------------------------------------
	public:
		bool executeQuery(const DBQuery& query);
	
		const std::string& table() const;


	// ------------------------------------------------------------------
	// Private data members
	// ------------------------------------------------------------------
	private:
		Seiscomp::IO::DatabaseInterfacePtr     _db;
		std::vector<std::vector<std::string> > _table;
		std::string                            _tableStr;
};

#endif
