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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline Enum<ENUMTYPE, END, NAMES>::Enum(ENUMTYPE value)
 : _value(value) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline Enum<ENUMTYPE, END, NAMES>::operator ENUMTYPE () const {
	return _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline void Enum<ENUMTYPE, END, NAMES>::serialize(Archive& ar) {
	std::string str;
	if ( ar.isReading() ) {
		ar.read(str);
		ar.setValidity(fromString(str));
	}
	else {
		str = NAMES::name(int(_value)-int(0));
		ar.write(str);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline bool
Enum<ENUMTYPE, END, NAMES>::operator==(ENUMTYPE value) const {
	return value == _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline bool
Enum<ENUMTYPE, END, NAMES>::operator!=(ENUMTYPE value) const {
	return value != _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline const char* Enum<ENUMTYPE, END, NAMES>::toString() const {
	return NAMES::name(_value-0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline bool
Enum<ENUMTYPE, END, NAMES>::fromString(const std::string& str) {
	int index = int(0);

	while( str != std::string(NAMES::name(index-0)) ) {
		++index;
		if ( index >= int(END) )
			return false;
	}

	_value = static_cast<ENUMTYPE>(index);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline int Enum<ENUMTYPE, END, NAMES>::toInt() const {
	return _value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
inline bool
Enum<ENUMTYPE, END, NAMES>::fromInt(int v) {
	if ( v < int(0) || v >= int(END) )
		return false;

	_value = static_cast<ENUMTYPE>(v);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
