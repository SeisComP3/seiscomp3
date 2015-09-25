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


#ifndef __SEISCOMP_UTILS_KEYVALUES_H__
#define __SEISCOMP_UTILS_KEYVALUES_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/core.h>

#include <map>
#include <string>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(KeyValues);

/**
 * @brief The KeyValues class manages a list of key value pairs
 *
 * Key value pairs are saved as strings and convenience functions exist
 * to convert to and from integer, doubles and bools.
 */
class SC_SYSTEM_CORE_API KeyValues : public Core::BaseObject {
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
		KeyValues();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns an iterator to the first element in the container.
		 *        If the container is empty, the returned iterator will be
		 *        equal to end.
		 * @return An iterator
		 */
		iterator begin() const;

		/**
		 * @brief Returns an iterator to the element following the last
		 *        element of the container. Attempting to acces it results
		 *        in undefined behaviour.
		 * @return The iterator pointing past the last element.
		 */
		iterator end() const;

		/**
		 * @brief Sets parameter _name_ to _value_.
		 *
		 * If no parameter with _name_ exists it is created. Otherwise it is
		 * overwritten by _value_.
		 * @param name The parameters name
		 * @param value The parameters value
		 * @return Always true
		 */
		bool setString(const std::string &name, const std::string &value);

		/**
		 * @brief Sets parameter _name_ to _value_.
		 *
		 * If no parameter with _name_ exists it is created. Otherwise it is
		 * overwritten by _value_.
		 * @param name The parameters name
		 * @param value The parameters value that is being converted to string.
		 * @return Always true
		 */
		bool setInt(const std::string &name, int value);

		/**
		 * @brief Sets parameter _name_ to _value_.
		 *
		 * If no parameter with _name_ exists it is created. Otherwise it is
		 * overwritten by _value_.
		 * @param name The parameters name
		 * @param value The parameters value that is being converted to string.
		 * @return Always true
		 */
		bool setDouble(const std::string &name, double value);

		/**
		 * @brief Sets parameter _name_ to _value_.
		 *
		 * If no parameter with _name_ exists it is created. Otherwise it is
		 * overwritten by _value_.
		 * @param name The parameters name
		 * @param value The parameters value that is being converted to string.
		 * @return Always true
		 */
		bool setBool(const std::string &name, bool value);

		/**
		 * @brief Unsets the parameter _name_ and removes it from the list.
		 *
		 * Post condition: get(tmp, "_name_") == false
		 * @param name The parameters name
		 * @return True if it has been unset, false if the parameter does not exist.
		 */
		bool unset(const std::string &name);

		/**
		 * @brief Retrieves the string value of parameter _name_.
		 * @param value The output value. It is only touched if true is returned.
		 * @param name The parameters name
		 * @return False if the parameter does not exist, true otherwise
		 */
		bool getString(std::string &value, const std::string &name) const;

		/**
		 * @brief Retrieves the string value of parameter _name_.
		 * @param value The output value. It is only touched if true is returned.
		 * @param name The parameters name
		 * @return False if the parameter does not exist or if it could not be
		 *         converted into an integer, true otherwise
		 */
		bool getInt(int &value, const std::string &name) const;

		/**
		 * @brief Retrieves the string value of parameter _name_.
		 * @param value The output value. It is only touched if true is returned.
		 * @param name The parameters name
		 * @return False if the parameter does not exist or if it could not be
		 *         converted into a double, true otherwise
		 */
		bool getDouble(double &value, const std::string &name) const;

		/**
		 * @brief Retrieves the string value of parameter _name_.
		 * @param value The output value. It is only touched if true is returned.
		 * @param name The parameters name
		 * @return False if the parameter does not exist or if it could not be
		 *         converted into a bool, true otherwise
		 */
		bool getBool(bool &value, const std::string &name) const;

		/**
		 * @brief Reads all parameters from a ParameterSet recursively.
		 *
		 * Each ParameterSet can refer to a base parameter set which is being
		 * read in bottom top order. First the base parameter set is being read
		 * and then the current. As a result, parameters of the input parameter
		 * set have higher priority than the parameters of the base parameter set.
		 * @param ps The ParameterSet
		 */
		void init(DataModel::ParameterSet *ps);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		NameValueMap _nameValueMap;
};


}
}

#endif
