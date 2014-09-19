/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_DATALOGGER_H__
#define __SEISCOMP_STATIONXML_DATALOGGER_H__


#include <stationxml/metadata.h>
#include <stationxml/countertype.h>
#include <stationxml/equipment.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(Datalogger);


/**
 * \brief Datalogger complex type. It extends the equipment type. Covers
 * \brief recorder/datalogger parameters in V0.
 */
class Datalogger : public Equipment {
	DECLARE_CASTS(Datalogger);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Datalogger();

		//! Copy constructor
		Datalogger(const Datalogger& other);

		//! Destructor
		~Datalogger();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Datalogger& operator=(const Datalogger& other);
		bool operator==(const Datalogger& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Total number of channels datalogger can record. Corresponds to V0
		//! integer header parameter 34.
		//! XML tag: TotalChannels
		void setTotalChannels(const OPT(CounterType)& totalChannels);
		CounterType& totalChannels() throw(Seiscomp::Core::ValueException);
		const CounterType& totalChannels() const throw(Seiscomp::Core::ValueException);

		//! Number of channels recorded by this datalogger.
		//! XML tag: RecordedChannels
		void setRecordedChannels(const OPT(CounterType)& recordedChannels);
		CounterType& recordedChannels() throw(Seiscomp::Core::ValueException);
		const CounterType& recordedChannels() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(CounterType) _totalChannels;
		OPT(CounterType) _recordedChannels;
};


}
}


#endif
