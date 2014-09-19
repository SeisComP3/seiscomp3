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

#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_DATABASEREADER_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_DATABASEREADER_H__


#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/vs/api.h>


namespace Seiscomp {
namespace DataModel {
namespace VS {

class VS;
class Envelope;
class EnvelopeChannel;
class EnvelopeValue;

DEFINE_SMARTPOINTER(StrongMotionReader);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Database reader class for the scheme classes
 *  This class uses a database interface to read objects from a database.
 *  Different database backends can be implemented by creating a driver
 *  in seiscomp3/services/database.
 */
class SC_VS_API VSReader : public DatabaseQuery {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		VSReader(Seiscomp::IO::DatabaseInterface* dbDriver);

		//! Destructor
		~VSReader();


	// ----------------------------------------------------------------------
	//  Read methods
	// ----------------------------------------------------------------------
	public:
		VS* loadVS();
		int load(VS*);
		int loadEnvelopes(VS*);
		int load(Envelope*);
		int loadEnvelopeChannels(Envelope*);
		int load(EnvelopeChannel*);
		int loadEnvelopeValues(EnvelopeChannel*);

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}


#endif
