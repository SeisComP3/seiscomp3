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


#include <fstream>
#include <iostream>

#include <seiscomp3/core/strings.h>
#include "patterntable.h"


using namespace std;


namespace Seiscomp {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PatternTable::PatternTable() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PatternTable::read(const char *filename) {
	ifstream ifs;
	ifs.open(filename);
	if ( !ifs.is_open() ) return false;

	string line;
	while ( getline(ifs, line) ) {
		Core::trim(line);
		if ( line.empty() ) continue;
		if ( line[0] == '#' ) continue;

		vector<string> cols;
		Core::split(cols, line.c_str(), " ");
		if ( cols.size() == 4 )
			_rows.push_back(Row(Key(cols[0], cols[1]), Value(cols[2], cols[3])));
		else if ( cols.size() == 3 )
			_rows.push_back(Row(Key(cols[0], cols[1]), Value(cols[2], "")));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PatternTable::Results PatternTable::matches(const Key &key) const {
	Results res;
	Rows::const_iterator it;

	for ( it = _rows.begin(); it != _rows.end(); ++it ) {
		if ( Core::wildcmp(it->first.first, key.first) &&
		     Core::wildcmp(it->first.second, key.second) )
			res.push_back(it->second);
	}

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
