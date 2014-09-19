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


#ifndef __SEISCOMP_DATAMODEL_ARCLINK_EXCHANGE_H__
#define __SEISCOMP_DATAMODEL_ARCLINK_EXCHANGE_H__


#include <seiscomp3/io/xml/importer.h>
#include <seiscomp3/io/xml/exporter.h>


namespace Seiscomp {
namespace DataModel {


class ImporterArclink : public IO::XML::Importer {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ImporterArclink();
};


class ExporterArclink : public IO::XML::Exporter {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterArclink();
};


}
}


#endif
