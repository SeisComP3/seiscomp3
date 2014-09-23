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


#ifndef __SEISCOMP_DATAMODEL_RESPONSEPAZ_H__
#define __SEISCOMP_DATAMODEL_RESPONSEPAZ_H__


#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/datamodel/complexarray.h>
#include <string>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ResponsePAZ);

class Inventory;


class SC_SYSTEM_CORE_API ResponsePAZIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponsePAZIndex();
		ResponsePAZIndex(const std::string& name);

		//! Copy constructor
		ResponsePAZIndex(const ResponsePAZIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ResponsePAZIndex&) const;
		bool operator!=(const ResponsePAZIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a sensor response using poles and zeros
 */
class SC_SYSTEM_CORE_API ResponsePAZ : public PublicObject {
	DECLARE_SC_CLASS(ResponsePAZ);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ResponsePAZ();

	public:
		//! Copy constructor
		ResponsePAZ(const ResponsePAZ& other);

		//! Constructor with publicID
		ResponsePAZ(const std::string& publicID);

		//! Destructor
		~ResponsePAZ();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ResponsePAZ* Create();
		static ResponsePAZ* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ResponsePAZ* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ResponsePAZ& operator=(const ResponsePAZ& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ResponsePAZ& other) const;
		bool operator!=(const ResponsePAZ& other) const;

		//! Wrapper that calls operator==
		bool equal(const ResponsePAZ& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique response name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Response type (43.05/53.03): A - Laplace transform analog
		//! response in rad/sec, B - Analog response in Hz, C -
		//! Composite (currently undefined), D - Digital (Z - transform)
		void setType(const std::string& type);
		const std::string& type() const;

		//! Gain of response (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const throw(Seiscomp::Core::ValueException);

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const throw(Seiscomp::Core::ValueException);

		//! A0 normalization factor (43.08/53.07)
		void setNormalizationFactor(const OPT(double)& normalizationFactor);
		double normalizationFactor() const throw(Seiscomp::Core::ValueException);

		//! Normalization frequency (43.09/53.08)
		void setNormalizationFrequency(const OPT(double)& normalizationFrequency);
		double normalizationFrequency() const throw(Seiscomp::Core::ValueException);

		//! Number of zeros (43.10/53.09)
		void setNumberOfZeros(const OPT(int)& numberOfZeros);
		int numberOfZeros() const throw(Seiscomp::Core::ValueException);

		//! Number of poles (43.15/53.14)
		void setNumberOfPoles(const OPT(int)& numberOfPoles);
		int numberOfPoles() const throw(Seiscomp::Core::ValueException);

		//! Zeros (43.16-19/53.10-13)
		void setZeros(const OPT(ComplexArray)& zeros);
		ComplexArray& zeros() throw(Seiscomp::Core::ValueException);
		const ComplexArray& zeros() const throw(Seiscomp::Core::ValueException);

		//! Poles (43.11-14/53.15-18)
		void setPoles(const OPT(ComplexArray)& poles);
		ComplexArray& poles() throw(Seiscomp::Core::ValueException);
		const ComplexArray& poles() const throw(Seiscomp::Core::ValueException);

		void setRemark(const OPT(Blob)& remark);
		Blob& remark() throw(Seiscomp::Core::ValueException);
		const Blob& remark() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ResponsePAZIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ResponsePAZ* lhs) const;

	
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
		ResponsePAZIndex _index;

		// Attributes
		std::string _type;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(double) _normalizationFactor;
		OPT(double) _normalizationFrequency;
		OPT(int) _numberOfZeros;
		OPT(int) _numberOfPoles;
		OPT(ComplexArray) _zeros;
		OPT(ComplexArray) _poles;
		OPT(Blob) _remark;

	DECLARE_SC_CLASSFACTORY_FRIEND(ResponsePAZ);
};


}
}


#endif
