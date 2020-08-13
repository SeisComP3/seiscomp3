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



#define SEISCOMP_COMPONENT EventList
#include "eventlistview.h"
#include <seiscomp3/gui/core/connectiondialog.h>
#include <seiscomp3/gui/core/messages.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/scheme.h>
#include <seiscomp3/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp3/gui/datamodel/utils.h>
#include <seiscomp3/gui/datamodel/ui_eventfilterwidget.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/originreference.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/focalmechanismreference.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/messages.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/seismology/regions.h>

#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QTreeWidgetItem>
#include <QHeaderView>


using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;


namespace Seiscomp {
namespace Gui {


namespace {


#define CMD_MERGE_EVENT  "EvMerge"
#define CMD_GRAB_ORIGIN  "EvGrabOrg"
#define CMD_NEW_EVENT    "EvNewEvent"
#define CMD_SPLIT_ORIGIN "EvSplitOrg"


MAKEENUM(
	EventListColumns,
	EVALUES(
		COL_OTIME,
		COL_EVENTTYPE_CERTAINTY,
		COL_EVENTTYPE,
		COL_M,
		COL_MTYPE,
		COL_PHASES,
		COL_RMS,
		COL_LAT,
		COL_LON,
		COL_DEPTH,
		COL_DEPTH_TYPE,
		COL_TYPE,
		COL_FM,
		COL_AGENCY,
		COL_AUTHOR,
		COL_REGION,
		COL_ID
	),
	ENAMES(
		"OT(%1)",
		"Certainty",
		"Type",
		"M",
		"MType",
		"Phases",
		"RMS",
		"Lat",
		"Lon",
		"Depth",
		"DType",
		"Stat",
		"FM",
		"Agency",
		"Author",
		"Region",
		"ID"
	)
);


bool colVisibility[EventListColumns::Quantity] = {
	true,
	false,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true
};


class ByteArrayBuf : public std::streambuf {
	public:
		ByteArrayBuf(QByteArray &array) : _array(array) {}

	protected:
		int_type overflow (int_type c) {
			_array.append((char)c);
			return c;
		}

		std::streamsize xsputn(const char* s, std::streamsize n) {
			_array += QByteArray(s, n);
			return n;
		}

	private:
		QByteArray &_array;
};


#define _T(name) ar->driver()->convertColumnName(name)

DatabaseIterator getEvents(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;

	oss << "select PEvent." + _T("publicID") + ",Event.*"
	    << "from Origin, PublicObject as POrigin, Event, PublicObject as PEvent ";

	if ( filterMagnitude ) oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where POrigin." + _T("publicID") + "=Event." + _T("preferredOriginID") + " and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss << "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    << "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss  << "Origin._oid=POrigin._oid and Event._oid=PEvent._oid";

	return ar->getObjectIterator(oss.str(), Event::TypeInfo());
}


DatabaseIterator getEventOriginReferences(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select OriginReference.* "
	    << "from PublicObject as POrigin, Origin, "
	    << "OriginReference, Event ";

	if ( filterMagnitude )
		oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where POrigin._oid = Origin._oid and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID") << " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss <<       "OriginReference._parent_oid = Event._oid";

	return ar->getObjectIterator(oss.str(), OriginReference::TypeInfo());
}


DatabaseIterator getEventFocalMechanismReferences(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select FocalMechanismReference.* "
	    << "from PublicObject as POrigin, Origin, "
	    << "FocalMechanismReference, Event ";

	if ( filterMagnitude )
		oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where POrigin._oid = Origin._oid and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    <<  "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss <<       "Event." << _T("preferredOriginID") << " = POrigin." << _T("publicID") << " and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss <<       "FocalMechanismReference._parent_oid = Event._oid";

	return ar->getObjectIterator(oss.str(), FocalMechanismReference::TypeInfo());
}


DatabaseIterator getEventOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select POrigin." << _T("publicID") << ", Origin.* "
	    << "from PublicObject as POrigin, Origin, "
	    <<      "Event, OriginReference, "
	    <<      "PublicObject as PPrefOrigin, Origin as PrefOrigin ";

	if ( filterMagnitude )
		oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where POrigin._oid = Origin._oid and PPrefOrigin._oid = PrefOrigin._oid and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss <<       "Event." << _T("preferredOriginID") << " = PPrefOrigin." << _T("publicID") << " and "
	    <<       "PrefOrigin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "PrefOrigin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss <<       "OriginReference._parent_oid = Event._oid and "
	    <<       "OriginReference." << _T("originID") << " = POrigin." << _T("publicID");

	return ar->getObjectIterator(oss.str(), Origin::TypeInfo());
}


DatabaseIterator getEventFocalMechanisms(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select PFocalMechanism." << _T("publicID") << ", FocalMechanism.* "
	    << "from PublicObject as PFocalMechanism, FocalMechanism, "
	    <<      "Event, FocalMechanismReference, "
	    <<      "PublicObject as PPrefOrigin, Origin as PrefOrigin ";

	if ( filterMagnitude )
		oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where PFocalMechanism._oid = FocalMechanism._oid and PPrefOrigin._oid = PrefOrigin._oid and ";


	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss <<       "Event." << _T("preferredOriginID") << " = PPrefOrigin." << _T("publicID") << " and "
	    <<       "PrefOrigin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "PrefOrigin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss <<       "FocalMechanismReference._parent_oid = Event._oid and "
	    <<       "FocalMechanismReference." << _T("focalMechanismID") << " = PFocalMechanism." << _T("publicID");

	return ar->getObjectIterator(oss.str(), FocalMechanism::TypeInfo());
}


DatabaseIterator getEventMomentTensors(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select PMomentTensor." << _T("publicID") << ", MomentTensor.* "
	    << "from PublicObject as PFocalMechanism, FocalMechanism, "
	    <<      "PublicObject as PMomentTensor, MomentTensor, "
	    <<      "Event, FocalMechanismReference, "
	    <<      "PublicObject as PPrefOrigin, Origin as PrefOrigin ";

	if ( filterMagnitude )
		oss << ", PublicObject as PMagnitude,  Magnitude ";

	oss << "where PFocalMechanism._oid = FocalMechanism._oid and PMomentTensor._oid = MomentTensor._oid and "
	    <<       "PPrefOrigin._oid = PrefOrigin._oid and ";


	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	oss <<       "Event." << _T("preferredOriginID") << " = PPrefOrigin." << _T("publicID") << " and "
	    <<       "PrefOrigin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "PrefOrigin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";


	oss <<       "FocalMechanismReference._parent_oid = Event._oid and "
	    <<       "FocalMechanismReference." << _T("focalMechanismID") << " = PFocalMechanism." << _T("publicID") << " and "
	    <<       "MomentTensor._parent_oid = FocalMechanism._oid";

	return ar->getObjectIterator(oss.str(), MomentTensor::TypeInfo());
}

DatabaseIterator getUnassociatedOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if ( !ar->driver() ) return DatabaseIterator();

	std::ostringstream oss;

	oss << "select POrigin." << _T("publicID") << ", Origin.* "
	    << "from Origin, PublicObject as POrigin "
	    << "left join OriginReference on POrigin." << _T("publicID") << " = OriginReference." << _T("originID") << " "
	    << "where POrigin._oid = Origin._oid and "
	    <<       "Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
	    <<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime) << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";

	oss <<       "OriginReference." << _T("originID") << " is NULL";

	return ar->getObjectIterator(oss.str(), Origin::TypeInfo());
}


DatabaseIterator getComments4Origins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() )
		return DatabaseIterator();

	std::ostringstream oss;
	oss	<< "select Comment.* "
		<< "from Origin, "
		<<      "Comment "
		<< "where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "' and ";

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";

	oss <<       "Comment._parent_oid = Origin._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}


DatabaseIterator getComments4Events(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() )
		return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select Comment.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "Comment ";

	if ( filterMagnitude )
		oss <<  ", PublicObject as PMagnitude,  Magnitude ";

	oss	<< "where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "' and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";


	oss	<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "Comment._parent_oid = Event._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}


DatabaseIterator getComments4PrefOrigins(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() )
		return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select Comment.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "Comment ";

	if ( filterMagnitude )
		oss <<  ", PublicObject as PMagnitude,  Magnitude ";


	oss	<< "where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "' and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= '" << *filter.minLatitude << "' and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";


	oss	<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "Comment._parent_oid = Origin._oid";

	return ar->getObjectIterator( oss.str(), Comment::TypeInfo() );
}


DatabaseIterator getDescriptions4Events(DatabaseArchive *ar, const EventListView::Filter& filter) {
	if( !ar->driver() )
		return DatabaseIterator();

	bool filterMagnitude = filter.minMagnitude ||  filter.maxMagnitude;

	std::ostringstream oss;
	oss << "select EventDescription.* "
		<< "from Event, "
		<<      "Origin, "
		<<      "PublicObject as POrigin, "
		<<      "EventDescription ";

	if ( filterMagnitude )
		oss <<  ", PublicObject as PMagnitude,  Magnitude ";

	oss	<< "where Origin." << _T("time_value") << " >= '" << ar->driver()->timeToString(filter.startTime) << "' and "
		<<       "Origin." << _T("time_value") << " <= '" << ar->driver()->timeToString(filter.endTime)   << "' and ";

	if ( filterMagnitude ) {
		oss << "PMagnitude._oid = Magnitude._oid and "
		    << "Event." << _T("preferredMagnitudeID") << " = PMagnitude." << _T("publicID") << " and ";
	}

	if ( filter.minLatitude )
		oss << "Origin." << _T("latitude_value") << " >= " << *filter.minLatitude << " and ";
	if ( filter.maxLatitude )
		oss << "Origin." << _T("latitude_value") << " <= '" << *filter.maxLatitude << "' and ";
	if ( filter.minLongitude )
		oss << "Origin." << _T("longitude_value") << " >= '" << *filter.minLongitude << "' and ";
	if ( filter.maxLongitude )
		oss << "Origin." << _T("longitude_value") << " <= '" << *filter.maxLongitude << "' and ";
	if ( filter.minDepth )
		oss << "Origin." << _T("depth_value") << " >= '" << *filter.minDepth << "' and ";
	if ( filter.maxDepth )
		oss << "Origin." << _T("depth_value") << " <= '" << *filter.maxDepth << "' and ";
	if ( filter.minMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " >= '" << *filter.minMagnitude << "' and ";
	if ( filter.maxMagnitude )
		oss << "Magnitude." << _T("magnitude_value") << " <= '" << *filter.maxMagnitude << "' and ";

	oss	<<       "Origin._oid = POrigin._oid and "
		<<       "POrigin." << _T("publicID") << " = Event." << _T("preferredOriginID") << " and "
		<<       "EventDescription._parent_oid = Event._oid";

	 return ar->getObjectIterator( oss.str(), EventDescription::TypeInfo() );
}

typedef QPair<QTreeWidgetItem*,int> SortItem;
typedef bool(*LessThan)(const SortItem&,const SortItem&);

bool itemLessThan(const SortItem& left, const SortItem& right) {
	return left.first->data(left.second, Qt::UserRole).toDouble() <
	       right.first->data(right.second, Qt::UserRole).toDouble();
}

bool itemGreaterThan(const SortItem& left, const SortItem& right) {
	return left.first->data(left.second, Qt::UserRole).toDouble() >
	       right.first->data(right.second, Qt::UserRole).toDouble();
}

bool itemTextLessThan(const SortItem& left, const SortItem& right) {
	return left.first->text(left.second) < right.first->text(right.second);
}

bool itemTextGreaterThan(const SortItem& left, const SortItem& right) {
	return left.first->text(left.second) > right.first->text(right.second);
}


}


namespace Private {


enum SchemeType {
	ST_None = 0,
	ST_Event,
	ST_FocalMechanism,
	ST_Origin,
	ST_OriginGroup,
	ST_FocalMechanismGroup
};


class TreeItem : public QTreeWidgetItem {
	public:
		explicit TreeItem(int type, const EventListView::ItemConfig &cfg) : QTreeWidgetItem(type), _enabled(true), config(cfg) {}
		explicit TreeItem(QTreeWidget *view, int type, const EventListView::ItemConfig &cfg) : QTreeWidgetItem(view, type), _enabled(true), config(cfg) {}
		explicit TreeItem(QTreeWidgetItem *parent, int type, const EventListView::ItemConfig &cfg) : QTreeWidgetItem(parent, type), _enabled(true), config(cfg) {}

		virtual void setEnabled(bool e) {
			if ( _enabled == e ) return;
			_enabled = e;

			/*
			if ( e )
				setFlags(flags() | Qt::ItemIsEnabled);
			else
				setFlags(flags() & ~Qt::ItemIsEnabled);
			*/
		}

		bool isEnabled() const { return _enabled; }

		QVariant data(int column, int role) const {
			if ( !_enabled && role == Qt::TextColorRole )
				return config.disabledColor;
			return QTreeWidgetItem::data(column, role);
		}

	private:
		bool _enabled;

	protected:
		const EventListView::ItemConfig &config;
};


class SchemeTreeItem : public TreeItem {
	protected:
		SchemeTreeItem(int type, PublicObject* object, const EventListView::ItemConfig &cfg, QTreeWidgetItem * parent = 0)
		: TreeItem(parent, type, cfg), _object(object) { init(); }

	public:
		void init() {
			setTextAlignment(config.columnMap[COL_ID], Qt::AlignLeft | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_OTIME], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_TYPE], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_FM], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_PHASES], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_RMS], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_M], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_MTYPE], Qt::AlignLeft | Qt::AlignVCenter);
			//setTextAlignment(MCOUNT, Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_LAT], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_LON], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_DEPTH], Qt::AlignRight | Qt::AlignVCenter);
			setTextAlignment(config.columnMap[COL_DEPTH_TYPE], Qt::AlignCenter);
			setTextAlignment(config.columnMap[COL_REGION], Qt::AlignLeft | Qt::AlignVCenter);

			if ( config.customColumn != -1 )
				setTextAlignment(config.customColumn, Qt::AlignCenter);

			for ( int i = 0; i < config.originScriptColumns.size(); ++i )
				setTextAlignment(config.originScriptColumns[i].pos, Qt::AlignCenter);
			for ( int i = 0; i < config.eventScriptColumns.size(); ++i )
				setTextAlignment(config.eventScriptColumns[i].pos, Qt::AlignCenter);
		}

		virtual void update(EventListView*) = 0;

		PublicObject* object() const { return _object.get(); }

	private:
		PublicObjectPtr      _object;
};


class OriginTreeItem : public SchemeTreeItem {
	public:
		OriginTreeItem(Origin* origin, const EventListView::ItemConfig &config, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(ST_Origin, origin, config, parent),_published( false ) {
			update(NULL);

			QFont f = font(config.columnMap[COL_REGION]);
			f.setItalic(true);
			setFont(config.columnMap[COL_REGION], f);
		}

		~OriginTreeItem() {
			/*
			if ( origin() )
				std::cout << "removed origin " << origin()->publicID() << " from list" << std::endl;
			else
				std::cout << "removed empty origin item from list" << std::endl;
			*/
		}

		Origin* origin() const { return static_cast<Origin*>(object()); }

		void setPublishState(bool ps) {
			_published = ps;
		}

		void update(EventListView*) {
			Origin* ori = origin();
			setText(config.columnMap[COL_ID], QString("%1").arg(ori->publicID().c_str()));
			try {
				setText(config.columnMap[COL_AGENCY], ori->creationInfo().agencyID().c_str());
				setText(config.columnMap[COL_AUTHOR], ori->creationInfo().author().c_str());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_AGENCY], "");
				setText(config.columnMap[COL_AUTHOR], "");
			}
			setText(config.columnMap[COL_OTIME], timeToString(ori->time().value(), "... %T"));
			setData(config.columnMap[COL_OTIME], Qt::UserRole, QVariant((double)ori->time().value()));
			setText(config.columnMap[COL_M], "-"); // Mag
			setText(config.columnMap[COL_MTYPE], "-"); // MagType
			//setText(MCOUNT, "-"); // MagCount
			try {
				setText(config.columnMap[COL_PHASES], QString("%1").arg(ori->quality().usedPhaseCount()));
				setData(config.columnMap[COL_PHASES], Qt::UserRole, ori->quality().usedPhaseCount());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_PHASES], "-");
				setData(config.columnMap[COL_PHASES], Qt::UserRole, QVariant());
			}

			try {
				setText(config.columnMap[COL_RMS], QString("%1").arg(ori->quality().standardError(), 0, 'f', SCScheme.precision.rms));
				setData(config.columnMap[COL_RMS], Qt::UserRole, ori->quality().standardError());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_RMS], "-");
				setData(config.columnMap[COL_RMS], Qt::UserRole, QVariant());
			}

			double lat = ori->latitude();
			double lon = ori->longitude();

			setText(config.columnMap[COL_LAT], QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
			setData(config.columnMap[COL_LAT], Qt::UserRole, lat);
			setText(config.columnMap[COL_LON], QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon
			setData(config.columnMap[COL_LON], Qt::UserRole, lon);

			try {
				setText(config.columnMap[COL_DEPTH], depthToString(ori->depth(), SCScheme.precision.depth) + " km");
				setData(config.columnMap[COL_DEPTH], Qt::UserRole, ori->depth().value());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_DEPTH], "-"); // Depth
				setData(config.columnMap[COL_DEPTH], Qt::UserRole, QVariant());
			}

			try {
				setText(config.columnMap[COL_DEPTH_TYPE], ori->depthType().toString());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_DEPTH_TYPE], "-"); // Depth
			}

			char stat = objectStatusToChar(ori);
			setText(config.columnMap[COL_TYPE],
					QString("%1%2")
					.arg(_published?">":"")
					.arg(stat)
					); // Type

			try {
				switch ( ori->evaluationMode() ) {
					case DataModel::AUTOMATIC:
						setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
						break;
					case DataModel::MANUAL:
						setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
						break;
					default:
						break;
				};
			}
			catch ( ... ) {
				setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
			}

			setText(config.columnMap[COL_REGION], Regions::getRegionName(lat,lon).c_str()); // Region
			//setText(2, QString("%1").arg(origin->arrivalCount()));

			if ( config.customColumn != -1 ) {
				setText(config.customColumn, config.customDefaultText);
				setData(config.customColumn, Qt::ForegroundRole, QVariant());
				if ( !config.originCommentID.empty() ) {
					for ( size_t i = 0; i < ori->commentCount(); ++i ) {
						if ( ori->comment(i)->id() == config.originCommentID ) {
							setText(config.customColumn, ori->comment(i)->text().c_str());
							QMap<std::string, QColor>::const_iterator it =
								config.customColorMap.find(ori->comment(i)->text());
							if ( it != config.customColorMap.end() )
								setData(config.customColumn, Qt::ForegroundRole, it.value());
							break;
						}
					}
				}
			}

			setToolTip(config.columnMap[COL_OTIME], timeToString(ori->time().value(), "%F %T", true));
			setToolTip(config.columnMap[COL_ID], text(config.columnMap[COL_ID]));
			setToolTip(config.columnMap[COL_REGION], text(config.columnMap[COL_REGION])); // Region ToolTip
		}


	protected:
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			setData(config.columnMap[COL_ID], Qt::UserRole, highlight);

			/*
			QTreeWidgetItem * p = parent();
			if ( p && highlight ) {
				bool expanded = false;
				QTreeWidget* tree = treeWidget();
				if ( tree )
					expanded = tree->isItemExpanded(p);
				//SEISCOMP_DEBUG("Reposition child");
				p->insertChild(0, p->takeChild(p->indexOfChild(this)));
				if ( !expanded && tree )
					tree->collapseItem(p);
			}
			*/
		}
	private:
		bool _published;

	friend class EventTreeItem;
};


