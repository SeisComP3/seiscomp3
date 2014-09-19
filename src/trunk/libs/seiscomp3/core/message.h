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


#ifndef __SEISCOMP_CORE_MESSAGE_H__
#define __SEISCOMP_CORE_MESSAGE_H__

#include <seiscomp3/core/baseobject.h>


namespace Seiscomp {
namespace Core {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(Message);

/**
 * The MessageIterator is an iterator that iterates over the objects
 * attached to a message. Not all message types support attachements
 * what resulted in a generic iterator concept. Each derived message
 * type that wants to implement its iteration has to provide an
 * implementation class derived from MessageType::Impl
 * The iterator protocol looks like this:
 * \code
 * for ( Iterator it = msg->iter(); *it != NULL; ++it ) {
 *   BaseObject* o = *it;
 *   // do something with o
 * }
 * \endcode
 *
 */
class SC_SYSTEM_CORE_API MessageIterator {
	public:
		//! Implementation class for a message iterator
		class Impl {
			public:
				virtual ~Impl() {}

				//! Clones the iterator implementation
				virtual Impl* clone() const = 0;

				//! Returns the current element
				virtual Seiscomp::Core::BaseObject* get() const = 0;

				//! Go to the next element
				virtual void next() = 0;
		};


	public:
		MessageIterator();
		MessageIterator(const MessageIterator& iter);
		~MessageIterator();

	protected:
		MessageIterator(Impl*);

	public:
		Seiscomp::Core::BaseObject* get() const;

		MessageIterator& operator=(const MessageIterator& it);
		Seiscomp::Core::BaseObject* operator*() const;

		MessageIterator& operator++();
		MessageIterator& operator++(int);

	private:
		Impl* _impl;

	friend class Message;
};


class SC_SYSTEM_CORE_API Message : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Message);
	DECLARE_SERIALIZATION;

	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	protected:
		//! Constructor
		Message();

	public:
		//! Destructor
		~Message();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * Returns an iterator to iterate through the attachements.
		 * To filter a particular object type, a class typeinfo can
		 * be used to do that.
		 * E.g., to filter only object of type A (and derived), use
		 * A::TypeInfo() as input parameter.
		 * \code
		 * MessageIterator it = msg->iter(A::TypeInfo());
		 * while ( it.next() ) {
		 *   A* a = static_cast<A*>(*it);
		 * }
		 * \endcode
		 * @param typeInfo The object type filter
		 * @return The iterator
		 */
		MessageIterator iter() const;

		/**
		 * @return Returns the number of objects attached to a message
		 */
		virtual int size() const;

		/**
		 * Checks whether a message is empty or not.
		 * @retval true The message is empty
		 * @retval false The message is not empty
		 */
		virtual bool empty() const = 0;

		/**
		 * Erases the content of the message.
		 */
		virtual void clear() {}

		/**
		 * Sets the datasize needed to encode the message into the
		 * transport stream.
		 * @param size The datasize
		 */
		void setDataSize(int size);
	
		/**
		 * @return The datasize for the message after sending or receiving.
		 */
		int dataSize() const;


	protected:
		/**
		 * Returns an iterator implementation for a particular message.
		 * This method has to be implemented in derived classes to provide
		 * generic iteration.
		 * @return The implementation.
		 */
		virtual MessageIterator::Impl* iterImpl() const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		int _dataSize;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline int Message::dataSize() const {
	return _dataSize;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
