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


#ifndef __SEISCOMP_DATAMODEL_RESPONSEFAP_H__
#define __SEISCOMP_DATAMODEL_RESPONSEFAP_H__


#include <seiscomp3/datamodel/blob.h>
#include <string>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ResponseFAP);

class Inventory;


class SC_SYSTEM_CORE_API ResponseFAPIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ResponseFAPIndex();
		ResponseFAPIndex(const std::string& name);

		//! Copy constructor
		ResponseFAPIndex(const ResponseFAPIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ResponseFAPIndex&) const;
		bool operator!=(const ResponseFAPIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a sensor response composed of
 * \brief frequency/amplitude/phase angle tuples. According to the
 * \brief SEED manual (blockette 55) this description alone is not an
 * \brief acceptable response description.
 */
class SC_SYSTEM_CORE_API ResponseFAP : public PublicObject {
	DECLARE_SC_CLASS(ResponseFAP);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ResponseFAP();

	public:
		//! Copy constructor
		ResponseFAP(const ResponseFAP& other);

		//! Constructor with publicID
		ResponseFAP(const std::string& publicID);

		//! Destructor
		~ResponseFAP();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ResponseFAP* Create();
		static ResponseFAP* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ResponseFAP* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ResponseFAP& operator=(const ResponseFAP& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ResponseFAP& other) const;
		bool operator!=(const ResponseFAP& other) const;

		//! Wrapper that calls operator==
		bool equal(const ResponseFAP& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique response name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Gain of response (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		//! The number of fap tuples in the response
		void setNumberOfTuples(const OPT(int)& numberOfTuples);
		int numberOfTuples() const;

		//! The tuples organized as linear array. The array size must
		//! be numberOfTuples * 3. Each tuple consists of frequency (in
		//! Hz), amplitude and phase angle (in degree).
		void setTuples(const OPT(RealArray)& tuples);
		RealArray& tuples();
		const RealArray& tuples() const;

		//! Optional remark
		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ResponseFAPIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ResponseFAP* lhs) const;

	
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
		ResponseFAPIndex _index;

		// Attributes
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(int) _numberOfTuples;
		OPT(RealArray) _tuples;
		OPT(Blob) _remark;

	DECLARE_SC_CLASSFACTORY_FRIEND(ResponseFAP);
};


}
}


#endif
