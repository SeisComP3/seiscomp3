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



#define SEISCOMP_COMPONENT XMLHandler
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/xml/handler.h>
#include <libxml/xmlreader.h>
#include <string.h>
#include <iostream>


namespace Seiscomp {
namespace IO {
namespace XML {


OutputHandler::~OutputHandler() {}


bool NodeHandler::strictNsCheck = true;

NodeHandler::~NodeHandler() {}


void NodeHandler::propagate(Core::BaseObject *o, bool ni, bool opt) {
	object = o;
	newInstance = ni;
	isOptional = opt;
	isAnyType = false;
	childHandler = NULL;
	memberHandler = NULL;
}


std::string NodeHandler::content(void *n) const {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);
	for ( xmlNodePtr child = node->children; child != NULL; child = child->next ) {
		if ( child->type == XML_TEXT_NODE ) {
			xmlChar* content = xmlNodeGetContent(child);
			std::string str;
			if ( content ) {
				str = (const char*)content;
				Core::trim(str);
				xmlFree(content);
			}

			return str;
		}
	}

	return "";
}


bool NodeHandler::equalsTag(void *n, const char *tag, const char *ns) const {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);

	if ( strictNsCheck ) {
		// No namespace requested
		if ( ns == NULL || *ns == '\0' ) {
			// The node also must not have an associated namespace
			if ( node->ns && node->ns->href && *node->ns->href != '\0' )
				return false;

			return strcmp((const char*)node->name, tag) == 0;
		}
	}

	if ( strcmp((const char*)node->name, tag) )
		return false;
	else if ( !strictNsCheck )
		return true;

	xmlNsPtr nns = node->ns;
	for ( ; nns != NULL; nns = nns->next ) {
		if ( !strcmp((const char*)nns->href, ns) )
			return true;
	}

	return false;
}



MemberHandler::~MemberHandler() {}


bool MemberHandler::put(Core::BaseObject *object, const char *tag, const char *ns,
                        bool opt, OutputHandler *output, NodeHandler *h) {
	try {
		std::string v = value(object);
		if ( !v.empty() ) {
			output->openElement(tag, ns);
			output->put(v.c_str());
			output->closeElement(tag, ns);
			return true;
		}
		else if ( !opt ) {
			output->openElement(tag, ns);
			output->closeElement(tag, ns);
			return true;
		}
	}
	catch ( ... ) {}

	return false;
}


MemberNodeHandler::MemberNodeHandler(const char *t, const char *ns, bool opt, MemberHandler *s)
	: tag(t), nameSpace(ns), optional(opt) {
	setter = boost::shared_ptr<MemberHandler>(s);
}


bool MemberNodeHandler::put(Core::BaseObject *obj, OutputHandler *output, NodeHandler *h) {
	return setter->put(obj, tag.c_str(), nameSpace.c_str(), optional, output, h);
}


bool MemberNodeHandler::get(Core::BaseObject *obj, void *n, NodeHandler *h) {
	if ( setter->get(obj, n, h) ) return true;

	if ( !optional ) {
		if ( tag.empty() )
			throw Core::ValueException("invalid value in CDATA: non optional member");
		else
			throw Core::ValueException(std::string("invalid value '") + h->content(n) + "' in " + tag + ": non optional member");
	}

	return false;
}


bool MemberNodeHandler::finalize(Core::BaseObject *parent, Core::BaseObject *child) {
	return setter->finalize(parent, child);
}


bool PropertyHandler::get(Core::BaseObject *object, void *n, NodeHandler *h) {
	if ( _property->isClass() ) {
		if ( _property->isOptional() ) {
			Core::BaseObject *member = _property->createClass();
			if ( member != NULL ) {
				if ( !_property->write(object, member) ) {
					delete member;
					h->propagate(NULL, false, true);
					return true;
				}
				delete member;
			}
		}

		h->propagate(boost::any_cast<Core::BaseObject*>(_property->read(object)), false, _property->isOptional());
		return true;
	}

	std::string v = h->content(n);
	//if ( v.empty() ) return false;
	return _property->writeString(object, v);
}


bool PropertyHandler::finalize(Core::BaseObject *parent, Core::BaseObject *member) {
	if ( !member ) {
		if ( _property->isClass() && _property->isOptional() )
			_property->write(parent, Core::MetaValue());
		return true;
	}
	_property->write(parent, member);
	/*
	if ( _property->isOptional() )
		delete member;
	*/
	return true;
}



bool NoneHandler::put(Core::BaseObject *, const char *, const char *, OutputHandler *) {
	return false;
}


bool NoneHandler::get(Core::BaseObject *, void *n) {
	//xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);
	//std::cout << "ignored node " << node->name << std::endl;
	return false;
}


bool GenericHandler::put(Core::BaseObject *, const char *, const char *, OutputHandler *) {
	return false;
}

bool GenericHandler::get(Core::BaseObject *, void *n) {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);

