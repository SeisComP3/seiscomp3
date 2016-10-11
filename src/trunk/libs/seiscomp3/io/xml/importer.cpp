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


#define SEISCOMP_COMPONENT XMLImport
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/xml/importer.h>

#include <libxml/xmlreader.h>

#if LIBXML_VERSION < 20900
#  define XML_PARSE_BIG_LINES 4194304
#endif


namespace Seiscomp {
namespace IO {
namespace XML {


namespace {


int streamBufReadCallback(void* context, char* buffer, int len) {
	std::streambuf* buf = static_cast<std::streambuf*>(context);
	if ( buf == NULL ) return -1;

	int count = 0;
	int ch = buf->sgetc();
	while ( ch != EOF && len-- && ch != '\0' ) {
		*buffer++ = (char)buf->sbumpc();
		ch = buf->sgetc();
		++count;
  	}

	return count;
}


int streamBufCloseCallback(void* context) {
	return 0;
}


}


NoneHandler Importer::_none;


Importer::Importer() {
	_typemap = NULL;
	_strictNamespaceCheck = true;
}

TypeMap* Importer::typeMap() {
	return _typemap;
}

void Importer::setTypeMap(TypeMap *map) {
	_typemap = map;
}


void Importer::setStrictNamespaceCheck(bool e) {
	_strictNamespaceCheck = e;
}


void Importer::setRootName(std::string h) {
	_headerNode = h;
}


Core::BaseObject *Importer::get(std::streambuf* buf) {
	if ( _typemap == NULL ) return NULL;
	if ( buf == NULL ) return NULL;

	_result = NULL;

	xmlDocPtr doc;
	doc = xmlReadIO(streamBufReadCallback,
	                streamBufCloseCallback,
	                buf, NULL, NULL, XML_PARSE_BIG_LINES);

	if ( doc == NULL )
		return NULL;

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if ( cur == NULL ) {
		xmlFreeDoc(doc);
		return NULL;
	}

	_any.mapper = _typemap;

	bool saveStrictNsCheck = NodeHandler::strictNsCheck;

	if ( !_headerNode.empty() ) {
		// Check the root tag matching "seiscomp"
		if ( xmlStrcmp(cur->name, (const xmlChar*)_headerNode.c_str()) ) {
			SEISCOMP_WARNING("Invalid root tag: %s, expected: %s", cur->name, _headerNode.c_str());
			xmlFreeDoc(doc);
			return NULL;
		}

		NodeHandler::strictNsCheck = _strictNamespaceCheck;
		_hasErrors = traverse(&_any, cur, cur->children, NULL) == false;
	}
	else {
		NodeHandler::strictNsCheck = _strictNamespaceCheck;
		_hasErrors = traverse(&_any, NULL, cur, NULL) == false;
	}

	NodeHandler::strictNsCheck = saveStrictNsCheck;

	xmlFreeDoc(doc);

	return _result;
}


bool Importer::traverse(NodeHandler *handler, void *n, void *c, Core::BaseObject *target) {
	xmlNodePtr node = reinterpret_cast<xmlNodePtr>(n);
	xmlNodePtr childs = reinterpret_cast<xmlNodePtr>(c);
	ChildList remaining;
	TagSet mandatory;

	handler->init(target, n, mandatory);

	bool result = true;

	for ( xmlNodePtr child = childs; child != NULL; child = child->next ) {
		if ( child->type != XML_ELEMENT_NODE ) continue;

		handler->propagate(NULL, false, true);

		try {
			handler->get(target, child);
		}
		catch ( std::exception &e ) {
			if ( handler->isOptional )
				SEISCOMP_WARNING("L%ld: (optional) %s.%s: %s",
				                 xmlGetLineNo(node),
				                 node->name, child->name, e.what());
			else
				throw e;
		}

		if ( !handler->isOptional )
			mandatory.erase((const char*)child->name);

		if ( handler->object == NULL && handler->isAnyType ) {
			if ( _any.get(target, child) ) {
				handler->object = _any.object;
				handler->childHandler = _any.childHandler;
				handler->newInstance = _any.newInstance;
			}
		}

		Core::BaseObject *newTarget = handler->object;
		MemberNodeHandler *memberHandler = handler->memberHandler;
		NodeHandler *childHandler = handler->childHandler;
		bool newInstance = handler->newInstance;
		bool optional = handler->isOptional;

		if ( newTarget ) {
			if ( childHandler == NULL ) {
				childHandler = _typemap->getHandler(newTarget->className());
				if ( childHandler == NULL ) {
					SEISCOMP_WARNING("No class handler for %s", newTarget->className());
					if ( newInstance )
						delete newTarget;
					handler->object = NULL;
					newTarget = NULL;
					childHandler = &_none;
				}
			}
		}
		else
			childHandler = &_none;

		try {
			if ( traverse(childHandler, child, child->children, handler->object) ) {
				if ( newTarget && newInstance && !memberHandler )
					remaining.push_back(newTarget);

			}
			else {
				if ( newTarget && newInstance )
					delete newTarget;
				newTarget = NULL;
				if ( optional )
					SEISCOMP_INFO("L%ld: Invalid %s element: ignoring",
					              xmlGetLineNo(child), child->name);
				else {
					SEISCOMP_WARNING("L%ld: %s is not optional within %s",
					                 xmlGetLineNo(child),
					                 child->name, node->name);
					result = false;
				}
			}
		}
		catch ( std::exception &e ) {
			SEISCOMP_WARNING("L%ld: %s: %s", xmlGetLineNo(child), child->name, e.what());
			if ( newTarget ) {
				if ( newInstance )
					delete newTarget;

				if ( !optional ) {
					SEISCOMP_WARNING("L%ld: %s is not optional within %s",
					                 xmlGetLineNo(child), child->name, node->name);
					result = false;
				}
				else
					SEISCOMP_WARNING("L%ld: %s: ignoring optional member %s: invalid",
					                 xmlGetLineNo(child),
					                 node->name, child->name);

				newTarget = NULL;
			}
		}

		if ( memberHandler ) {
			if ( !memberHandler->finalize(target, newTarget) ) {
				if ( newTarget && newInstance )
					remaining.push_back(newTarget);
			}
		}
	}

	handler->finalize(target, &remaining);

	if ( target != NULL ) {
		for ( ChildList::iterator it = remaining.begin(); it != remaining.end(); ++it )
			if ( *it != NULL ) delete *it;
	}
	else {
		for ( ChildList::iterator it = remaining.begin(); it != remaining.end(); ++it ) {
			if ( *it != NULL ) {
				if ( _result == NULL )
					_result = *it;
				else
					delete *it;
			}
		}
	}

	if ( !mandatory.empty() ) {
		std::string attribs;
		for ( TagSet::iterator it = mandatory.begin(); it != mandatory.end(); ++it ) {
			if ( it != mandatory.begin() ) attribs += ", ";
			attribs += *it;
		}
		SEISCOMP_WARNING("L%ld: %s: missing mandatory attribute%s: %s",
		                 xmlGetLineNo(node), node->name,
		                 mandatory.size() == 1?"":"s", attribs.c_str());
		return false;
	}

	return result;
}


}
}
}
