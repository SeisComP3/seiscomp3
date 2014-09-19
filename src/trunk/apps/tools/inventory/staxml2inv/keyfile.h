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


#include <map>
#include <string>


namespace Seiscomp {


class Keyfile {
	public:
		typedef std::pair<std::string, std::string> KeyValue;
		typedef std::list<KeyValue>                 KeyValues;
		typedef KeyValues::const_iterator           iterator;

		Keyfile();


	public:
		iterator begin() const { return _values.begin(); }
		iterator end() const { return _values.end(); }

		void clear();

		bool set(const std::string &name, const std::string &value);
		bool get(std::string &value, const std::string &name) const;

		void setComment(const std::string &comment);

		bool read(const char *filename);
		bool write(const char *filename) const;


	private:
		std::string _comment;
		KeyValues   _values;
};


}
