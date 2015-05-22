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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct VectorReader {};

template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct VectorWriter {};

template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct ListReader {};

template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct ListWriter {};

template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct ContainerReader {};

template <typename ROOT_TYPE, typename T, int CLASS_TYPE>
struct ContainerWriter {};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

	template <typename DERIVEDCLASS, typename BASECLASS>
	class IsTypeOf {
		class No { };
		class Yes { No no[2]; };
	
		static Yes Test(BASECLASS*); // declared, but not defined
		static No Test(...); // declared, but not defined
	
		public:
			enum { Value = sizeof(Test(static_cast<DERIVEDCLASS*>(0))) == sizeof(Yes) };
	};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline bool checkRootType(TYPE*&) {
	return IsTypeOf<TYPE, ROOT_TYPE>::Value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline bool checkRootType(::boost::intrusive_ptr<TYPE>&) {
	return IsTypeOf<TYPE, ROOT_TYPE>::Value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline bool checkRootType(TYPE&) {
	return IsTypeOf<TYPE, ROOT_TYPE>::Value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline const char* checkClassName(const ROOT_TYPE**, const TYPE*) {
	return TYPE::ClassName();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline const char* checkClassName(const ROOT_TYPE*, const TYPE&) {
	return TYPE::ClassName();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline const char* checkClassName(const ::boost::intrusive_ptr<TYPE>*, const ::boost::intrusive_ptr<TYPE>&) {
	return TYPE::ClassName();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline const char* checkClassName(const void**, const TYPE*) {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename TYPE>
inline const char* checkClassName(const void*, const TYPE&) {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T, int CLASS_TYPE>
struct ClassQuery {};

template <typename T>
struct ClassQuery<T,0> {
	const char* operator()() const { return NULL; }
};
template <typename T>
struct ClassQuery<T,1> {
	const char* operator()() const { return T::ClassName(); }
};

template <typename ROOT_TYPE, typename TYPE>
const char* checkClassName(const ::boost::optional<TYPE>*, const ::boost::optional<TYPE>&) {
	ClassQuery<TYPE, boost::is_base_of<ROOT_TYPE, TYPE>::value> query;
	return query();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator<<(T*& object) {
	write(NULL, object, T::ClassName());
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator<<(::boost::intrusive_ptr<T>& object) {
	write(NULL, object.get(), T::ClassName());
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct ContainerWriter<ROOT_TYPE, T,1> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<T>& namedObject) {
		ar << ObjectNamer<typename T::ContainerType>(namedObject.name(), namedObject.object().container(), namedObject.hint());
		/*
		namedObject.object().reset();
		while ( namedObject.object().next() )
			ar.write(namedObject.name(), namedObject.object().get(), checkClassName<ROOT_TYPE>(&namedObject.object().get(), namedObject.object().get()));
		*/
	}
};


template <typename ROOT_TYPE, typename T>
struct ContainerWriter<ROOT_TYPE, T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<T>& namedObject) {
		ar.write(namedObject.name(), namedObject.object(), checkClassName<ROOT_TYPE>(&namedObject.object(), namedObject.object()));
	}
};

template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator<<(const ObjectNamer<T>& namedObject) {
	int h = setChildHint(namedObject.hint());
	//setHint(h | namedObject.hint());
	ContainerWriter<ROOT_TYPE, T,boost::is_const<T>::value?1:0> writer;
	writer(*this, namedObject);
	setHint(h);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct VectorWriter<ROOT_TYPE, T,1> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::vector<T> >& namedObject) {
		typename std::vector<T>::iterator it;

		ar.writeSequence(namedObject.object().size());

		ar._first = true;
		for ( it = namedObject.object().begin(); it != namedObject.object().end(); ++it ) {
			ar.write(namedObject.name(), *it, checkClassName<ROOT_TYPE>(&(*it), *it));
			ar._first = false;
		}
		ar._first = true;
	}
};


template <typename ROOT_TYPE, typename T>
struct VectorWriter<ROOT_TYPE, T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::vector<T> >& namedObject) {
		if ( ar.locateObjectByName(namedObject.name(), NULL, false) )
			ar.write(namedObject.object());
	}
};


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator<<(const ObjectNamer<std::vector<T> >& namedObject) {
	int h = setChildHint(namedObject.hint());

	typedef typename boost::remove_pointer<T>::type RAW_T;

	VectorWriter<ROOT_TYPE,T,
		boost::is_class<RAW_T>::value?
			(
				boost::is_same<std::complex<double>,T>::value?
				0
				:
				(
					boost::is_same<std::string,T>::value?
					0
					:
					1
				)
			)
			:
			0
	> writer;

	writer(*this, namedObject);

	setHint(h);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct ListWriter<ROOT_TYPE, T,1> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::list<T> >& namedObject) {
		typename std::list<T>::iterator it;

		ar.writeSequence(namedObject.object().size());

		ar._first = true;
		for ( it = namedObject.object().begin(); it != namedObject.object().end(); ++it ) {
			ar.write(namedObject.name(), *it, checkClassName<ROOT_TYPE>(&(*it), *it));
			ar._first = false;
		}
		ar._first = true;
	}
};


template <typename ROOT_TYPE, typename T>
struct ListWriter<ROOT_TYPE, T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::list<T> >& namedObject) {
		if ( ar.locateObjectByName(namedObject.name(), NULL, false) )
			ar.write(namedObject.object());
	}
};


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator<<(const ObjectNamer<std::list<T> >& namedObject) {
	int h = setChildHint(namedObject.hint());

	typedef typename boost::remove_pointer<T>::type RAW_T;

	ListWriter<ROOT_TYPE,T,boost::is_class<RAW_T>::value?(boost::is_same<std::complex<double>,T>::value?0:1):0> writer;
	writer(*this, namedObject);

	setHint(h);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(T*& object) {
	const char* classname = T::ClassName();
	if ( classname == NULL )
		return *this;

	//_validObject = true;
	
	read(classname, object, classname);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(::boost::intrusive_ptr<T>& object) {
	//_validObject = true;
	const char* classname = T::ClassName();
	if ( classname == NULL )
		return *this;

	read(classname, object, classname);

	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct ContainerReader<ROOT_TYPE, T,1> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<T>& namedObject) {
		const char* objectName = namedObject.name();
		typename T::Type value;

		bool oldState = ar.success();

		ar.readSequence();

		ObjectNamer<typename T::Type> namedItem = nameObject(objectName, value, namedObject.hint());

		ar._first = true;

		ar >> namedItem;
		while ( ar._found ) {
			if ( ar.success() )
				namedObject.object().add(value);
			ar._first = false;
			ar >> namedItem;
		}

		ar._first = true;
		ar._validObject = oldState;
	}
};


template <typename ROOT_TYPE, typename T>
struct ContainerReader<ROOT_TYPE,T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<T>& namedObject) {
		// goto the object location in the archive
		const char* classname = checkClassName<ROOT_TYPE>(&namedObject.object(), namedObject.object());
		ar.read(namedObject.name(), namedObject.object(), classname);
	}
};


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(const ObjectIterator<T>& it) {
	//_validObject = true;
	_first = it.first();
	*this >> it.object();
	_first = true;
	return *this;
}


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(const ObjectNamer<T>& namedObject) {
	int h = setChildHint(namedObject.hint());
	//setHint(h | namedObject.hint());
	
	//_validObject = true;
	ContainerReader<ROOT_TYPE,T,boost::is_const<T>::value?1:0> reader;
	reader(*this, namedObject);

	setHint(h);
	
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct VectorReader<ROOT_TYPE, T,1> {
void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::vector<T> >& namedObject) {
		const char* objectName = namedObject.name();
		T value;

		bool oldState = ar.success();

		ar.readSequence();
		ar._first = true;

		ObjectNamer<T> namedItem = nameObject(objectName, value, namedObject.hint());

		ar >> namedItem;
		while ( ar._found ) {
			if ( ar.success() )
				namedObject.object().push_back(value);
			ar._first = false;
			ar >> namedItem;
		}

		ar._first = true;
		ar._validObject = oldState;
	}
};