class FocalMechanismTreeItem : public SchemeTreeItem {
	public:
		FocalMechanismTreeItem(FocalMechanism* origin, const EventListView::ItemConfig &config, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(ST_FocalMechanism, origin, config, parent),_published(false) {
			update(NULL);
		}

		~FocalMechanismTreeItem() {}

		FocalMechanism* focalMechanism() const { return static_cast<FocalMechanism*>(object()); }

		void setPublishState(bool ps) {
			_published = ps;
		}

		void update(EventListView*) {
			FocalMechanism* fm = focalMechanism();
			setText(config.columnMap[COL_ID], QString("%1").arg(fm->publicID().c_str()));
			try {
				setText(config.columnMap[COL_AGENCY], fm->creationInfo().agencyID().c_str());
				setText(config.columnMap[COL_AUTHOR], fm->creationInfo().author().c_str());
			}
			catch ( ... ) {
				setText(config.columnMap[COL_AGENCY], "");
				setText(config.columnMap[COL_AUTHOR], "");
			}

			Origin *fmBaseOrg;

			if ( fm->momentTensorCount() > 0 ) {
				MomentTensor *mt = fm->momentTensor(0);
				fmBaseOrg = Origin::Find(mt->derivedOriginID());

				Magnitude *momentmag = Magnitude::Find(mt->momentMagnitudeID());
				if ( momentmag ) {
					setText(config.columnMap[COL_M], QString("%1").arg(momentmag->magnitude().value(), 0, 'f', 1));
					setData(config.columnMap[COL_M], Qt::UserRole, QVariant(momentmag->magnitude().value()));
					setText(config.columnMap[COL_MTYPE], QString("%1").arg(momentmag->type().c_str()));
				}
				else {
					setText(config.columnMap[COL_M], "-"); // Mag
					setText(config.columnMap[COL_MTYPE], "-"); // MagType
				}
			}
			else
				fmBaseOrg = Origin::Find(fm->triggeringOriginID());

			if ( fmBaseOrg ) {
				setText(config.columnMap[COL_OTIME], timeToString(fmBaseOrg->time().value(), "... %T"));
				setData(config.columnMap[COL_OTIME], Qt::UserRole, QVariant((double)fmBaseOrg->time().value()));

				try {
					setText(config.columnMap[COL_PHASES], QString("%1").arg(fmBaseOrg->quality().usedPhaseCount()));
				}
				catch ( ... ) {
					setText(config.columnMap[COL_PHASES], "-");
				}

				try {
					setText(config.columnMap[COL_RMS], QString("%1").arg(fmBaseOrg->quality().standardError()));
				}
				catch ( ... ) {
					setText(config.columnMap[COL_RMS], "-");
				}

				double lat = fmBaseOrg->latitude();
				double lon = fmBaseOrg->longitude();

				setText(config.columnMap[COL_LAT], QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
				setText(config.columnMap[COL_LON], QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon

				try {
					setText(config.columnMap[COL_DEPTH], depthToString(fmBaseOrg->depth(), SCScheme.precision.depth) + " km");
				}
				catch ( ... ) {
					setText(config.columnMap[COL_DEPTH], "-"); // Depth
				}

				try {
					setText(config.columnMap[COL_DEPTH_TYPE], fmBaseOrg->depthType().toString());
				}
				catch ( ... ) {
					setText(config.columnMap[COL_DEPTH_TYPE], "-"); // Depth
				}

				setText(config.columnMap[COL_REGION], Regions::getRegionName(lat, lon).c_str()); // Region
			}

			char stat = objectStatusToChar(fm);
			setText(config.columnMap[COL_TYPE], QString("%1").arg(stat));

			try {
				switch ( fm->evaluationMode() ) {
					case DataModel::AUTOMATIC:
						setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
						break;
					case DataModel::MANUAL:
						setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
						break;
					default:
						break;
				};
			}
			catch ( ... ) {
				setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
			}
		}


	protected:
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			setData(config.columnMap[COL_ID], Qt::UserRole, highlight);
		}
	private:
		bool _published;

	friend class EventTreeItem;
};


class EventTreeItem : public SchemeTreeItem {
	public:
		EventTreeItem(Event* event, const EventListView::ItemConfig &config, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(ST_Event, event, config, parent) {
			_showOnlyOnePerAgency = false;
			_resort = false;
			_hasMultipleAgencies = false;
			_published = false;

			setText(config.columnMap[COL_PHASES], "-");
			setText(config.columnMap[COL_RMS], "-");
			setText(config.columnMap[COL_M], "-");
			setText(config.columnMap[COL_MTYPE], "-");
			setText(config.columnMap[COL_DEPTH], "-");
			setText(config.columnMap[COL_DEPTH_TYPE], "-");

			QFont f = SCApp->font();
			f.setUnderline(true);
			setData(config.columnMap[COL_FM], Qt::FontRole, f);
			setData(config.columnMap[COL_FM], Qt::ForegroundRole, SCApp->palette().color(QPalette::Link));

			_origins = NULL;
			_focalMechanisms = NULL;

			update(NULL);
		}

		~EventTreeItem() {
			/*
			if ( event() )
				std::cout << "removed event " << event()->publicID() << " from list" << std::endl;
			else
				std::cout << "removed empty event item from list" << std::endl;
			*/
			//if ( _origins ) delete _origins;
			//if ( _focalMechanisms ) delete _focalMechanisms;
		}

		Event* event() const { return static_cast<Event*>(object()); }

		QTreeWidgetItem *origins() const { return _origins; }

		int originItemCount() const { return _origins?_origins->childCount():0; }
		QTreeWidgetItem *originItem(int i) const { return _origins?_origins->child(i):NULL; }
		QTreeWidgetItem *takeOrigin(int i) const { return _origins?_origins->takeChild(i):NULL; }

		void addOriginItem(QTreeWidgetItem *item) {
			if ( _origins == NULL ) {
				_origins = new TreeItem(this, ST_OriginGroup, config);
				_origins->setEnabled(isEnabled());
				_origins->setFlags(_origins->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_origins->setFont(0, f);
				_origins->setText(0, "Origins");
			}

			_origins->addChild(item);
		}

		void addOriginItem(int i, QTreeWidgetItem *item) {
			if ( _origins == NULL ) {
				_origins = new TreeItem(this, ST_OriginGroup, config);
				_origins->setEnabled(isEnabled());
				_origins->setFlags(_origins->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_origins->setFont(0, f);
				_origins->setText(0, "Origins");
			}

			_origins->insertChild(i, item);
		}

		QTreeWidgetItem *focalMechanisms() const { return _focalMechanisms; }

		int focalMechanismItemCount() const { return _focalMechanisms?_focalMechanisms->childCount():0; }
		QTreeWidgetItem *focalMechanismItem(int i) const { return _focalMechanisms?_focalMechanisms->child(i):NULL; }
		QTreeWidgetItem *takeFocalMechanism(int i) {
			if ( !_focalMechanisms ) return NULL;
			QTreeWidgetItem *item = _focalMechanisms->takeChild(i);
			if ( _focalMechanisms->childCount() == 0 ) {
				delete _focalMechanisms;
				_focalMechanisms = NULL;
			}
			return item;
		}

		void addFocalMechanismItem(QTreeWidgetItem *item) {
			if ( _focalMechanisms == NULL ) {
				_focalMechanisms = new TreeItem(this, ST_FocalMechanismGroup, config);
				_focalMechanisms->setEnabled(isEnabled());
				_focalMechanisms->setFlags(_focalMechanisms->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _origins->font(0);
				f.setItalic(true);
				_focalMechanisms->setFont(0, f);
				_focalMechanisms->setText(0, "FocalMechanisms");
			}

			_focalMechanisms->addChild(item);
		}

		void addFocalMechanismItem(int i, QTreeWidgetItem *item) {
			if ( _focalMechanisms == NULL ) {
				_focalMechanisms = new TreeItem(this, ST_FocalMechanismGroup, config);
				_focalMechanisms->setEnabled(isEnabled());
				_focalMechanisms->setFlags(_focalMechanisms->flags() & ~Qt::ItemIsDragEnabled);
				QFont f = _focalMechanisms->font(0);
				f.setItalic(true);
				_focalMechanisms->setFont(0, f);
				_focalMechanisms->setText(0, "FocalMechanisms");
			}

			_focalMechanisms->insertChild(0, item);
		}

		void setShowOneItemPerAgency(bool e) {
			if ( _showOnlyOnePerAgency != e ) {
				_showOnlyOnePerAgency = e;
				updateHideState();
			}
		}

		void setPublishState(bool p) {
			_published = p;

			/*
			setData(TYPE, Qt::BackgroundRole, _published?Qt::green:QVariant());
			QFont f = font(TYPE);
			f.setBold(_published);
			setFont(TYPE, f);
			*/
		}

		void resort() {
			_resort = true;
		}

		void updateHideState() {
			if ( _origins != NULL ) {
				if ( _showOnlyOnePerAgency ) {
					QMap<QString, QTreeWidgetItem*> seenAgencies;

					for ( int i = 0; i < _origins->childCount(); ++i ) {
						OriginTreeItem *oitem = (OriginTreeItem*)_origins->child(i);
						QString agency = oitem->text(config.columnMap[COL_AGENCY]);

						QMap<QString, QTreeWidgetItem*>::iterator it = seenAgencies.find(agency);

						bool hide = false;

						// Not the first origin this agency
						if ( it != seenAgencies.end() ) {
							// Is it the preferred origin?
							if ( oitem->data(config.columnMap[COL_ID], Qt::UserRole).toBool() ) {
								treeWidget()->setItemHidden(it.value(), true);
							}
							else
								hide = true;
						}
						else
							seenAgencies.insert(agency, oitem);

						if ( treeWidget()->isItemHidden(oitem) != hide )
							treeWidget()->setItemHidden(oitem, hide);
					}
				}
				else {
					for ( int i = 0; i < _origins->childCount(); ++i ) {
						if ( treeWidget()->isItemHidden(_origins->child(i)) )
							treeWidget()->setItemHidden(_origins->child(i), false);
					}
				}
			}

			if ( _focalMechanisms != NULL ) {
				if ( _showOnlyOnePerAgency ) {
					QMap<QString, QTreeWidgetItem*> seenAgencies;

					for ( int i = 0; i < _focalMechanisms->childCount(); ++i ) {
						FocalMechanismTreeItem *fmitem = (FocalMechanismTreeItem*)_focalMechanisms->child(i);
						QString agency = fmitem->text(config.columnMap[COL_AGENCY]);

						QMap<QString, QTreeWidgetItem*>::iterator it = seenAgencies.find(agency);

						bool hide = false;

						// Not the first origin this agency
						if ( it != seenAgencies.end() ) {
							// Is it the preferred origin?
							if ( fmitem->data(config.columnMap[COL_ID], Qt::UserRole).toBool() ) {
								treeWidget()->setItemHidden(it.value(), true);
							}
							else
								hide = true;
						}
						else
							seenAgencies.insert(agency, fmitem);

						if ( treeWidget()->isItemHidden(fmitem) != hide )
							treeWidget()->setItemHidden(fmitem, hide);
					}
				}
				else {
					for ( int i = 0; i < _origins->childCount(); ++i ) {
						if ( treeWidget()->isItemHidden(_origins->child(i)) )
							treeWidget()->setItemHidden(_origins->child(i), false);
					}
				}
			}
		}

		void update(EventListView *view) {
			Event* ev = event();
			if ( ev ) {
				if ( _resort && _origins != NULL ) {
					_hasMultipleAgencies = false;

					// Reset origin process columns
					for ( int i = 0; i < config.originScriptColumns.size(); ++i ) {
						int pos = config.originScriptColumns[i].pos;
						if ( config.eventScriptPositions.contains(pos) )
							continue;
						setBackground(pos, Qt::NoBrush);
						setForeground(pos, Qt::NoBrush);
						setText(pos, "");
						setToolTip(pos, "");
					}

					// Preferred origin changed => resort origins
					QList<QTreeWidgetItem*> childs = _origins->takeChildren();
					if ( !childs.empty() ) {
						qStableSort(childs.begin(), childs.end(), originItemLessThan);

						QString firstAgency;

						for ( QList<QTreeWidgetItem*>::iterator it = childs.begin();
						      it != childs.end(); ++it ) {
							OriginTreeItem *oi = static_cast<OriginTreeItem*>(*it);

							if ( it == childs.begin() )
								firstAgency = oi->text(config.columnMap[COL_AGENCY]);
							else if ( firstAgency != oi->text(config.columnMap[COL_AGENCY]) )
								_hasMultipleAgencies = true;

							oi->setHighlight(false);
						}

						for ( QList<QTreeWidgetItem*>::iterator it = childs.begin();
						      it != childs.end(); ++it ) {
							OriginTreeItem *oi = static_cast<OriginTreeItem*>(*it);
							if ( oi->object()->publicID() == ev->preferredOriginID() ) {
								oi->setHighlight(true);

								// Copy item states from preferred origin item
								// if column is not part of event script columns
								for ( int i = 0; i < config.originScriptColumns.size(); ++i ) {
									int pos = config.originScriptColumns[i].pos;
									if ( config.eventScriptPositions.contains(pos) )
										continue;
									setText(pos, oi->text(pos));
									setBackground(pos, oi->background(pos));
									setForeground(pos, oi->foreground(pos));
									setToolTip(pos, oi->toolTip(pos));
								}

								childs.erase(it);
								childs.push_front(oi);
								break;
							}
						}

						_origins->insertChildren(0, childs);

						if ( _showOnlyOnePerAgency )
							updateHideState();
					}
				}

				if ( _resort && _focalMechanisms != NULL ) {
					// Preferred origin changed => resort origins
					QList<QTreeWidgetItem*> childs = _focalMechanisms->takeChildren();
					if ( !childs.empty() ) {
						qStableSort(childs.begin(), childs.end(), fmItemLessThan);

						for ( QList<QTreeWidgetItem*>::iterator it = childs.begin();
						      it != childs.end(); ++it ) {
							FocalMechanismTreeItem *fmi = static_cast<FocalMechanismTreeItem*>(*it);
							fmi->setHighlight(false);
						}

						for ( QList<QTreeWidgetItem*>::iterator it = childs.begin();
						      it != childs.end(); ++it ) {
							FocalMechanismTreeItem *fmi = static_cast<FocalMechanismTreeItem*>(*it);
							if ( fmi->object()->publicID() == ev->preferredFocalMechanismID() ) {
								fmi->setHighlight(true);
								childs.erase(it);
								childs.push_front(fmi);
								break;
							}
						}

						_focalMechanisms->insertChildren(0, childs);

						if ( _showOnlyOnePerAgency )
							updateHideState();
					}
				}

				_resort = false;

				try {
					setText(config.columnMap[COL_EVENTTYPE], ev->type().toString());
				}
				catch ( ... ) {
					setText(config.columnMap[COL_EVENTTYPE], "");
				}

				try {
					setText(config.columnMap[COL_EVENTTYPE_CERTAINTY], ev->typeCertainty().toString());
				}
				catch ( ... ) {
					setText(config.columnMap[COL_EVENTTYPE_CERTAINTY], "");
				}

				if ( ev->preferredFocalMechanismID().empty() ) {
					setData(config.columnMap[COL_FM], Qt::DisplayRole, QVariant());
					setData(config.columnMap[COL_FM], Qt::ToolTipRole, QVariant());
					setData(config.columnMap[COL_FM], Qt::UserRole+1, QVariant());
				}
				else if ( treeWidget() && view ) {
					setText(config.columnMap[COL_FM], QObject::tr("Yes"));
					setData(config.columnMap[COL_FM], Qt::ToolTipRole, QObject::tr("Load event and open the focal mechanism tab"));
					setData(config.columnMap[COL_FM], Qt::UserRole+1, QVariant::fromValue<void*>(ev));
				}

				Origin* origin = Origin::Find(ev->preferredOriginID());
				Magnitude* nm = Magnitude::Find(ev->preferredMagnitudeID());

				setText(config.columnMap[COL_ID], QString("%1").arg(ev->publicID().c_str()));
				setText(config.columnMap[COL_REGION], QString("%1").arg(eventRegion(ev).c_str()));

				if ( nm ){
					QFont f = font(config.columnMap[COL_M]);
					f.setBold(true);
					setFont(config.columnMap[COL_M],f);
					setText(config.columnMap[COL_M], QString("%1").arg(nm->magnitude().value(), 0, 'f', 1));
					setData(config.columnMap[COL_M], Qt::UserRole, QVariant(nm->magnitude().value()));
					setText(config.columnMap[COL_MTYPE], QString("%1").arg(nm->type().c_str()));

					/*
					//! display the station Count of a magnitude
					try {
						int staCount = nm->stationCount();
						setText(MCOUNT, QString("%1").arg(staCount, 0, 'd', 0, ' '));
					}
					catch(...){
						setText(MCOUNT, QString("-"));
					}
					//! -----------------------------------------------------
					*/

				}
				else if ( ev->preferredMagnitudeID().empty() ) {
					QFont f = font(config.columnMap[COL_M]);
					f.setBold(false);
					setFont(config.columnMap[COL_M],f);
					setText(config.columnMap[COL_M], "-");
					setText(config.columnMap[COL_MTYPE], "-"); // stationCount
					//setText(MCOUNT, "-"); // stationCount
				}

				//! this lines are for displaying defining Phase Count of an origin
				//
				if ( origin ) {
					try {
						setText(config.columnMap[COL_AGENCY], origin->creationInfo().agencyID().c_str());
						setText(config.columnMap[COL_AUTHOR], origin->creationInfo().author().c_str());
					}
					catch ( ... ) {
						setText(config.columnMap[COL_AGENCY], "");
						setText(config.columnMap[COL_AUTHOR], "");
					}

					int column = config.columnMap[COL_OTIME];
					setText(column, timeToString(origin->time().value(), "%F %T"));
					setData(column, Qt::UserRole, QVariant((double)origin->time().value()));

					double lat = origin->latitude();
					double lon = origin->longitude();

					column = config.columnMap[COL_LAT];
					setText(column, QString("%1 %2").arg(fabs(lat), 0, 'f', SCScheme.precision.location).arg(lat < 0?"S":"N")); // Lat
					setData(column, Qt::UserRole, lat);

					column = config.columnMap[COL_LON];
					setText(column, QString("%1 %2").arg(fabs(lon), 0, 'f', SCScheme.precision.location).arg(lon < 0?"W":"E")); // Lon
					setData(column, Qt::UserRole, lon);

					column = config.columnMap[COL_DEPTH];
					try {
						setText(column, QString("%1 km").arg(depthToString(origin->depth(), SCScheme.precision.depth))); // Depth
						setData(column, Qt::UserRole, origin->depth().value());
					}
					catch ( ... ) {
						setText(column, "-");
					}

					column = config.columnMap[COL_DEPTH_TYPE];
					try {
						setText(column, origin->depthType().toString()); // Depth type
					}
					catch ( ... ) {
						setText(column, "-");
					}

					char stat = objectStatusToChar(origin);
					setText(config.columnMap[COL_TYPE],
						QString("%1%2%3")
						 .arg(_published?"*":"")
						 .arg(stat)
						 .arg(_hasMultipleAgencies?"+":"")
					); // Type
					try {
						switch ( origin->evaluationMode() ) {
							case DataModel::AUTOMATIC:
								setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
								break;
							case DataModel::MANUAL:
								setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.manual);
								break;
							default:
								break;
						};
					}
					catch ( ... ) {
						setTextColor(config.columnMap[COL_TYPE], SCScheme.colors.originStatus.automatic);
					}

					try{
						const OriginQuality &quality = origin->quality();
						setText(config.columnMap[COL_PHASES], QString("%1").arg(quality.usedPhaseCount(), 0, 'd', 0, ' '));
						setData(config.columnMap[COL_PHASES], Qt::UserRole, (double)quality.usedPhaseCount());
					}
					catch(...){
						setText(config.columnMap[COL_PHASES], "-");
						setData(config.columnMap[COL_PHASES], Qt::UserRole, QVariant());
					}

					try{
						const OriginQuality &quality = origin->quality();
						setText(config.columnMap[COL_RMS], QString("%1").arg(quality.standardError(), 0, 'f', SCScheme.precision.rms));
						setData(config.columnMap[COL_RMS], Qt::UserRole, (double)quality.standardError());
					}
					catch(...){
						setText(config.columnMap[COL_RMS], "-");
						setData(config.columnMap[COL_RMS], Qt::UserRole, QVariant());
					}

					if ( config.customColumn != -1 ) {
						setData(config.customColumn, Qt::ForegroundRole, QVariant());
						setText(config.customColumn, config.customDefaultText);
						if ( !config.originCommentID.empty() ) {
							for ( size_t i = 0; i < origin->commentCount(); ++i ) {
								if ( origin->comment(i)->id() == config.originCommentID ) {
									setText(config.customColumn, origin->comment(i)->text().c_str());
									QMap<std::string, QColor>::const_iterator it =
										config.customColorMap.find(origin->comment(i)->text());
									if ( it != config.customColorMap.end() )
										setData(config.customColumn, Qt::ForegroundRole, it.value());
									break;
								}
							}
						}
						else if ( !config.eventCommentID.empty() ) {
							for ( size_t i = 0; i < ev->commentCount(); ++i ) {
								if ( ev->comment(i)->id() == config.eventCommentID ) {
									if( ev->comment(i)->text().empty() ) break;
									setText(config.customColumn, ev->comment(i)->text().c_str());
									QMap<std::string, QColor>::const_iterator it =
										config.customColorMap.find(ev->comment(i)->text());
									if ( it != config.customColorMap.end() )
										setData(config.customColumn, Qt::ForegroundRole, it.value());
									break;
								}
							}
						}
					}
				}
				/*
				else {
					setText(AGENCY, "");
				}
				*/
				//! --------------------------------------------------------------

				try {
					if ( config.hiddenEventTypes.contains(ev->type()) )
					//if ( et == OTHER_EVENT || et == NOT_EXISTING )
						setEnabled(false);
					else
						setEnabled(true);
				}
				catch (...) {
					setEnabled(true);
				}

				QString summary;

				for ( int i = 0; i < columnCount(); ++i ) {
					if ( i > 0 ) summary += '\n';
					summary += QString("%1: %2").arg(config.header[i]).arg(text(i));
				}

				setToolTip(config.columnMap[COL_ID], summary);
			}
			else {
				setText(config.columnMap[COL_ID], "<>");
				setText(config.columnMap[COL_OTIME], "Unassociated");
			}
		}

	private:
		static bool originItemLessThan(const QTreeWidgetItem *i1, const QTreeWidgetItem *i2) {
			const OriginTreeItem *oi1 = static_cast<const OriginTreeItem*>(i1);
			const OriginTreeItem *oi2 = static_cast<const OriginTreeItem*>(i2);

			Origin *o1 = static_cast<Origin*>(oi1->object());
			Origin *o2 = static_cast<Origin*>(oi2->object());

			if ( !o1 ) return true;
			if ( !o2 ) return false;

			try {
				return o1->creationInfo().creationTime() > o2->creationInfo().creationTime();
			}
			catch ( ... ) {
				return false;
			}
		}

		static bool fmItemLessThan(const QTreeWidgetItem *i1, const QTreeWidgetItem *i2) {
			const FocalMechanismTreeItem *fmi1 = static_cast<const FocalMechanismTreeItem*>(i1);
			const FocalMechanismTreeItem *fmi2 = static_cast<const FocalMechanismTreeItem*>(i2);

			FocalMechanism *fm1 = static_cast<FocalMechanism *>(fmi1->object());
			FocalMechanism *fm2 = static_cast<FocalMechanism *>(fmi2->object());

			if ( !fm1 ) return true;
			if ( !fm2 ) return false;

			try {
				return fm1->creationInfo().creationTime() > fm2->creationInfo().creationTime();
			}
			catch ( ... ) {
				return false;
			}
		}

		TreeItem *_origins;
		TreeItem *_focalMechanisms;

		std::string _lastPreferredOriginID;
		bool _showOnlyOnePerAgency;
		bool _resort;
		bool _hasMultipleAgencies;
		bool _published;
};


bool sendJournal(const std::string &objectID, const std::string &action,
                 const std::string &params, const char *group) {
	JournalEntryPtr entry = new JournalEntry;
	entry->setObjectID(objectID);
	entry->setAction(action);
	entry->setParameters(params);
	entry->setSender(SCApp->author());
	entry->setCreated(Core::Time::GMT());

	NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
	NotifierMessagePtr nm = new NotifierMessage;
	nm->attach(n.get());
	return SCApp->sendMessage(group, nm.get());
}


class TreeWidget : public QTreeWidget {
	public:
		TreeWidget(QWidget *p)
		: QTreeWidget(p), _lastDropItem(NULL) {}

		void startDrag(Qt::DropActions supportedActions) {
			if ( !currentItem() || currentItem()->type() == ST_None ) {
				SEISCOMP_DEBUG("About to drag an item without type");
				return;
			}

			SchemeTreeItem *item = (SchemeTreeItem*)currentItem();
			if ( !item->object() ) {
				SEISCOMP_DEBUG("Item has no object attached");
				return;
			}

			QMimeData *mimeData = NULL;
			switch ( item->type() ) {
				case ST_Event:
					mimeData = new QMimeData;
					mimeData->setData("uri/event", item->object()->publicID().c_str());
					break;
				case ST_Origin:
					mimeData = new QMimeData;
					mimeData->setData("uri/origin", item->object()->publicID().c_str());
					break;
				default:
					SEISCOMP_DEBUG("Unknown item type");
					break;
			}

			if ( mimeData == NULL ) return;

			for ( int i = 0; i < item->columnCount(); ++i ) {
				item->setBackground(i, palette().color(QPalette::Highlight));
				item->setForeground(i, palette().color(QPalette::HighlightedText));
			}

			//SEISCOMP_DEBUG("Start drag");
			QDrag *drag = new QDrag(this);
			drag->setMimeData(mimeData);
#if QT_VERSION < 0x040300
			drag->start(Qt::MoveAction);
#else
			drag->exec(Qt::MoveAction);
#endif

			for ( int i = 0; i < item->columnCount(); ++i ) {
				item->setBackground(i, Qt::NoBrush);
				item->setForeground(i, Qt::NoBrush);
			}
		}

		void dragEnterEvent(QDragEnterEvent *event) {
			if ( event->source() == this )
				event->accept();
			else {
				event->ignore();
				return;
			}

			dragMoveEvent(event);
			event->accept();
		}

		void dragMoveEvent(QDragMoveEvent *event) {
			QTreeWidget::dragMoveEvent(event);

			SchemeTreeItem *item = (SchemeTreeItem*)itemAt(event->pos());

			if ( _lastDropItem && item != _lastDropItem ) {
				/*
				for ( int i = 0; i < _lastDropItem->columnCount(); ++i )
					_lastDropItem->setBackground(i, Qt::NoBrush);
				*/
			}

			setCurrentItem(NULL);

			if ( !item || item->type() != ST_Event ) {
				//SEISCOMP_DEBUG("Drop item is not an event");
				event->ignore();
				return;
			}

			if ( item->object() ) {
				_lastDropItem = item;
				setCurrentItem(_lastDropItem);
				/*
				for ( int i = 0; i < item->columnCount(); ++i )
					item->setBackground(i, Qt::green);
				*/
			}
			else {
				//SEISCOMP_DEBUG("Drop item has no object attached");
				event->ignore();
				return;
			}

			event->accept();
		}

		void dragLeaveEvent(QDragLeaveEvent *event) {
			setCurrentItem(NULL);
			/*
			if ( _lastDropItem ) {
				for ( int i = 0; i < _lastDropItem->columnCount(); ++i )
					_lastDropItem->setBackground(i, Qt::NoBrush);
			}
			*/

			_lastDropItem = NULL;
		}

		void dropEvent(QDropEvent *event) {
			_lastDropItem = NULL;
		}


	private:
		QTreeWidgetItem *_lastDropItem;
};


class CommandWaitDialog : public QDialog {
	public:
		CommandWaitDialog(QWidget *parent)
		: QDialog(parent) {
			setWindowTitle("Status");
			setWindowModality(Qt::ApplicationModal);

			QVBoxLayout *l = new QVBoxLayout;
			setLayout(l);

			QVBoxLayout *topvl = new QVBoxLayout;

			QHBoxLayout *hl = new QHBoxLayout;
			_labelCommand = new QLabel;
			QFont f = _labelCommand->font();
			f.setBold(true);
			_labelCommand->setFont(f);

			_labelStatus = new QLabel;
			hl->addWidget(_labelCommand);
			hl->addWidget(_labelStatus);
			hl->addStretch();

			_labelMessage = new QLabel;
			f = _labelMessage->font();
			f.setItalic(true);
			_labelMessage->setFont(f);

			_progressBar = new QProgressBar;
			_progressBar->setRange(0,0);

			topvl->addLayout(hl);
			topvl->addWidget(_labelMessage);

			hl = new QHBoxLayout;
			hl->addLayout(topvl);
			hl->addStretch();

			_labelIcon = new QLabel;
			topvl = new QVBoxLayout;
			topvl->addWidget(_labelIcon);
			topvl->addStretch();

			hl->addLayout(topvl);
			l->addLayout(hl);

			l->addWidget(_progressBar);

			hl = new QHBoxLayout;
			hl->addStretch();

			l->addStretch();

			QPushButton *closeButton = new QPushButton("Close");
			connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
			hl->addWidget(closeButton);

			l->addLayout(hl);
		}

		void setCommand(const std::string &obj, const std::string &cmd) {
			_objectID = obj;
			_command = cmd;

			if ( _command == CMD_SPLIT_ORIGIN )
				_labelCommand->setText("Split origin");
			else if ( _command == CMD_NEW_EVENT )
				_labelCommand->setText("Form new event");
			else if ( _command == CMD_MERGE_EVENT )
				_labelCommand->setText("Merge events");
			else if ( _command == CMD_GRAB_ORIGIN )
				_labelCommand->setText("Move origin");
			else
				_labelCommand->setText(_command.c_str());

			_labelStatus->setPalette(QPalette());

			QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
			_labelIcon->setPixmap(icon.pixmap(32,32));

			_labelStatus->setText("(waiting)");
			_labelMessage->setText("Waiting for command to finish...");

			_progressBar->setRange(0,0);
			_progressBar->setValue(0);
		}

		bool handle(JournalEntry *entry) {
			if ( entry->objectID() != _objectID )
				return false;
			if ( entry->action().compare(0, _command.size(), _command) != 0 )
				return false;

			if ( entry->action().compare(_command.size(), 2, "OK") == 0 ) {
				close();
				return true;
			}

			std::string status = entry->action().substr(_command.size());

			_progressBar->setRange(0,100);
			_progressBar->setValue(100);
			_labelMessage->setText(entry->parameters().c_str());
			_labelStatus->setText(QString("(%1)").arg(status.c_str()));

			if ( status == "Failed" ) {
				QPalette pal = _labelStatus->palette();
				pal.setColor(QPalette::WindowText, Qt::red);
				_labelStatus->setPalette(pal);
				QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxCritical);
				_labelIcon->setPixmap(icon.pixmap(32,32));
			}

			return true;
		}

	private:
		QLabel       *_labelIcon;
		QLabel       *_labelCommand;
		QLabel       *_labelStatus;
		QLabel       *_labelMessage;
		QProgressBar *_progressBar;
		std::string   _objectID;
		std::string   _command;
};


struct ConfigProcessColumn {
	int     pos;
	QString label;
	QString originScript;
	QString eventScript;
};


class EventFilterWidget : public QWidget {
	public:
		EventFilterWidget(QWidget *parent = 0)
		: QWidget(parent) {
			_ui.setupUi(this);
		}

	public:
		void setFilter(const EventListView::Filter &filter) {
			_ui.fromLatitude->setValue(filter.minLatitude ? *filter.minLatitude : _ui.fromLatitude->minimum());
			_ui.fromLongitude->setValue(filter.minLongitude ? *filter.minLongitude : _ui.fromLongitude->minimum());
			_ui.fromDepth->setValue(filter.minDepth ? *filter.minDepth : _ui.fromDepth->minimum());
			_ui.fromMagnitude->setValue(filter.minMagnitude ? *filter.minMagnitude : _ui.fromMagnitude->minimum());
		}

		/**
		 * Returns a filter structure according to the current settings.
		 */
		EventListView::Filter filter() const {
			EventListView::Filter f;

			if ( _ui.fromLatitude->isValid() )
				f.minLatitude = _ui.fromLatitude->value();
			if ( _ui.toLatitude->isValid() )
				f.maxLatitude = _ui.toLatitude->value();

			if ( _ui.fromLongitude->isValid() )
				f.minLongitude = _ui.fromLongitude->value();
			if ( _ui.toLongitude->isValid() )
				f.maxLongitude = _ui.toLongitude->value();

			if ( _ui.fromDepth->isValid() )
				f.minDepth = _ui.fromDepth->value();
			if ( _ui.toDepth->isValid() )
				f.maxDepth = _ui.toDepth->value();

			if ( _ui.fromMagnitude->isValid() )
				f.minMagnitude = _ui.fromMagnitude->value();
			if ( _ui.toMagnitude->isValid() )
				f.maxMagnitude = _ui.toMagnitude->value();

			return f;
		}

	private:
		Ui::EventFilter _ui;
};


// Helper class to allow proper positioning of the tool buttons menu
class CustomWidgetMenu : public QMenu {
	public:
		CustomWidgetMenu(QWidget *parent = 0) : QMenu(parent) {}

	public:
		QSize sizeHint() const { return QWidget::sizeHint(); }
};


}


using namespace Private;


EventListView::EventListView(Seiscomp::DataModel::DatabaseQuery* reader, bool withOrigins,
                             bool withFocalMechanisms, QWidget * parent, Qt::WindowFlags f)
 : QWidget(parent, f), _reader(reader),
   _withOrigins(withOrigins), _withFocalMechanisms(withFocalMechanisms),
   _blockSelection(false), _blockRemovingOfExpiredEvents(false) {
	_ui.setupUi(this);

	_regionIndex = 0;
	_commandWaitDialog = NULL;

	QBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	_ui.frameList->setLayout(l);

	_treeWidget = new TreeWidget(_ui.frameList);
	_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	_treeWidget->setMouseTracking(true);
	_treeWidget->viewport()->installEventFilter(this);
	_treeWidget->setAutoScroll(true);

	l->addWidget(_treeWidget);

	setSortingEnabled(true);

	_unassociatedEventItem = NULL;
	_updateLocalEPInstance = false;

	_itemConfig.disabledColor = palette().color(QPalette::Disabled, QPalette::Text);
	_itemConfig.columnMap.clear();
	for ( int i = 0; i < EventListColumns::Quantity; ++i ) {
		if ( i == COL_OTIME ) {
			if ( SCScheme.dateTime.useLocalTime )
				_itemConfig.header << QString(EEventListColumnsNames::name(i)).arg(Core::Time::LocalTimeZone().c_str());
			else
				_itemConfig.header << QString(EEventListColumnsNames::name(i)).arg("UTC");
		}
		else
			_itemConfig.header << EEventListColumnsNames::name(i);
		_itemConfig.columnMap.append(i);
	}

	_itemConfig.customColumn = -1;
	_itemConfig.customDefaultText = "-";

	try {
		std::vector<std::string> cols = SCApp->configGetStrings("eventlist.visibleColumns");
		for ( int i = 0; i < EventListColumns::Quantity; ++i )
			colVisibility[i] = false;

		for ( size_t i = 0; i < cols.size(); ++i ) {
			EventListColumns v;
			if ( !v.fromString(cols[i]) ) {
				if ( cols[i] != "TP" ) {
					std::cerr << "ERROR: eventlist.visibleColumns: invalid column name '"
					          << cols[i] << "' at index " << i << ", ignoring" << std::endl;
					continue;
				}
				else {
					v = COL_MTYPE;
					std::cerr << "WARNING: eventlist.visibleColumns: name 'TP' "
					             "has changed to 'MType', please update your configuration"
					          << std::endl;
				}
			}

			colVisibility[v] = true;
		}

		// First column is always visible
		colVisibility[COL_OTIME] = true;
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> types = SCApp->configGetStrings("eventlist.filter.types.blacklist");
		for ( size_t i = 0; i < types.size(); ++i ) {
			EventType type;
			if ( type.fromString(types[i]) )
				_itemConfig.hiddenEventTypes.insert(type);
			else
				std::cerr << "WARNING: eventlist.filter.types.blacklist: unknown type: "
				          << types[i] << std::endl;
		}
	}
	catch ( ... ) {
		_itemConfig.hiddenEventTypes.insert(NOT_EXISTING);
		_itemConfig.hiddenEventTypes.insert(OTHER_EVENT);
	}

	try {
		std::vector<std::string> prefAgencies = SCApp->configGetStrings("eventlist.filter.agencies.whitelist");
		for ( size_t i = 0; i < prefAgencies.size(); ++i )
			_itemConfig.preferredAgencies.insert(prefAgencies[i].c_str());
	}
	catch ( ... ) {
		_itemConfig.preferredAgencies.insert(SCApp->agencyID().c_str());
	}

	try {
		_checkEventAgency = SCApp->configGetString("eventlist.filter.agencies.type") == "events";
	}
	catch ( ... ) {
		_checkEventAgency = true;
	}

	try { _ui.cbHideForeign->setText(SCApp->configGetString("eventlist.filter.agencies.label").c_str()); }
	catch ( ... ) {}

	try { _ui.cbHideForeign->setChecked(SCApp->configGetBool("eventlist.filter.agencies.enabled")); }
	catch ( ... ) {}

	try { _ui.cbHideOther->setText(SCApp->configGetString("eventlist.filter.types.label").c_str()); }
	catch ( ... ) {}

	try { _ui.cbHideOther->setChecked(SCApp->configGetBool("eventlist.filter.types.enabled")); }
	catch ( ... ) {}

	try { _ui.cbFilterRegions->setChecked(SCApp->configGetBool("eventlist.filter.regions.enabled")); }
	catch ( ... ) {}

	try {
		_itemConfig.customColumn = SCApp->configGetInt("eventlist.customColumn.pos");
	}
	catch ( ... ) {}

	try {
		_itemConfig.customDefaultText = SCApp->configGetString("eventlist.customColumn.default").c_str();
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> customColors = SCApp->configGetStrings("eventlist.customColumn.colors");
		for ( size_t i = 0; i < customColors.size(); ++i ) {
			size_t pos = customColors[i].rfind(':');
			if ( pos == std::string::npos ) continue;
			std::string value = customColors[i].substr(0, pos);
			std::string strColor = customColors[i].substr(pos+1);
			QColor color;
			if ( fromString(color, strColor) )
				_itemConfig.customColorMap[value] = color;
		}
	}
	catch ( ... ) {}

	try {
		std::string customColumn = SCApp->configGetString("eventlist.customColumn");
		if ( !customColumn.empty() ) {
			if ( _itemConfig.customColumn >= 0 && _itemConfig.customColumn < _itemConfig.header.size() )
				_itemConfig.header.insert(_itemConfig.customColumn, customColumn.c_str());
			else {
				_itemConfig.header.append(customColumn.c_str());
				_itemConfig.customColumn = _itemConfig.header.size()-1;
			}

			if ( _itemConfig.customColumn >= 0 && _itemConfig.customColumn < _itemConfig.columnMap.size() ) {
				for ( int i = _itemConfig.customColumn; i < _itemConfig.columnMap.size(); ++i )
					_itemConfig.columnMap[i] = i+1;
			}
		}
	}
	catch ( ... ) {}

	try {
		_itemConfig.originCommentID = SCApp->configGetString("eventlist.customColumn.originCommentID");
	}
	catch ( ... ) {}
	try {
		_itemConfig.eventCommentID = SCApp->configGetString("eventlist.customColumn.eventCommentID");
	}
	catch ( ... ) {}

	// Read script column configuration. A column is shared between origins and
	// events if the label and position are equal.
	QVector<ConfigProcessColumn> scriptColumns;
	std::vector<std::string> processProfiles;
	try {
		processProfiles = SCApp->configGetStrings("eventlist.scripts.columns");
	}
	catch ( ... ) {}

	for ( std::vector<std::string>::const_iterator it = processProfiles.begin();
	      it != processProfiles.end(); ++it ) {
		ConfigProcessColumn item;
		try {
			item.pos = SCApp->configGetInt("eventlist.scripts.column." + *it + ".pos");
		}
		catch ( ... ) {
			item.pos = -1;
		}

		QString script;
		try {
			script = Environment::Instance()->absolutePath(
			             SCApp->configGetString("eventlist.scripts.column." + *it + ".script")).c_str();
		}
		catch ( ... ) {}

		if ( script.isEmpty() ) {
			std::cerr << "WARNING: eventlist.scripts.column." << *it
			          << ".script is not set: ignoring" << std::endl;
			continue;
		}

		try {
			item.label = SCApp->configGetString("eventlist.scripts.column." + *it + ".label").c_str();
		}
		catch ( ... ) {
			std::cerr << "WARNING: eventlist.scripts.column." << *it
			          << ".label is not set: ignoring" << std::endl;
			continue;
		}


		try {
			std::vector<std::string> types = SCApp->configGetStrings("eventlist.scripts.column." + *it + ".types");
			for ( std::vector<std::string>::const_iterator t_it = types.begin();
			      t_it != types.end(); ++t_it ) {
				if ( !compareNoCase(*t_it, "Origin") )
					item.originScript = script;
				else if ( !compareNoCase(*t_it, "Event") )
					item.eventScript = script;
				else
					std::cerr << "WARNING: eventlist.scripts.column." << *it
					          << ".types: type '" << *t_it
					          << "' unsupported" << std::endl;
			}
		}
		catch ( ... ) {
			item.originScript = script;
		}
		if ( item.originScript.isEmpty() && item.eventScript.isEmpty() ) {
			std::cerr << "WARNING: eventlist.scripts.column." << *it
			          << ".types: no valid type found, ignoring" << std::endl;
			continue;
		}

		// event run: check for columns with same position and label
		bool matchFound = false;
		for ( int i = 0; i < scriptColumns.size(); ++i ) {
			ConfigProcessColumn &other = scriptColumns[i];
			if ( other.pos == item.pos && other.label == item.label ) {
				if ( !item.originScript.isEmpty() )
					other.originScript = item.originScript;
				if ( !item.eventScript.isEmpty() )
					other.eventScript = item.eventScript;
				matchFound = true;
				break;
			}
		}
		if ( !matchFound ) {
			scriptColumns.append(item);
		}
	}

	// Apply process column configuration
	for ( int i = 0; i < scriptColumns.size(); ++i ) {
		ConfigProcessColumn &item = scriptColumns[i];

		if ( item.pos >= 0 && item.pos < _itemConfig.header.size() ) {
			_itemConfig.header.insert(item.pos, item.label);
			if ( item.pos <= _itemConfig.customColumn )
				++_itemConfig.customColumn;
		}
		else {
			_itemConfig.header.append(item.label);
			item.pos = _itemConfig.header.size()-1;
		}

		if ( item.pos >= 0 && item.pos < _itemConfig.columnMap.size() ) {
			// Remap predefined columns
			for ( int i = 0; i < _itemConfig.columnMap.size(); ++i ) {
				if ( _itemConfig.columnMap[i] >= item.pos )
					++_itemConfig.columnMap[i];
			}

			// Remap origin and event process columns
			for ( int i = 0; i < _itemConfig.originScriptColumns.size(); ++i ) {
				ProcessColumn &col = _itemConfig.originScriptColumns[i];
				if ( col.pos >= item.pos ) {
					++col.pos;
					++_itemConfig.originScriptColumnMap[col.script];
				}
			}
			for ( int i = 0; i < _itemConfig.eventScriptColumns.size(); ++i ) {
				ProcessColumn &col = _itemConfig.eventScriptColumns[i];
				if ( col.pos >= item.pos ) {
					++col.pos;
					++_itemConfig.eventScriptColumnMap[col.script];
				}
			}
		}

		if ( !item.originScript.isEmpty() ) {
			ProcessColumn col;
			col.pos = item.pos;
			col.script = item.originScript;
			_itemConfig.originScriptColumns.append(col);
			_itemConfig.originScriptColumnMap[col.script] = col.pos;
		}
		if ( !item.eventScript.isEmpty() ) {
			ProcessColumn col;
			col.pos = item.pos;
			col.script = item.eventScript;
			_itemConfig.eventScriptColumns.append(col);
			_itemConfig.eventScriptColumnMap[col.script] = col.pos;
		}
	}
	// Create set of event script column positions for faster lookup
	for ( int i = 0; i < _itemConfig.eventScriptColumns.size(); ++i )
		_itemConfig.eventScriptPositions << _itemConfig.eventScriptColumns[i].pos;

	if ( !_withOrigins && !_withFocalMechanisms )
		_treeWidget->setRootIsDecorated(false);

	Region reg;
	reg.name = "- custom -";
	reg.minLat = -90;
	reg.minLong = -180;
	reg.maxLat = 90;
	reg.maxLong = 180;
	_filterRegions.append(reg);

	// Read region definitions for filters
	try {
		std::vector<std::string> regionProfiles =
			SCApp->configGetStrings("eventlist.regions");

		for ( size_t i = 0; i < regionProfiles.size(); ++i ) {
			std::string name;
			std::vector<double> defs;

			try { name = SCApp->configGetString("eventlist.region." + regionProfiles[i] + ".name"); }
			catch ( ... ) {
				std::cerr << "WARNING: eventlist.region."
				          << regionProfiles[i] << ".name is not set: ignoring"
				          << std::endl;
				continue;
			}

			try { defs = SCApp->configGetDoubles("eventlist.region." + regionProfiles[i] + ".rect"); }
			catch ( ... ) {
				std::cerr << "WARNING: eventlist.region."
				          << regionProfiles[i] << ".rect requires exactly 4 parameters (nothing given): ignoring"
				          << std::endl;
				continue;
			}

			if ( name.empty() ) {
				std::cerr << "WARNING: eventlist.region."
				          << regionProfiles[i] << ".name is empty: ignoring"
				          << std::endl;
				continue;
			}

			if ( defs.size() != 4 ) {
				std::cerr << "WARNING: eventlist.region."
				          << regionProfiles[i] << ".rect requires exactly 4 parameters ("
				          << defs.size() << " given): ignoring"
				          << std::endl;
				continue;
			}

			reg.name = name.c_str();
			reg.minLat = defs[0];
			reg.minLong = defs[1];
			reg.maxLat = defs[2];
			reg.maxLong = defs[3];

			_filterRegions.append(reg);
		}
	}
	catch ( ... ) {}

	// Initialize database filter
	try { _filter.minLatitude = SCApp->configGetDouble("eventlist.filter.database.minlat"); }
	catch ( ... ) {}
	try { _filter.maxLatitude = SCApp->configGetDouble("eventlist.filter.database.maxlat"); }
	catch ( ... ) {}
	try { _filter.minLongitude = SCApp->configGetDouble("eventlist.filter.database.minlon"); }
	catch ( ... ) {}
	try { _filter.maxLongitude = SCApp->configGetDouble("eventlist.filter.database.maxlon"); }
	catch ( ... ) {}
	try { _filter.minDepth = SCApp->configGetDouble("eventlist.filter.database.mindepth"); }
	catch ( ... ) {}
	try { _filter.maxDepth = SCApp->configGetDouble("eventlist.filter.database.maxdepth"); }
	catch ( ... ) {}
	try { _filter.minMagnitude = SCApp->configGetDouble("eventlist.filter.database.minmag"); }
	catch ( ... ) {}
	try { _filter.maxMagnitude = SCApp->configGetDouble("eventlist.filter.database.maxmag"); }
	catch ( ... ) {}

	for ( int i = 0; i < _filterRegions.size(); ++i )
		_ui.lstFilterRegions->addItem(_filterRegions[i].name);

	if ( _ui.lstFilterRegions->count() > 1 ) {
		_regionIndex = 1;
		_ui.lstFilterRegions->setCurrentIndex(_regionIndex);
		_ui.btnChangeRegion->hide();
	}

	_ui.btnFilter->setPopupMode(QToolButton::InstantPopup);

	_filterWidget = new EventFilterWidget;
	_filterWidget->setFilter(_filter);

	QVBoxLayout *vl = new QVBoxLayout;
	vl->addWidget(_filterWidget);

	QMenu *menu = new CustomWidgetMenu(_ui.btnFilter);
	menu->setLayout(vl);
	_ui.btnFilter->setMenu(menu);

	connect(_ui.lstFilterRegions, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(regionSelectionChanged(int)));

	connect(_ui.btnChangeRegion, SIGNAL(clicked()), this, SLOT(changeRegion()));

	//_treeWidget->setHeaderLabels(QStringList() << "PublicID" << "Desc/Time" << "Mag" << "StaCount" << "defPhaseCount");
	_treeWidget->setHeaderLabels(_itemConfig.header);
	_treeWidget->setAlternatingRowColors(true);

	for ( int i = 0; i < EventListColumns::Quantity; ++i )
		_treeWidget->header()->setSectionHidden(_itemConfig.columnMap[i], !colVisibility[i]);

	_treeWidget->header()->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_treeWidget->header(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(headerContextMenuRequested(const QPoint &)));

	connect(_treeWidget->header(), SIGNAL(sectionClicked(int)),
	        this, SLOT(sortItems(int)));

	addAction(_ui.actionCopyRowToClipboard);

	QAction* expandAll = new QAction(this);
	expandAll->setShortcut(Qt::CTRL + Qt::Key_E);

	QAction* collapseAll = new QAction(this);
	collapseAll->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_E);

	addAction(expandAll);
	addAction(collapseAll);

	_ui.btnReadDays->setEnabled(_reader != NULL);
	_ui.btnReadInterval->setEnabled(_reader != NULL);

	_ui.dateTimeEditStart->setDateTime(QDateTime::currentDateTime().toUTC());
	_ui.dateTimeEditEnd->setDateTime(QDateTime::currentDateTime().toUTC());

	initTree();

	_autoSelect = false;

	connect(_ui.cbHideOther, SIGNAL(stateChanged(int)), this,  SLOT(onShowOtherEvents(int)));
	_hideOtherEvents = _ui.cbHideOther->checkState() == Qt::Checked;

	connect(_ui.cbHideForeign, SIGNAL(stateChanged(int)), this,  SLOT(onShowForeignEvents(int)));
	_hideForeignEvents = _ui.cbHideForeign->checkState() == Qt::Checked;

	connect(_ui.cbFilterRegions, SIGNAL(stateChanged(int)), this,  SLOT(onHideOutsideRegion(int)));
	_hideOutsideRegion = _ui.cbFilterRegions->checkState() == Qt::Checked;

	connect(_ui.cbShowLatestOnly, SIGNAL(stateChanged(int)), this,  SLOT(updateAgencyState()));
	_showOnlyLatestPerAgency = _ui.cbShowLatestOnly->checkState() == Qt::Checked;

	if ( !_withOrigins )
		_ui.cbShowLatestOnly->setVisible(false);

	connect(_ui.btnReadDays, SIGNAL(clicked()), this, SLOT(readLastDays()));
	connect(_ui.btnReadInterval, SIGNAL(clicked()), this, SLOT(readInterval()));
	connect(_ui.btnClear, SIGNAL(clicked()), this, SLOT(clear()));
	connect(_treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(itemSelected(QTreeWidgetItem*,int)));
	connect(_treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*,int)), this, SLOT(itemPressed(QTreeWidgetItem*,int)));
	connect(_treeWidget, SIGNAL(itemEntered(QTreeWidgetItem*,int)), this, SLOT(itemEntered(QTreeWidgetItem*,int)));
	connect(_treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(itemExpanded(QTreeWidgetItem*)));
	connect(_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
	connect(_ui.actionCopyRowToClipboard, SIGNAL(triggered(bool)), this, SLOT(copyRowToClipboard()));

	connect(expandAll, SIGNAL(triggered()), _treeWidget, SLOT(expandAll()));
	connect(collapseAll, SIGNAL(triggered()), _treeWidget, SLOT(collapseAll()));
	//_withComments = true;

	_busyIndicator = new QMovie(this);
	_busyIndicator->setFileName(":/images/images/loader.mng");
	_busyIndicator->setCacheMode(QMovie::CacheAll);

	_busyIndicatorLabel = new QLabel(_treeWidget->viewport());
	_busyIndicatorLabel->hide();
	_busyIndicatorLabel->setMovie(_busyIndicator);
	_busyIndicatorLabel->setToolTip("PublicObject evaluator is running ...");

	connect(_busyIndicator, SIGNAL(resized(const QSize &)),
	        this, SLOT(indicatorResized(const QSize &)));

	connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultAvailable(const QString &, const QString &, const QString &, const QString &)),
	        this, SLOT(evalResultAvailable(const QString &, const QString &, const QString &, const QString &)));
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(resultError(const QString &, const QString &, const QString &, int)),
	        this, SLOT(evalResultError(const QString &, const QString &, const QString &, int)));

	// Start movie when the thread starts
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(started()),
	        _busyIndicatorLabel, SLOT(show()));
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(started()),
	        _busyIndicator, SLOT(start()));

	// Stop movie and hide label when the thread finishes
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(finished()),
	        _busyIndicatorLabel, SLOT(hide()));
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(terminated()),
	        _busyIndicatorLabel, SLOT(hide()));
