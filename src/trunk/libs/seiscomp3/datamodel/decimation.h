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


#ifndef __SEISCOMP_DATAMODEL_DECIMATION_H__
#define __SEISCOMP_DATAMODEL_DECIMATION_H__


#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Decimation);

class Datalogger;


class SC_SYSTEM_CORE_API DecimationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DecimationIndex();
		DecimationIndex(int sampleRateNumerator,
		                int sampleRateDenominator);

		//! Copy constructor
		DecimationIndex(const DecimationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DecimationIndex&) const;
		bool operator!=(const DecimationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		int sampleRateNumerator;
		int sampleRateDenominator;
};


/**
 * \brief This type describes a decimation to a certain sample rate
 */
class SC_SYSTEM_CORE_API Decimation : public Object {
	DECLARE_SC_CLASS(Decimation);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Decimation();

		//! Copy constructor
		Decimation(const Decimation& other);

		//! Destructor
		~Decimation();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Decimation& operator=(const Decimation& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Decimation& other) const;
		bool operator!=(const Decimation& other) const;

		//! Wrapper that calls operator==
		bool equal(const Decimation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Output sample rate (numerator); referred from
		//! network/station/Stream/@sampleRateNumerator
		void setSampleRateNumerator(int sampleRateNumerator);
		int sampleRateNumerator() const;

		//! Output sample rate (denominator); referred from
		//! network/station/Stream/@sampleRateDenominator
		void setSampleRateDenominator(int sampleRateDenominator);
		int sampleRateDenominator() const;

		//! Specifies analogue filters between seismometer and
		//! digitizer. Each element (separated by space) references
		//! responsePAZ/@publicID
		void setAnalogueFilterChain(const OPT(Blob)& analogueFilterChain);
		Blob& analogueFilterChain() throw(Seiscomp::Core::ValueException);
		const Blob& analogueFilterChain() const throw(Seiscomp::Core::ValueException);

		//! Specifies digital filters (decimation, gain removal). Each
		//! element (separated by space) references
		//! responsePAZ@publicID or responseFIR/@publicID
		void setDigitalFilterChain(const OPT(Blob)& digitalFilterChain);
		Blob& digitalFilterChain() throw(Seiscomp::Core::ValueException);
		const Blob& digitalFilterChain() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DecimationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Decimation* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Datalogger* datalogger() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		DecimationIndex _index;

		// Attributes
		OPT(Blob) _analogueFilterChain;
		OPT(Blob) _digitalFilterChain;
};


}
}


#endif
