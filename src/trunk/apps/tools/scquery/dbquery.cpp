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



#include "dbquery.h"

#include <iomanip>
#include <sstream>

#define SEISCOMP_COMPONENT scquery
#include <seiscomp3/logging/log.h>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream& operator<<(std::ostream& os, const DBQuery& query)
{
	os << "Name: " << query.name() << std::endl;
	os << "Description: " << query.description() << std::endl;
	os << "Query: " << query.query() << std::endl;
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DBQuery::DBQuery(const std::string& name,
				 const std::string& description,
				 const std::string& query) :
		_name(name),
		_description(description)
{
	_stopWord = "##";
	setQuery(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DBQuery::~DBQuery()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DBQuery::setQuery(const std::string& query)
{
	_query.clear();
	std::istringstream is(query);
	std::ostringstream os;

	while (is)
	{
		std::string word;
		is >> word;
		os << word << " ";
	}
	_query = os.str();
	
	// find parameters
	size_t pos0 = 0;
	while(true)
	{
		pos0 = _query.find(_stopWord, pos0);
		if (pos0 == std::string::npos)
			break;
		pos0 += _stopWord.size();
		if (pos0 >= _query.size())
			break;
		size_t pos1 = _query.find(_stopWord.c_str(), pos0);
		if (pos1 == std::string::npos)
			break;

		// Get parameter
		std::string param = _query.substr(pos0, pos1-pos0);
		if ( find(_parameter.begin(), _parameter.end(), param) == _parameter.end() )
			_parameter.push_back(param);
		pos0 = pos1 + _stopWord.size();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& DBQuery::query() const
{
	return _query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& DBQuery::name() const
{
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& DBQuery::description() const
{
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DBQuery::hasParameter() const
{
	return _parameter.size() > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector<std::string>& DBQuery::parameter() const
{
	return _parameter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DBQuery::setParameter(const std::vector<std::string>& params)
{
	if(parameter().size() != params.size())
		return false;

	for ( size_t i = 0; i < params.size(); ++i ) {
		std::string var = _stopWord + _parameter[i] + _stopWord;
		size_t pos;
		while ( (pos = _query.find(var)) != std::string::npos )
			_query.replace(pos, var.size(), params[i]);
	}

	return true;

	int i = 0;
	while(true)
	{
		size_t idx = _query.find(_stopWord);
		if (idx == std::string::npos)
			break;

		size_t pos0 = idx + _stopWord.size();
		size_t pos1 = _query.find(_stopWord.c_str(), pos0);
		if (pos1 == std::string::npos)
			break;

		std::string param = _query.substr(pos0, pos1-pos0);
		int pi = -1;
		if ( param == _parameter[i] )
			pi = i;
		else {
			for ( size_t p = 0; p < _parameter.size(); ++p )
				if ( _parameter[p] == param ) {
					pi = p;
					break;
				}
		}

		if ( pi != -1 )
			_query.replace(idx,
			               std::string(_stopWord + _parameter[pi] + _stopWord).size(),
			               params[pi]);
		else
			return false;

		++i;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
