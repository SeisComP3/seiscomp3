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



#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/datamodel/metadata.h>
#include <string>


namespace Seiscomp {
namespace DataModel {


namespace {


//! Optional baseobject property specialization
template <typename T, typename U, typename F1, typename F2>
class BaseObjectProperty : public Core::MetaProperty {
	public:
		BaseObjectProperty(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				throw Core::ValueException("object must not be NULL");
			else {
				Core::BaseObject *v;
				try {
					v = boost::any_cast<Core::BaseObject*>(value);
				}
				catch ( boost::bad_any_cast & ) {
					v = boost::any_cast<U*>(value);
				}

				if ( v == NULL )
					throw Core::GeneralException("object must not be NULL");

				U *uv = U::Cast(v);
				if ( uv == NULL )
					throw Core::GeneralException("object has wrong type");

				(target->*_setter)(uv);
			}

			return true;
		}

		Core::MetaValue read(Core::BaseObject *object) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return static_cast<Core::BaseObject*>((target->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};


template <class C, typename R1, typename T1, typename T2>
Core::MetaPropertyHandle objectProperty2(
	const std::string& name, const std::string& type,
	bool isIndex, bool isOptional,
	R1 (C::*setter)(T1*), T2* (C::*getter)() const) {
	return Core::createProperty<BaseObjectProperty>(
			name, type, false, true, isIndex, false,
			isOptional, false, NULL, setter, getter);
}


}


IMPLEMENT_SC_CLASS(Notifier, "notifier");


Notifier::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("parentID", "string", false, false, false, false, true, false, NULL, &Notifier::setParentID, &Notifier::parentID));
	addProperty(enumProperty("operation", "Operation", false, false, &MetaOperation, &Notifier::setOperation, &Notifier::operation));
	addProperty(objectProperty2("object", "Object", false, false, &Notifier::setObject, &Notifier::object));
}


IMPLEMENT_METAOBJECT(Notifier)

IMPLEMENT_MESSAGE_FOR(Notifier, NotifierMessage, "notifier_message");
Notifier::Pool Notifier::_notifiers;
boost::thread_specific_ptr<bool> Notifier::_lock;
bool Notifier::_checkOnCreate = true;


Notifier::Notifier()
 : _operation(OP_UNDEFINED), _object(NULL) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier::Notifier(const std::string& parentID, Operation op, Object* object)
 : _parentID(parentID),
   _operation(op),
   _object(object) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier::~Notifier() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier* Notifier::Create(const std::string& parentId,
                           Operation op,
	                       Object* object) {
	if ( !IsEnabled() ) return NULL;

	if ( parentId.empty() ) {
		SEISCOMP_ERROR("cannot create a notifier without a publicId");
		return NULL;
	}

	if ( object == NULL ) {
		SEISCOMP_ERROR("cannot create a notifier without an object");
		return NULL;
	}

	NotifierPtr notifier = new Notifier(parentId, op, object);

	if ( _checkOnCreate ) {
		PoolIterator it = _notifiers.begin();
		for ( ; it != _notifiers.end(); ++it ) {
			CompareResult res = (*it)->cmp(notifier.get());
			// If there is already an equal notifier stored, discard the
			// current one
			if ( res == CR_EQUAL ) {
				SEISCOMP_DEBUG("equal notifiers found => discarding the given (%s(%s, %s), %s(%s, %s))",
				               (*it)->parentID().c_str(),
				               (*it)->operation().toString(),
				               (*it)->object()->className(),
				               notifier->parentID().c_str(),
				               notifier->operation().toString(),
				               notifier->object()->className());
				return NULL;
			}
			// If the notifier neutralize each other, remove the stored
			// and discard the current one
			else if ( res == CR_OPPOSITE ) {
				SEISCOMP_DEBUG("opposite notifier found => removing the stored one");
				_notifiers.erase(it);
				return NULL;
			}
		}
	}

	_notifiers.push_back(notifier);
	return notifier.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier* Notifier::Create(PublicObject* parent,
                           Operation op,
                           Object* object) {
	if ( parent == NULL ) {
		SEISCOMP_ERROR("cannot create notifier (%s: %s) without parent object",
		               op.toString(), object?object->className():"[NULL]");
		return NULL;
	}
	return Create(parent->publicID(), op, object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NotifierMessage* Notifier::GetMessage(bool allNotifier) {
	if ( _notifiers.empty() )
		return NULL;

	NotifierMessage* msg = new NotifierMessage;
	PoolIterator it = _notifiers.begin();

	if ( allNotifier ) {
		for ( ; it != _notifiers.end(); ++it )
			msg->attach((*it).get());
		_notifiers.clear();
	}
	else {
		msg->attach((*it).get());
		_notifiers.erase(it);
	}

	return msg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Notifier::Size() {
	return _notifiers.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::Clear() {
	_notifiers.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::Disable() {
	SetEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::Enable() {
	SetEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::SetEnabled(bool e) {
	if ( _lock.get() == NULL )
		// Store a new thread specific pointer value with 'enable'
		_lock.reset(new bool(!e));
	else
		*_lock.get() = !e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Notifier::IsEnabled() {
	if ( _lock.get() == NULL ) {
		// Store a new thread specific pointer value with default: true
		bool *value = new bool(true);
		_lock.reset(value);
	}

	return *_lock.get() == false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::SetCheckEnabled(bool e) {
	_checkOnCreate = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Notifier::IsCheckEnabled() {
	return _checkOnCreate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Notifier::apply() const {
	if ( _object == NULL ) {
		SEISCOMP_ERROR("cannot apply notifier without an object");
		return false;
	}

	PublicObject* publicObject = PublicObject::Find(_parentID);
	if ( publicObject == NULL ) {
		if ( _operation == OP_UPDATE ) {
			PublicObject *po = PublicObject::Cast(_object);
			if ( po != NULL ) {
				publicObject = PublicObject::Find(po->publicID());
				if ( publicObject && publicObject != po ) {
					bool saveState = IsEnabled();
					Disable();
					publicObject->assign(po);
					publicObject->update();
					SetEnabled(saveState);
					return true;
				}
				else {
					//SEISCOMP_DEBUG("could not find object width publicID '%s' to update", po->publicID().c_str());
					return false;
				}
			}
			else {
				//SEISCOMP_DEBUG("parent object for %s not found, discarding UPDATE notifier", _object->className());
				return false;
			}
		}

		//SEISCOMP_DEBUG("object with publicId '%s' has not been found while applying a notifier", _parentID.c_str());
		return false;
	}

	bool saveState = IsEnabled();
	bool result = false;
	Disable();

	switch ( _operation ) {
		case OP_ADD:
			result = _object->attachTo(publicObject);
			break;
		case OP_REMOVE:
			result = _object->detachFrom(publicObject);
			break;
		case OP_UPDATE:
			result = publicObject->updateChild(_object.get());
			break;
		default:
			break;
	}

	SetEnabled(saveState);

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::setParentID(const std::string &parentID) {
	_parentID = parentID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Notifier::parentID() const {
	return _parentID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::setOperation(Operation op) {
	_operation = op;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Operation Notifier::operation() const {
	return _operation;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::setObject(Object* object) {
	_object = object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Notifier::object() const {
	return _object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier::CompareResult Notifier::cmp(const Notifier* n) const {
	if ( n == NULL ) return CR_DIFFERENT;
	return cmp(*n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Notifier::CompareResult Notifier::cmp(const Notifier& n) const {
	static CompareResult ResultTable[Operation::Quantity][Operation::Quantity] =
	{
		{CR_DIFFERENT, CR_DIFFERENT, CR_DIFFERENT, CR_DIFFERENT},
		{CR_DIFFERENT, CR_EQUAL,     CR_OPPOSITE,  CR_EQUAL},
		{CR_DIFFERENT, CR_OPPOSITE,  CR_EQUAL,     CR_EQUAL},
		{CR_DIFFERENT, CR_OVERRIDE,  CR_OVERRIDE,  CR_EQUAL}
	};

	if ( this == &n ) return CR_EQUAL;

	Object *obj1 = object();
	Object *obj2 = n.object();

	// At least one notifier does not refer to an object
	if ( obj1 == NULL || obj2 == NULL )
		return CR_DIFFERENT;

	if ( obj1 != obj2 ) {
		// Referred objects are not from the same type
		//if ( !obj2->typeInfo().isTypeOf(obj1->typeInfo()) )
			return CR_DIFFERENT;
	}

	Operation op1 = operation();
	Operation op2 = n.operation();

	if ( op1 >= Operation::Quantity || op2 >= Operation::Quantity )
		return CR_DIFFERENT;

	if ( _parentID != n._parentID )
		return CR_DIFFERENT;

	return ResultTable[op1][op2];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Notifier::serialize(Archive& ar) {
	ar & NAMED_OBJECT("parentID", _parentID);
	ar & NAMED_OBJECT("operation", _operation);
	ar & NAMED_OBJECT_HINT("", _object, Archive::IGNORE_CHILDS);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NotifierCreator::NotifierCreator(Operation op)
	: Visitor(op == OP_REMOVE?TM_BOTTOMUP:TM_TOPDOWN), _operation(op) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool NotifierCreator::visit(PublicObject* publicObject) {
	return Notifier::Create(publicObject->parent(), _operation, publicObject) != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void NotifierCreator::visit(Object* object) {
	Notifier::Create(object->parent(), _operation, object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
