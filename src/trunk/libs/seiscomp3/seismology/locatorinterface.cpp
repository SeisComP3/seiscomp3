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


#include <seiscomp3/seismology/locatorinterface.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/core/interfacefactory.ipp>


#define SEISCOMP_COMPONENT Locator


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Seismology::LocatorInterface, SC_SYSTEM_CORE_API);

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Seismology {


SensorLocationDelegate::SensorLocationDelegate() {}


LocatorInterface::LocatorInterface() {
	_usingFixedDepth = false;
	_enableDistanceCutOff = false;
	_ignoreInitialLocation = false;
}


LocatorInterface::~LocatorInterface() {
}


LocatorInterface* LocatorInterface::Create(const char* service) {
	if ( service == NULL ) return NULL;
	return LocatorInterfaceFactory::Create(service);
}


const std::string &LocatorInterface::name() const {
	return _name;
}


void LocatorInterface::setSensorLocationDelegate(SensorLocationDelegate *delegate) {
	_sensorLocationDelegate = delegate;
}


LocatorInterface::IDList LocatorInterface::parameters() const {
	return IDList();
}


std::string LocatorInterface::parameter(const std::string &name) const {
	return "";
}


bool LocatorInterface::setParameter(const std::string &name,
                                    const std::string &value) {
	return false;
}


std::string LocatorInterface::lastMessage(MessageType) const {
	return "";
}


bool LocatorInterface::supports(Capability c) const {
	return (capabilities() & c) > 0;
}


void LocatorInterface::setFixedDepth(double depth, bool use) {
	_usingFixedDepth = use;
	_fixedDepth = depth;
}

void LocatorInterface::useFixedDepth(bool use) {
	_usingFixedDepth = use;
}


void LocatorInterface::releaseDepth() {
	useFixedDepth(false);
}


void LocatorInterface::setDistanceCutOff(double distance) {
	_enableDistanceCutOff = true;
	_distanceCutOff = distance;
}


void LocatorInterface::releaseDistanceCutOff() {
	_enableDistanceCutOff = false;
}


Pick* LocatorInterface::getPick(Arrival* arrival) const {
	DataModel::Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
	if ( pick == NULL )
		return NULL;

	return pick;
}


SensorLocation* LocatorInterface::getSensorLocation(Pick* pick) const {
	if ( _sensorLocationDelegate )
		return _sensorLocationDelegate->getSensorLocation(pick);

	Inventory *inv = Inventory::Cast(PublicObject::Find("Inventory"));
	return DataModel::getSensorLocation(inv, pick);
}


PickNotFoundException::PickNotFoundException()
: Core::GeneralException() {}


PickNotFoundException::PickNotFoundException(const std::string& str)
: Core::GeneralException(str) {}


LocatorException::LocatorException()
: Core::GeneralException() {}


LocatorException::LocatorException(const std::string& str)
: Core::GeneralException(str) {}


StationNotFoundException::StationNotFoundException()
: Core::GeneralException() {}


StationNotFoundException::StationNotFoundException(const std::string& str)
: Core::GeneralException(str) {}


} // of namespace Seismology
} // of namespace Seiscomp
