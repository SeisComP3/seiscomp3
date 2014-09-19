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


#include <seiscomp3/core/strings.h>
#include <fstream>
#include "keyfile.h"


using namespace std;


namespace Seiscomp {
namespace {


string escape(const string &str) {
	string tmp(str);
	size_t pos = tmp.find('\'');
	while ( pos != string::npos ) {
		tmp.insert(pos, "\'\"\'\"");
		pos = tmp.find('\'', pos+5);
	}

	return tmp;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Keyfile::Keyfile() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Keyfile::clear() {
	_values.clear();
	_comment.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Keyfile::set(const std::string &name, const std::string &value) {
	KeyValues::iterator it;
	for ( it = _values.begin(); it != _values.end(); ++it ) {
		if ( it->first == name ) {
			it->second = value;
			return true;
		}
	}

	_values.push_back(KeyValue(name,value));
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Keyfile::get(std::string &value, const std::string &name) const {
	for ( iterator it = begin(); it != end(); ++it ) {
		if ( it->first == name ) {
			value = it->second;
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Keyfile::setComment(const std::string &comment) {
	_comment = comment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Keyfile::read(const char *filename) {
	ifstream ifs;
	ifs.open(filename);
	if ( !ifs.is_open() ) return false;

	string line;
	while ( getline(ifs, line) ) {
		Core::trim(line);
		if ( line.empty() ) continue;
		if ( line[0] == '#' ) continue;

		size_t sep = line.find('=');
		if ( sep == string::npos ) continue;

		string key = line.substr(0, sep);
		Core::trim(key);

		string value = line.substr(sep+1);
		Core::trim(value);

		if ( value.size() >= 2 ) {
			if ( *value.begin() == '\'' ) value.erase(value.begin());
			if ( *value.rbegin() == '\'' ) value.resize(value.size()-1);
		}

		set(key, value);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Keyfile::write(const char *filename) const {
	ofstream ofs;
	ofs.open(filename, ios::trunc);
	if ( !ofs.is_open() ) return false;

	size_t cmtStart = 0;
	size_t nlPos = _comment.find('\n');
	while ( nlPos != string::npos ) {
		ofs << "# " << _comment.substr(cmtStart, nlPos-cmtStart) << endl;
		cmtStart = _comment.find_first_not_of('\n', nlPos);
		nlPos = _comment.find('\n', cmtStart+1);
	}

	if ( cmtStart < _comment.size() )
		ofs << "# " << _comment.substr(cmtStart) << endl;

	iterator it;
	for ( it = begin(); it != end(); ++it )
		ofs << it->first << "='" << escape(it->second) << "'" << endl;

	ofs.close();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