	const char *classname = mapper->getClassname((const char*)node->name, node->ns?(const char*)node->ns->href:"", strictNsCheck);
	if ( classname == NULL ) {
		SEISCOMP_DEBUG("No class mapping for %s, ns = '%s'", node->name, node->ns?(const char*)node->ns->href:"");
		return false;
	}

	Core::BaseObject *obj = mapper->createClass(classname);
	if ( obj == NULL ) {
		SEISCOMP_WARNING("Unable to create instance of %s", classname);
		return false;
	}

	propagate(obj, true, true);
	return true;
}


TypeHandler::~TypeHandler() {}


bool ClassHandler::init(Core::BaseObject *obj, void *n, TagSet &mandatory) {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);

	// Fill in mandatory tags
	for ( MemberList::iterator it = attributes.begin();
	      it != attributes.end(); ++it ) {
		if ( !it->optional ) mandatory.insert(it->tag);
	}

	for ( MemberList::iterator it = elements.begin();
	      it != elements.end(); ++it ) {
		if ( !it->optional && !it->tag.empty() ) mandatory.insert(it->tag);
	}

	for ( MemberList::iterator it = childs.begin();
	      it != childs.end(); ++it ) {
		if ( !it->optional && !it->tag.empty() ) mandatory.insert(it->tag);
	}

	if ( cdataUsed )
		cdata.get(obj, n, this);

	if ( attributes.empty() ) return true;

	for ( xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next ) {
		if ( attr->children ) {
			for ( MemberList::iterator it = attributes.begin();
			      it != attributes.end(); ++it ) {
				if ( equalsTag(attr, it->tag.c_str(), it->nameSpace.c_str()) ) {
					if ( it->get(obj, attr, this) && !it->optional ) {
						mandatory.erase(it->tag);
						break;
					}
				}
			}
		}
	}

	return true;
}


bool ClassHandler::get(Core::BaseObject *obj, void *n) {
	MemberNodeHandler *anyMemberHandler = NULL;
	bool anyIsOptional = true;

	for ( MemberList::iterator it = elements.begin();
	      it != elements.end(); ++it ) {
		if ( it->tag == "" ) {
			if ( anyMemberHandler == NULL ) {
				anyMemberHandler = &*it;
				anyIsOptional = it->optional;
			}
		}
		else if ( equalsTag(n, it->tag.c_str(), it->nameSpace.c_str()) ) {
			it->get(obj, n, this);
			isOptional = it->optional;
			memberHandler = &*it;
			return true;
		}
	}

	for ( MemberList::iterator it = childs.begin();
	      it != childs.end(); ++it ) {
		if ( it->tag == "" ) {
			if ( anyMemberHandler == NULL ) {
				anyMemberHandler = &*it;
				anyIsOptional = it->optional;
			}
		}
		else if ( equalsTag(n, it->tag.c_str(), it->nameSpace.c_str()) ) {
			it->get(obj, n, this);
			isOptional = it->optional;
			memberHandler = &*it;
			return true;
		}
	}

	if ( anyMemberHandler ) {
		memberHandler = anyMemberHandler;
		isOptional = anyIsOptional;
		isAnyType = true;
		return true;
	}

	return false;
}


