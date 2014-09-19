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

#ifndef __SEISCOMP_APPLICATIONS_TYPES_H__
#define __SEISCOMP_APPLICATIONS_TYPES_H__

#include <map>
#include <list>

#include <seiscomp3/communication/connectioninfo.h>


namespace Seiscomp {
namespace Applications {

typedef std::map<Communication::ConnectionInfoTag, std::string> ClientInfoData;

struct ClientTableEntry {
	ClientTableEntry(const ClientInfoData &data)
	: info(data), processed(false), printed(false) {}

	ClientTableEntry &operator=(const ClientInfoData &data) {
		if ( info != data ) {
			info = data;
			processed = false;
			printed = false;
		}
		return *this;
	}

	operator ClientInfoData &() { return info; }
	operator const ClientInfoData &() const { return info; }

	ClientInfoData info; //! The client information
	bool processed;      //! Has this entry already been seen by a plugin?
	bool printed;        //! Has this entry already been seen by an output
	                     //! plugin?
};

typedef std::list<ClientTableEntry>                             ClientTable;

} // namespace Applications
} // namespace Seiscomp

#endif
