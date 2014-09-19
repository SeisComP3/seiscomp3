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


#ifndef __SEISCOMP_PROCESSING_PARAMETERS_H__
#define __SEISCOMP_PROCESSING_PARAMETERS_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/client.h>

#include <map>
#include <string>


namespace Seiscomp {

namespace Processing  {


DEFINE_SMARTPOINTER(Parameters);

class SC_SYSTEM_CLIENT_API Parameters : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::map<std::string, std::string> NameValueMap;
		typedef NameValueMap::const_iterator       iterator;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Parameters();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Returns iterators to iterate over existing parameters.
		iterator begin() const;
		iterator end() const;

		//! Sets parameter _name_ to _value_. If no parameter with
		//! _name_ exists it is created. Otherwise it is overwritten
		//! by _value_.
		bool setString(const std::string &name, const std::string &value);
		bool setInt(const std::string &name, int value);
		bool setDouble(const std::string &name, double value);
		bool setBool(const std::string &name, bool value);

		//! Removes the parameter _name_ and causes get(tmp, "_name_")
		//! to return false.
		bool unset(const std::string &name);

		//! Retrieves the value of parameter _name_. If no parameter
		//! with this name exists it returns false otherwise true.
		//! value is only touched if a corresponding parameter exists.
		bool getString(std::string &value, const std::string &name) const;
		bool getInt(int &value, const std::string &name) const;
		bool getDouble(double &value, const std::string &name) const;
		bool getBool(bool &value, const std::string &name) const;

		void readFrom(DataModel::ParameterSet *ps);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		NameValueMap _nameValueMap;
};


}

}

#endif
