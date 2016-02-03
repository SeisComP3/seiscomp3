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



#ifndef __SEISCOMP_DATAMODEL_NOTIFIER_H__
#define __SEISCOMP_DATAMODEL_NOTIFIER_H__


#include <seiscomp3/core.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/genericmessage.h>
#include <boost/thread/tss.hpp>
#include <list>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Notifier);
// Full namespace specifier needed due to a bug (?) in SWIG >1.3.27
DEFINE_MESSAGE_FOR(Seiscomp::DataModel::Notifier, NotifierMessage, SC_SYSTEM_CORE_API);

class SC_SYSTEM_CORE_API Notifier : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(Notifier);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;


	// ----------------------------------------------------------------------
	//  Enumerations
	// ----------------------------------------------------------------------
	public:
		enum CompareResult {
			CR_DIFFERENT,
			CR_EQUAL,
			CR_OPPOSITE,
			CR_OVERRIDE,
			CR_QUANTITY
		};

	// ------------------------------------------------------------------
	//  Types
	// ------------------------------------------------------------------
	private:
		typedef std::list<NotifierPtr> Pool;
		typedef Pool::iterator PoolIterator;
		typedef Pool::const_iterator PoolConstIterator;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		/**
		 * Constructs a notifier. This custom created notifier will not be
		 * inserted into the notifier pool. The user is responsible for
		 * sending the notifier in a message.
		 */
		Notifier(const std::string& parentID, Operation, Object* object);

	protected:
		//! Constructor
		Notifier();

	public:
		//! Destructor
		~Notifier();


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		//! Enables the notifier pool.
		static void Enable();

		//! Disables the notifier pool. No notifications will be
		//! created at all.
		static void Disable();

		//! Sets the state of the notifier pool
		static void SetEnabled(bool);

		//! Returns the notification pool state.
		//! The default is TRUE.
		static bool IsEnabled();

		//! Enables/disables checking previous inserted notifiers
		//! when a new notifiers is about to be queued. When
		//! enabled, and OP_ADD and OP_UPDATE of the same object
		//! results in only one OP_ADD notifier.
		static void SetCheckEnabled(bool);

		//! Returns the current 'check' state
		static bool IsCheckEnabled();


		/**
		 * Returns a message holding all notifications since the
		 * last call. All stored notifications will be removed from
		 * the notification pool.
		 * @param allNotifier Defines whether to return one message
		 *                    including all notifier or one message
		 *                    including one notifier
		 * @return The message object, if there is one. If each
		 *         notifier is send by its own message one should
		 *         call this method until it returns NULL.
		 */
		static NotifierMessage* GetMessage(bool allNotifier = true);

		//! Returns the size of the notifier objects currently stored.
		static size_t Size();

		//! Clears all buffered notifiers.
		static void Clear();

		/**
		 * Creates a notifier object managed by the global notifier pool.
		 * If the notifier pool is disabled no notifier instance will
		 * be created.
		 * @param parentID The publicId of the parent object that is target
		 *                 of the operation
		 * @param op The operation applied to the parent object
		 * @param object The object that is the operation's "operand"
		 * @return The notifier object. The returned object MUST NOT be
		 *         deleted by the caller explicitly. Nevertheless one can
		 *         store a smartpointer to the object.
		 */
		static Notifier* Create(const std::string& parentID,
		                        Operation op,
		                        Object* object);

		/**
		 * Creates a notifier object managed by the global notifier pool.
		 * If the notifier pool is disabled no notifier instance will
		 * be created.
		 * @param parent The parent object that is target of the operation
		 * @param op The operation applied to the parent object
		 * @param object The object that is the operation's "operand"
		 * @return The notifier object. The returned object MUST NOT be
		 *         deleted by the caller explicitly. Nevertheless one can
		 *         store a smartpointer to the object.
		 */
		static Notifier* Create(PublicObject* parent,
		                        Operation op,
		                        Object* object);

		//! Apply the notification
		bool apply() const;


		//! Sets the publicID of the parent object
		void setParentID(const std::string &);

		//! Returns publicId of the parent object
		const std::string& parentID() const;


		//! Sets the operation
		void setOperation(Operation);

		//! Returns the operation defined for the notifier
		Operation operation() const;


		//! Sets the object regarding the operation
		void setObject(Object* object);

		//! Returns the object regarding the operation
		Object* object() const;

		/**
		 * Compares to notifier by each other and returns the
		 * result. Notifier can be EQUAL, that means they express
		 * the same thing. They can be DIFFERENT, which means that
		 * they do not have anything in common and they can be
		 * OPPOSITE to each other meaning they neutralize each other.
		 * E.g. n1 = ABCNotifier(OP_ADD, myObject)
		 *      n2 = ABCNotifier(OP_REMOVE, myObject)
		 *      n1.cmp(n2) = CR_OPPOSITE
		 */
		CompareResult cmp(const Notifier*) const;
		CompareResult cmp(const Notifier&) const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		std::string _parentID;
		Operation _operation;
		ObjectPtr _object;

		static boost::thread_specific_ptr<bool> _lock;
		static Pool _notifiers;
		static bool _checkOnCreate;

	DECLARE_SC_CLASSFACTORY_FRIEND(Notifier);
};


/**
 * \brief A visitor that creates notifiers for a given subtree
 * \brief and appends them to the global notifier pool.
 */
class NotifierCreator : public Visitor {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		NotifierCreator(Operation op);


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		bool visit(PublicObject*);
		void visit(Object*);

	private:
		Operation _operation;
};


/**
 * \brief A visitor that creates notifiers for a given subtree
 * \brief and appends them to a local list
 */
template <class T>
class NotifierStoreAppender : public Visitor {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		NotifierStoreAppender(T &store, Operation op, const std::string &parentID = "")
		: Visitor(op == OP_REMOVE?TM_BOTTOMUP:TM_TOPDOWN),
		  _store(&store), _operation(op), _parentID(parentID) {}


	// ----------------------------------------------------------------------
	//  Interface
	// ----------------------------------------------------------------------
	public:
		bool visit(PublicObject *po) {
			if ( po->parent() == NULL ) {
				if ( _parentID.empty() )
					return false;
				_store->push_back(new Notifier(_parentID, _operation, po));
			}
			else
				_store->push_back(new Notifier(po->parent()->publicID(), _operation, po));
			return true;
		}

		void visit(Object *o) {
			if ( o->parent() == NULL ) {
				if ( _parentID.empty() )
					return;
				_store->push_back(new Notifier(_parentID, _operation, o));
			}
			else
				_store->push_back(new Notifier(o->parent()->publicID(), _operation, o));
		}

	private:
		T                 *_store;
		Operation          _operation;
		const std::string &_parentID;
};


template <class T>
void AppendNotifier(T &store, Operation op, Object *o, const std::string parentID = "") {
	// Remember the size of the store before any modifications
	size_t endPos = store.size();
	
	// Create a store appender and visit all child objects
	NotifierStoreAppender<T> nsa(store, op, parentID);
	o->accept(&nsa);
	
	// If a parent id was specified and elements have been added to the
	// store, override the parent id of the first object (o). Note: The
	// position of the Notifier of o depends on the operation
	// (OP_ADD vs. OP_REMOVE). It is either the first or the last element
	// added to the store.
	if ( !parentID.empty() && store.size() > endPos ) {
		size_t pos = op == OP_REMOVE ? store.size()-1 : endPos;
		NotifierPtr n = store.at(pos);
		n->setParentID(parentID);
	}
}


} // of NS DataModel
} // of NS Seiscomp


#endif
