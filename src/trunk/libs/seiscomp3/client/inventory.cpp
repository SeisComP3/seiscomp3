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


#include <seiscomp3/client/inventory.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/responsefap.h>
#include <seiscomp3/datamodel/inventory_package.h>

#include <set>
#include <iostream>


namespace Seiscomp {
namespace Client {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory Inventory::_instance;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationLocation::StationLocation()
: latitude(999.0)
, longitude(999.0)
, elevation(999.0) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationLocation::StationLocation(double lat, double lon, double elv)
: latitude(lat)
, longitude(lon)
, elevation(elv) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::Inventory() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory* Inventory::Instance() {
	return &_instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::load(const char *filename) {
	IO::XMLArchive ar;

	if ( !ar.open(filename) )
		throw Core::GeneralException(std::string(filename) + " not found");

	ar >> _inventory;
	ar.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::load(DataModel::DatabaseReader* reader) {
	if ( reader == NULL ) return;

	//_inventory = reader->loadInventory();

	loadStations(reader);

	if ( _inventory == NULL ) return;

	DataModel::DatabaseIterator it;

	// Read station groups
	std::map<int, DataModel::StationGroupPtr> groups;
	it = reader->getObjects(_inventory.get(), DataModel::StationGroup::TypeInfo());

	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::StationGroupPtr group = DataModel::StationGroup::Cast(obj);
		if ( group ) {
			groups.insert(std::make_pair(it.oid(), group));
			_inventory->add(group.get());
		}
	}

	it.close();

	// Read station references
	it = reader->getObjects(NULL, DataModel::StationReference::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::StationReferencePtr staref = DataModel::StationReference::Cast(obj);
		if ( staref ) {
			std::map<int, DataModel::StationGroupPtr>::iterator p = groups.find(it.parentOid());
			if ( p != groups.end() )
				p->second->add(staref.get());
			else
				std::cerr << "cannot find StationReference parent StationGroup with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read aux devices
	std::map<int, DataModel::AuxDevicePtr> auxs;
	it = reader->getObjects(_inventory.get(), DataModel::AuxDevice::TypeInfo());

	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::AuxDevicePtr aux = DataModel::AuxDevice::Cast(obj);
		if ( aux ) {
			auxs.insert(std::make_pair(it.oid(), aux));
			_inventory->add(aux.get());
		}
	}

	it.close();

	// Read aux sources
	it = reader->getObjects(NULL, DataModel::AuxSource::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::AuxSourcePtr auxsource = DataModel::AuxSource::Cast(obj);
		if ( auxsource ) {
			std::map<int, DataModel::AuxDevicePtr>::iterator p = auxs.find(it.parentOid());
			if ( p != auxs.end() )
				p->second->add(auxsource.get());
			else
				std::cerr << "cannot find AuxSource parent AuxDevice with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read sensors
	std::map<int, DataModel::SensorPtr> sensors;
	it = reader->getObjects(_inventory.get(), DataModel::Sensor::TypeInfo());

	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::SensorPtr sensor = DataModel::Sensor::Cast(obj);
		if ( sensor ) {
			sensors.insert(std::make_pair(it.oid(), sensor));
			_inventory->add(sensor.get());
		}
	}

	it.close();

	// Read sensor calibrations
	it = reader->getObjects(NULL, DataModel::SensorCalibration::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::SensorCalibrationPtr sencal = DataModel::SensorCalibration::Cast(obj);
		if ( sencal ) {
			std::map<int, DataModel::SensorPtr>::iterator p = sensors.find(it.parentOid());
			if ( p != sensors.end() )
				p->second->add(sencal.get());
			else
				std::cerr << "cannot find SensorCalibration parent Sensor with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read dataloggers
	std::map<int, DataModel::DataloggerPtr> dataloggers;
	it = reader->getObjects(_inventory.get(), DataModel::Datalogger::TypeInfo());

	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::DataloggerPtr datalogger = DataModel::Datalogger::Cast(obj);
		if ( datalogger ) {
			dataloggers.insert(std::make_pair(it.oid(), datalogger));
			_inventory->add(datalogger.get());
		}
	}

	it.close();

	// Read datalogger calibrations
	it = reader->getObjects(NULL, DataModel::DataloggerCalibration::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::DataloggerCalibrationPtr dlcal = DataModel::DataloggerCalibration::Cast(obj);
		if ( dlcal ) {
			std::map<int, DataModel::DataloggerPtr>::iterator p = dataloggers.find(it.parentOid());
			if ( p != dataloggers.end() )
				p->second->add(dlcal.get());
			else
				std::cerr << "cannot find DataloggerCalibration parent DataLogger with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read decimations calibrations
	it = reader->getObjects(NULL, DataModel::Decimation::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::DecimationPtr deci = DataModel::Decimation::Cast(obj);
		if ( deci ) {
			std::map<int, DataModel::DataloggerPtr>::iterator p = dataloggers.find(it.parentOid());
			if ( p != dataloggers.end() )
				p->second->add(deci.get());
			else
				std::cerr << "cannot find Decimation parent DataLogger with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read poles and zeros
	it = reader->getObjects(_inventory.get(), DataModel::ResponsePAZ::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::ResponsePAZPtr paz = DataModel::ResponsePAZ::Cast(obj);
		if ( paz )
			_inventory->add(paz.get());
	}
	it.close();

	// Read FIR responses
	it = reader->getObjects(_inventory.get(), DataModel::ResponseFIR::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::ResponseFIRPtr fir = DataModel::ResponseFIR::Cast(obj);
		if ( fir )
			_inventory->add(fir.get());
	}
	it.close();

	// Read polynomial responses
	it = reader->getObjects(_inventory.get(), DataModel::ResponsePolynomial::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::ResponsePolynomialPtr poly = DataModel::ResponsePolynomial::Cast(obj);
		if ( poly )
			_inventory->add(poly.get());
	}
	it.close();

	// Read FAP responses
	if ( reader->supportsVersion<0,8>() ) {
		it = reader->getObjects(_inventory.get(), DataModel::ResponseFAP::TypeInfo());
		for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
			DataModel::ResponseFAPPtr fap = DataModel::ResponseFAP::Cast(obj);
			if ( fap )
				_inventory->add(fap.get());
		}
		it.close();
	}

	// Read IIR responses
	if ( reader->supportsVersion<0,10>() ) {
		it = reader->getObjects(_inventory.get(), DataModel::ResponseIIR::TypeInfo());
		for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
			DataModel::ResponseIIRPtr iir = DataModel::ResponseIIR::Cast(obj);
			if ( iir )
				_inventory->add(iir.get());
		}
		it.close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::loadStations(DataModel::DatabaseReader* reader) {
	if ( reader == NULL ) return;

	_inventory = new DataModel::Inventory();
	DataModel::DatabaseIterator it;

	// Read networks
	std::map<int, DataModel::NetworkPtr> networks;
	it = reader->getObjects(_inventory.get(), DataModel::Network::TypeInfo());

	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::NetworkPtr network = DataModel::Network::Cast(obj);
		if ( network ) {
			networks.insert(std::make_pair(it.oid(), network));
			_inventory->add(network.get());
		}
	}

	it.close();

	// Read stations
	std::map<int, DataModel::StationPtr> stations;
	it = reader->getObjects(NULL, DataModel::Station::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::StationPtr station = DataModel::Station::Cast(obj);
		if ( station ) {
			stations.insert(std::make_pair(it.oid(), station));
			std::map<int, DataModel::NetworkPtr>::iterator p = networks.find(it.parentOid());
			if ( p != networks.end() )
				p->second->add(station.get());
			else
				std::cerr << "cannot find Stations parent Network with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read sensor locations
	std::map<int, DataModel::SensorLocationPtr> sensorLocations;
	it = reader->getObjects(NULL, DataModel::SensorLocation::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::SensorLocationPtr sensorLocation = DataModel::SensorLocation::Cast(obj);
		if ( sensorLocation ) {
			sensorLocations.insert(std::make_pair(it.oid(), sensorLocation));
			std::map<int, DataModel::StationPtr>::iterator p = stations.find(it.parentOid());
			if ( p != stations.end() )
				p->second->add(sensorLocation.get());
			else
				std::cerr << "cannot find SensorLocations parent Station with id " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read streams
	// Note, prior to version 0.10 the stream type was not a PublicObject and
	// therefore we must not join with table PublicObject
	it = reader->getObjects(NULL, DataModel::Stream::TypeInfo(), reader->isLowerVersion<0,10>());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::StreamPtr stream = DataModel::Stream::Cast(obj);
		if ( stream ) {
			std::map<int, DataModel::SensorLocationPtr>::iterator p = sensorLocations.find(it.parentOid());
			if ( p != sensorLocations.end() )
				p->second->add(stream.get());
			else
				std::cerr << "cannot find Stream parent SensorLocation with " << it.parentOid() << std::endl;
		}
	}

	it.close();

	// Read auxStreams
	it = reader->getObjects(NULL, DataModel::AuxStream::TypeInfo());
	for ( DataModel::ObjectPtr obj; (obj = *it); ++it ) {
		DataModel::AuxStreamPtr auxStream = DataModel::AuxStream::Cast(obj);
		if ( auxStream ) {
//			std::cout << "_oid=" << it.oid() << ", _parent_oid=" << it.parentOid() << ", " << seisStream->code() << std::endl;
			std::map<int, DataModel::SensorLocationPtr>::iterator p = sensorLocations.find(it.parentOid());
			if ( p != sensorLocations.end() )
				p->second->add(auxStream.get());
			else
				std::cerr << "cannot find AuxStream parent SensorLocation with " << it.parentOid() << std::endl;
		}
	}

	it.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Inventory::filter(const Util::StringFirewall *networkTypeFW,
                      const Util::StringFirewall *stationTypeFW) {
	int filtered = 0;

	if ( !_inventory ) return filtered;

	for ( size_t n = 0; n < _inventory->networkCount(); ) {
		DataModel::Network *net = _inventory->network(n);
		const std::string &net_type = net->type();

		// Type blocked?
		if ( (networkTypeFW != NULL) && networkTypeFW->isDenied(net_type) ) {
			_inventory->removeNetwork(n);
			filtered += net->stationCount();
			continue;
		}

		++n;

		for ( size_t s = 0; s < net->stationCount(); ) {
			DataModel::Station *sta = net->station(s);
			const std::string &sta_type = sta->type();

			// Type not blocked?
			if ( (stationTypeFW != NULL) && stationTypeFW->isAllowed(sta_type) ) {
				++s;
				continue;
			}

			++filtered;

			// Remove station
			net->removeStation(s);
		}
	}

	return filtered;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::setInventory(DataModel::Inventory *inv) {
	_inventory = inv;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationLocation Inventory::stationLocation(const std::string& networkCode,
                                           const std::string& stationCode,
                                           const Core::Time& time) const {
	if ( _inventory == NULL )
		throw Core::ValueException("inventory is empty");

	DataModel::Station* station = getStation(networkCode, stationCode, time);
	
	if ( station == NULL )
		throw Core::ValueException("station [" + networkCode + "." + stationCode + "] not found");

	return StationLocation(station->latitude(),
	                       station->longitude(),
	                       station->elevation());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Station* Inventory::getStation(const std::string &networkCode,
                                          const std::string &stationCode,
                                          const Core::Time &time,
                                          DataModel::InventoryError *error) const {
	return DataModel::getStation(_inventory.get(), networkCode, stationCode, time, error);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::SensorLocation*
Inventory::getSensorLocation(const std::string &networkCode,
                             const std::string &stationCode,
                             const std::string &locationCode,
                             const Core::Time &time,
                             DataModel::InventoryError *error) const {
	return DataModel::getSensorLocation(_inventory.get(), networkCode, stationCode,
	                                    locationCode, time, error);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ThreeComponents
Inventory::getThreeComponents(const std::string& networkCode,
                              const std::string& stationCode,
                              const std::string& locationCode,
                              const std::string& channelCode,
                              const Core::Time &time) const {
	DataModel::SensorLocation *loc = getSensorLocation(networkCode, stationCode, locationCode, time);
	if ( loc == NULL )
		throw Core::ValueException("sensor location not found");

	DataModel::ThreeComponents tc;
	if ( channelCode.size() >= 3 )
		DataModel::getThreeComponents(tc, loc, channelCode.substr(0, channelCode.size()-1).c_str(), time);
	else
		DataModel::getThreeComponents(tc, loc, channelCode.c_str(), time);

	return tc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Stream*
Inventory::getStream(const std::string &networkCode,
                     const std::string &stationCode,
                     const std::string &locationCode,
                     const std::string &channelCode,
                     const Core::Time &time,
                     DataModel::InventoryError *error) const {
	return DataModel::getStream(_inventory.get(), networkCode, stationCode,
	                            locationCode, channelCode, time, error);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Inventory::getAllStations(StationList& stationList,
                              const Core::Time& t) {
	if ( _inventory == NULL )
		return 0;

	std::set<std::string> networkMap;

	for ( size_t i = 0; i < _inventory->networkCount(); ++i ) {
		DataModel::Network* network = _inventory->network(i);

		if ( networkMap.find(network->code()) != networkMap.end() ) continue;
		try {
			if ( network->end() < t ) continue;
		}
		catch (...) {}

		if ( network->start() > t ) continue;

		networkMap.insert(network->code());

		std::set<std::string> stationMap;

		for ( size_t j = 0; j < network->stationCount(); ++j ) {
			DataModel::Station* station = network->station(j);

			if ( stationMap.find(station->code()) != stationMap.end() ) continue;

			try {
				if ( station->end() < t ) continue;
			}
			catch (...) {}

			if ( station->start() > t ) continue;

			stationList.push_back(station);
			stationMap.insert(station->code());
		}
	}

	return stationList.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Station* Inventory::getStation(const DataModel::Pick* pick) const {
	return DataModel::getStation(_inventory.get(), pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::SensorLocation* Inventory::getSensorLocation(const DataModel::Pick *pick) const {
	return DataModel::getSensorLocation(_inventory.get(), pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ThreeComponents Inventory::getThreeComponents(const DataModel::Pick *pick) const {
	DataModel::SensorLocation *loc = getSensorLocation(pick);
	if ( loc == NULL )
		throw Core::ValueException("sensor location not found");

	DataModel::ThreeComponents tc;
	if ( pick->waveformID().channelCode().size() >= 3 )
		DataModel::getThreeComponents(tc, loc, pick->waveformID().channelCode().substr(0, pick->waveformID().channelCode().size()-1).c_str(), pick->time().value());
	else
		DataModel::getThreeComponents(tc, loc, pick->waveformID().channelCode().c_str(), pick->time().value());

	return tc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Inventory::getGain(const std::string& networkCode,
                          const std::string& stationCode,
                          const std::string& locationCode,
                          const std::string& channelCode,
                          const Core::Time& t) {
	if ( channelCode.size() != 3 )
		throw Core::ValueException("invalid channel code");

	DataModel::Stream *stream = getStream(networkCode, stationCode, locationCode, channelCode, t);
	if ( !stream )
		throw Core::ValueException("stream not found");

	return stream->gain();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Inventory* Inventory::inventory() {
	return _inventory.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
