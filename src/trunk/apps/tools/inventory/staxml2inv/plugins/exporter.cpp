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


#include <seiscomp3/io/exporter.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/core/plugin.h>
#include <stationxml/xml.h>
#include <stationxml/stamessage.h>
#include "convert2staxml.h"


using namespace Seiscomp;
using namespace Seiscomp::IO;


ADD_SC_PLUGIN("StationXML export plugin", "gempa GmbH <info@gempa.de>", 1, 0, 0)


class ExporterStaXML : public Exporter {
	public:
		ExporterStaXML() {}

		bool put(std::streambuf* buf, Core::BaseObject *obj) {
			DataModel::Inventory *inv = DataModel::Inventory::Cast(obj);
			if ( inv == NULL ) return false;

			StationXML::StaMessage msg;

			if ( SCCoreApp )
				msg.setSender(SCCoreApp->agencyID());

			msg.setSentDate(Core::Time::GMT());
			msg.setSource("SeisComP3");
			Convert2StaXML cnv(&msg);
			cnv.push(inv);

			StationXML::Exporter out;
			out.setFormattedOutput(_prettyPrint);
			out.setIndent(_indentation);

			return out.write(buf, &msg);
		}
};


REGISTER_EXPORTER_INTERFACE(ExporterStaXML, "staxml");
