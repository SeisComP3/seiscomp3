/***************************************************************************
 *   Copyright (C) by GFZ Potsdam and gempa GmbH                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_EVENTINFO_H__
#define __SEISCOMP_APPLICATIONS_EVENTINFO_H__


#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/publicobjectcache.h>

#include "constraints.h"
#include "config.h"

#include <list>
#include <set>
#include <map>
#include <string>


namespace Seiscomp {

namespace Client {


DEFINE_SMARTPOINTER(EventInformation);

struct EventInformation : public Seiscomp::Core::BaseObject {
	typedef DataModel::PublicObjectCache Cache;

	EventInformation(Cache *cache, Config *cfg);

	EventInformation(Cache *cache, Config *cfg,
	                 DataModel::DatabaseQuery *q, const std::string &eventID);

	EventInformation(Cache *cache, Config *cfg,
	                 DataModel::DatabaseQuery *q, DataModel::EventPtr &event);

	~EventInformation();

	//! Loads an event from the database
	void load(DataModel::DatabaseQuery *q,
	          const std::string &eventID);

	void load(DataModel::DatabaseQuery *q,
	          DataModel::EventPtr &event);

	void loadAssocations(DataModel::DatabaseQuery *q);

	//! Returns the number of matching picks
	size_t matchingPicks(DataModel::DatabaseQuery *q, DataModel::Origin *o);

	bool valid() const;

	bool associate(DataModel::Origin *o);
	bool associate(DataModel::FocalMechanism *fm);

	bool addJournalEntry(DataModel::JournalEntry *e);

	bool setEventName(DataModel::JournalEntry *e, std::string &error);
	bool setEventOpComment(DataModel::JournalEntry *e, std::string &error);

	void insertPick(DataModel::Pick *p);

	Cache                                 *cache;
	Config                                *cfg;

	typedef std::multimap<std::string, DataModel::PickPtr> PickAssociation;
	bool                                   created;
	std::set<std::string>                  pickIDs;
	PickAssociation                        picks;
	DataModel::EventPtr                    event;
	DataModel::OriginPtr                   preferredOrigin;
	DataModel::FocalMechanismPtr           preferredFocalMechanism;
	DataModel::MagnitudePtr                preferredMagnitude;
	std::list<DataModel::JournalEntryPtr>  journal;

	Constraints                            constraints;

	bool                                   aboutToBeRemoved;
	bool                                   dirtyPickSet;
};


}

}


#endif
