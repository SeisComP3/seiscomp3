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


#ifndef __SEISCOMP_DATAMODEL_VS_ENVELOPECHANNEL_H__
#define __SEISCOMP_DATAMODEL_VS_ENVELOPECHANNEL_H__


#include <vector>
#include <string>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/vs/api.h>


namespace Seiscomp {
namespace DataModel {
namespace VS {


DEFINE_SMARTPOINTER(EnvelopeChannel);
DEFINE_SMARTPOINTER(EnvelopeValue);

class Envelope;


class SC_VS_API EnvelopeChannel : public PublicObject {
	DECLARE_SC_CLASS(EnvelopeChannel);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		EnvelopeChannel();

	public:
		//! Copy constructor
		EnvelopeChannel(const EnvelopeChannel& other);

		//! Constructor with publicID
		EnvelopeChannel(const std::string& publicID);

		//! Destructor
		~EnvelopeChannel();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static EnvelopeChannel* Create();
		static EnvelopeChannel* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static EnvelopeChannel* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		EnvelopeChannel& operator=(const EnvelopeChannel& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const EnvelopeChannel& other) const;
		bool operator!=(const EnvelopeChannel& other) const;

		//! Wrapper that calls operator==
		bool equal(const EnvelopeChannel& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(EnvelopeValue* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(EnvelopeValue* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEnvelopeValue(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t envelopeValueCount() const;

		//! Index access
		//! @return The object at index i
		EnvelopeValue* envelopeValue(size_t i) const;

		//! Find an object by its unique attribute(s)
		EnvelopeValue* findEnvelopeValue(EnvelopeValue* envelopeValue) const;

		Envelope* envelope() const;

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
		// Attributes
		std::string _name;
		WaveformStreamID _waveformID;

		// Aggregations
		std::vector<EnvelopeValuePtr> _envelopeValues;

	DECLARE_SC_CLASSFACTORY_FRIEND(EnvelopeChannel);
};


}
}
}


#endif
