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


#ifndef __SEISCOMP_DATAMODEL_RESPONSEIIR_H__
#define __SEISCOMP_DATAMODEL_RESPONSEIIR_H__


#include <seiscomp3/datamodel/blob.h>
#include <string>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ResponseIIR);

class Inventory;


class SC_SYSTEM_CORE_API ResponseIIRIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponseIIRIndex();
		ResponseIIRIndex(const std::string& name);

		//! Copy constructor
		ResponseIIRIndex(const ResponseIIRIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ResponseIIRIndex&) const;
		bool operator!=(const ResponseIIRIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a infinite impulse response filter
 */
class SC_SYSTEM_CORE_API ResponseIIR : public PublicObject {
	DECLARE_SC_CLASS(ResponseIIR);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ResponseIIR();

	public:
		//! Copy constructor
		ResponseIIR(const ResponseIIR& other);

		//! Constructor with publicID
		ResponseIIR(const std::string& publicID);

		//! Destructor
		~ResponseIIR();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ResponseIIR* Create();
		static ResponseIIR* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ResponseIIR* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ResponseIIR& operator=(const ResponseIIR& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ResponseIIR& other) const;
		bool operator!=(const ResponseIIR& other) const;

		//! Wrapper that calls operator==
		bool equal(const ResponseIIR& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique response name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Response type (43.05/53.03/54.03): A - Laplace transform
		//! analog response in rad/sec, B - Analog response in Hz, C -
		//! Composite (currently undefined), D - Digital (Z - transform)
		void setType(const std::string& type);
		const std::string& type() const;

		//! Gain of response (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		//! Decimation factor (47.06/57.05)
		void setDecimationFactor(const OPT(int)& decimationFactor);
		int decimationFactor() const;

		//! Estimated delay (47.08/57.07)
		void setDelay(const OPT(double)& delay);
		double delay() const;

		//! Applied correction (47.09/57.08)
		void setCorrection(const OPT(double)& correction);
		double correction() const;

		//! Number of numerators (54.07)
		void setNumberOfNumerators(const OPT(int)& numberOfNumerators);
		int numberOfNumerators() const;

		//! Number of denominators (54.10)
		void setNumberOfDenominators(const OPT(int)& numberOfDenominators);
		int numberOfDenominators() const;

		//! Numerators (54.08-09)
		void setNumerators(const OPT(RealArray)& numerators);
		RealArray& numerators();
		const RealArray& numerators() const;

		//! Denominators (54.11-12)
		void setDenominators(const OPT(RealArray)& denominators);
		RealArray& denominators();
		const RealArray& denominators() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ResponseIIRIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ResponseIIR* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Inventory* inventory() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		ResponseIIRIndex _index;

		// Attributes
		std::string _type;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(int) _decimationFactor;
		OPT(double) _delay;
		OPT(double) _correction;
		OPT(int) _numberOfNumerators;
		OPT(int) _numberOfDenominators;
		OPT(RealArray) _numerators;
		OPT(RealArray) _denominators;
		OPT(Blob) _remark;

	DECLARE_SC_CLASSFACTORY_FRIEND(ResponseIIR);
};


}
}


#endif
