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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!

#define SEISCOMP_COMPONENT StrongMotionReader
#include <seiscomp3/datamodel/strongmotion/databasereader.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/strongmotion/strongmotionparameters_package.h>
#include <seiscomp3/datamodel/comment.h>
#include <boost/bind.hpp>

using namespace std;

namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


StrongMotionReader::StrongMotionReader(Seiscomp::IO::DatabaseInterface* dbDriver)
: DatabaseQuery(dbDriver) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionReader::~StrongMotionReader() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongMotionParameters* StrongMotionReader::loadStrongMotionParameters() {
	if ( !validInterface() ) return NULL;

	StrongMotionParameters *strongMotionParameters = new StrongMotionParameters;

	load(strongMotionParameters);

	return strongMotionParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::load(StrongMotionParameters* strongMotionParameters) {
	size_t count = 0;

	count += loadSimpleFilters(strongMotionParameters);
	{
		size_t elementCount = strongMotionParameters->simpleFilterCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(strongMotionParameters->simpleFilter(i));
	}

	count += loadRecords(strongMotionParameters);
	{
		size_t elementCount = strongMotionParameters->recordCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(strongMotionParameters->record(i));
	}

	count += loadStrongOriginDescriptions(strongMotionParameters);
	{
		size_t elementCount = strongMotionParameters->strongOriginDescriptionCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(strongMotionParameters->strongOriginDescription(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadSimpleFilters(StrongMotionParameters* strongMotionParameters) {
	if ( !validInterface() || strongMotionParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(strongMotionParameters, SimpleFilter::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			strongMotionParameters->add(SimpleFilter::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StrongMotionParameters::add(SimpleFilter) -> SimpleFilter has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadRecords(StrongMotionParameters* strongMotionParameters) {
	if ( !validInterface() || strongMotionParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(strongMotionParameters, Record::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			strongMotionParameters->add(Record::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StrongMotionParameters::add(Record) -> Record has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadStrongOriginDescriptions(StrongMotionParameters* strongMotionParameters) {
	if ( !validInterface() || strongMotionParameters == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(strongMotionParameters, StrongOriginDescription::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			strongMotionParameters->add(StrongOriginDescription::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StrongMotionParameters::add(StrongOriginDescription) -> StrongOriginDescription has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::load(SimpleFilter* simpleFilter) {
	size_t count = 0;

	count += loadFilterParameters(simpleFilter);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadFilterParameters(SimpleFilter* simpleFilter) {
	if ( !validInterface() || simpleFilter == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(simpleFilter, FilterParameter::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			simpleFilter->add(FilterParameter::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("SimpleFilter::add(FilterParameter) -> FilterParameter has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::load(Record* record) {
	size_t count = 0;

	count += loadSimpleFilterChainMembers(record);

	count += loadPeakMotions(record);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadSimpleFilterChainMembers(Record* record) {
	if ( !validInterface() || record == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(record, SimpleFilterChainMember::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			record->add(SimpleFilterChainMember::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Record::add(SimpleFilterChainMember) -> SimpleFilterChainMember has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadPeakMotions(Record* record) {
	if ( !validInterface() || record == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(record, PeakMotion::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			record->add(PeakMotion::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Record::add(PeakMotion) -> PeakMotion has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::load(StrongOriginDescription* strongOriginDescription) {
	size_t count = 0;

	count += loadEventRecordReferences(strongOriginDescription);

	count += loadRuptures(strongOriginDescription);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadEventRecordReferences(StrongOriginDescription* strongOriginDescription) {
	if ( !validInterface() || strongOriginDescription == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(strongOriginDescription, EventRecordReference::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			strongOriginDescription->add(EventRecordReference::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StrongOriginDescription::add(EventRecordReference) -> EventRecordReference has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StrongMotionReader::loadRuptures(StrongOriginDescription* strongOriginDescription) {
	if ( !validInterface() || strongOriginDescription == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(strongOriginDescription, Rupture::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			strongOriginDescription->add(Rupture::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("StrongOriginDescription::add(Rupture) -> Rupture has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
