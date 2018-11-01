/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/io/importer.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/inventory.h>
#include <fdsnxml/xml.h>
#include <fdsnxml/fdsnstationxml.h>
#include "convert2sc3.h"


using namespace Seiscomp;
using namespace Seiscomp::IO;


namespace {


class ImporterFDSNStaXML : public Importer {
	public:
		ImporterFDSNStaXML() {}

		virtual Core::BaseObject *get(std::streambuf* buf) {
			FDSNXML::Importer imp;
			Core::BaseObjectPtr obj = imp.read(buf);

			// Nothing has been read
			if ( !obj ) return NULL;

			FDSNXML::FDSNStationXMLPtr msg = FDSNXML::FDSNStationXML::Cast(obj);
			// It is not a FDSNXML message
			if ( !msg ) return NULL;

			DataModel::Inventory *inv = new DataModel::Inventory;

			Convert2SC3 cnv(inv);
			cnv.push(msg.get());

			// Clean up the inventory after pushing all messages
			cnv.cleanUp();

			return inv;
		}
};


REGISTER_IMPORTER_INTERFACE(ImporterFDSNStaXML, "fdsnxml");


}