bool ClassHandler::put(Core::BaseObject *obj, const char *tag, const char *ns, OutputHandler *output) {
	if ( !output->openElement(tag, ns) ) return false;

	for ( MemberList::iterator it = attributes.begin();
	      it != attributes.end(); ++it ) {
		std::string v = it->value(obj);
		if ( v.empty() && it->optional ) continue;
		output->addAttribute(it->tag.c_str(), it->nameSpace.c_str(), v.c_str());
	}

	for ( MemberRefList::iterator it = orderedMembers.begin();
	      it != orderedMembers.end(); ++it ) {
		(*it)->put(obj, output, this);
	}

	/*
	for ( MemberList::iterator it = elements.begin();
	      it != elements.end(); ++it ) {
		it->put(obj, output, this);
	}

	for ( MemberList::iterator it = childs.begin();
	      it != childs.end(); ++it ) {
		it->put(obj, output, this);
	}
	*/

	if ( cdataUsed ) {
		std::string v = cdata.value(obj);
		if ( !v.empty() )
			output->put(v.c_str());
	}

	output->closeElement(tag, ns);

	return true;
}


void ClassHandler::addMember(const char *t, const char *ns, Type opt, Location l, MemberHandler *s) {
	switch ( l ) {
		case Attribute:
			attributes.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			break;
		case Element:
			elements.push_back(MemberNodeHandler(t, ns, (bool)opt, s));
			orderedMembers.push_back(&elements.back());
			break;
		case CDATA:
			cdata = MemberNodeHandler(t, ns, opt, s);
			cdataUsed = true;
			break;
		default:
			break;
	}
}


void ClassHandler::addChild(const char *t, const char *ns, MemberHandler *s) {
	childs.push_back(MemberNodeHandler(t, ns, true, s));
	orderedMembers.push_back(&childs.back());
}


TypeMap::Tag::Tag() {}


TypeMap::Tag::Tag(const std::string &name_, const std::string &ns_)
 : name(name_), ns(ns_) {}


bool TypeMap::Tag::operator<(const Tag &other) const {
	if ( name < other.name ) return true;
	if ( name > other.name ) return false;
	return ns < other.ns;
}


TypeMap::TypeMap() {}


TypeMap::~TypeMap() {
	for ( HandlerMap::iterator it = handlers.begin(); it != handlers.end(); ++it )
		delete it->second;
}


void TypeMap::registerMapping(const char *tag, const char *ns, const char *classname, NodeHandler *handler) {
	tags[Tag(tag, ns)] = classname;

	std::pair<RawTagMap::iterator,bool> itp;
	itp = tagsWithoutNs.insert(RawTagMap::value_type(tag, classname));

	// Tag exists already -> set invalid classname
	if ( !itp.second ) itp.first->second.clear();

	classes[classname] = Tag(tag, ns);
	handlers[classname] = new TypeNameHandler(handler, classname);
}


const char *TypeMap::getClassname(const char *tag, const char *ns, bool strictNsCheck) {
	TagMap::iterator it = tags.find(Tag(tag, ns));
	if ( it == tags.end() ) {
		if ( !strictNsCheck ) {
			RawTagMap::iterator rit = tagsWithoutNs.find(tag);
			if ( rit == tagsWithoutNs.end() ) return NULL;
			if ( rit->second.empty() ) return NULL;
			return rit->second.c_str();
		}
		return NULL;
	}
	return it->second.c_str();
}


const TypeMap::Tag *TypeMap::getTag(const char *classname) {
	ClassMap::iterator it = classes.find(classname);
	if ( it == classes.end() ) return NULL;
	return &it->second;
}


NodeHandler *TypeMap::getHandler(const char *classname) {
	HandlerMap::iterator it = handlers.find(classname);
	if ( it == handlers.end() ) return NULL;
	return it->second->nodeHandler;
}


Core::BaseObject *TypeMap::createClass(const char *classname) {
	HandlerMap::iterator it = handlers.find(classname);
	if ( it == handlers.end() ) return NULL;
	return it->second->createClass();
}


}
}
}
