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



#ifndef __DBQUERY_H__
#define __DBQUERY_H__

#include <string>
#include <vector>

#include <seiscomp3/io/database.h>


class DBQuery
{
	
	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
public:
	DBQuery(const std::string& name,
	        const std::string& description,
	        const std::string& query);
	~DBQuery();

	
	// ------------------------------------------------------------------
	// Public interface
	// ------------------------------------------------------------------
public:
	void setQuery(const std::string& query);
	const std::string& query() const;
	
	const std::string& name() const;
	const std::string& description() const;
	
	bool hasParameter() const;
	const std::vector<std::string>& parameter() const;
	bool setParameter(const std::vector<std::string>& param);
		
	
	// ------------------------------------------------------------------
	// Private data members
	// ------------------------------------------------------------------
private:
	std::string              _name;
	std::string              _description;
	std::string              _query;
	std::string              _stopWord;
	std::vector<std::string> _parameter;
};


// ------------------------------------------------------------------
// Operators
// ------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const DBQuery& query);

#endif