#endif
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(finished()),
	        _busyIndicator, SLOT(stop()));
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
	connect(&PublicObjectEvaluator::Instance(), SIGNAL(terminated()),
	        _busyIndicator, SLOT(stop()));
#endif

	setFMLinkEnabled(_itemConfig.createFMLink);

	PublicObjectEvaluator::Instance().setDatabaseURI(SCApp->databaseURI().c_str());
}


void EventListView::indicatorResized(const QSize &size) {
	_busyIndicatorLabel->resize(size);
	_busyIndicatorLabel->move(
		(_treeWidget->viewport()->width()-_busyIndicatorLabel->width())/2,
		(_treeWidget->viewport()->height()-_busyIndicatorLabel->height())/2
	);
}


void EventListView::onShowOtherEvents(int checked) {
	_hideOtherEvents = checked == Qt::Checked;
	updateHideState();
}


void EventListView::onShowForeignEvents(int checked) {
	_hideForeignEvents = checked == Qt::Checked;
	updateHideState();
}


void EventListView::onHideOutsideRegion(int checked) {
	_hideOutsideRegion = checked == Qt::Checked;
	updateHideState();
}


void EventListView::updateAgencyState() {
	_showOnlyLatestPerAgency = _ui.cbShowLatestOnly->checkState() == Qt::Checked;

	_treeWidget->setUpdatesEnabled(false);

	QProgressDialog progress(this);
	//progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Please wait..."));
	progress.setRange(0, _treeWidget->topLevelItemCount());
	progress.setLabelText(tr("Checking states..."));
	progress.setModal(true);
	progress.setCancelButton(NULL);

	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		EventTreeItem* item = (EventTreeItem*)_treeWidget->topLevelItem(i);

		progress.setValue(i);
		qApp->processEvents();

		item->setShowOneItemPerAgency(_showOnlyLatestPerAgency);
	}

	_treeWidget->setUpdatesEnabled(true);
}


