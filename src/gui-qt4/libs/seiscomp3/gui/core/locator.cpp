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



#include <seiscomp3/gui/core/locator.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/seismology/locsat.h>


namespace Seiscomp {
namespace Gui {


namespace {

struct comparePick {
	bool operator()(const Seismology::LocatorInterface::PickItem &first,
	                const Seismology::LocatorInterface::PickItem &second) const {
		return first.pick->time().value() < second.pick->time().value();
	}
};


}


DataModel::Origin* relocate(Seismology::LocatorInterface *locator, DataModel::Origin* origin) {
	if ( !locator )
		throw Core::GeneralException("No locator type set.");

	DataModel::Origin* newOrg = NULL;
	std::string errorMsg;

	try {
		newOrg = locator->relocate(origin);
		if ( newOrg )
			return newOrg;

		errorMsg = "The Relocation failed for some reason.";
	}
	catch ( Core::GeneralException& e ) {
		errorMsg = e.what();
	}

	// if no initial location is supported throw the error
	// after the first try
	if ( !locator->supports(Seismology::LocatorInterface::InitialLocation) )
		throw Core::GeneralException(errorMsg);

	Seismology::LocatorInterface::PickList picks;
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::Arrival* arrival = origin->arrival(i);
		try {
			if ( arrival->weight() < 0.5 ) continue;
		}
		catch ( ... ) {}

		DataModel::Pick* pick = locator->getPick(arrival);
		if ( !pick )
			throw Core::GeneralException("pick '" + arrival->pickID() + "' not found");

		picks.push_back(Seismology::LocatorInterface::PickItem(pick,
		                                                       Seismology::LocatorInterface::F_ALL));
	}

	if ( picks.empty() )
		throw Core::GeneralException("No picks given to relocate");

	std::sort(picks.begin(), picks.end(), comparePick());
	DataModel::SensorLocation *sloc = locator->getSensorLocation(picks.front().pick.get());
	if ( !sloc )
		throw Core::GeneralException("station '" + picks.front().pick->waveformID().networkCode() +
		                             "." + picks.front().pick->waveformID().stationCode() + "' not found");

	DataModel::OriginPtr tmp = DataModel::Origin::Create();
	*tmp = *origin;
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		DataModel::ArrivalPtr ar = new DataModel::Arrival(*origin->arrival(i));
		tmp->add(ar.get());
	}

	tmp->setLatitude(sloc->latitude());
	tmp->setLongitude(sloc->longitude());
	tmp->setDepth(DataModel::RealQuantity(11.0));
	tmp->setTime(picks.front().pick->time());

	newOrg = locator->relocate(tmp.get());

	if ( newOrg )
		return newOrg;

	throw Core::GeneralException(errorMsg);
}


}
}
