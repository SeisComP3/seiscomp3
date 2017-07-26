/***************************************************************************
 * Copyright (C) 2007 by GFZ Potsdam
 *
 * Author: Jan Becker
 * Email: seiscomp-devel@gfz-potsdam.de
 * $Date$
 *
 * $Revision$
 * $LastChangedDate$
 * $LastChangedBy$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ***************************************************************************/

#ifndef __SEISCOMP_FDSNXML_METADATA_H__
#define __SEISCOMP_FDSNXML_METADATA_H__


#include <seiscomp3/core/metaproperty.h>


namespace Seiscomp {
namespace FDSNXML {


namespace Generic {


template <typename T, typename U, typename F1, typename F2, int>
class EnumPropertyBase {};


//! Non-optional enum property specialization
template <typename T, typename U, typename F1, typename F2>
class EnumPropertyBase<T, U, F1, F2, 0> : public Core::MetaProperty {
	public:
		EnumPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;
			U tmp;
			if ( !tmp.fromInt(boost::any_cast<int>(value)) )
				return false;

			(target->*_setter)(tmp);
			return true;
		}

		bool writeString(Core::BaseObject *object, const std::string &value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;
			typename Core::Generic::remove_optional<U>::type tmp;
			if ( !tmp.fromString(value.c_str()) )
				return false;

			(target->*_setter)(tmp);
			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toInt();
		}

		std::string readString(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toString();
		}

	private:
		F1 _setter;
		F2 _getter;
};


//! Optional enum property specialization
template <typename T, typename U, typename F1, typename F2>
class EnumPropertyBase<T, U, F1, F2, 1> : public Core::MetaProperty {
	public:
		EnumPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				typename U::value_type tmp;
				if ( !tmp.fromInt(boost::any_cast<int>(value)) )
					return false;

				(target->*_setter)(tmp);
			}

			return true;
		}

		bool writeString(Core::BaseObject *object, const std::string &value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				typename Core::Generic::remove_optional<U>::type tmp;
				if ( !tmp.fromString(value.c_str()) )
					return false;

				(target->*_setter)(tmp);
			}

			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toInt();
		}

		std::string readString(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return (target->*_getter)().toString();
		}

	private:
		F1 _setter;
		F2 _getter;
};



template <typename A, typename T, typename U, typename F1, typename F2, int>
class BaseObjectPropertyBase {};


//! Non-optional baseobject property specialization
template <typename A, typename T, typename U, typename F1, typename F2>
class BaseObjectPropertyBase<A, T, U, F1, F2, 0> : public Core::MetaClassProperty<A> {
	public:
		BaseObjectPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;

			const Core::BaseObject *v;
			try {
				v = boost::any_cast<const Core::BaseObject*>(value);
			}
			catch ( boost::bad_any_cast & ) {
				try {
					v = boost::any_cast<Core::BaseObject*>(value);
				}
				catch ( boost::bad_any_cast & ) {
					try {
						v = boost::any_cast<const U*>(value);
					}
					catch ( boost::bad_any_cast & ) {
						v = boost::any_cast<U*>(value);
					}
				}
			}

			if ( v == NULL )
				throw Core::GeneralException("value must not be NULL");

			const U *uv = U::ConstCast(v);
			if ( uv == NULL )
				throw Core::GeneralException("value has wrong classtype");

			(target->*_setter)(*uv);
			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return static_cast<Core::BaseObject*>(&(const_cast<T*>(target)->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};


//! Optional baseobject property specialization
template <typename A, typename T, typename U, typename F1, typename F2>
class BaseObjectPropertyBase<A, T, U, F1, F2, 1> : public Core::MetaClassProperty<A> {
	public:
		BaseObjectPropertyBase(F1 setter, F2 getter)
		 : _setter(setter), _getter(getter) {}

		bool write(Core::BaseObject *object, Core::MetaValue value) const {
			T *target = T::Cast(object);
			if ( !target ) return false;

			if ( value.empty() )
				(target->*_setter)(Core::None);
			else {
				const Core::BaseObject *v;
				try {
					v = boost::any_cast<const Core::BaseObject*>(value);
				}
				catch ( boost::bad_any_cast & ) {
					try {
						v = boost::any_cast<Core::BaseObject*>(value);
					}
					catch ( boost::bad_any_cast & ) {
						try {
							v = boost::any_cast<const typename U::value_type*>(value);
						}
						catch ( boost::bad_any_cast & ) {
							v = boost::any_cast<typename U::value_type*>(value);
						}
					}
				}

				if ( v == NULL )
					throw Core::GeneralException("value must not be NULL");

				const typename U::value_type *uv = U::value_type::ConstCast(v);
				if ( uv == NULL )
					throw Core::GeneralException("value has wrong classtype");

				(target->*_setter)(*uv);
				return true;
			}

			return true;
		}

		Core::MetaValue read(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");
			return static_cast<Core::BaseObject*>(&(const_cast<T*>(target)->*_getter)());
		}

	private:
		F1 _setter;
		F2 _getter;
};



template <typename T, typename U, typename FCOUNT, typename FOBJ, typename FADD, typename FERASE1, typename FERASE2>
class ArrayProperty : public Core::MetaProperty {
	public:
		ArrayProperty(FCOUNT countObjects, FOBJ getObj, FADD addObj, FERASE1 eraseObjIndex, FERASE2 eraseObjPointer)
		 : _countObjects(countObjects),
		   _getObj(getObj),
		   _addObj(addObj),
		   _eraseObjIndex(eraseObjIndex),
		   _eraseObjPointer(eraseObjPointer) {}

		size_t arrayElementCount(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return static_cast<size_t>((target->*_countObjects)());
		}

		Core::BaseObject *arrayObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_getObj)(i);
		}

