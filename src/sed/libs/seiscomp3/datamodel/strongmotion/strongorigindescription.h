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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_STRONGORIGINDESCRIPTION_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_STRONGORIGINDESCRIPTION_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(StrongOriginDescription);
DEFINE_SMARTPOINTER(EventRecordReference);
DEFINE_SMARTPOINTER(Rupture);

class StrongMotionParameters;


class SC_STRONGMOTION_API StrongOriginDescription : public PublicObject {
	DECLARE_SC_CLASS(StrongOriginDescription);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		StrongOriginDescription();

	public:
		//! Copy constructor
		StrongOriginDescription(const StrongOriginDescription& other);

		//! Constructor with publicID
		StrongOriginDescription(const std::string& publicID);

		//! Destructor
		~StrongOriginDescription();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static StrongOriginDescription* Create();
		static StrongOriginDescription* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static StrongOriginDescription* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		StrongOriginDescription& operator=(const StrongOriginDescription& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const StrongOriginDescription& other) const;
		bool operator!=(const StrongOriginDescription& other) const;

		//! Wrapper that calls operator==
		bool equal(const StrongOriginDescription& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setOriginID(const std::string& originID);
		const std::string& originID() const;

		void setWaveformCount(const OPT(int)& waveformCount);
		int waveformCount() const throw(Seiscomp::Core::ValueException);

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
		bool add(EventRecordReference* obj);
		bool add(Rupture* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(EventRecordReference* obj);
		bool remove(Rupture* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEventRecordReference(size_t i);
		bool removeRupture(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t eventRecordReferenceCount() const;
		size_t ruptureCount() const;

		//! Index access
		//! @return The object at index i
		EventRecordReference* eventRecordReference(size_t i) const;
		Rupture* rupture(size_t i) const;

		//! Find an object by its unique attribute(s)
		EventRecordReference* findEventRecordReference(EventRecordReference* eventRecordReference) const;
		Rupture* findRupture(const std::string& publicID) const;

		StrongMotionParameters* strongMotionParameters() const;

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
		std::string _originID;
		OPT(int) _waveformCount;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<EventRecordReferencePtr> _eventRecordReferences;
		std::vector<RupturePtr> _ruptures;

	DECLARE_SC_CLASSFACTORY_FRIEND(StrongOriginDescription);
};


}
}
}


#endif
