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


#ifndef __SEISCOMP_DATAMODEL_SCDM_0_51_EXCHANGE_H__
#define __SEISCOMP_DATAMODEL_SCDM_0_51_EXCHANGE_H__


#include <seiscomp3/io/xml/importer.h>
#include <seiscomp3/io/xml/exporter.h>


namespace Seiscomp {
namespace DataModel {

namespace SCDM051  {

struct GenericHandler : public IO::XML::GenericHandler {
	GenericHandler();

};

}

class ImporterSCDM051 : public IO::XML::Importer {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ImporterSCDM051();
};


class ExporterSCDM051 : public IO::XML::Exporter {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		ExporterSCDM051();
};


}
}


#endif