		bool arrayAddObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_addObj)(child);
		}

		bool arrayRemoveObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_eraseObjIndex)(i);
		}

		bool arrayRemoveObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_eraseObjPointer)(child);
		}

	private:
		FCOUNT _countObjects;
		FOBJ _getObj;
		FADD _addObj;
		FERASE1 _eraseObjIndex;
		FERASE2 _eraseObjPointer;
};


template <typename A, typename T, typename U, typename FCOUNT, typename FOBJ, typename FADD, typename FERASE1, typename FERASE2>
class ArrayClassProperty : public Core::MetaClassProperty<A> {
	public:
		ArrayClassProperty(FCOUNT countObjects, FOBJ getObj, FADD addObj, FERASE1 eraseObjIndex, FERASE2 eraseObjPointer)
		 : _countObjects(countObjects),
		   _getObj(getObj),
		   _addObj(addObj),
		   _eraseObjIndex(eraseObjIndex),
		   _eraseObjPointer(eraseObjPointer) {}

		size_t arrayElementCount(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return static_cast<size_t>((target->*_countObjects)());
		}

		Core::BaseObject *arrayObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_getObj)(i);
		}

		bool arrayAddObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_addObj)(child);
		}

		bool arrayRemoveObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_eraseObjIndex)(i);
		}

		bool arrayRemoveObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_eraseObjPointer)(child);
		}

	private:
		FCOUNT _countObjects;
		FOBJ _getObj;
		FADD _addObj;
		FERASE1 _eraseObjIndex;
		FERASE2 _eraseObjPointer;
};


class MetaAnyClassProperty : public Core::MetaProperty {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		MetaAnyClassProperty() : Core::MetaProperty() {}

		MetaAnyClassProperty(const std::string& name,
		                     const std::string& type,
		                     bool  isArray,
		                     bool  isIndex,
		                     bool  isOptional)
		: Core::MetaProperty(name, type, isArray, true, isIndex,
		                     isOptional, false, false, NULL) {}


	public:
		Core::BaseObject *createClass() const {
			return NULL;
		}
};


template <typename T, typename U, typename FCOUNT, typename FOBJ, typename FADD, typename FERASE1, typename FERASE2>
class ArrayAnyClassProperty : public MetaAnyClassProperty {
	public:
		ArrayAnyClassProperty(FCOUNT countObjects, FOBJ getObj, FADD addObj, FERASE1 eraseObjIndex, FERASE2 eraseObjPointer)
		 : _countObjects(countObjects),
		   _getObj(getObj),
		   _addObj(addObj),
		   _eraseObjIndex(eraseObjIndex),
		   _eraseObjPointer(eraseObjPointer) {}

		size_t arrayElementCount(const Core::BaseObject *object) const {
			const T *target = T::ConstCast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return static_cast<size_t>((target->*_countObjects)());
		}

		Core::BaseObject *arrayObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_getObj)(i);
		}

		bool arrayAddObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_addObj)(child);
		}

		bool arrayRemoveObject(Core::BaseObject *object, int i) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			return (target->*_eraseObjIndex)(i);
		}

		bool arrayRemoveObject(Core::BaseObject *object, Core::BaseObject *ch) const {
			T *target = T::Cast(object);
			if ( !target ) throw Core::GeneralException("invalid object");

			U *child = U::Cast(ch);
			if ( !child )  throw Core::GeneralException("wrong child class type");

			return (target->*_eraseObjPointer)(child);
		}

	private:
		FCOUNT _countObjects;
		FOBJ _getObj;
		FADD _addObj;
		FERASE1 _eraseObjIndex;
		FERASE2 _eraseObjPointer;
};


}



template <typename T, typename U, typename F1, typename F2>
class EnumProperty : public Generic::EnumPropertyBase<T, U, F1, F2, Core::Generic::is_optional<U>::value> {
	public:
		EnumProperty(F1 setter, F2 getter)
		 : Generic::EnumPropertyBase<T, U, F1, F2, Core::Generic::is_optional<U>::value>(setter, getter) {}
};


template <class C, typename R1, typename T1, typename T2>
Core::MetaPropertyHandle enumProperty(
	const std::string& name, const std::string& type,
	bool isIndex, bool isOptional,
	const Core::MetaEnum *enumeration,
	R1 (C::*setter)(T1), T2 (C::*getter)() const) {
	return Core::createProperty<EnumProperty>(
			name, type, false, false, isIndex,
			false, isOptional, true, enumeration,
			setter, getter);
}