void EventListView::updateHideState() {
	bool changed = false;

	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		EventTreeItem* item = (EventTreeItem*)_treeWidget->topLevelItem(i);
		if ( updateHideState(item) ) changed = true;
	}

	if ( changed )
		emit eventsUpdated();
}


bool EventListView::updateHideState(QTreeWidgetItem *item) {
	EventTreeItem *eitem = static_cast<EventTreeItem*>(item);
	Event* event = eitem->event();
	if ( !event ) return false;

	bool hide = false;

	if ( _hideOtherEvents ) {
		try {
			if ( _itemConfig.hiddenEventTypes.contains(event->type()) )
				hide = true;
		}
		catch ( Core::ValueException & ) {}
	}

	if ( !hide && _hideForeignEvents ) {
		if ( _checkEventAgency ) {
			try {
				if ( !_itemConfig.preferredAgencies.contains(item->text(_itemConfig.columnMap[COL_AGENCY])) )
					hide = true;
			}
			catch ( Core::ValueException & ) {
				hide = true;
			}
		}
		else {
			bool hasOwnOrigin = false;
			int originItems = eitem->originItemCount();
			for ( int i = 0; i < originItems; ++i ) {
				OriginTreeItem *oitem = static_cast<OriginTreeItem*>(eitem->originItem(i));
				if ( _itemConfig.preferredAgencies.contains(oitem->text(_itemConfig.columnMap[COL_AGENCY])) ) {
					hasOwnOrigin = true;
					break;
				}
			}

			if ( !hasOwnOrigin ) hide = true;
		}
	}

	if ( !hide && _hideOutsideRegion && _regionIndex >= 0 ) {
		const Region &reg = _filterRegions[_regionIndex];
		double lat = item->data(_itemConfig.columnMap[COL_LAT], Qt::UserRole).toDouble();
		double lon = item->data(_itemConfig.columnMap[COL_LON], Qt::UserRole).toDouble();
		if ( lat < reg.minLat || lat > reg.maxLat )
			hide = true;
		else {
			if ( reg.minLong <= reg.maxLong ) {
				if ( lon < reg.minLong || lon > reg.maxLong )
					hide = true;
			}
			else {
				if ( lon < reg.minLong && lon > reg.maxLong )
					hide = true;
			}
		}
	}

	if ( hide != _treeWidget->isItemHidden(item) ) {
		_treeWidget->setItemHidden(item, hide);
		if ( hide )
			emit eventRemovedFromList(event);
		else
			emit eventAddedToList(event, false);

		return true;
	}

	return false;
}