template <typename ROOT_TYPE, typename T>
struct VectorReader<ROOT_TYPE,T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::vector<T> >& namedObject) {
		if ( ar.locateObjectByName(namedObject.name(), NULL, false) )
			ar.read(namedObject.object());
	}
};


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(const ObjectNamer<std::vector<T> >& namedObject) {
	int h = setChildHint(namedObject.hint());
	//setHint(h | namedObject.hint());

	typedef typename boost::remove_pointer<T>::type RAW_T;
	
	//_validObject = true;

	VectorReader<ROOT_TYPE,T,
		boost::is_class<RAW_T>::value?
			(
				boost::is_same<std::complex<double>,T>::value?
				0
				:
				(
					boost::is_same<std::string,T>::value?
					0
					:
					1
				)
			)
			:
			0
	> reader;

	reader(*this, namedObject);

	setHint(h);
	
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE, typename T>
struct ListReader<ROOT_TYPE, T,1> {
void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::list<T> >& namedObject) {
		const char* objectName = namedObject.name();
		T value;

		bool oldState = ar.success();

		ar.readSequence();
		ar._first = true;

		ObjectNamer<T> namedItem = nameObject(objectName, value, namedObject.hint());

		ar >> namedItem;
		while ( ar._found ) {
			if ( ar.success() )
				namedObject.object().push_back(value);
			ar._first = false;
			ar >> namedItem;
		}

		ar._first = true;
		ar._validObject = oldState;
	}
};


