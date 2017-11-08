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


#include <sstream>


namespace Seiscomp {
namespace Core {


template <typename T>
inline std::string toString(const T& v) {
	std::ostringstream os;
	os.precision(10);
	os << v;
	return os.str();
}


template <typename T>
inline std::string toString(const std::complex<T>& v) {
	std::ostringstream os;
	os << "(" << toString(v.real()) << "," << toString(v.imag()) << ")";
	return os.str();
}

template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
std::string toString(const Enum<ENUMTYPE, END, NAMES>& value) {
	return value.toString();
}

template <typename T>
inline std::string toString(const std::vector<T>& v) {
	typename std::vector<T>::const_iterator it = v.begin();
	std::string str;
	if ( it != v.end() )
		str += toString(*it);
	else
		return "";

	++it;
	
	while ( it != v.end() ) {
		str += " ";
		str += toString(*it);
		++it;
	}

	return str;
}


template <typename T>
inline std::string toString(const ::boost::optional<T>& v) {
	if ( !v )
		return "None";

	return toString(*v);
}


template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
bool fromString(Enum<ENUMTYPE, END, NAMES>& value, const std::string& str) {
	return value.fromString(str);
}


template <typename T>
inline bool fromString(std::complex<T>& value, const std::string& str) {
	size_t s = str.find_first_not_of(' ');
	size_t e = str.find_last_not_of(' ');
	if ( s == std::string::npos || e == std::string::npos )
		return false;

	if ( str[s] != '(' || str[e] != ')' )
		return false;

	size_t delimPos = str.find(',', s+1);
	if ( delimPos == std::string::npos )
		return false;

	T realPart, imgPart;
	
	if ( !fromString(realPart, str.substr(s+1, delimPos-s-1)) ) return false;
	if ( !fromString(imgPart, str.substr(delimPos+1, e-delimPos-1)) ) return false;

	value = std::complex<T>(realPart, imgPart);

	return true;
}


template <typename T>
inline bool fromString(std::vector<T>& vec, const std::string& str) {
	std::vector<std::string> tokens;
	split(tokens, str.c_str(), " ");
	for ( int i = 0; i < (int)tokens.size(); ++i ) {
		T v;
		if ( !fromString(v, tokens[i]) )
			return false;
		vec.push_back(v);
	}

	return true;
}


template <typename T>
inline bool fromString(std::vector<std::complex<T> >& vec, const std::string& str) {
	std::vector<std::string> tokens;
	split(tokens, str.c_str(), " ");
	for ( int i = 0; i < (int)tokens.size(); ++i ) {
		std::complex<T> v;
		int count = 1;

		size_t countPos = tokens[i].find_first_not_of(' ');
		if ( countPos != std::string::npos ) {
			if ( tokens[i][countPos] != '(' ) {
				size_t bracketPos = tokens[i].find('(', countPos);
				// Invalid complex string
				if ( bracketPos == std::string::npos ) continue;
				if ( !fromString(count, tokens[i].substr(countPos, bracketPos-countPos)) )
					return false;
				tokens[i] = tokens[i].substr(bracketPos);
			}
		}
		
		if ( !fromString(v, tokens[i]) ) return false;
		for ( int i = 0; i < count; ++i )
			vec.push_back(v);
	}

	return true;
}


}
}