EventListView::~EventListView() {
	PublicObjectEvaluator::Instance().clear(this);
}


void EventListView::setRelativeMinimumEventTime(const Seiscomp::Core::TimeSpan& timeAgo) {
	_timeAgo = timeAgo;
}


void EventListView::add(Seiscomp::DataModel::Event* event,
                        Seiscomp::DataModel::Origin* origin) {
	if ( !origin && !event ) return;

	_blockRemovingOfExpiredEvents = true;

	if ( !origin ) {
		SchemeTreeItem *item = findEvent(event->publicID());
		std::map<std::string, OriginPtr> orgs;
		std::map<std::string, FocalMechanismPtr> fms;
		MagnitudePtr prefMag;

		if ( !event->preferredMagnitudeID().empty() )
			prefMag = Magnitude::Find(event->preferredMagnitudeID());

		for ( size_t i = 0; i < event->originReferenceCount(); ++i ) {
			Origin *org = Origin::Find(event->originReference(i)->originID());
			if ( org && orgs.find(org->publicID()) == orgs.end() )
				orgs[org->publicID()] = org;
		}

		if ( _withFocalMechanisms ) {
			for ( size_t i = 0; i < event->focalMechanismReferenceCount(); ++i ) {
				FocalMechanism *fm = FocalMechanism::Find(event->focalMechanismReference(i)->focalMechanismID());
				if ( fm && fms.find(fm->publicID()) == fms.end() )
					fms[fm->publicID()] = fm;
			}
		}

		if ( _reader ) {
			if ( event->originReferenceCount() == 0 )
				_reader->load(event);

			DatabaseIterator it = _reader->getOrigins(event->publicID());
			while ( *it ) {
				Origin *org = Origin::Cast(*it);
				if ( org && orgs.find(org->publicID()) == orgs.end() )
					orgs[org->publicID()] = org;
				++it;
			}
			it.close();

			if ( !prefMag && !event->preferredMagnitudeID().empty() )
				prefMag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
		}

		if ( item == NULL )
			item = addEvent(event, false);

		for ( std::map<std::string, OriginPtr>::iterator it = orgs.begin(); it != orgs.end(); ++it )
			addOrigin(it->second.get(), item, true);

		if ( _withFocalMechanisms ) {
			for ( std::map<std::string, FocalMechanismPtr>::iterator it = fms.begin(); it != fms.end(); ++it )
				addFocalMechanism(it->second.get(), item);
		}

		item->update(this);

	}
	else if ( event ) {
		SchemeTreeItem *item = findEvent(event->publicID());
		MagnitudePtr prefMag = Magnitude::Find(event->preferredMagnitudeID());

		if ( item == NULL ) {
			OriginPtr prefOrg = Origin::Find(event->preferredOriginID());

			if ( !prefOrg && _reader ) {
				prefOrg = Origin::Cast(_reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));

				if ( (_itemConfig.customColumn != -1) && prefOrg && prefOrg->commentCount() == 0 )
					_reader->loadComments(prefOrg.get());
			}

			if ( !prefMag && _reader && !event->preferredMagnitudeID().empty() )
				prefMag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));

			item = addEvent(event, false);
			if ( prefOrg ) {
				if ( event->originReference(prefOrg->publicID()) == NULL )
					event->add(new OriginReference(prefOrg->publicID()));
				addOrigin(prefOrg.get(), item, true);
			}
		}

		if ( event->originReference(origin->publicID()) == NULL )
			event->add(new OriginReference(origin->publicID()));

		if ( findOrigin(origin->publicID()) == NULL )
			addOrigin(origin, item, true);

		item->update(this);
	}
	else {
		QTreeWidgetItem *item = findOrigin(origin->publicID());
		if ( item == NULL )
			addOrigin(origin, NULL, false);
	}

	_blockRemovingOfExpiredEvents = false;
}


void EventListView::setMessagingEnabled(bool e) {
	_updateLocalEPInstance = !e;
	if ( _updateLocalEPInstance )
		_treeWidget->setDragEnabled(false);
}


void EventListView::setEventModificationsEnabled(bool e) {
	_treeWidget->setDragEnabled(e);
	_treeWidget->setAcceptDrops(e);
}


void EventListView::initTree() {

	_treeWidget->clear();

	if ( _withOrigins )
		_unassociatedEventItem = addEvent(NULL, false);
	else
		_unassociatedEventItem = NULL;

	for (int i = 0; i < _treeWidget->columnCount(); i++)
		_treeWidget->resizeColumnToContents(i);

	PublicObjectEvaluator::Instance().clear(this);
	PublicObjectEvaluator::Instance().setDatabaseURI(SCApp->databaseURI().c_str());

	emit reset();
}


bool EventListView::eventFilter(QObject *obj, QEvent *ev) {
	if ( obj == _treeWidget->viewport() ) {
		if ( ev->type() == QEvent::Drop ) {
			QDropEvent *event = static_cast<QDropEvent*>(ev);
			SchemeTreeItem *item = (SchemeTreeItem*)_treeWidget->itemAt(event->pos());
			if ( !item || item->type() == ST_None ) {
				event->ignore();
				return true;
			}

			if ( item->type() == ST_Event ) {
				EventTreeItem *eitem = static_cast<EventTreeItem*>(item);
				Event *evt = eitem->event();
				if ( evt == NULL )
					return true;

				if ( event->mimeData()->hasFormat("uri/event") ) {
					QString eventID = event->mimeData()->data("uri/event");

					// Nothing to do, same eventID
					if ( eventID == item->object()->publicID().data() )
						return true;

					if ( QMessageBox::question(
						this, "Event merge",
						QString(
							"You requested a merge of event %1 into "
							"event %2. This command will modify the "
							"database.\n"
							"Are you sure you want to continue?"
						).arg(eventID).arg(item->object()->publicID().c_str()),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
						) == QMessageBox::No ) {
						event->ignore();
						return true;
					}

					sendJournalAndWait(item->object()->publicID(), CMD_MERGE_EVENT,
					                   eventID.toStdString(), SCApp->messageGroups().event.c_str());
				}
				else if ( event->mimeData()->hasFormat("uri/origin") ) {
					QString originID = event->mimeData()->data("uri/origin");

					if ( QMessageBox::question(
						this, "Origin move",
						QString(
							"You requested to associate origin %1 with "
							"event %2. This command will modify the "
							"database.\n"
							"Are you sure you want to continue?"
						).arg(originID).arg(item->object()->publicID().c_str()),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
						) == QMessageBox::No ) {
						event->ignore();
						return true;
					}

					sendJournalAndWait(eitem->event()->publicID(), CMD_GRAB_ORIGIN,
					                   originID.toStdString(), SCApp->messageGroups().event.c_str());
				}
			}
			else {
				event->ignore();
				return true;
			}

			event->accept();
		}
		else if ( ev->type() == QEvent::Resize ) {
			_busyIndicatorLabel->move((_treeWidget->viewport()->width()-_busyIndicatorLabel->width())/2,
			                          (_treeWidget->viewport()->height()-_busyIndicatorLabel->height())/2);
		}
	}

	return QObject::eventFilter(obj, ev);
}


void EventListView::clear() {
	initTree();
}


void EventListView::selectEventFM(const QString &url) {
	Event *ev = (Event*)sender()->property("eventPtr").value<void*>();
	if ( ev != NULL )
		eventFMSelected(ev);
}


void EventListView::regionSelectionChanged(int index) {
	_regionIndex = index;

	if ( _regionIndex == 0 )
		_ui.btnChangeRegion->show();
	else
		_ui.btnChangeRegion->hide();

	if ( _hideOutsideRegion ) updateHideState();
}


void EventListView::changeRegion() {
	EventListViewRegionFilterDialog dlg(this, &_filterRegions[0], &_filterRegions);
	if ( dlg.exec() == QDialog::Accepted && _hideOutsideRegion )
		updateHideState();
}


void EventListView::setInterval(const Seiscomp::Core::TimeWindow& tw) {
	QDateTime start, end;

	if ( !SCScheme.dateTime.useLocalTime ) {
		start.setTimeSpec(Qt::UTC);
		end.setTimeSpec(Qt::UTC);
		start.setTime_t(tw.startTime().seconds());
		end.setTime_t(tw.endTime().seconds());
	}
	else {
		start.setTime_t(tw.startTime().seconds());
		end.setTime_t(tw.endTime().seconds());
	}

	_ui.dateTimeEditStart->setDateTime(start);
	_ui.dateTimeEditEnd->setDateTime(end);
}


void EventListView::selectFirstEnabledEvent() {
	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		QTreeWidgetItem *item = _treeWidget->topLevelItem(i);
		if ( (item->flags() & Qt::ItemIsEnabled) == 0 ) continue;
		selectEvent(i);
		break;
	}
}


void EventListView::selectEvent(int index) {
	if ( index >= _treeWidget->topLevelItemCount() )
		return;

	_treeWidget->setCurrentItem(_treeWidget->topLevelItem(index));
	loadItem(_treeWidget->currentItem());
}


void EventListView::selectEventID(const std::string& publicID) {
	SchemeTreeItem *item = findEvent(publicID);
	if ( item ) {
		_treeWidget->setCurrentItem(item);
		loadItem(_treeWidget->currentItem());
	}
}


void EventListView::readLastDays() {
	_filter = _filterWidget->filter();
	_filter.endTime = Core::Time::GMT();
	_filter.startTime = _filter.endTime - Core::TimeSpan(_ui.spinBox->value()*86400);
	setInterval(Core::TimeWindow(_filter.startTime, _filter.endTime));
	readFromDatabase(_filter);
}

void EventListView::readInterval() {
	_filter = _filterWidget->filter();
	_filter.startTime = Core::Time(_ui.dateTimeEditStart->date().year(),
	                               _ui.dateTimeEditStart->date().month(),
	                               _ui.dateTimeEditStart->date().day(),
	                               _ui.dateTimeEditStart->time().hour(),
	                               _ui.dateTimeEditStart->time().minute(),
	                               _ui.dateTimeEditStart->time().second());

	_filter.endTime = Core::Time(_ui.dateTimeEditEnd->date().year(),
	                             _ui.dateTimeEditEnd->date().month(),
	                             _ui.dateTimeEditEnd->date().day(),
	                             _ui.dateTimeEditEnd->time().hour(),
	                             _ui.dateTimeEditEnd->time().minute(),
	                             _ui.dateTimeEditEnd->time().second());

	readFromDatabase(_filter);
}


void EventListView::readFromDatabase() {
	readInterval();
}