template <typename ROOT_TYPE, typename T>
struct ListReader<ROOT_TYPE,T,0> {
	void operator()(Archive<ROOT_TYPE>& ar, const ObjectNamer<std::list<T> >& namedObject) {
		if ( ar.locateObjectByName(namedObject.name(), NULL, false) )
			ar.read(namedObject.object());
	}
};


template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator>>(const ObjectNamer<std::list<T> >& namedObject) {
	int h = setChildHint(namedObject.hint());
	//setHint(h | namedObject.hint());

	typedef typename boost::remove_pointer<T>::type RAW_T;
	
	//_validObject = true;
	typename boost::mpl::if_c<boost::is_class<RAW_T>::value,
	                          typename boost::mpl::if_c<boost::is_same<std::complex<double>, T>::value,
	                                                    ListReader<ROOT_TYPE,T,0>,
	                                                    ListReader<ROOT_TYPE,T,1> >::type,
	                          ListReader<ROOT_TYPE,T,0> >::type reader;
	reader(*this, namedObject);

	setHint(h);
	
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline Archive<ROOT_TYPE>& Archive<ROOT_TYPE>::operator&(ObjectNamer<T> namedObject) {
	isReading() ? *this >> namedObject : *this << namedObject;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(T*& object) {
	readPtr(object, object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(::boost::intrusive_ptr<T>& object) {
	T* ref = NULL;
	read(ref);
	object = ref;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(::boost::optional<T>& object) {
	bool oldState = success();

	object = T();

	read(*object);
	if ( !success() )
		object = boost::none;

	_validObject = oldState;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::readPtr(ROOT_TYPE*, T*& object) {
	if ( (hint() & STATIC_TYPE) == 0 ) {
		std::string className = determineClassName();
		if ( className.empty() ) return;
		
		if ( !ClassFactoryInterface<ROOT_TYPE>::IsTypeOf(T::ClassName(), className.c_str()) ) {
			_validObject = false;
			return;
		}
		object = static_cast<T*>(ClassFactoryInterface<ROOT_TYPE>::Create(className.c_str()));
		if ( !object )
			throw ClassNotFound(className);
	}
	else {
		object = static_cast<T*>(ClassFactoryInterface<ROOT_TYPE>::Create(T::ClassName()));
		if ( !object )
			throw ClassNotFound(T::ClassName());
	}

	if ( object != NULL )
		read(*object);
	else
		_validObject = false;

	if ( !success() && object != NULL ) {
		delete object;
		object = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::readPtr(void*, T*& object) {
	object = new T;
	read(*object);
	if ( !success() ) {
		delete object;
		object = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(T& object) {
	typename boost::mpl::if_c<boost::is_base_of<ROOT_TYPE, T>::value,
	                          ROOT_TYPE*,
	                          TypedSerializeDispatcher<T> >::type t;
	t = &object;
	serialize(t);
	//_validObject = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(const char* name, T& object, const char* targetClass) {
	if ( findObject(name, targetClass, false) )
		read(object);
	else if ( !(boost::is_base_of<std::string, T>::value || boost::is_same<std::string, T>::value) )
		_validObject = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(const char* name, T*& object, const char* targetClass) {
	if ( findObject(name, targetClass, true) )
		read(object);
	else {
		//_validObject = false;
		object = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(const char* name, ::boost::intrusive_ptr<T>& object, const char* targetClass) {
	if ( findObject(name, targetClass, true) )
		read(object);
	else {
		//_validObject = false;
		object = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::read(const char* name, ::boost::optional<T>& object, const char* targetClass) {
	if ( findObject(name, targetClass, true) )
		read(object);
	else {
		//_validObject = false;
		object = boost::none;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(T& object) {
	typename boost::mpl::if_c<boost::is_base_of<ROOT_TYPE, T>::value,
	                          ROOT_TYPE*,
	                          TypedSerializeDispatcher<T> >::type t;

	t = &object;
	setClassName(hint() & STATIC_TYPE?NULL:t->className());
	serialize(t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(T* object) {
	write(*object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(const char* name, T& object, const char* targetClass) {
	// goto the object location in the archive
	findObject(name, targetClass, false);
	// write the object data
	write(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(const char* name, T* object, const char* targetClass) {
	if ( object == NULL ) {
		locateNullObjectByName(name, targetClass, _first);
		return;
	}
	findObject(name, targetClass, true);
	// write the object data
	write(*object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(const char* name, ::boost::intrusive_ptr<T>& object, const char* targetClass) {
	write(name, object.get(), targetClass);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename ROOT_TYPE>
template <typename T>
inline void Archive<ROOT_TYPE>::write(const char* name, ::boost::optional<T>& object, const char* targetClass) {
	write(name, object.get_ptr(), targetClass);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
