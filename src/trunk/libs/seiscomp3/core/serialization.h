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


#ifndef __SCSERIALIZATION_H__
#define __SCSERIALIZATION_H__


namespace Seiscomp {
namespace Core {
namespace Generic {

template <typename T> class Archive;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! Class used to give names to objects
template<typename T>
class ObjectNamer {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef T Type;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Construction requires a reference to the object and the name to give the object
		ObjectNamer(const char* name, Type& object, int h = 0)
		  : _pair(name, &object), _hint(h) {}


	// ------------------------------------------------------------------
	//  Public Interface
	// ------------------------------------------------------------------
	public:
		//! Returns the name of the object
		const char* name() const { return _pair.first; }

		//! Returns a reference to the object being named
		Type& object() const { return *_pair.second; }

		//! Returns the hint for serialization
		int hint() const { return _hint; }


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		std::pair<const char*, Type*> _pair; //!< The pair which maps a name to an object
		int _hint;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! Class used to iterate over a sequence of objects
template<typename T>
class ObjectIterator {
	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef T Type;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Construction requires a reference to the object and the name to give the object
		ObjectIterator(Type& object, bool first) : _object(&object), _first(first) {}


	// ------------------------------------------------------------------
	//  Public Interface
	// ------------------------------------------------------------------
	public:
		//! Returns the flag to state whether to read the first
		//! object or the next in the sequence
		bool first() const { return _first; }

		//! Returns a reference to the object being read
		Type& object() const { return *_object; }

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		Type* _object; //!< The object which has to be read
		bool _first; //!< Initialize
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//NOTE Because of a bug in the SUNPRO compiler up to version 5.10
//     a non template containertype is being used.
//template<template <typename, typename> class C, typename T, typename ADD>
template<class C, typename T, typename ADD>
class ObjectContainer {
	public:
		typedef T Type;
		//typedef C<T, std::allocator<T> > ContainerType;
		typedef C ContainerType;
	
		ObjectContainer(ContainerType& c, ADD a) : _container(&c), _add(a) {}

		ContainerType& container() const {
			return *_container;
		}
	
		bool add(T& v) const {
			_add(v);
			return true;
		}

	private:
		mutable ContainerType* _container;
		mutable ADD _add;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
ObjectNamer<T> nameObject(const char* name, T& object, int h = 0) {
	return ObjectNamer<T>(name, object, h);
}
template<typename T>
ObjectNamer<const T> nameObject(const char* name, const T& object, int h = 0) {
	return ObjectNamer<const T>(name, object, h);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
ObjectIterator<T> itObject(T& object, bool first) {
	return ObjectIterator<T>(object, first);
}
template<typename T>
ObjectIterator<const T> itObject(const T& object, bool first) {
	return ObjectIterator<const T>(object, first);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<template <typename, typename> class C, typename T, typename ADD>
ObjectContainer<C<T, std::allocator<T> >, T, ADD> containerMember(C<T, std::allocator<T> >& container, ADD add) {
	return ObjectContainer<C<T, std::allocator<T> >, T, ADD>(container, add);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename Tgt, class ret, class T, class Arg>
std::unary_compose<std::binder1st<std::mem_fun1_t<ret, T, Tgt*> >,
                   std::const_mem_fun_ref_t<Tgt*, typename Core::SmartPointer<Tgt>::Impl> >
bindMemberFunction(ret (T::*f)(Arg), T* c) {
	return std::compose1(
	         std::bind1st(
	           std::mem_fun(f),
	           c),
	         std::mem_fun_ref(&Core::SmartPointer<Tgt>::Impl::get)
	       );
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#define TAGGED_MEMBER(x) Seiscomp::Core::Generic::nameObject(#x, _##x)
#define NAMED_OBJECT(name, x) Seiscomp::Core::Generic::nameObject(name, x)
#define NAMED_OBJECT_HINT(name, x, h) Seiscomp::Core::Generic::nameObject(name, x, h)

#define SEQUENCE_OBJECT(object, first) Seiscomp::Core::Generic::itObject(object, first)
#define FIRST_OBJECT(object) Seiscomp::Core::Generic::itObject(object, true)
#define NEXT_OBJECT(object) Seiscomp::Core::Generic::itObject(object, false)


}
}
}


#endif
