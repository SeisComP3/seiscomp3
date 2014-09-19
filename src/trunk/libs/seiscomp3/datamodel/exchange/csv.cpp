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

#include "csv.h"

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace DataModel {

REGISTER_EXPORTER_INTERFACE(ExporterCSV, "csv");

Origin *findOrigin(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->originCount(); ++i )
		if ( ep->origin(i)->publicID() == id )
			return ep->origin(i);
	return NULL;
}

Magnitude *findMagnitude(Origin *o, const std::string& id) {
	for ( size_t i = 0; i < o->magnitudeCount(); ++i )
		if ( o->magnitude(i)->publicID() == id )
			return o->magnitude(i);
	return NULL;
}

ExporterCSV::ExporterCSV() {
	_delim = ",";
}

void ExporterCSV::setDelimiter(std::string &delim) {
	_delim = delim;
}

const std::string& ExporterCSV::getDelimiter() const {
	return _delim;
}

bool ExporterCSV::put(std::streambuf* buf, Core::BaseObject* obj) {
	if ( buf == NULL ) return false;
	if ( obj == NULL ) return false;
	EventParameters* ep = EventParameters::Cast(obj);
	if ( ep == NULL ) return false;

	std::ostream os(buf);

	// prettyPrint flag enables header output
	if ( _prettyPrint )
		os << "eventID" << _delim << "originTime(UTC)" << _delim << "latitude"
		   << _delim << "longitude" << _delim << "depth" << _delim
		   << "magnitude" << _delim << "description" << std::endl;

	for ( size_t i = 0; i < ep->eventCount(); ++i ) {
		Event* e = ep->event(i);
		os << e->publicID() << _delim;

		Origin* o = findOrigin(ep, e->preferredOriginID());
		if ( !o ) {
			os << _delim << _delim << _delim << _delim;
		}
		else {
			os << o->time().value().iso() << _delim
			   << o->latitude().value() << _delim
			   << o->longitude().value() << _delim;
			try {
				os << o->depth().value();
			} catch (Core::ValueException&) {}
			os << _delim;
			Magnitude* m = findMagnitude(o, e->preferredMagnitudeID());
			if ( m )
				os << m->magnitude().value();
		}
		os << _delim;

		for ( size_t j = 0; j < e->eventDescriptionCount(); ++j ) {
			EventDescription* desc = e->eventDescription(j);
			if ( desc->type() == REGION_NAME ) {
				os << "\"" << desc->text() << "\"";
				break;
			}
		}

		os << std::endl;
	}
	return true;
}

} // namespace DataModel
} // namesapce Seiscomp