void EventListView::readFromDatabase(const Filter &filter) {
	if ( _reader == NULL ) return;

	initTree();

	EventParameters ep;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	_blockSelection = true;
	_blockRemovingOfExpiredEvents = true;

	EventPtr event;
	size_t numberOfEvents = _reader->getObjectCount(&ep, Event::TypeInfo());
	size_t numberOfOrigins = numberOfEvents*20;
	size_t numberOfSteps = numberOfEvents*2;
	size_t currentStep = 0;

	if ( _withOrigins )
		numberOfSteps += numberOfOrigins;

	if ( _withFocalMechanisms )
		numberOfSteps += numberOfOrigins;

	QProgressDialog progress(this);
	//progress.setWindowModality(Qt::WindowModal);
	progress.setWindowTitle(tr("Please wait..."));
	progress.setRange(0, numberOfSteps);
	progress.setLabelText(tr("Reading data..."));

	QMap<int, EventPtr>  eventIDs;
	QMap<int, OriginPtr> originIDs;
	QMap<int, FocalMechanismPtr> fmIDs;

	//Core::TimeWindow timeWindow(Core::Time::GMT() - _timeAgo, Core::Time::GMT());

	_timeAgo = Core::Time::GMT() - filter.startTime;
	progress.setLabelText(tr("Reading events..."));
	DatabaseIterator it = getEvents(_reader, filter);
	while ( (event = static_cast<Event*>(*it)) != NULL ) {
		if ( progress.wasCanceled() )
			break;

		ep.add(event.get());
		eventIDs[it.oid()] = event.get();
		++it;
		progress.setValue(++currentStep);
	}
	it.close();

	int eventDiff = numberOfEvents - it.count();
	numberOfSteps -= eventDiff*2;

	if ( _withOrigins ) numberOfSteps -= eventDiff*20;
	if ( _withFocalMechanisms ) numberOfSteps -= eventDiff*20;

	progress.setRange(0, numberOfSteps);

	// Read comments
	CommentPtr comment;
	it = getComments4Events( _reader, filter);
	while ( (comment = Comment::Cast(*it)) != NULL ) {
		if( progress.wasCanceled() )
			break;
		EventPtr evt = eventIDs[it.parentOid()];
		if ( evt ) evt->add(comment.get());
		++it;
	}
	it.close();

	if ( _withOrigins ) {
		progress.setLabelText(tr("Reading origins..."));

		it = getEventOriginReferences(_reader, filter);

		OriginReferencePtr oref;

		while ( (oref = static_cast<OriginReference*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			QMap<int, EventPtr>::iterator mit = eventIDs.find(it.parentOid());
			if ( mit == eventIDs.end() ) continue;

			EventPtr ev = mit.value();

			ev->add(oref.get());

			++it;
		}
		it.close();

		currentStep += numberOfEvents*10;
		progress.setValue(currentStep);

		it = getEventOrigins(_reader, filter);

		OriginPtr origin;

		while ( (origin = static_cast<Origin*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			ep.add(origin.get());
			//if( _withComments )	originIDs[it.oid()] = origin.get();
			originIDs[it.oid()] = origin.get();
			//pubIDs[ origin->publicID() ] = it.oid();

			++it;
		}
		it.close();

		it = getUnassociatedOrigins(_reader, filter);

		while ( (origin = static_cast<Origin*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			ep.add(origin.get());
			//if( _withComments )	originIDs[it.oid()] = origin.get();
			originIDs[it.oid()] = origin.get();
			//pubIDs[ origin->publicID() ] = it.oid();

			++it;
		}
		it.close();

		currentStep += numberOfEvents*10;
		progress.setValue(currentStep);

		//fetch comments for relevant origins (marker for publishing)

		it = getComments4Origins(_reader, filter);

		while ( (comment = Comment::Cast(*it)) != NULL ){
			if( progress.wasCanceled() )
				break;
			OriginPtr org = originIDs[it.parentOid()];
			if ( org ) org->add(comment.get());
			++it;
		}
		it.close();
	}

	if ( _withFocalMechanisms ) {
		progress.setLabelText(tr("Reading focal mechanisms..."));

		it = getEventFocalMechanismReferences(_reader, filter);

		FocalMechanismReferencePtr fmref;

		while ( (fmref = static_cast<FocalMechanismReference*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			QMap<int, EventPtr>::iterator mit = eventIDs.find(it.parentOid());
			if ( mit == eventIDs.end() ) continue;

			EventPtr ev = mit.value();

			ev->add(fmref.get());

			++it;
		}
		it.close();

		currentStep += numberOfEvents*10;
		progress.setValue(currentStep);

		it = getEventFocalMechanisms(_reader, filter);

		FocalMechanismPtr fm;

		while ( (fm = static_cast<FocalMechanism*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			fmIDs[it.oid()] = fm;
			ep.add(fm.get());

			++it;
		}
		it.close();

		it = getEventMomentTensors(_reader, filter);

		MomentTensorPtr mt;
		std::set<std::string> derivedOriginIDs;

		while ( (mt = static_cast<MomentTensor*>(*it)) != NULL ) {
			if ( progress.wasCanceled() )
				break;

			fm = fmIDs[it.parentOid()];
			if ( fm ) {
				fm->add(mt.get());
				derivedOriginIDs.insert(mt->derivedOriginID());
			}

			++it;
		}
		it.close();

		// Load derived origin magnitudes
		for ( std::set<std::string>::iterator it = derivedOriginIDs.begin();
		      it != derivedOriginIDs.end(); ++it ) {
			OriginPtr org = Origin::Find(*it);
			if ( org == NULL ) {
				org = Origin::Cast(_reader->getObject(Origin::TypeInfo(), *it));
				if ( org ) ep.add(org.get());
			}

			if ( org && org->magnitudeCount() == 0 )
				_reader->loadMagnitudes(org.get());
		}
	}

	EventDescriptionPtr description;
	it = getDescriptions4Events(_reader, filter);
	while ( (description = EventDescription::Cast(*it)) != NULL ) {
		if( progress.wasCanceled() )
			break;

		EventPtr evt = eventIDs[it.parentOid()];
		if ( evt ) evt->add(description.get());
		++it;
	}
	it.close();

	QSet<void*> associatedOrigins;

	progress.setLabelText(tr("Reading magnitudes..."));
	std::vector<MagnitudePtr> prefMags;
	std::vector<OriginPtr> prefOrigins;
	prefMags.reserve(numberOfEvents);
	prefOrigins.reserve(numberOfEvents);

	it = _reader->getPreferredMagnitudes(filter.startTime, filter.endTime, "");
	MagnitudePtr mag;
	while ( (mag = static_cast<Magnitude*>(*it)) != NULL ) {
		prefMags.push_back(mag);
		progress.setValue(++currentStep);
		++it;
	}
	it.close();

	if ( !_withOrigins ) {
		it = _reader->getPreferredOrigins(filter.startTime, filter.endTime, "");
		OriginPtr org;
		while ( (org = static_cast<Origin*>(*it)) != NULL ) {
			prefOrigins.push_back(org);
			originIDs[it.oid()] = org;
			++it;
		}
		it.close();

		if ( _itemConfig.customColumn != -1 ) {
			it = getComments4PrefOrigins(_reader, filter);
			CommentPtr comment;

			while ( (comment = Comment::Cast(*it)) != NULL ) {
				if( progress.wasCanceled() )
					break;

				OriginPtr org = originIDs[it.parentOid()];
				if ( org ) org->add(comment.get());
				++it;
			}
			it.close();
		}
	}

	_treeWidget->setUpdatesEnabled (false);

	for ( size_t i = 0; i < ep.eventCount(); ++i ) {
		Event *event = ep.event(i);

		EventTreeItem *eventItem = addEvent(event, false);
		bool update = false;

		for ( size_t c = 0; c < event->commentCount(); ++c ) {
			if ( event->comment(c)->text() == "published" ) {
				update = true;
				eventItem->setPublishState(true);
			}
		}

		if ( _withOrigins && eventItem ) {
			Origin *prefOrg = Origin::Find(event->preferredOriginID());
			// Switch loading of all origin information on
			for ( size_t j = 0; j < event->originReferenceCount(); ++j ) {
				OriginReference *ref = event->originReference(j);
				Origin *o = Origin::Find(ref->originID());
				if ( o ) {
					update = true;
					OriginTreeItem *originItem = addOrigin(o, eventItem, prefOrg == o);
					for ( size_t c = 0; c < o->commentCount(); ++c ) {
						// "OriginPublished" shall be superseded by "published"
					        // but here we accept both
						if ( o->comment(c)->text() == "OriginPublished" ||
						     o->comment(c)->text() == "published" ) {
							originItem->setPublishState(true);
							originItem->update(this);
							break;
						}
					}

					associatedOrigins.insert(o);
				}
			}
		}

		if ( _withFocalMechanisms && eventItem ) {
			for ( size_t j = 0; j < event->focalMechanismReferenceCount(); ++j ) {
				FocalMechanismReference *ref = event->focalMechanismReference(j);
				FocalMechanism *o = FocalMechanism::Find(ref->focalMechanismID());
				if ( o ) {
					update = true;
					addFocalMechanism(o, eventItem);
				}
			}
		}

		if ( update ) {
			updateHideState(eventItem);
			eventItem->update(this);
		}
	}

	if ( _withOrigins ) {
		// Switch loading of all origin information on
		for ( size_t i = 0; i < ep.originCount(); ++i ) {
			if ( !associatedOrigins.contains(ep.origin(i)) ) {
				addOrigin(ep.origin(i), NULL, false);
			}
		}
	}

	for (int i = 0; i < _treeWidget->columnCount(); i++)
		_treeWidget->resizeColumnToContents(i);

	_treeWidget->setUpdatesEnabled(true);

	QApplication::restoreOverrideCursor();
	_blockSelection = false;
	_blockRemovingOfExpiredEvents = false;

	emit eventsUpdated();
}


void EventListView::setAutoSelect(bool s) {
	_autoSelect = s;
}


void EventListView::removeExpiredEvents() {
	if ( _blockRemovingOfExpiredEvents ) return;

	Core::Time now = Core::Time::GMT();

	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		EventTreeItem* item = (EventTreeItem*)_treeWidget->topLevelItem(i);
		Event* event = item->event();
		if ( event != NULL ) {
			Origin* o = Origin::Find(event->preferredOriginID());
			bool remove = false;
			if ( o )
				remove = (now - o->time()) > _timeAgo;
			else {
				double time = item->data(_itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble();
				remove = now - TimeSpan(time) > _timeAgo;
			}

			if ( remove ) {
				QTreeWidgetItem* item = _treeWidget->takeTopLevelItem(i);
				if ( item ) {
					delete item;
					eventRemovedFromList(event);
					--i;
				}
			}
		}
		else {
			for ( int j = 0; j < item->originItemCount(); ++j ) {
				OriginTreeItem *child = (OriginTreeItem*)item->originItem(j);
				if ( child ) {
					if ( child->origin() ) {
						if ( (now - child->origin()->time()) > _timeAgo ) {
							QTreeWidgetItem* it = item->takeOrigin(j);
							if ( it ) {
								delete it;
								--j;
							}
						}
					}
				}
			}

			/*
			for ( int j = 0; j < item->focalMechanismItemCount(); ++j ) {
				FocalMechanismTreeItem *child = (FocalMechanismTreeItem*)item->focalMechanismItem(j);
				if ( child ) {
					if ( child->focalMechanism() ) {
						if ( (now - child->focalMechanism()->time()) > _timeAgo ) {
							QTreeWidgetItem* it = item->takeOrigin(j);
							if ( it ) {
								delete it;
								--j;
							}
						}
					}
				}
			}
			*/
		}
	}
}


EventTreeItem* EventListView::addEvent(Seiscomp::DataModel::Event* event, bool fromNotification) {
	removeExpiredEvents();

	// Read preferred origin for display purpose
	OriginPtr preferredOrigin;
	if ( event ) {
		preferredOrigin = Origin::Find(event->preferredOriginID());
		if ( !preferredOrigin && _reader ) {
			preferredOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));
			if ( (_itemConfig.customColumn != -1) && preferredOrigin && preferredOrigin->commentCount() == 0 )
				_reader->loadComments(preferredOrigin.get());
		}
	}

	// Read preferred magnitude for display purpose
	MagnitudePtr preferredMagnitude;
	if ( event && !event->preferredMagnitudeID().empty() ) {
		preferredMagnitude = Magnitude::Find(event->preferredMagnitudeID());
		if ( !preferredMagnitude && _reader )
			preferredMagnitude = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
	}

	// Read preferred magnitude for display purpose
	FocalMechanismPtr preferredFocalMechanism;
	if ( event && _withFocalMechanisms ) {
		preferredFocalMechanism = FocalMechanism::Find(event->preferredFocalMechanismID());
		if ( !preferredFocalMechanism && _reader )
			preferredFocalMechanism = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), event->preferredFocalMechanismID()));
	}

	EventTreeItem *item = new EventTreeItem(event, _itemConfig);
	item->setShowOneItemPerAgency(_showOnlyLatestPerAgency);

	if ( _treeWidget->topLevelItemCount() == 0 )
		_treeWidget->insertTopLevelItem(0, item);
	else {
		int pos = _treeWidget->topLevelItemCount();
		for ( int i = 0; i  < _treeWidget->topLevelItemCount(); ++i ) {
			if ( _treeWidget->topLevelItem(i)->data(_itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble()
				  < item->data(_itemConfig.columnMap[COL_OTIME], Qt::UserRole).toDouble() ) {
				pos = i;
				break;
			}
		}
		_treeWidget->insertTopLevelItem(pos, item);
	}

	item->update(this);

	updateHideState(item);

	int fixedItems = 0;
	if ( _unassociatedEventItem ) fixedItems = 1;
	if ( _treeWidget->topLevelItemCount() - fixedItems == 1 ) {
		for (int i = 0; i < _treeWidget->columnCount(); i++)
			_treeWidget->resizeColumnToContents(i);
	}

	_ui.btnClear->setEnabled(true);

	updateEventProcessColumns(item, true);

	if ( (event != NULL) && !item->isHidden() )
		emit eventAddedToList(event, fromNotification);

	return item;
}


OriginTreeItem *
EventListView::addOrigin(Seiscomp::DataModel::Origin* origin, QTreeWidgetItem* parent, bool highPriority) {
	//removeExpiredEvents();

	OriginTreeItem* item = new OriginTreeItem(origin, _itemConfig);
	EventTreeItem *eitem = static_cast<EventTreeItem*>(parent?parent:_unassociatedEventItem);
	eitem->addOriginItem(0,item);
	eitem->resort();

	_ui.btnClear->setEnabled(true);

	updateOriginProcessColumns(item, highPriority);

	emit originAdded();

	return item;
}


void EventListView::updateOriginProcessColumns(QTreeWidgetItem *item, bool highPriority) {
	if ( _itemConfig.originScriptColumns.empty() ) return;
	if ( !item ) return;

	OriginTreeItem *oitem = static_cast<OriginTreeItem*>(item);
	Origin *origin = oitem->origin();

	if ( !origin ) return;

	QStringList scripts;
	for ( int i = 0; i < _itemConfig.originScriptColumns.size(); ++i ) {
		scripts << _itemConfig.originScriptColumns[i].script;
		oitem->setBackground(_itemConfig.originScriptColumns[i].pos,
		                     SCScheme.colors.records.gaps);
	}

	if ( highPriority ) {
		if ( !PublicObjectEvaluator::Instance().prepend(this, origin->publicID().c_str(), Origin::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding origin evaluation jobs failed",
			                 origin->publicID().c_str());
		}
	}
	else {
		if ( !PublicObjectEvaluator::Instance().append(this, origin->publicID().c_str(), Origin::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding origin evaluation jobs failed",
			                 origin->publicID().c_str());
		}
	}
}

void EventListView::updateEventProcessColumns(QTreeWidgetItem *item, bool highPriority) {
	if ( _itemConfig.eventScriptColumns.empty() ) return;
	if ( !item ) return;

	EventTreeItem *eitem = static_cast<EventTreeItem*>(item);
	Event *event = eitem->event();

	if ( !event ) return;

	QStringList scripts;
	for ( int i = 0; i < _itemConfig.eventScriptColumns.size(); ++i ) {
		scripts << _itemConfig.eventScriptColumns[i].script;
		eitem->setBackground(_itemConfig.eventScriptColumns[i].pos,
		                     SCScheme.colors.records.gaps);
	}

	if ( highPriority ) {
		if ( !PublicObjectEvaluator::Instance().prepend(this, event->publicID().c_str(), Event::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding event evaluation jobs failed",
			                 event->publicID().c_str());
		}
	}
	else {
		if ( !PublicObjectEvaluator::Instance().append(this, event->publicID().c_str(), Event::TypeInfo(), scripts) ) {
			SEISCOMP_WARNING("%s: adding event evaluation jobs failed",
			                 event->publicID().c_str());
		}
	}
}


FocalMechanismTreeItem* EventListView::addFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm, QTreeWidgetItem* parent) {
	FocalMechanismTreeItem* item = new FocalMechanismTreeItem(fm, _itemConfig);
	EventTreeItem *eitem = static_cast<EventTreeItem*>(parent?parent:_unassociatedEventItem);
	eitem->addFocalMechanismItem(0,item);
	eitem->resort();

	_ui.btnClear->setEnabled(true);

	emit focalMechanismAdded();
	return item;
}


void EventListView::messageAvailable(Seiscomp::Core::Message* msg, Seiscomp::Communication::NetworkMessage*) {
	CommandMessage *cmsg = CommandMessage::Cast(msg);
	if ( cmsg ) {
		onCommand(cmsg);
		return;
	}

	ArtificialOriginMessage *aomsg = ArtificialOriginMessage::Cast(msg);
	if ( aomsg ) {
		Origin* o = aomsg->origin();
		if ( o )
			emit originSelected(o, NULL);
	}
}


void EventListView::onCommand(Seiscomp::Gui::CommandMessage* cmsg) {
	if ( cmsg->command() == CM_SHOW_ORIGIN ) {
		QTreeWidgetItem* item = findOrigin(cmsg->parameter());
		if ( item ) {
			loadItem(item);
			return;
		}

		OriginPtr o = Origin::Find(cmsg->parameter());
		if ( !o ) {
			// lets read it
			if ( _reader ) {
				o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), cmsg->parameter()));
				//readPicks(o.get());
			}
		}

		if ( o ) {
			SchemeTreeItem* parent = NULL;
			EventPtr ev = _reader->getEvent(o->publicID());

			if ( ev ) {
				parent = findEvent(ev->publicID());
				if ( !parent ) {
					parent = addEvent(ev.get(), false);
				}
			}

			//readPicks(o);
			//emit originSelected(o, NULL);

			QTreeWidgetItem* item = addOrigin(o.get(), parent, true);
			if ( parent ) parent->update(this);
			loadItem(item);
		}
		else
			QMessageBox::warning(NULL, tr("Load origin"), tr("Received a request to show origin %1\nwhich has not been found.").arg(o->publicID().c_str()));
	}
	else if ( cmsg->command() == CM_OBSERVE_LOCATION ) {
		Origin* o = Origin::Cast(cmsg->object());
		if ( o )
			emit originSelected(o, NULL);
	}

	return;
}


