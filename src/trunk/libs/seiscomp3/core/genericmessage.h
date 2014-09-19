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


#ifndef __SEISCOMP_CORE_GENERICMESSAGE_H__
#define __SEISCOMP_CORE_GENERICMESSAGE_H__

#include <seiscomp3/core/message.h>
#include <list>


namespace Seiscomp {
namespace Core {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
class GenericMessage : public ::Seiscomp::Core::Message {
	// ----------------------------------------------------------------------
	//  Public Types
	// ----------------------------------------------------------------------
	public:
		typedef T AttachementType;
		typedef typename std::list<typename Seiscomp::Core::SmartPointer<T>::Impl> AttachementList;
		typedef typename AttachementList::iterator iterator;
		typedef typename AttachementList::const_iterator const_iterator;

	DECLARE_SERIALIZATION;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		GenericMessage();

		//! Destructor
		~GenericMessage();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * Attaches an object to the message
		 * @param  attachement A pointer to the object
		 * @retval true The operation was successfull and the object has been attached properly
		 * @retval false The object is NULL or the object has been attached already
		 */
		bool attach(AttachementType* attachement);
		bool attach(typename Seiscomp::Core::SmartPointer<AttachementType>::Impl& attachement);

		/**
		 * Detaches an already attached object from the message
		 * @param  object Pointer to an object in the messagebody
		 * @retval true The object has been detached successfully
		 * @retval false The object has not been attached before
		 */
		bool detach(AttachementType* attachement);
		bool detach(typename Seiscomp::Core::SmartPointer<AttachementType>::Impl& attachement);

		/**
		 * Detaches an object from the message
		 * @param it The iterator pointing to the object
		 * @retval true The object has been detached successfully
		 * @retval false The iterator is invalid
		 */
		iterator detach(iterator it);

		//! Removes all attachements from the message
		void clear();

		//! Returns the iterators for begin and end of
		//! the attachement list
		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

		//! Implemented from baseclass
		bool empty() const;

		/**
		 * @return Returns the number of objects attached to a message
		 */
		int size() const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		MessageIterator::Impl* iterImpl() const;

	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	protected:
		AttachementList _attachements;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DEFINE_MESSAGE_FOR(CLASS, TYPENAME, APIDef) \
	class APIDef TYPENAME : public ::Seiscomp::Core::GenericMessage<CLASS> { \
		DECLARE_SC_CLASS(TYPENAME); \
	}; \
	typedef ::Seiscomp::Core::SmartPointer<TYPENAME>::Impl TYPENAME##Ptr

#define IMPLEMENT_MESSAGE_FOR(CLASS, TYPENAME, NAME) \
	IMPLEMENT_SC_CLASS_DERIVED(TYPENAME, ::Seiscomp::Core::Message, NAME)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include <seiscomp3/core/genericmessage.ipp>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
