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


#define SEISCOMP_COMPONENT HYP71Sum2k


#include "hyp71sum2k.h"

#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>


using namespace Seiscomp::Core;


namespace Seiscomp {
namespace DataModel {


REGISTER_EXPORTER_INTERFACE(ExporterHYP71SUM2K, "hyp71sum2k");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExporterHYP71SUM2K::ExporterHYP71SUM2K() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterHYP71SUM2K::put(std::streambuf* buf, BaseObject* obj) {
	if ( buf == NULL ) return false;
	if ( obj == NULL ) return false;
	EventParameters* ep = EventParameters::Cast(obj);
	if ( ep == NULL ) return false;

	std::ostream os(buf);
	Origin *o = NULL;
	Magnitude *m = NULL;

	// prettyPrint flag enables header output
	if ( _prettyPrint )
		os << "#   DATE ORIGIN     LAT N     LONG W   DEPTH    MAG NO GAP DMIN  RMS  ERH  ERZ QM" << std::endl;

	for ( size_t i = 0; i < ep->eventCount(); ++i ) {
		Event* e = ep->event(i);

		for ( size_t i = 0; i < ep->originCount(); ++i )
			if ( ep->origin(i)->publicID() == e->preferredOriginID() )
				o = ep->origin(i);

		for ( size_t i = 0; i < o->magnitudeCount(); ++i )
			if ( o->magnitude(i)->publicID() == e->preferredMagnitudeID() )
				m = o->magnitude(i);

		std::string date = o->time().value().toString("%Y%m%d");
		std::string hour = o->time().value().toString("%H%M");
		std::string sec = o->time().value().toString(" %S.%2f");

		std::string latd, latm;
		latd = stringify("%3d", abs(o->latitude().value()) );
		latm = stringify("%#05.2f", fabs(60 * (o->latitude().value() - (int)o->latitude().value())) );

		std::string lond, lonm;
		lond = stringify("%4d", abs(o->longitude().value()) );
		lonm = stringify("%#05.2f", fabs(60 * (o->longitude().value() - (int)o->longitude().value())) );

		std::string depth;
		try {
			depth = stringify("%#7.2f", o->depth().value());
		} catch ( ... ) {
			depth = " ******";
		}

		std::string magnitude;
		if ( m )
		    try {
			    magnitude = stringify("%#5.2f", m->magnitude().value());
		    } catch ( ... ) {
		    	magnitude = " ****";
		    }

		std::string Narrivals;
		try {
			Narrivals = stringify("%3d", o->arrivalCount());
		} catch ( ... ) {
			Narrivals = " **";
		}

		std::string azigap;
		try {
			azigap = stringify("%#4.0f", o->quality().azimuthalGap());
		} catch ( ... ) {
			azigap = " ***";
		}

		std::string dMin;
		try {
			dMin = stringify("%#5.1f", Seiscomp::Math::Geo::deg2km(o->quality().minimumDistance()));
		} catch ( ... ) {
			dMin = " ****";
		}

		// Teleseismic fix: ensure nothing comes and perturb the 5chars rule
		if ( dMin.length() > 5 )
		    dMin = " ****";

		std::string rms;
		try {
			rms = stringify("%#5.2f", o->quality().standardError());
		} catch ( ... ) {
			rms = " ****";
		}

		// Teleseismic fix: ensure nothing comes and perturb the 5 chars rule
		if ( rms.length() > 5 )
		    rms = " ****";

		std::string erh;
		try {
			erh = stringify("%#5.1f", o->latitude().uncertainty());
		} catch ( ... ) {
			erh = " ****";
		}

		std::string erz;
		try {
			erz = stringify("%#5.1f", o->depth().uncertainty());
		} catch ( ... ) {
			erz = " ****";
		}

		std::string quality;
		try {
			quality = o->quality().groundTruthLevel();
		} catch ( ... ) {
			quality = " ";
		}
		if ( quality.length() != 1 )
		    quality = " ";

		if ( !o ) {
			os << "                                                                                    "
					<< e->publicID();
		}
		else {
			// Date YYYYMMDD col 1-9
			os << date << " ";

			// Hour Minute col 10-14
			os << hour;

			// Seconds col 14-20 F6.2
			os << sec;

			// Latitude col 20-29
			if ( o->latitude().value() < 0 )
				os << latd << "S" << latm;
			else
				os << latd << " " << latm;

			// Longitude col 30-39
			if ( o->longitude().value() > 0 )
				os << lond << "E" << lonm;
			else
				os << lond << " " << lonm;

			// Depth col 39-46
			os << depth;

			// Blank col 46
			os << " ";

			// Magnitude code col 47
			if ( m ) {
				std::string magLetter = m->type().substr(1, 1);
				os << magLetter;
			}
			else {
				os << " ";
			}

			// Magnitude col 48-53
			os << magnitude;

			// Number of P & S times with weights greater than 0.1.
			// becomes here the number of phases col 53-56
			os << Narrivals;

			// Azimuthal gap col 56-60
			os << azigap;

			// Distance to nearest station col 60-65
			os << dMin;

			// RMS travel time residual col 65-70
			os << rms;

			// Horizontal error (km) col 70-75
			os << erh;

			// Vertical error (km) col 75-80
			os << erz;

			// Remark assigned by analyst (i.e. Q for quarry blast) col 80
			os << " ";

			// Quality code A-D col 81
			os << quality;

			// Most common data source (i.e. W= earthworm) col 82
			os << " ";

			// Auxiliary remark from program (i.e. “-“ for depth fixed, etc.) col 83
			os << " ";

			// Event identification number col 84-94
			os << e->publicID();

		}
		os << std::endl;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace DataModel
} // namesapce Seiscomp
