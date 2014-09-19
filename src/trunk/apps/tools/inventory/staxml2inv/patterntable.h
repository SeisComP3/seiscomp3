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
#include <list>
#include <string>


namespace Seiscomp {


class PatternTable {
	public:
		typedef std::pair<std::string, std::string> Key;
		typedef std::pair<std::string, std::string> Value;
		typedef std::pair<Key, Value>               Row;
		typedef std::list<Row>                      Rows;
		typedef std::list<Value>                    Results;

		PatternTable();


	public:
		bool read(const char *filename);
		Results matches(const Key &key) const;


	private:
		Rows _rows;
};


}
