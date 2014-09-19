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


#ifndef __SEISCOMP_DATAMODEL_VS_ENVELOPE_H__
#define __SEISCOMP_DATAMODEL_VS_ENVELOPE_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/creationinfo.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/vs/api.h>


namespace Seiscomp {
namespace DataModel {
namespace VS {


DEFINE_SMARTPOINTER(Envelope);
DEFINE_SMARTPOINTER(EnvelopeChannel);

class VS;


class SC_VS_API Envelope : public PublicObject {
	DECLARE_SC_CLASS(Envelope);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Envelope();

	public:
		//! Copy constructor
		Envelope(const Envelope& other);

		//! Constructor with publicID
		Envelope(const std::string& publicID);

		//! Destructor
		~Envelope();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Envelope* Create();
		static Envelope* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Envelope* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Envelope& operator=(const Envelope& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Envelope& other) const;
		bool operator!=(const Envelope& other) const;

		//! Wrapper that calls operator==
		bool equal(const Envelope& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setNetwork(const std::string& network);
		const std::string& network() const;

		void setStation(const std::string& station);
		const std::string& station() const;

		void setTimestamp(Seiscomp::Core::Time timestamp);
		Seiscomp::Core::Time timestamp() const;

		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo() throw(Seiscomp::Core::ValueException);
		const CreationInfo& creationInfo() const throw(Seiscomp::Core::ValueException);

	
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
		bool add(EnvelopeChannel* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(EnvelopeChannel* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEnvelopeChannel(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t envelopeChannelCount() const;

		//! Index access
		//! @return The object at index i
		EnvelopeChannel* envelopeChannel(size_t i) const;

		//! Find an object by its unique attribute(s)
		EnvelopeChannel* findEnvelopeChannel(const std::string& publicID) const;

		VS* vS() const;

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
		std::string _network;
		std::string _station;
		Seiscomp::Core::Time _timestamp;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<EnvelopeChannelPtr> _envelopeChannels;

	DECLARE_SC_CLASSFACTORY_FRIEND(Envelope);
};


}
}
}


#endif
