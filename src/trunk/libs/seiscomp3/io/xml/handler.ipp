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

template <typename T>
void TypedClassHandler<T>::addProperty(const char *t, const char *ns, Type opt, Location l, const Core::MetaProperty *prop) {
	addMember(t, ns, opt, l, new PropertyHandler(prop));
}


template <typename T>
void TypedClassHandler<T>::addProperty(const char *t, const char *ns, Type opt, Location l, const char *property) {
	const Core::MetaObject *obj = T::Meta();
	if ( obj == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	const Core::MetaProperty *prop = NULL;

	while ( obj && prop == NULL ) {
		prop = obj->property(property);
		obj = obj->base();
	}

	if ( prop == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty " + property);

	addProperty(t, ns, opt, l, prop);
}


template <typename T>
void TypedClassHandler<T>::addChildProperty(const char *t, const char *ns, const char *property) {
	const Core::MetaObject *obj = T::Meta();
	if ( obj == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	const Core::MetaProperty *prop = NULL;

	while ( obj && prop == NULL ) {
		prop = obj->property(property);
		obj = obj->base();
	}

	if ( prop == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty " + property);

	if ( !prop->isArray() )
		throw Core::TypeException(std::string(T::ClassName()) + ": " + property + " property is not an array");

	addChild(t, ns, new ChildPropertyHandler(prop));
}


template <class C, typename R, typename T>
void ClassHandler::addMember(const char *t, const char *ns, Type opt, Location l, R (C::*s)(T)) {
	switch ( l ) {
		case Attribute:
			attributes.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			break;
		case Element:
			elements.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			orderedMembers.push_back(&elements.back());
			break;
		case CDATA:
			cdata = MemberNodeHandler(t, ns, (bool)opt, s);
			cdataUsed = true;
			break;
		default:
			break;
	}
}

template <class C, typename T1, typename T2, typename R>
void ClassHandler::addMember(const char *t, const char *ns, Type opt, R (C::*s)(T1), T2 (C::*g)()) {
	elements.push_back(MemberNodeHandler(t, ns, (bool)opt, g, s));
	orderedMembers.push_back(&childs.back());
}


template <typename T>
void TypeMap::registerMapping(const char *tag, const char *ns, NodeHandler *handler) {
	TypeHandler *h = new TypeStaticHandler<T>(handler);
	tags[Tag(tag, ns)] = h->className();

	std::pair<RawTagMap::iterator,bool> itp;
	itp = tagsWithoutNs.insert(RawTagMap::value_type(tag, h->className()));

	// Tag exists already -> set invalid classname
	if ( !itp.second ) itp.first->second.clear();

	classes[h->className()] = Tag(tag, ns);
	handlers[h->className()] = h;
}
