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


#include <seiscomp3/io/exporter.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/client/application.h>
#include <fdsnxml/xml.h>
#include <fdsnxml/fdsnstationxml.h>
#include "convert2fdsnxml.h"


using namespace Seiscomp;
using namespace Seiscomp::IO;


namespace {


class ExporterFDSNStaXML : public Exporter {
	public:
		ExporterFDSNStaXML() {}

		bool put(std::streambuf* buf, Core::BaseObject *obj) {
			DataModel::Inventory *inv = DataModel::Inventory::Cast(obj);
			if ( inv == NULL ) return false;

			FDSNXML::FDSNStationXML msg;

			if ( SCCoreApp )
				msg.setSender(SCCoreApp->agencyID());

			msg.setCreated(Core::Time::GMT());
			msg.setSource("SeisComP3");
			Convert2FDSNStaXML cnv(&msg);
			cnv.push(inv);

			FDSNXML::Exporter out;
			out.setFormattedOutput(_prettyPrint);
			out.setIndent(_indentation);

			return out.write(buf, &msg);
		}
};


REGISTER_EXPORTER_INTERFACE(ExporterFDSNStaXML, "fdsnxml");


}
