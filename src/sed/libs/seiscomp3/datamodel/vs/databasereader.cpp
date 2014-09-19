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
#include <seiscomp3/datamodel/vs/databasereader.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/vs/vs_package.h>
#include <seiscomp3/datamodel/comment.h>
#include <boost/bind.hpp>

using namespace std;

namespace Seiscomp {
namespace DataModel {
namespace VS {


VSReader::VSReader(Seiscomp::IO::DatabaseInterface* dbDriver)
: DatabaseQuery(dbDriver) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VSReader::~VSReader() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
VS* VSReader::loadVS() {
	if ( !validInterface() ) return NULL;

	VS *vS = new VS;

	load(vS);

	SEISCOMP_DEBUG("objects in cache: %d", getCacheSize());
	
	return vS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::load(VS* vS) {
	size_t count = 0;

	count += loadEnvelopes(vS);
	{
		size_t elementCount = vS->envelopeCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(vS->envelope(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::loadEnvelopes(VS* vS) {
	if ( !validInterface() || vS == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(vS, Envelope::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			vS->add(Envelope::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("VS::add(Envelope) -> Envelope has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::load(Envelope* envelope) {
	size_t count = 0;

	count += loadEnvelopeChannels(envelope);
	{
		size_t elementCount = envelope->envelopeChannelCount();
		for ( size_t i = 0; i < elementCount; ++i )
			load(envelope->envelopeChannel(i));
	}

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::loadEnvelopeChannels(Envelope* envelope) {
	if ( !validInterface() || envelope == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(envelope, EnvelopeChannel::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			envelope->add(EnvelopeChannel::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("Envelope::add(EnvelopeChannel) -> EnvelopeChannel has already another parent");
		++it;
	}
	it.close();

	Notifier::SetEnabled(saveState);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::load(EnvelopeChannel* envelopeChannel) {
	size_t count = 0;

	count += loadEnvelopeValues(envelopeChannel);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int VSReader::loadEnvelopeValues(EnvelopeChannel* envelopeChannel) {
	if ( !validInterface() || envelopeChannel == NULL ) return 0;

	bool saveState = Notifier::IsEnabled();
	Notifier::Disable();

	DatabaseIterator it;
	size_t count = 0;
	it = getObjects(envelopeChannel, EnvelopeValue::TypeInfo());
	while ( *it ) {
		if ( (*it)->parent() == NULL ) {
			envelopeChannel->add(EnvelopeValue::Cast(*it));
			++count;
		}
		else
			SEISCOMP_INFO("EnvelopeChannel::add(EnvelopeValue) -> EnvelopeValue has already another parent");
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
