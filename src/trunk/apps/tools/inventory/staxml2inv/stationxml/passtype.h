/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_PASSTYPE_H__
#define __SEISCOMP_STATIONXML_PASSTYPE_H__


#include <stationxml/metadata.h>
#include <stationxml/frequencytype.h>
#include <stationxml/rolltype.h>
#include <stationxml/countertype.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(PassType);


/**
 * \brief Pass response, containing corner frequency and roll off.
 */
class PassType : public Core::BaseObject {
	DECLARE_CASTS(PassType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PassType();

		//! Copy constructor
		PassType(const PassType& other);

		//! Destructor
		~PassType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PassType& operator=(const PassType& other);
		bool operator==(const PassType& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: CornerFreq
		void setCornerFreq(const FrequencyType& cornerFreq);
		FrequencyType& cornerFreq();
		const FrequencyType& cornerFreq() const;

		//! XML tag: Roll
		void setRoll(const RollType& roll);
		RollType& roll();
		const RollType& roll() const;

		//! XML tag: Damping
		void setDamping(double damping);
		double damping() const;

		//! XML tag: Polenumber
		void setPolenumber(const CounterType& polenumber);
		CounterType& polenumber();
		const CounterType& polenumber() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		FrequencyType _cornerFreq;
		RollType _roll;
		double _damping;
		CounterType _polenumber;
};


}
}


#endif