template <typename A, typename T, typename U, typename F1, typename F2>
class ObjectProperty : public Generic::BaseObjectPropertyBase<A, T, U, F1, F2, Core::Generic::is_optional<U>::value> {
	public:
		ObjectProperty(F1 setter, F2 getter)
		 : Generic::BaseObjectPropertyBase<A, T, U, F1, F2, Core::Generic::is_optional<U>::value>(setter, getter) {}
};


template <typename A, typename C, typename R1, typename T1, typename T2>
Core::MetaPropertyHandle objectProperty(
	const std::string& name, const std::string& type, bool isIndex, bool isOptional,
	R1 (C::*setter)(T1), T2 (C::*getter)()) {

	typedef typename boost::remove_const<
		typename boost::remove_cv<
			typename boost::remove_pointer<
				typename boost::remove_reference<T1>::type
			>::type
		>::type
	>::type T;

	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(new ObjectProperty<A, C, T, R1 (C::*)(T1), T2 (C::*)()>(setter, getter));
	h->setInfo(name, type, false, true, isIndex, false, isOptional, false, NULL);
	return h;
}



template <template <typename, typename, typename, typename, typename, typename, typename> class P,
          class C, typename T>
Core::MetaPropertyHandle createArrayProperty(const std::string& name, const std::string& type,
                                             size_t (C::*counter)() const,
                                             T* (C::*getter)(size_t) const,
                                             bool (C::*adder)(T *),
                                             bool (C::*indexRemove)(size_t),
                                             bool (C::*ptrRemove)(T *)) {
	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(
		new P<C, T, size_t (C::*)() const, T* (C::*)(size_t i) const, bool (C::*)(T *), bool (C::*)(size_t i), bool (C::*)(T *)>(counter, getter ,adder, indexRemove, ptrRemove));
	h->setInfo(name, type, true, true, false, false, false, false, NULL);
	return h;
}


template <typename A, template <typename ,typename, typename, typename, typename, typename, typename, typename> class P,
          class C, typename T>
Core::MetaPropertyHandle createArrayClassProperty(const std::string& name, const std::string& type,
                                                  size_t (C::*counter)() const,
                                                  T* (C::*getter)(size_t) const,
                                                  bool (C::*adder)(T *),
                                                  bool (C::*indexRemove)(size_t),
                                                  bool (C::*ptrRemove)(T *)) {
	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(
		new P<A, C, T, size_t (C::*)() const, T* (C::*)(size_t i) const, bool (C::*)(T *), bool (C::*)(size_t i), bool (C::*)(T *)>(counter, getter ,adder, indexRemove, ptrRemove));
	h->setInfo(name, type, true, true, false, false, false, false, NULL);
	return h;
}


template <template <typename, typename, typename, typename, typename, typename, typename> class P,
          class C, typename T>
Core::MetaPropertyHandle createArrayAnyClassProperty(const std::string& name,
                                                  size_t (C::*counter)() const,
                                                  T* (C::*getter)(size_t) const,
                                                  bool (C::*adder)(T *),
                                                  bool (C::*indexRemove)(size_t),
                                                  bool (C::*ptrRemove)(T *)) {
	Core::MetaPropertyHandle h = Core::MetaPropertyHandle(
		new P<C, T, size_t (C::*)() const, T* (C::*)(size_t i) const, bool (C::*)(T *), bool (C::*)(size_t i), bool (C::*)(T *)>(counter, getter ,adder, indexRemove, ptrRemove));
	h->setInfo(name, "", true, true, false, false, false, false, NULL);
	return h;
}


template <typename C, typename T>
Core::MetaPropertyHandle arrayObjectProperty(
	const std::string& name, const std::string& type,
	size_t (C::*counter)() const,
	T* (C::*getter)(size_t) const,
	bool (C::*adder)(T *),
	bool (C::*indexRemove)(size_t),
	bool (C::*ptrRemove)(T *)
	) {
	return createArrayProperty<Generic::ArrayProperty>(name, type, counter, getter, adder, indexRemove, ptrRemove);
}


template <typename A, typename C, typename T>
Core::MetaPropertyHandle arrayClassProperty(
	const std::string& name, const std::string& type,
	size_t (C::*counter)() const,
	T* (C::*getter)(size_t) const,
	bool (C::*adder)(T *),
	bool (C::*indexRemove)(size_t),
	bool (C::*ptrRemove)(T *)
	) {
	return createArrayClassProperty<A, Generic::ArrayClassProperty>(name, type, counter, getter, adder, indexRemove, ptrRemove);
}


template <typename C, typename T>
Core::MetaPropertyHandle arrayAnyClassProperty(
	const std::string& name,
	size_t (C::*counter)() const,
	T* (C::*getter)(size_t) const,
	bool (C::*adder)(T *),
	bool (C::*indexRemove)(size_t),
	bool (C::*ptrRemove)(T *)
	) {
	return createArrayAnyClassProperty<Generic::ArrayAnyClassProperty>(name, counter, getter, adder, indexRemove, ptrRemove);
}


}
}


#endif
