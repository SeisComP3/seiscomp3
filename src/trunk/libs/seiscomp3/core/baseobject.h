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


#ifndef __SC_CORE_BASEOBJECT_H__
#define __SC_CORE_BASEOBJECT_H__

namespace Seiscomp {
namespace Core {
	class BaseObject;
}
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_add_ref(Seiscomp::Core::BaseObject *p);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_release(Seiscomp::Core::BaseObject *p);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_add_ref(const Seiscomp::Core::BaseObject *p);

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_release(const Seiscomp::Core::BaseObject *p);

#include <seiscomp3/core/defs.h>
#include <seiscomp3/core/rtti.h>
#include <seiscomp3/core/metaobject.h>
#include <seiscomp3/core/archive.h>
#include <seiscomp3/core/factory.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace Core {


#define DECLARE_CASTS(CLASS) \
		public: \
			static CLASS* Cast(Seiscomp::Core::BaseObject* o) { \
				return dynamic_cast<CLASS*>(o); \
			} \
			\
			static const CLASS* ConstCast(const Seiscomp::Core::BaseObject* o) { \
				return dynamic_cast<const CLASS*>(o); \
			} \
			\
			static CLASS* Cast(Seiscomp::Core::BaseObjectPtr o) { \
				return dynamic_cast<CLASS*>(o.get()); \
			} \
			static const CLASS* ConstCast(Seiscomp::Core::BaseObjectCPtr o) { \
				return dynamic_cast<const CLASS*>(o.get()); \
			}

#define DECLARE_SC_CLASS(CLASS) \
		DECLARE_RTTI; \
		DECLARE_CASTS(CLASS)


#define IMPLEMENT_SC_CLASS(CLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, Seiscomp::Core::BaseObject) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define IMPLEMENT_SC_CLASS_DERIVED(CLASS, BASECLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, BASECLASS) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_CLASS(Seiscomp::Core::BaseObject, CLASS)


#define IMPLEMENT_SC_ABSTRACT_CLASS(CLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, Seiscomp::Core::BaseObject) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_ABSTRACT_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define IMPLEMENT_SC_ABSTRACT_CLASS_DERIVED(CLASS, BASECLASS, CLASSNAME) \
        IMPLEMENT_RTTI(CLASS, CLASSNAME, BASECLASS) \
        IMPLEMENT_RTTI_METHODS(CLASS) \
        REGISTER_ABSTRACT_CLASS(Seiscomp::Core::BaseObject, CLASS)

#define DECLARE_SC_CLASSFACTORY_FRIEND(CLASS) \
        DECLARE_CLASSFACTORY_FRIEND(Seiscomp::Core::BaseObject, CLASS)


DEFINE_SMARTPOINTER(BaseObject);
typedef Generic::ClassFactoryInterface<BaseObject> ClassFactory;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief BaseObject has to be used for all classes that want to use
    \brief the provided serialization mechanism and reference counting.

    \author Jan Becker (jan.becker@gfz-potsdam.de)

	To derive from BaseObject the following basic steps are necessary:

    <b>1. Create a class that derives from BaseObject</b>

    \code
    class MyClass : public BaseObject
    \endcode 

    <b>2. Add the DECLARE_SC_CLASS macro to add the RTTI interface among other things</b>
    \code
    class MyClass : public BaseObject {
    	DECLARE_SC_CLASS(MyClass);

    	public:
    		MyClass();
    };
    \endcode

	Implement the class RTTI data in the .cpp file
	\code
	// First parameter is the classname, second parameter is the name inside RTTI
	IMPLEMENT_SC_CLASS(MyClass, "MyClass");	
	\endcode

	If the class is abstract (it has some pure virtual methods) another macro must be
	used:
	\code
	// First parameter is the classname, second parameter is the name inside RTTI
	IMPLEMENT_SC_ABSTRACR_CLASS(MyClass, "MyClass");
	\endcode

    <b>3. If you want your class to be serialized add the appropriate
       declaration</b>
	\code
	class MyClass : public BaseObject {
		DECLARE_SC_CLASS(MyClass);

		// Add serialization interface
		DECLARE_SERIALIZATION;

		public:
			MyClass();

		private:
			int _myMember;
	};
	\endcode

	The serialization method has to be implemented the following way:
	\code
	void MyClass::serialize(Archive& ar) {
		// the archive will bind the name 'var1' to the member variable
		// _myMember
		ar & NAMED_OBJECT("var1", _myMember);
	}
	\endcode
  */
class SC_SYSTEM_CORE_API BaseObject {
	DECLARE_SC_CLASS(BaseObject);
	DECLARE_ROOT_SERIALIZATION(BaseObject);
	DECLARE_METAOBJECT_INTERFACE;


	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	protected:
		//! Constructor
		BaseObject();
		BaseObject(const BaseObject&);

	public:
		//! Destructor
		virtual ~BaseObject();


	// ----------------------------------------------------------------------
	//  Public methods
	// ----------------------------------------------------------------------
	public:
		//! Returns a shallow copy of this instance
		virtual BaseObject *clone() const;


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		virtual BaseObject &operator=(const BaseObject&);


	// ----------------------------------------------------------------------
	//  Reference counting
	// ----------------------------------------------------------------------
	public:
		//! Increment the reference counter
		void incrementReferenceCount() const;

		//! Decrement the reference counter and deletes the object
		//! when reaching 0
		void decrementReferenceCount() const;

		/**
		 * Returns the number of references to this object when using smartpointers
		 * @return current reference count
		 */
		unsigned int referenceCount() const;

		/**
		 * Returns the number of created objects of type BaseObject at the time
		 * of calling this function.
		 * @return number of objects created
		 */
		static unsigned int ObjectCount();


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		mutable volatile unsigned int _referenceCount;
		static  volatile unsigned int _objectCount;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include <seiscomp3/core/baseobject.inl>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_add_ref(Seiscomp::Core::BaseObject *p) {
	p->incrementReferenceCount();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_release(Seiscomp::Core::BaseObject *p) {
	p->decrementReferenceCount();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_add_ref(const Seiscomp::Core::BaseObject *p) {
	p->incrementReferenceCount();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void intrusive_ptr_release(const Seiscomp::Core::BaseObject *p) {
	p->decrementReferenceCount();
}


#endif
