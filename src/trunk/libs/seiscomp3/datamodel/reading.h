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


#ifndef __SEISCOMP_DATAMODEL_READING_H__
#define __SEISCOMP_DATAMODEL_READING_H__


#include <vector>
#include <seiscomp3/datamodel/pickreference.h>
#include <seiscomp3/datamodel/amplitudereference.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Reading);
DEFINE_SMARTPOINTER(PickReference);
DEFINE_SMARTPOINTER(AmplitudeReference);

class EventParameters;


/**
 * \brief This class groups Pick and Amplitude elements which are
 * \brief thought to belong
 * \brief to the same event, but for which the event identification
 * \brief is not known.
 */
class SC_SYSTEM_CORE_API Reading : public PublicObject {
	DECLARE_SC_CLASS(Reading);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Reading();

	public:
		//! Copy constructor
		Reading(const Reading& other);

		//! Constructor with publicID
		Reading(const std::string& publicID);

		//! Destructor
		~Reading();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Reading* Create();
		static Reading* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Reading* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Reading& operator=(const Reading& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Reading& other) const;
		bool operator!=(const Reading& other) const;

		//! Wrapper that calls operator==
		bool equal(const Reading& other) const;

	
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
		bool add(PickReference* obj);
		bool add(AmplitudeReference* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(PickReference* obj);
		bool remove(AmplitudeReference* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removePickReference(size_t i);
		bool removePickReference(const PickReferenceIndex& i);
		bool removeAmplitudeReference(size_t i);
		bool removeAmplitudeReference(const AmplitudeReferenceIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t pickReferenceCount() const;
		size_t amplitudeReferenceCount() const;

		//! Index access
		//! @return The object at index i
		PickReference* pickReference(size_t i) const;
		PickReference* pickReference(const PickReferenceIndex& i) const;

		AmplitudeReference* amplitudeReference(size_t i) const;
		AmplitudeReference* amplitudeReference(const AmplitudeReferenceIndex& i) const;

		//! Find an object by its unique attribute(s)

		EventParameters* eventParameters() const;

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
		// Aggregations
		std::vector<PickReferencePtr> _pickReferences;
		std::vector<AmplitudeReferencePtr> _amplitudeReferences;

	DECLARE_SC_CLASSFACTORY_FRIEND(Reading);
};


}
}


#endif
