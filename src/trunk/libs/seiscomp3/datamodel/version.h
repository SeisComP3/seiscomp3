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


#ifndef __SEISCOMP_DATAMODEL_VERSION_H__
#define __SEISCOMP_DATAMODEL_VERSION_H__


#define SEISCOMP_DATAMODEL_XMLNS_ROOT "http://geofon.gfz-potsdam.de/ns/seiscomp3-schema/"
#define SEISCOMP_DATAMODEL_XMLNS SEISCOMP_DATAMODEL_XMLNS_ROOT "0.10"

namespace Seiscomp {
namespace DataModel {


struct Version {
	enum {
		Major = 0,
		Minor = 10
	};
};


}
}


#endif