void EventListView::notifierAvailable(Seiscomp::DataModel::Notifier *n) {
	_treeWidget->setUpdatesEnabled(false);

	if ( _withOrigins ) {
		Origin* o = Origin::Cast(n->object());
		if ( o != NULL ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						QTreeWidgetItem* item = addOrigin(o, NULL, false);
						if ( _autoSelect )
							//_treeWidget->setItemSelected(item, true);
							loadItem(item);
					}
					break;
				case OP_UPDATE:
					{
						SchemeTreeItem* item = (SchemeTreeItem*)findOrigin(o->publicID());
						if ( item ) {
							updateOriginProcessColumns(item, true);
							item->update(this);
							emit originUpdated(static_cast<Origin*>(item->object()));
							EventTreeItem* parent = static_cast<EventTreeItem*>(item->parent()->parent());
							Event *e = static_cast<Event*>(parent->object());
							if ( e && e->preferredOriginID() == o->publicID() ) {
								parent->update(this);
								emit eventUpdatedInList(e);
							}
						}
					}
					break;
				default:
					break;
			}

			_treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	if ( _withFocalMechanisms ) {
		FocalMechanism *fm = FocalMechanism::Cast(n->object());
		if ( fm != NULL ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						addFocalMechanism(fm, NULL);
					}
					break;
				case OP_UPDATE:
					{
						SchemeTreeItem* item = (SchemeTreeItem*)findFocalMechanism(fm->publicID());
						if ( item ) {
							item->update(this);
							emit focalMechanismUpdated(static_cast<FocalMechanism*>(item->object()));
							EventTreeItem* parent = static_cast<EventTreeItem*>(item->parent()->parent());
							Event *e = static_cast<Event*>(parent->object());
							if ( e && e->preferredFocalMechanismID() == fm->publicID() ) {
								parent->update(this);
								emit eventUpdatedInList(e);
							}
						}
					}
					break;
				default:
					break;
			}

			_treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	Event* e = Event::Cast(n->object());
	if ( e != NULL ) {
		switch ( n->operation() ) {
			case OP_ADD:
				{
					EventTreeItem* item = (EventTreeItem*)findEvent(e->publicID());
					if ( !item ) {
						addEvent(e, true);
						emit
						break;
					}
				}
			case OP_REMOVE:
				{
					EventTreeItem* item = (EventTreeItem*)findEvent(e->publicID());
					if ( item ) {
						SEISCOMP_DEBUG("Delete event item %s", e->publicID().c_str());
						delete item;
						break;
					}
				}
			case OP_UPDATE:
				{
					EventTreeItem* item = (EventTreeItem*)findEvent(e->publicID());
					if ( !item )
						item = (EventTreeItem*)addEvent(e, true);
					else
						updateHideState(item);

					if ( item ) {
						Event* event = static_cast<Event*>(item->object());

						OriginTreeItem *originItem = findOrigin(event->preferredOriginID());
						OriginPtr preferredOrigin;

						if ( originItem ) {
							if ( originItem->parent()->parent() != item ) {
								int index = originItem->parent()->indexOfChild(originItem);
								SEISCOMP_DEBUG("Reparent originItem (update Event), index(%d)", index);
								if ( index >= 0 ) {
									QTreeWidgetItem *taken = originItem->parent()->takeChild(index);
									if ( taken ) {
										item->addOriginItem(taken);
										item->resort();
									}
								}
							}
							else if ( item->child(0) != originItem )
								item->resort();
						}
						else {
							preferredOrigin = Origin::Find(event->preferredOriginID());
							if (!preferredOrigin && _reader) {
								preferredOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), event->preferredOriginID()));
								if ( preferredOrigin && _withOrigins )
									addOrigin(preferredOrigin.get(), item, true);
							}
						}

						MagnitudePtr nm;
						if ( !event->preferredMagnitudeID().empty() ) {
							nm = Magnitude::Find(event->preferredMagnitudeID());
							if ( !nm && _reader )
								nm = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
						}

						updateEventProcessColumns(item, true);
						item->update(this);

						if ( _withFocalMechanisms ) {
							FocalMechanismTreeItem *fmItem = findFocalMechanism(event->preferredFocalMechanismID());
							FocalMechanismPtr preferredFM;
							OriginPtr derivedOrigin;

							if ( fmItem ) {
								if ( fmItem->parent()->parent() != item ) {
									int index = fmItem->parent()->indexOfChild(fmItem);
									SEISCOMP_DEBUG("Reparent originItem (update Event), index(%d)", index);
									if ( index >= 0 ) {
										QTreeWidgetItem *taken = fmItem->parent()->takeChild(index);
										if ( taken ) {
											item->addFocalMechanismItem(taken);
											item->resort();
										}
									}
								}
								else if ( item->child(0) != fmItem )
									item->resort();
							}
							else {
								preferredFM = FocalMechanism::Find(event->preferredFocalMechanismID());
								if ( !preferredFM && _reader )
									preferredFM = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), event->preferredFocalMechanismID()));

								if ( preferredFM && _reader ) {
									if ( preferredFM->momentTensorCount() == 0 )
										_reader->loadMomentTensors(preferredFM.get());
								}

								if ( preferredFM && (preferredFM->momentTensorCount() > 0) ) {
									derivedOrigin = Origin::Find(preferredFM->momentTensor(0)->derivedOriginID());
									if ( !derivedOrigin && _reader ) {
										derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), preferredFM->momentTensor(0)->derivedOriginID()));
										_reader->loadMagnitudes(derivedOrigin.get());
									}
								}

								if ( preferredFM )
									addFocalMechanism(preferredFM.get(), item);
							}

							item->update(this);
						}

						emit eventUpdatedInList(event);
					}
				}
				break;
			default:
				break;
		}

		Comment *c = Comment::Cast(n->object());
		if ( c != NULL ) {
			EventTreeItem* item = (EventTreeItem*)findEvent(n->parentID());
			if ( item ) item->update(this);
		}

		_treeWidget->setUpdatesEnabled(true);
		return;
	}

	if ( _withOrigins ) {
		OriginReference* ref = OriginReference::Cast(n->object());
		if ( ref != NULL ) {
			switch ( n->operation() ) {
				case OP_ADD:
				{
					EventTreeItem* eventItem = (EventTreeItem*)findEvent(n->parentID());
					if ( eventItem ) {
						SEISCOMP_INFO("found eventitem with publicID '%s', registered(%d)", eventItem->object()->publicID().c_str(), eventItem->object()->registered());
						OriginTreeItem* originItem = findOrigin(ref->originID());
						if ( originItem && originItem->parent()->parent() != eventItem ) {
							int index = originItem->parent()->indexOfChild(originItem);
							SEISCOMP_DEBUG("Reparent originItem (add OriginReference), index(%d)", index);
							if ( index >= 0 ) {
								QTreeWidgetItem *taken = originItem->parent()->takeChild(index);
								if ( taken ) {
									eventItem->addOriginItem(taken);
									eventItem->resort();
									eventItem->update(this);
								}
							}
						}
						if ( !_checkEventAgency ) updateHideState(eventItem);
					}
					else {
						OriginTreeItem* originItem = findOrigin(ref->originID());
						if ( originItem )
							delete originItem;
					}
					break;
				}
				case OP_REMOVE:
				{
					EventTreeItem* eventItem = (EventTreeItem*)findEvent(n->parentID());
					if ( eventItem ) {
						OriginTreeItem* originItem = findOrigin(ref->originID());
						if ( originItem && originItem->parent()->parent() == eventItem ) {
							int index = originItem->parent()->indexOfChild(originItem);
							SEISCOMP_DEBUG("Reparent originItem (remove OriginReference), index(%d)", index);
							if ( index >= 0 ) {
								QTreeWidgetItem *taken = originItem->parent()->takeChild(index);
								if ( taken ) {
									eventItem = (EventTreeItem*)_unassociatedEventItem;
									if ( eventItem )
										eventItem->addOriginItem(taken);
									else
										delete taken;
								}
							}
						}
						if ( !_checkEventAgency ) updateHideState(eventItem);
					}
					break;
				}
				default:
					break;
			};

			_treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	if ( _withFocalMechanisms ) {
		FocalMechanismReference* fm_ref = FocalMechanismReference::Cast(n->object());
		if ( fm_ref != NULL ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						EventTreeItem* eventItem = (EventTreeItem*)findEvent(n->parentID());
						if ( eventItem ) {
							SEISCOMP_INFO("found eventitem with publicID '%s', registered(%d)", eventItem->object()->publicID().c_str(), eventItem->object()->registered());
							FocalMechanismTreeItem* fmItem = findFocalMechanism(fm_ref->focalMechanismID());
							if ( fmItem && fmItem->parent()->parent() != eventItem ) {
								int index = fmItem->parent()->indexOfChild(fmItem);
								SEISCOMP_DEBUG("Reparent fmItem (add FocalMechanismReference), index(%d)", index);
								if ( index >= 0 ) {
									QTreeWidgetItem *taken = fmItem->parent()->takeChild(index);
									if ( taken ) {
										eventItem->addFocalMechanismItem(taken);
										eventItem->resort();
										eventItem->update(this);
									}
								}
							}
							else if ( fmItem == NULL ) {
								FocalMechanismPtr fm = FocalMechanism::Find(fm_ref->focalMechanismID());
								OriginPtr derivedOrigin;
								if ( !fm && _reader )
									fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), fm_ref->focalMechanismID()));

								if ( fm && _reader ) {
									if ( fm->momentTensorCount() == 0 )
										_reader->loadMomentTensors(fm.get());
								}

								if ( fm && (fm->momentTensorCount() > 0) ) {
									derivedOrigin = Origin::Find(fm->momentTensor(0)->derivedOriginID());
									if ( !derivedOrigin && _reader ) {
										derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), fm->momentTensor(0)->derivedOriginID()));
										_reader->loadMagnitudes(derivedOrigin.get());
									}
								}

								if ( fm ) {
									addFocalMechanism(fm.get(), eventItem);
									eventItem->update(this);
								}
							}
						}
						else {
							FocalMechanismTreeItem* fmItem = findFocalMechanism(fm_ref->focalMechanismID());
							if ( fmItem )
								delete fmItem;
						}
					}
					break;
				case OP_REMOVE:
				{
					EventTreeItem* eventItem = (EventTreeItem*)findEvent(n->parentID());
					if ( eventItem ) {
						FocalMechanismTreeItem* fmItem = findFocalMechanism(fm_ref->focalMechanismID());
						if ( fmItem && fmItem->parent()->parent() == eventItem ) {
							int index = fmItem->parent()->indexOfChild(fmItem);
							SEISCOMP_DEBUG("Remove fmItem (remove FocalMechanismReference), index(%d)", index);
							if ( index >= 0 ) {
								QTreeWidgetItem *taken = eventItem->takeFocalMechanism(index);
								if ( taken ) delete taken;
							}
						}
					}
					break;
				}
				default:
					break;
			};

			_treeWidget->setUpdatesEnabled(true);
			return;
		}
	}

	Magnitude* mag = Magnitude::Cast(n->object());
	if ( mag != NULL ) {
		for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
			EventTreeItem* item = (EventTreeItem*)_treeWidget->topLevelItem(i);
			if ( item->event() && item->event()->preferredMagnitudeID() == mag->publicID() ) {
				item->update(this);
				emit eventUpdatedInList(item->event());
			}
		}

		_treeWidget->setUpdatesEnabled(true);
		return;
	}

	Comment *comment = Comment::Cast(n->object());
	if ( comment != NULL ) {
		switch ( n->operation() ) {
			case OP_ADD:
			case OP_UPDATE:
				{
					EventTreeItem *eventItem = (EventTreeItem*)findEvent(n->parentID());
					if ( eventItem ) {
						if ( comment->text() == "published" )
							eventItem->setPublishState(true);
						updateEventProcessColumns(eventItem, true);
						eventItem->update(this);
					}
					else if ( _withOrigins ) {
						OriginTreeItem *origItem = findOrigin( n->parentID() );
						if ( origItem ) {
							// "OriginPublished" shall be superseded by "published"
							// but here we accept both
							if( comment->text() == "OriginPublished" ||
							    comment->text() == "published")
								origItem->setPublishState(true);
							updateOriginProcessColumns(origItem, true);
							origItem->update(this);

							eventItem = static_cast<EventTreeItem*>(origItem->parent()->parent());
							Origin *o = static_cast<Origin*>(origItem->object());
							Event *e = static_cast<Event*>(eventItem->object());
							if ( e && e->preferredOriginID() == o->publicID() )
								eventItem->update(this);
						}
					}
				}
				break;
			default:
				break;
		}

		_treeWidget->setUpdatesEnabled(true);
		return;
	}

	JournalEntry* je = JournalEntry::Cast(n->object());
	if ( je != NULL ) {
		if ( n->operation() == OP_ADD ) {
			if ( _commandWaitDialog == NULL ) {
				_treeWidget->setUpdatesEnabled(true);
				return;
			}

			CommandWaitDialog *dlg = static_cast<CommandWaitDialog*>(_commandWaitDialog);
			dlg->handle(je);
		}

		_treeWidget->setUpdatesEnabled(true);
		return;
	}

	_treeWidget->setUpdatesEnabled(true);
}


void EventListView::updateOrigin(Seiscomp::DataModel::Origin* origin) {
	EventParametersPtr ep;

	if ( _updateLocalEPInstance )
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	else
		ep = new EventParameters;

	if ( ep == NULL ) return;

	bool wasEnabled = Notifier::IsEnabled();

	if ( !_updateLocalEPInstance || origin->parent() == NULL ) {
		Notifier::Disable();
		ep->add(origin);
		Notifier::Enable();
	}

	// create the update notifier
	origin->update();

	SchemeTreeItem* item = findOrigin(origin->publicID());
	if ( item ) {
		item->update(this);
		EventTreeItem* parent = static_cast<EventTreeItem*>(item->parent()->parent());
		Event *e = static_cast<Event*>(parent->object());
		if ( e && e->preferredOriginID() == origin->publicID() ) {
			parent->update(this);
			emit eventUpdatedInList(e);
		}
	}

	if ( !_updateLocalEPInstance ) {
		// send the notifier
		MessagePtr msg = Notifier::GetMessage();
		if ( msg )
			//SCApp->sendMessage("LOGGING", msg.get());
			SCApp->sendMessage(SCApp->messageGroups().location.c_str(), msg.get());
	}

	Notifier::SetEnabled(wasEnabled);
}


void EventListView::insertOrigin(Seiscomp::DataModel::Origin* origin,
                                 Seiscomp::DataModel::Event* baseEvent,
                                 const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick> &changedPicks,
                                 const std::vector<Seiscomp::DataModel::AmplitudePtr>& newAmplitudes)
{
	EventParametersPtr ep;

	if ( _updateLocalEPInstance )
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	else
		ep = new EventParameters;

	if ( ep == NULL ) return;

	bool wasEnabled = Notifier::IsEnabled();

	Notifier::Enable();

	// Send picks
	for ( Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>::const_iterator it = changedPicks.begin();
	      it != changedPicks.end(); ++it ) {
		if ( it->second )
			ep->add(it->first.get());
		// TODO: handle updates
	}

	NotifierMessagePtr msg = Notifier::GetMessage();
	if ( msg && !_updateLocalEPInstance )
		//SCApp->sendMessage("LOGGING", msg.get());
		SCApp->sendMessage(SCApp->messageGroups().pick.c_str(), msg.get());

	for ( std::vector<Seiscomp::DataModel::AmplitudePtr>::const_iterator it = newAmplitudes.begin();
	      it != newAmplitudes.end(); ++it )
		ep->add((*it).get());

	msg = Notifier::GetMessage();
	if ( msg && !_updateLocalEPInstance )
		SCApp->sendMessage(SCApp->messageGroups().amplitude.c_str(), msg.get());

	// Insert origin to Eventparameters
	ep->add(origin);

	// When a baseOrigin was given insert the new origin to the event of
	// of the baseOrigin when available so the EventAssociationTool has
	// less work to do to find a appropriate Event
	OriginReferencePtr ref;

	if ( baseEvent ) {
		if ( !_updateLocalEPInstance ) {
			Notifier::Disable();
			ep->add(baseEvent);
			Notifier::Enable();
		}

		ref = new OriginReference(origin->publicID());
		baseEvent->add(ref.get());
	}

	// Send new origin and maybe the manual event assoziation
	msg = Notifier::GetMessage();
	if ( msg ) {
		//SCApp->sendMessage("LOGGING", msg.get());
		if ( !_updateLocalEPInstance ) SCApp->sendMessage(SCApp->messageGroups().location.c_str(), msg.get());
		if ( ref && baseEvent ) emit originReferenceAdded(baseEvent->publicID(), ref.get());
	}

	Notifier::SetEnabled(wasEnabled);

	for ( NotifierMessage::iterator it = msg->begin(); it != msg->end(); ++it )
		notifierAvailable((*it).get());
}


void EventListView::updateFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm) {
	EventParametersPtr ep;

	if ( _updateLocalEPInstance )
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	else
		ep = new EventParameters;

	if ( ep == NULL ) return;

	bool wasEnabled = Notifier::IsEnabled();

	if ( !_updateLocalEPInstance || fm->parent()->parent() == NULL ) {
		Notifier::Disable();
		ep->add(fm);
		Notifier::Enable();
	}

	// create the update notifier
	fm->update();

	SchemeTreeItem* item = findFocalMechanism(fm->publicID());
	if ( item ) {
		item->update(this);
		EventTreeItem* parent = static_cast<EventTreeItem*>(item->parent()->parent());
		Event *e = static_cast<Event*>(parent->object());
		if ( e && e->preferredOriginID() == fm->publicID() ) {
			parent->update(this);
			emit eventUpdatedInList(e);
		}
	}

	if ( !_updateLocalEPInstance ) {
		// send the notifier
		MessagePtr msg = Notifier::GetMessage();
		if ( msg )
			//SCApp->sendMessage("LOGGING", msg.get());
			SCApp->sendMessage(SCApp->messageGroups().focalMechanism.c_str(), msg.get());
	}

	Notifier::SetEnabled(wasEnabled);
}


void EventListView::insertFocalMechanism(Seiscomp::DataModel::FocalMechanism *fm,
                                         Seiscomp::DataModel::Event *event,
                                         Seiscomp::DataModel::Origin *origin) {
	EventParametersPtr ep;

	if ( _updateLocalEPInstance )
 		ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
	else
		ep = new EventParameters;

	if ( ep == NULL ) return;

	bool wasEnabled = Notifier::IsEnabled();

	Notifier::Enable();

	// Send derived origins
	for ( size_t i = 0; i < fm->momentTensorCount(); ++i ) {
		Origin *org = Origin::Find(fm->momentTensor(i)->derivedOriginID());
		if ( org )
			ep->add(org);
	}

	// Insert origin to Eventparameters
	ep->add(fm);

	// When an event was given insert the new fm to the event
	// so the EventAssociationTool has less work to  find an
	// appropriate event.
	if ( event ) {
		if ( !_updateLocalEPInstance ) {
			Notifier::Disable();
			ep->add(event);
			Notifier::Enable();
		}

		// Add focal mechanism to event
		FocalMechanismReferencePtr fmref = new FocalMechanismReference;
		fmref->setFocalMechanismID(fm->publicID());
		event->add(fmref.get());
	}


	// Send new fm and maybe the manual event association
	NotifierMessagePtr msg = Notifier::GetMessage();
	if ( msg ) {
		//SCApp->sendMessage("LOGGING", msg.get());
		if ( !_updateLocalEPInstance ) SCApp->sendMessage(SCApp->messageGroups().focalMechanism.c_str(), msg.get());
	}

	Notifier::SetEnabled(wasEnabled);

	for ( NotifierMessage::iterator it = msg->begin(); it != msg->end(); ++it )
		notifierAvailable((*it).get());
}


EventTreeItem* EventListView::findEvent(const std::string& publicID) {
	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		SchemeTreeItem* item = (SchemeTreeItem*)_treeWidget->topLevelItem(i);
		if ( item->object() && item->object()->publicID() == publicID ) {
			return (EventTreeItem*)item;
		}
	}

	return NULL;
}


OriginTreeItem* EventListView::findOrigin(const std::string& publicID) {
	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		QTreeWidgetItem* item = _treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			QTreeWidgetItem* citem = item->child(j);
			for ( int k = 0; k < citem->childCount(); ++k ) {
				SchemeTreeItem* schemeItem = (SchemeTreeItem*)citem->child(k);
				if ( schemeItem->object() && schemeItem->object()->publicID() == publicID ) {
					return (OriginTreeItem*)schemeItem;
				}
			}
		}
	}

	return NULL;
}


FocalMechanismTreeItem* EventListView::findFocalMechanism(const std::string &publicID) {
	for ( int i = 0; i < _treeWidget->topLevelItemCount(); ++i ) {
		QTreeWidgetItem* item = _treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			QTreeWidgetItem* citem = item->child(j);
			for ( int k = 0; k < citem->childCount(); ++k ) {
				SchemeTreeItem* schemeItem = (SchemeTreeItem*)citem->child(k);
				if ( schemeItem->object() && schemeItem->object()->publicID() == publicID ) {
					return (FocalMechanismTreeItem*)schemeItem;
				}
			}
		}
	}

	return NULL;
}


