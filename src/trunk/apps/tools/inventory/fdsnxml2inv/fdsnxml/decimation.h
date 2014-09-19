/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_DECIMATION_H__
#define __SEISCOMP_FDSNXML_DECIMATION_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <fdsnxml/frequencytype.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Decimation);


/**
 * \brief Corresponds to SEED blockette 57.
 */
class Decimation : public Core::BaseObject {
	DECLARE_CASTS(Decimation);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Decimation();

		//! Copy constructor
		Decimation(const Decimation &other);

		//! Destructor
		~Decimation();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Decimation& operator=(const Decimation &other);
		bool operator==(const Decimation &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: InputSampleRate
		void setInputSampleRate(const FrequencyType& inputSampleRate);
		FrequencyType& inputSampleRate();
		const FrequencyType& inputSampleRate() const;

		//! XML tag: Factor
		void setFactor(int factor);
		int factor() const;

		//! XML tag: Offset
		void setOffset(int offset);
		int offset() const;

		//! XML tag: Delay
		void setDelay(const FloatType& delay);
		FloatType& delay();
		FloatType delay() const;

		//! XML tag: Correction
		void setCorrection(const FloatType& correction);
		FloatType& correction();
		FloatType correction() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		FrequencyType _inputSampleRate;
		int _factor;
		int _offset;
		FloatType _delay;
		FloatType _correction;
};


}
}


#endif
