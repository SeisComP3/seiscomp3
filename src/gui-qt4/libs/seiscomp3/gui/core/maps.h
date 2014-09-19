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

#ifndef __SEISCOMP_GUI_MAPS_H__
#define __SEISCOMP_GUI_MAPS_H__


#include <seiscomp3/gui/qt4.h>
#include <QString>


namespace Seiscomp {
namespace Gui {


#define TILESTORE_VERSION 1


struct SC_GUI_API MapsDesc {
	QString location;
	QString type;
	bool    isMercatorProjected;
	size_t  cacheSize;
};



}
}


#endif