void EventListView::loadItem(QTreeWidgetItem *item) {
	if ( _blockSelection ) return;

	SchemeTreeItem* schemeItem = dynamic_cast<SchemeTreeItem*>(item);
	if ( schemeItem == NULL ) return;

	_blockSelection = true;

	Origin* o = Origin::Cast(schemeItem->object());
	if ( o ) {
		Event* event = NULL;
		SchemeTreeItem* parentItem = (SchemeTreeItem*)schemeItem->parent()->parent();
		if ( parentItem ) event = Event::Cast(parentItem->object());

		//readPicks(o);

		PublicObjectEvaluator::Instance().moveToFront(o->publicID().c_str());
		emit originSelected(o, event);

		_blockSelection = false;

		return;
	}

	FocalMechanism* fm = FocalMechanism::Cast(schemeItem->object());
	if ( fm ) {
		Event* event = NULL;
		SchemeTreeItem* parentItem = (SchemeTreeItem*)schemeItem->parent()->parent();
		if ( parentItem ) event = Event::Cast(parentItem->object());

		emit focalMechanismSelected(fm, event);

		_blockSelection = false;

		return;
	}

	Event* e = Event::Cast(schemeItem->object());
	if ( e ) {
		PublicObjectEvaluator::Instance().moveToFront(e->publicID().c_str());
		emit eventSelected(e);

		if ( _withOrigins ) {
			Origin* o = Origin::Find(e->preferredOriginID());
			if ( o ) {
				//readPicks(o);
				PublicObjectEvaluator::Instance().moveToFront(o->publicID().c_str());
				emit originSelected(o, e);
			}
		}
		else if ( _withFocalMechanisms ) {
			FocalMechanism* fm = FocalMechanism::Find(e->preferredFocalMechanismID());
			if ( fm )
				emit focalMechanismSelected(fm, e);
		}

		_blockSelection = false;

		return;
	}

	_blockSelection = false;
}


void EventListView::itemSelected(QTreeWidgetItem* item, int column) {
	if ( QApplication::keyboardModifiers() != Qt::NoModifier ) return;
	if ( column == _itemConfig.columnMap[COL_FM] ) {
		Event *ev = (Event*)item->data(column, Qt::UserRole+1).value<void*>();
		if ( ev ) {
			eventFMSelected(ev);
			return;
		}
	}

	loadItem(item);
}


void EventListView::itemPressed(QTreeWidgetItem *item, int column) {
	if ( QApplication::mouseButtons() != Qt::RightButton ) return;

	QMenu popup(this);
	QAction *load = popup.addAction("Select");
	popup.addSeparator();
	QAction *cc = popup.addAction("Copy cell to clipboard");
	QAction *ca = popup.addAction("Copy row to clipboard");
	QAction *cs = popup.addAction("Copy selected rows to clipboard");

	QAction *newEvent = NULL;
	QAction *splitOrg = NULL;

	OriginTreeItem *oitem = (OriginTreeItem*)item;
	Origin *org = NULL;

	if ( item->type() == ST_Origin && !_updateLocalEPInstance ) {
		org = oitem->origin();
		if ( org ) {
			popup.addSeparator();

			if ( oitem->parent()->parent() == _unassociatedEventItem )
				newEvent = popup.addAction("Form new event for origin");
			else
				splitOrg = popup.addAction("Split origin and create new event");
		}
	}

	QAction *action = popup.exec(QCursor::pos());
	if ( action == NULL ) return;

	if ( action == load )
		loadItem(item);
	else if ( action == cc ) {
		QClipboard *cb = QApplication::clipboard();
		if ( cb )
			cb->setText(item->text(column));
	}
	else if ( action == ca ) {
		QClipboard *cb = QApplication::clipboard();
		QString text;
		for ( int i = 0; i < item->columnCount(); ++i ) {
			if ( i > 0 ) text += ";";
			text += item->text(i);
		}

		if ( cb )
			cb->setText(text);
	}
	else if ( action == cs )
		SCApp->copyToClipboard(_treeWidget, _treeWidget->header());
	else if ( action == newEvent ) {
		if ( QMessageBox::question(
			this, "Form a new event",
			QString(
				"You requested to form a new event for origin %1. "
				"This command will modify the  database.\n"
				"Are you sure you want to continue?"
			).arg(org->publicID().c_str()),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
			) == QMessageBox::No ) {
			return;
		}

		sendJournalAndWait(org->publicID(), CMD_NEW_EVENT, "", SCApp->messageGroups().event.c_str());
	}
	else if ( action == splitOrg ) {
		if ( QMessageBox::question(
			this, "Split origin",
			QString(
				"You requested to remove origin %1 from its event and create "
				"a new event. "
				"This command will modify the  database.\n"
				"Are you sure you want to continue?"
			).arg(org->publicID().c_str()),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes
			) == QMessageBox::No ) {
			return;
		}

		EventTreeItem *eitem = (EventTreeItem*)oitem->parent()->parent();
		Event *e = eitem->event();
		if ( e )
			sendJournalAndWait(e->publicID(), CMD_SPLIT_ORIGIN, org->publicID(), SCApp->messageGroups().event.c_str());
		else {
			QMessageBox::critical(
				this, "Error",
				"Internal error."
			);
		}
	}
}


void EventListView::copyRowToClipboard() {
	QClipboard *cb = QApplication::clipboard();
	if ( !cb ) return;

	QString text;
	QList<QTreeWidgetItem *> items = _treeWidget->selectedItems();
	foreach ( QTreeWidgetItem* item, items ) {
		if ( !text.isEmpty() )
			text += '\n';

		for ( int i = 0; i < item->columnCount(); ++i ) {
			if ( i > 0 ) text += ";";
			text += item->text(i);
		}
	}

	cb->setText(text);

}


QList<Event*> EventListView::selectedEvents() {
	QList<Event*> events;
	if ( _blockSelection ) return events;

	_blockSelection = true;

	QList<QTreeWidgetItem *> items = _treeWidget->selectedItems();
	foreach ( QTreeWidgetItem* item, items ) {
		SchemeTreeItem* schemeItem = dynamic_cast<SchemeTreeItem*>(item);
		if ( schemeItem == NULL ) continue;

		Event* e = Event::Cast(schemeItem->object());
		events.push_back(e);
	}

	_blockSelection = false;

	return events;
}


Seiscomp::DataModel::Event *
EventListView::eventFromTreeItem(QTreeWidgetItem *item) const {
	SchemeTreeItem* schemeItem = dynamic_cast<SchemeTreeItem*>(item);
	if ( schemeItem == NULL ) return NULL;
	return Event::Cast(schemeItem->object());
}


int EventListView::eventCount() const {
	return _treeWidget->topLevelItemCount()-1;
}


void EventListView::setSortingEnabled(bool enable) {
	QHeaderView* header = _treeWidget->header();
	if ( header == NULL ) return;

	if ( enable ) {
		header->setSortIndicator(0, Qt::DescendingOrder);
		header->setSortIndicatorShown(true);
#if QT_VERSION >= 0x050000
		header->setSectionsClickable(true);
#else
		header->setClickable(true);
#endif
	}
	else {
		header->setSortIndicator(-1, Qt::DescendingOrder);
		header->setSortIndicatorShown(false);
#if QT_VERSION >= 0x050000
		header->setSectionsClickable(true);
#else
		header->setClickable(true);
#endif
	}
}


void EventListView::sortItems(int col) {
	int count = _treeWidget->topLevelItemCount();
	QHeaderView* header = _treeWidget->header();
	if ( header == NULL ) return;

	Qt::SortOrder order = header->sortIndicatorOrder();

	_treeWidget->blockSignals(true);

	QVector<SortItem> items(count);
	for ( int i = 0; i < items.count(); ++i ) {
		items[i].first = _treeWidget->takeTopLevelItem(0);
		items[i].second = col;
	}

	LessThan compare;

	if ( col == _itemConfig.columnMap[COL_OTIME] ||
	     col == _itemConfig.columnMap[COL_M] ||
	     col == _itemConfig.columnMap[COL_PHASES] ||
	     col == _itemConfig.columnMap[COL_RMS] ||
	     col == _itemConfig.columnMap[COL_LAT] ||
	     col == _itemConfig.columnMap[COL_LON] ||
	     col == _itemConfig.columnMap[COL_DEPTH] )
		compare = (order == Qt::AscendingOrder ? &itemLessThan : &itemGreaterThan);
	else
		compare = (order == Qt::AscendingOrder ? &itemTextLessThan : &itemTextGreaterThan);

	qStableSort(items.begin(), items.end(), compare);

	for ( int i = 0; i < items.count(); ++i ) {
		_treeWidget->addTopLevelItem(items[i].first);
		static_cast<SchemeTreeItem*>(items[i].first)->update(this);
	}

	updateHideState();

	_treeWidget->blockSignals(false);

}


void EventListView::setControlsHidden(bool hide) {
	_ui.frameControls->setHidden(hide);
}


void EventListView::setCustomControls(QWidget* widget) const {
	_ui.frameCustomControls->setLayout(new QHBoxLayout());
	_ui.frameCustomControls->layout()->setMargin(0);
	_ui.frameCustomControls->layout()->addWidget(widget);
}


void EventListView::setFMLinkEnabled(bool e) {
	_itemConfig.createFMLink = e;
}



void EventListView::moveSection(int from, int to) {
	QHeaderView* header = _treeWidget->header();
	if ( header == NULL ) return;

	int count = header->count();
	if ( from < 0 || to < 0 ) return;
	if ( from >= count || to >= count ) return;

	header->moveSection(from, to);
}


void EventListView::headerContextMenuRequested(const QPoint &pos) {
	int count = _treeWidget->header()->count();
	QAbstractItemModel *model = _treeWidget->header()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!_treeWidget->header()->isSectionHidden(i));

		if ( i == 0 ) actions[i]->setEnabled(false);
	}

	QAction *result = menu.exec(_treeWidget->header()->mapToGlobal(pos));
	if ( result == NULL ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	//std::cout << "switch visibility[" << section << "] = " << result->isChecked() << std::endl;
	_treeWidget->header()->setSectionHidden(section, !result->isChecked());
	//std::cout << "visibility[" << section << "] = " << !_treeWidget->header()->isSectionHidden(section) << std::endl;
}


bool EventListView::sendJournalAndWait(const std::string &objectID,
                                       const std::string &action,
                                       const std::string &params,
                                       const char *group) {
	if ( !sendJournal(objectID, action, params, group) )
		return false;

	if ( _commandWaitDialog == NULL ) {
		_commandWaitDialog = new CommandWaitDialog(this);
		_commandWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
		connect(_commandWaitDialog, SIGNAL(destroyed(QObject*)),
		        this, SLOT(waitDialogDestroyed(QObject*)));
		static_cast<CommandWaitDialog*>(_commandWaitDialog)->show();
	}

	static_cast<CommandWaitDialog*>(_commandWaitDialog)->setCommand(objectID, action);

	return true;
}


void EventListView::waitDialogDestroyed(QObject *o) {
	if ( _commandWaitDialog == o ) _commandWaitDialog = NULL;
}


void EventListView::itemEntered(QTreeWidgetItem *item, int column) {
	if ( column == _itemConfig.columnMap[COL_FM]
	  && item->data(column, Qt::UserRole+1).isValid() )
		setCursor(Qt::PointingHandCursor);
	else
		unsetCursor();
}


void EventListView::itemExpanded(QTreeWidgetItem *item) {
	if ( item->type() != ST_OriginGroup ) return;
	if ( _itemConfig.originScriptColumns.isEmpty() ) return;

	// If an origin group is expanded, raise its priority

	int rows = item->childCount();
	for ( int i = rows-1; i >= 0; --i ) {
		QTreeWidgetItem *oitem = item->child(i);
		PublicObjectEvaluator::Instance().moveToFront(oitem->text(_itemConfig.columnMap[COL_ID]));
	}
}


void EventListView::currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
	if ( !current ) return;
	if ( (current->type() == ST_Origin && !_itemConfig.originScriptColumns.isEmpty()) ||
	     (current->type() == ST_Event && !_itemConfig.eventScriptColumns.isEmpty())) {
		PublicObjectEvaluator::Instance().moveToFront(current->text(_itemConfig.columnMap[COL_ID]));
	}
}


void EventListView::evalResultAvailable(const QString &publicID,
                                        const QString &className,
                                        const QString &script,
                                        const QString &result) {
	std::string pid = publicID.toStdString();

	// Origin processing result
	if ( className == Origin::TypeInfo().className() ) {
		OriginTreeItem *item = findOrigin(pid);
		if ( item == NULL ) return;

		QHash<QString,int>::iterator it = _itemConfig.originScriptColumnMap.find(script);
		if ( it == _itemConfig.originScriptColumnMap.end() ) return;

		item->setText(it.value(), result);
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::NoBrush);

		// Update the event item
		EventTreeItem *eitem = static_cast<EventTreeItem*>(item->parent()->parent());
		if ( eitem->event() == NULL )
			return;

		// If it is the preferred item, copy the column states, unless the
		// column defines a specific event script
		if ( eitem->event()->preferredOriginID() == pid ) {
			for ( int i = 0; i < _itemConfig.originScriptColumns.size(); ++i ) {
				int pos = _itemConfig.originScriptColumns[i].pos;
				if ( _itemConfig.eventScriptPositions.contains(pos) )
					continue;
				eitem->setText(pos, item->text(pos));
				eitem->setBackground(pos, item->background(pos));
				eitem->setForeground(pos, item->foreground(pos));
			}
		}
	}
	// Origin processing result
	else if ( className == Event::TypeInfo().className() ) {
		EventTreeItem *item = findEvent(pid);
		if ( item == NULL ) return;

		QHash<QString,int>::iterator it = _itemConfig.eventScriptColumnMap.find(script);
		if ( it == _itemConfig.eventScriptColumnMap.end() ) return;

		item->setText(it.value(), result);
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::NoBrush);
	}
}


void EventListView::evalResultError(const QString &publicID,
                                    const QString &className,
                                    const QString &script,
                                    int error) {
	std::string pid = publicID.toStdString();

	// Origin processing result
	if ( className == Origin::TypeInfo().className() ) {
		OriginTreeItem *item = findOrigin(pid);
		if ( item == NULL ) return;

		QHash<QString,int>::iterator it = _itemConfig.originScriptColumnMap.find(script);
		if ( it == _itemConfig.originScriptColumnMap.end() ) return;

		// Error state
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::darkRed);

		item->setText(it.value(), "!");
		item->setToolTip(it.value(),
		                 QString("%1\n\n%2")
		                 .arg(script)
		                 .arg(PublicObjectEvaluator::Instance().errorMsg(error)));
	}
	// Event processing result
	else if ( className == Event::TypeInfo().className() ) {
		EventTreeItem *item = findEvent(pid);
		if ( item == NULL ) return;

		QHash<QString,int>::iterator it = _itemConfig.eventScriptColumnMap.find(script);
		if ( it == _itemConfig.eventScriptColumnMap.end() ) return;

		// Error state
		item->setBackground(it.value(), Qt::NoBrush);
		item->setForeground(it.value(), Qt::darkRed);

		item->setText(it.value(), "!");
		item->setToolTip(it.value(),
		                 QString("%1\n\n%2")
		                 .arg(script)
		                 .arg(PublicObjectEvaluator::Instance().errorMsg(error)));
	}
}


EventListViewRegionFilterDialog::EventListViewRegionFilterDialog(QWidget *parent,
                                                                 EventListView::Region *target,
                                                                 EventListView::FilterRegions *regionList)
: QDialog(parent), _target(target), _regionList(regionList) {
	_ui.setupUi(this);

	_ui.edMinLat->setText(QString::number(_target->minLat));
	_ui.edMaxLat->setText(QString::number(_target->maxLat));
	_ui.edMinLon->setText(QString::number(_target->minLong));
	_ui.edMaxLon->setText(QString::number(_target->maxLong));

	QValidator *valLat = new QDoubleValidator(-90,90,6,this);
	QValidator *valLong = new QDoubleValidator(-180,180,6,this);

	_ui.edMinLat->setValidator(valLat); _ui.edMaxLat->setValidator(valLat);
	_ui.edMinLon->setValidator(valLong); _ui.edMaxLon->setValidator(valLong);

	if ( _regionList->isEmpty() ) return;

	for ( int i = 0; i < _regionList->size(); ++i )
		_ui.cbRegions->addItem((*_regionList)[i].name);

	connect(_ui.cbRegions, SIGNAL(currentIndexChanged(const QString &)),
	        this, SLOT(regionSelectionChanged(const QString &)));

	connect(_ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(_ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}


void EventListViewRegionFilterDialog::regionSelectionChanged(const QString &text) {
	for ( int i = 0; i < _regionList->size(); ++i ) {
		if ( (*_regionList)[i].name == text ) {
			_ui.edMinLat->setText(QString::number((*_regionList)[i].minLat));
			_ui.edMaxLat->setText(QString::number((*_regionList)[i].maxLat));
			_ui.edMinLon->setText(QString::number((*_regionList)[i].minLong));
			_ui.edMaxLon->setText(QString::number((*_regionList)[i].maxLong));
			return;
		}
	}

	_ui.edMinLat->setText("");
	_ui.edMaxLat->setText("");
	_ui.edMinLon->setText("");
	_ui.edMaxLon->setText("");
}


void EventListViewRegionFilterDialog::showError(const QString &msg) {
	QMessageBox::critical(this, "Error", msg);
}


void EventListViewRegionFilterDialog::accept() {
	// Copy minimum latitude
	if ( _ui.edMinLat->text().isEmpty() ) {
		showError("Minimum latitude must not be empty.");
		_ui.edMinLat->setFocus();
		return;
	}
	_target->minLat = _ui.edMinLat->text().toDouble();

	// Copy maximum latitude
	if ( _ui.edMaxLat->text().isEmpty() ) {
		showError("Maximum latitude must not be empty.");
		_ui.edMaxLat->setFocus();
		return;
	}
	_target->maxLat = _ui.edMaxLat->text().toDouble();

	// Copy minimum longitude
	if ( _ui.edMinLon->text().isEmpty() ) {
		showError("Minimum longitude must not be empty.");
		_ui.edMinLon->setFocus();
		return;
	}
	_target->minLong = _ui.edMinLon->text().toDouble();

	// Copy maximum longitude
	if ( _ui.edMaxLon->text().isEmpty() ) {
		showError("Maximum longitude must not be empty.");
		_ui.edMaxLon->setFocus();
		return;
	}
	_target->maxLong = _ui.edMaxLon->text().toDouble();

	QDialog::accept();
}


}
}
