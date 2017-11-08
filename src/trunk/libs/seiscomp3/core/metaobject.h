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


#ifndef __SC_METAOBJECT_H__
#define __SC_METAOBJECT_H__

#include <seiscomp3/core/rtti.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/exceptions.h>

#include <complex>
#include <vector>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>


namespace Seiscomp {
namespace Core {

class BaseObject;

typedef boost::any MetaValue;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API PropertyNotFoundException : public GeneralException {
	public:
		PropertyNotFoundException();
		PropertyNotFoundException(const std::string& str);
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API MetaEnum {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		MetaEnum();
		virtual ~MetaEnum();

	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Returns the number of keys in the enumeration
		virtual int keyCount() const = 0;

		//! Returns the key name at a given index
		virtual const char *key(int index) const = 0;

		virtual const char *valueToKey(int value) const = 0;
		virtual int keyToValue(const char *key) const = 0;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API MetaProperty {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		MetaProperty();
		MetaProperty(const std::string& name,
		             const std::string& type,
		             bool  isArray,
		             bool  isClass,
		             bool  isIndex,
		             bool  isReference,
		             bool  isOptional,
		             bool  isEnum,
		             const MetaEnum *enumeration = NULL);

		virtual ~MetaProperty();

	// ------------------------------------------------------------------
	//  Initialization
	// ------------------------------------------------------------------
	public:
		void setInfo(const std::string& name,
		             const std::string& type,
		             bool  isArray,
		             bool  isClass,
		             bool  isIndex,
		             bool  isReference,
		             bool  isOptional,
		             bool  isEnum,
		             const MetaEnum *enumeration = NULL);


	// ------------------------------------------------------------------
	//  Getters
	// ------------------------------------------------------------------
	public:
		const std::string& name() const;
		const std::string& type() const;

		//! Returns a meta enum description if this property
		//! is an enumeration
		const MetaEnum *enumerator() const;

		bool isArray() const;
		bool isClass() const;
		bool isIndex() const;
		bool isReference() const;
		bool isEnum() const;
		bool isOptional() const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Creates an instance of the properties class type
		virtual BaseObject *createClass() const;

		//! Returns the number of elements in the array if isArray() is
		//! true, -1 otherwise
		virtual size_t arrayElementCount(const BaseObject *object) const;

		//! Returns the object at position i, NULL otherwise
		virtual BaseObject *arrayObject(BaseObject *object, int i) const;

		//! Adds a child to an object.
		//! If the property is not an array type, nothing will be done
		//! and 'false' will be returned.
		virtual bool arrayAddObject(BaseObject *object, BaseObject *child) const;

		//! Removes a child from an object at a given index.
		virtual bool arrayRemoveObject(BaseObject *object, int i) const;

		//! Removes a child from an object.
		virtual bool arrayRemoveObject(BaseObject *object, BaseObject *child) const;

		//! Writes a value to an objects property.
		//! Returns true when the value has been written, false otherwise
		//! Throws:
		//!  PropertyNotFoundException: The property does not exist
		//!  TypeException: The type of the property is not compatible with value
		virtual bool write(BaseObject *object, MetaValue value) const;

		//! Writes a value (as string representation) to an objects property.
		//! Returns true when the value has been written, false otherwise
		//! Throws:
		//!  PropertyNotFoundException: The property does not exist
		//!  TypeException: The type of the property is not compatible with value
		virtual bool writeString(BaseObject *object, const std::string &value) const;

		//! Reads a value from an objects property.
		//! The returned value can be empty (empty()) if the object
		//! does not have a corresponding property or the property
		//! is not set.
		//! Throws:
		//!  PropertyNotFoundException: The property does not exist
		virtual MetaValue read(const BaseObject *object) const;

		//! Reads a value from an objects property as string representation.
		//! The returned value can be empty (empty()) if the object
		//! does not have a corresponding property or the property
		//! is not set or the datatype does not support string conversion
		//! Throws:
		//!  PropertyNotFoundException: The property does not exist
		virtual std::string readString(const BaseObject *object) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		std::string _name;
		std::string _type;
		bool _isArray;
		bool _isClass;
		bool _isIndex;
		bool _isReference;
		bool _isEnum;
		bool _isOptional;

		const MetaEnum *_enumeration;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief A metaobject (class information and access) class

	To implement it into custom classes, use the supported METAOBJECT
	macros.

	\code
	class MyClass {
		DECLARE_METAOBJECT;
		...
		public:
			MyClass();
		...
	};
	\endcode

	To implement the metaobject interface, another macro has to be used.
	\code
	IMPLEMENT_METAOBJECT(MyClass);
	\endcode
 */
typedef boost::shared_ptr<MetaProperty> MetaPropertyHandle;

class SC_SYSTEM_CORE_API MetaObject {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		MetaObject(const RTTI* rtti, const MetaObject *base = NULL);
		~MetaObject();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the corresponding RTTI object
		const RTTI *rtti() const;

		//! Returns the base MetaObject if available
		const MetaObject *base() const;

		//! Returns the number of properties set
		size_t propertyCount() const;

		//! Returns the property at a given position
		const MetaProperty *property(size_t index) const;

		//! Returns the property with a given name
		const MetaProperty *property(const std::string &name) const;


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		void clearProperties();

		bool addProperty(const std::string& name, const std::string& type,
		                 bool  isArray, bool  isClass, bool  isIndex,
		                 bool isReference, bool  isOptional, bool  isEnum,
		                 const MetaEnum *enumeration = NULL);

		bool addProperty(MetaPropertyHandle);

		bool removeProperty(size_t index);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		const RTTI* _rtti;
		const MetaObject *_base;
		std::vector<MetaPropertyHandle> _properties;
};

typedef boost::shared_ptr<MetaObject> MetaObjectHandle;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DECLARE_METAOBJECT_INTERFACE \
	public: \
		static const Seiscomp::Core::MetaObject *Meta(); \
		\
		virtual const Seiscomp::Core::MetaObject *meta() const

#define DECLARE_METAOBJECT \
	DECLARE_METAOBJECT_INTERFACE; \
	protected: \
		class MetaObject  : public Seiscomp::Core::MetaObject { \
			public: \
				MetaObject(const Seiscomp::Core::RTTI* rtti); \
		}

#define DECLARE_METAOBJECT_DERIVED \
	DECLARE_METAOBJECT_INTERFACE; \
	protected: \
		class MetaObject  : public Seiscomp::Core::MetaObject { \
			public: \
				MetaObject(const Seiscomp::Core::RTTI* rtti, const Seiscomp::Core::MetaObject *base = NULL); \
		}

#define IMPLEMENT_METAOBJECT_EMPTY_METHODS(CLASS) \
	const Seiscomp::Core::MetaObject *CLASS::Meta() { \
		return NULL; \
	} \
	\
	const Seiscomp::Core::MetaObject *CLASS::meta() const { \
		return NULL; \
	}

#define IMPLEMENT_METAOBJECT_METHODS(CLASS) \
	const Seiscomp::Core::MetaObject *CLASS::meta() const { \
		return Meta(); \
	}


#define IMPLEMENT_METAOBJECT(CLASS) \
	IMPLEMENT_METAOBJECT_METHODS(CLASS) \
	const Seiscomp::Core::MetaObject *CLASS::Meta() { \
		static CLASS::MetaObject classMetaObject(&CLASS::TypeInfo()); \
		return &classMetaObject; \
	}


#define IMPLEMENT_METAOBJECT_DERIVED(CLASS, BASECLASS) \
	IMPLEMENT_METAOBJECT_METHODS(CLASS) \
	const Seiscomp::Core::MetaObject *CLASS::Meta() { \
		static CLASS::MetaObject classMetaObject(&CLASS::TypeInfo(), BASECLASS::Meta()); \
		return &classMetaObject; \
	}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}


#endif
