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


#define SEISCOMP_COMPONENT XMLArchive
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/platform/platform.h>
#include <seiscomp3/datamodel/version.h>

#include <libxml/xmlreader.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>


#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>


namespace Seiscomp {
namespace IO {

#define CLASSNAME_AS_TAGNAME


namespace {


const char *null_file = "";


void xmlStructuredErrorHandler(void * userData, xmlErrorPtr error) {
	SEISCOMP_ERROR("%s: %s", (const char*)userData, error->message);
}


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

int streamBufWriteCallback(void* context, const char* buffer, int len) {
	std::streambuf* buf = static_cast<std::streambuf*>(context);
	if ( buf == NULL ) return -1;
	return buf->sputn(buffer, len);
}

int streamBufCloseCallback(void* context) {
	return 0;
}


bool isValidTag(xmlNodePtr node, const xmlChar* name, const char* targetClass) {
	if ( node->type != XML_ELEMENT_NODE )
		return false;

	xmlChar* prop = xmlGetProp(node, (const xmlChar*)"role");	
	bool isValid = !xmlStrcmp(prop, (const xmlChar*)name) || (prop == NULL && *name == '\0');
	// if the role attribute is not the correct one but
	// a role attribute has been defined and set to another value
	// this child element has to be ignored
	if ( !isValid ) {
		if ( prop != NULL && *prop != '\0' ) {
			xmlFree(prop);
			return false;
		}
		xmlFree(prop);
	}
	else {
		xmlFree(prop);

		if ( !Seiscomp::Core::ClassFactory::IsTypeOf(targetClass, (const char*)node->name) ) {
			//SEISCOMP_WARNING("requested type \"%s\", but type \"%s\" found -> ignoring", targetClass, node->name);
			return false;
		}

		return true;
	}

	// if no role attribute has been defined the tagname can hold
	// the role name
	if ( !xmlStrcmp(node->name, (const xmlChar*)name) )
		return true;

	// if the tagname is not the exact classname we check for tagnames
	// that define derived classes
	if ( Seiscomp::Core::ClassFactory::IsTypeOf((const char*)name, (const char*)node->name) )
		return true;

	return false;
}


xmlNodePtr findTag(xmlDocPtr doc, xmlNodePtr node, const char* name, const char* targetClass) {
	// find the child node with name = param name
	xmlNodePtr child = node->children;
	while ( child != NULL ) {
#ifndef CLASSNAME_AS_TAGNAME
		if ( !xmlStrcmp(child->name, (const xmlChar*)name) )
#else
		if ( isValidTag(child, (const xmlChar*)name, targetClass) )
#endif
			return child;

		child = child->next;
	}

	return NULL;
}


xmlNodePtr findNextTag(xmlDocPtr doc, xmlNodePtr node, const char* name, const char* targetClass) {
	xmlNodePtr sibling = node->next;
	while ( sibling != NULL ) {
#ifndef CLASSNAME_AS_TAGNAME
		if ( !xmlStrcmp(sibling->name, (const xmlChar*)name) )
#else
		if ( isValidTag(sibling, (const xmlChar*)name, targetClass) )
#endif
			return sibling;

		sibling = sibling->next;
	}

	return NULL;
}


xmlChar *nodeGetContent(xmlNodePtr node) {
	for ( xmlNodePtr child = node->xmlChildrenNode; child != NULL; child = child->next )
		if ( child->type == XML_TEXT_NODE || child->type == XML_CDATA_SECTION_NODE )
			return xmlNodeGetContent(child);

	return NULL;
}


}

XMLArchive::XMLArchive() : Seiscomp::Core::Archive() {
	_document = NULL;
	_current = NULL;
	_objectLocation = NULL;
	_deleteOnClose = false;
	_buf = NULL;
	_formattedOutput = false;
	_compression = false;
	_compressionMethod = ZIP;
	_rootTag = "seiscomp";
	_forceWriteVersion = -1;

	//xmlGenericErrorFunc handler = xmlGenericErrorHandler;
	//initGenericErrorDefaultFunc(&handler);

	xmlStructuredErrorFunc handler = xmlStructuredErrorHandler;
	xmlSetStructuredErrorFunc((char*)null_file, handler);
}


XMLArchive::XMLArchive(std::streambuf* buf, bool isReading, int forceWriteVersion) {
	_document = NULL;
	_current = NULL;
	_objectLocation = NULL;
	_deleteOnClose = false;
	_buf = NULL;
	_formattedOutput = false;
	_compression = false;
	_rootTag = "seiscomp";
	_forceWriteVersion = forceWriteVersion;

	//xmlGenericErrorFunc handler = xmlGenericErrorHandler;
	//initGenericErrorDefaultFunc(&handler);

	xmlStructuredErrorFunc handler = xmlStructuredErrorHandler;
	xmlSetStructuredErrorFunc(this, handler);

	if ( isReading )
		open(buf);
	else
		create(buf);
}


XMLArchive::~XMLArchive() {
	close();
}


bool XMLArchive::open(std::streambuf* buf) {
	close();

	if ( buf == NULL ) return false;

	_buf = buf;
	_deleteOnClose = false;

	return open();
}


bool XMLArchive::open(const char* filename) {
	close();

	if ( !strcmp(filename, "-") ) {
		_buf = std::cin.rdbuf();
		_deleteOnClose = false;
	}
	else {
		std::filebuf* fb = new std::filebuf();
		if ( fb->open(filename, std::ios::in) == NULL ) {
			delete fb;
			return false;
		}
	
		_buf = fb;
		_deleteOnClose = true;
	}

	return open();
}


bool XMLArchive::open() {
	if ( !Seiscomp::Core::Archive::open(NULL) )
		return false;

	xmlDocPtr doc;

	if ( _compression ) {
		boost::iostreams::filtering_istreambuf filtered_buf;

		switch ( _compressionMethod ) {
			case ZIP:
				filtered_buf.push(boost::iostreams::zlib_decompressor());
				break;
			case GZIP:
				filtered_buf.push(boost::iostreams::gzip_decompressor());
				break;
			default:
				break;
		}

		filtered_buf.push(*_buf);

		doc = xmlReadIO(streamBufReadCallback,
		                streamBufCloseCallback,
		                &filtered_buf, NULL, NULL, 0);
	}
	else
		doc = xmlReadIO(streamBufReadCallback,
		                streamBufCloseCallback,
		                _buf, NULL, NULL, 0);

	if ( doc == NULL )
		return false;

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if ( cur == NULL ) {
		xmlFreeDoc(doc);
		return false;
	}

	xmlNsPtr* xmlNsList = xmlGetNsList(doc, cur);
	if ( xmlNsList != NULL ) {
		xmlNsPtr* xmlNsRunner = xmlNsList;
		while ( *xmlNsRunner != NULL ) {
			if ( (*xmlNsRunner)->prefix != NULL )
				_namespace.first = (const char*)(*xmlNsRunner)->prefix;

			if ( (*xmlNsRunner)->href != NULL )
				_namespace.second = (const char*)(*xmlNsRunner)->href;

			break;
			//++xmlNsRunner;
		}
	
		xmlFree(xmlNsList);
	}

	if ( xmlStrcmp(cur->name, (const xmlChar*)_rootTag.c_str()) ) {
		//xmlFreeDoc(doc);
		//return false;
		cur = (xmlNodePtr)doc;
	}

	xmlChar* version = xmlGetProp(cur, (const xmlChar*)"version");
	if ( version != NULL ) {
		char* seperator = strchr((char*)version, '.');
		if ( seperator != NULL ) {
			*seperator++ = '\0';
			setVersion(Core::Version(atoi((char*)version), atoi((char*)seperator)));
		}
		else
			setVersion(Core::Version(atoi((char*)version),0));
	
		xmlFree(version);
	}
	else
		setVersion(Core::Version(0,0));

	_document = doc;
	_current = cur;

	return true;
}


bool XMLArchive::create(std::streambuf* buf, bool writeVersion, bool headerNode) {
	close();

	_buf = buf;
	_deleteOnClose = false;

	return create(writeVersion, headerNode);
}


bool XMLArchive::create(const char* filename, bool writeVersion, bool headerNode) {
	close();

	if ( !strcmp(filename, "-") ) {
		_buf = std::cout.rdbuf();
		_deleteOnClose = false;
	}
	else {
		std::filebuf* fb = new std::filebuf();
		if ( fb->open(filename, std::ios::out) == NULL ) {
			delete fb;
			return false;
		}

		_buf = fb;
		_deleteOnClose = true;
	}

	return create(writeVersion, headerNode);
}


bool XMLArchive::create(bool writeVersion, bool headerNode) {
	if ( !Seiscomp::Core::Archive::create(NULL) )
		return false;

	if ( writeVersion ) {
		if ( _forceWriteVersion >= 0 )
			setVersion(Core::Version(_forceWriteVersion));
		else if ( versionMajor() == 0 && versionMinor() == 0 )
			setVersion(Core::Version(DataModel::Version::Major, DataModel::Version::Minor));
	}
	else
		setVersion(Core::Version(0,0));

	_namespace.second = SEISCOMP_DATAMODEL_XMLNS_ROOT + version().toString();

	xmlDocPtr doc = xmlNewDoc(NULL);
	void* rootNode = NULL;

	_document = doc;

	if ( headerNode )
		rootNode = addRootNode(_rootTag.c_str());
	
	_current = rootNode;
	_objectLocation = _current;

	return true;
}


void XMLArchive::close() {
	if ( _document != NULL ) {
		if ( !isReading() ) {
			if ( _buf ) {
				xmlOutputBufferPtr xmlBuf;
				xmlBuf = xmlAllocOutputBuffer(NULL);
				if ( xmlBuf ) {

					boost::iostreams::filtering_ostreambuf filtered_buf;

					if ( _compression ) {
						switch ( _compressionMethod ) {
							case ZIP:
								filtered_buf.push(boost::iostreams::zlib_compressor());
								break;
							case GZIP:
								filtered_buf.push(boost::iostreams::gzip_compressor());
								break;
							default:
								break;
						}

						filtered_buf.push(*_buf);
						xmlBuf->context = &filtered_buf;
					}
					else
						xmlBuf->context = _buf;

					xmlBuf->writecallback = streamBufWriteCallback;
					xmlBuf->closecallback = streamBufCloseCallback;

					xmlSaveFormatFileTo(xmlBuf, static_cast<xmlDocPtr>(_document), "UTF-8",
					                    _formattedOutput?1:0);
				}
			}
		}

		xmlFreeDoc(static_cast<xmlDocPtr>(_document));
		_document = NULL;
	}

	if ( _deleteOnClose && _buf )
		delete _buf;
	else if ( _buf && _isReading ) {
		if ( _buf->sgetc() == '\0' )
			_buf->sbumpc();
	}

	_deleteOnClose = false;
	_buf = NULL;

	_current = NULL;
	_property = "";

	_namespace.first = "";
	_namespace.second = "";

	_forceWriteVersion = -1;

	initGenericErrorDefaultFunc(NULL);
	setVersion(Core::Version(0,0));
}


const std::string& XMLArchive::rootNamespace() const {
	return _namespace.first;
}


const std::string& XMLArchive::rootNamespaceUri() const {
	return _namespace.second;
}


void XMLArchive::setRootNamespace(const std::string& name, const std::string& uri) {
	_namespace.first = name;
	_namespace.second = uri;
}


void XMLArchive::setRootName(const std::string &name) {
	_rootTag = name;
}


void XMLArchive::setFormattedOutput(bool enable) {
	_formattedOutput = enable;
}


void XMLArchive::setCompression(bool enable) {
	_compression = enable;
}


void XMLArchive::setCompressionMethod(CompressionMethod method) {
	_compressionMethod = method;
}


int writeBufferCallback(void* context, const char* buffer, int len) {
	std::streambuf* buf = static_cast<std::streambuf*>(context);
	return buf->sputn(buffer, len);
}


void XMLArchive::read(time_t& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(Seiscomp::Core::Time& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(int& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(float& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(double& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<char>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<int>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<float>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<double>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<std::string>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::complex<float>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::complex<double>& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}


void XMLArchive::read(std::vector<std::complex<double> >& value) {
	setValidity(Seiscomp::Core::fromString(value, _property));
}

void XMLArchive::read(bool& value) {
	value = _property == "true" || _property == "1";
}

void XMLArchive::read(std::string& value) {
	value = _property;
}


void XMLArchive::write(time_t value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(Seiscomp::Core::Time& value) {
	if ( (hint() & XML_MANDATORY) || value.valid() )
		writeAttrib(Seiscomp::Core::toString(value));
	else
		_property = "";
}


void XMLArchive::write(int value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(float value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(double value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<char>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<int>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<float>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<double>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<std::string>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::complex<float>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::complex<double>& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(std::vector<std::complex<double> >& value) {
	writeAttrib(Seiscomp::Core::toString(value));
}


void XMLArchive::write(bool value) {
	writeAttrib(value?"true":"false");
}


void XMLArchive::write(std::string& value) {
	writeAttrib(value);
}


bool XMLArchive::locateObjectByName(const char* name, const char* targetClass, bool) {
	if ( _current == NULL && isReading() )
		return false;

	if ( targetClass != NULL ) {
		if ( (hint() & STATIC_TYPE) == 0 ) {
			if ( isReading() )
				_objectLocation = findTag(static_cast<xmlDocPtr>(_document),
				                          static_cast<xmlNodePtr>(_current),
				                          name,
				                          targetClass);
			else
				addChild(name, targetClass);

			return _objectLocation != NULL;
		}
		else {
			if ( isReading() ) {
				_objectLocation = findTag(static_cast<xmlDocPtr>(_document),
				                          static_cast<xmlNodePtr>(_current),
				                          name, NULL);
				return _objectLocation != NULL;
			}
			else {
				_objectLocation = xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL, (const xmlChar*)name, NULL);
				return true;
			}
		}
	}
	else {
		if ( isReading() ) {
			_attribName.clear();

			if ( name && *name ) {
				if ( hint() & XML_ELEMENT ) {
					xmlNodePtr attrNode = static_cast<xmlNodePtr>(_current)->children;
					while ( attrNode != NULL ) {
						if ( attrNode->type == XML_ELEMENT_NODE && !xmlStrcmp(attrNode->name, (const xmlChar*)name) ) {
							_objectLocation = attrNode;
							xmlChar* content = nodeGetContent(attrNode);
							if ( content ) {
								_property = (const char*)content;
								xmlFree(content);
							}
							else
								_property.clear();
							return true;
						}
						attrNode = attrNode->next;
					}

					_property.clear();
					return false;
				}
				else if ( hint() & XML_CDATA ) {
					xmlChar* content = nodeGetContent(static_cast<xmlNodePtr>(_current));
					if ( !content ) {
						_property.clear();
						return false;
					}

					_property = (const char*)content;
					xmlFree(content);
				}
				else {
					xmlChar* prop = xmlGetProp(static_cast<xmlNodePtr>(_current), (const xmlChar*)name);
					if ( prop == NULL ) {
						_property.clear();
						return false;
					}
	
					_attribName = name;
					_property = (const char*)prop;
		
					xmlFree(prop);
				}
			}
			else {
				xmlChar* content = nodeGetContent(static_cast<xmlNodePtr>(_current));
				if ( content ) {
					_property = (const char*)content;
					xmlFree(content);
				}
			}
		}
		else {
			_objectLocation = _current;
			_property = name;
		}

		return true;
	}
	
	return false;
}


bool XMLArchive::locateNextObjectByName(const char* name, const char* targetClass) {
	if ( _objectLocation == NULL )
		return false;

	if ( targetClass != NULL ) {
		if ( isReading() ) {
			if ( hint() & STATIC_TYPE )
				_objectLocation = findNextTag(static_cast<xmlDocPtr>(_document),
				                              static_cast<xmlNodePtr>(_objectLocation),
				                              name,
				                              NULL);
			else
				_objectLocation = findNextTag(static_cast<xmlDocPtr>(_document),
				                              static_cast<xmlNodePtr>(_objectLocation),
				                              name, targetClass);
		}
		else {
			if ( hint() & STATIC_TYPE )
				_objectLocation = xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL, (const xmlChar*)name, NULL);
			else
				addChild(name, targetClass);
		}

		return _objectLocation != NULL;
	}
	else {
		return false;
	}
	
	return false;
}


std::string XMLArchive::determineClassName() {
	// if there is no current node, the classname cannot be retrieved
	//_objectLocation = getFirstElementNode(static_cast<xmlNodePtr>(_current));
	if ( _objectLocation == NULL )
		return "";

	xmlNodePtr locationNode = static_cast<xmlNodePtr>(_objectLocation);
#ifndef CLASSNAME_AS_TAGNAME
	xmlChar* classname = xmlGetProp(locationNode, (const xmlChar*)"_classname");
	std::string strClassname;
	if ( classname != NULL ) {
		strClassname = (const char*)classname;
		xmlFree(classname);
	}
	return strClassname;
#else	
	return (const char*)locationNode->name;
#endif
}


void XMLArchive::setClassName(const char* className) {
	xmlNodePtr locationNode = static_cast<xmlNodePtr>(_objectLocation);
#ifdef CLASSNAME_AS_TAGNAME
	xmlNodeSetName(locationNode, (const xmlChar*)className);
#else
	xmlSetProp(locationNode, (const xmlChar*)"_classname", (const xmlChar*)className);
#endif
}


void XMLArchive::writeAttrib(const std::string& value) {
	bool force = hint() & XML_MANDATORY;

	if ( value.empty() && !force ) {
		_property = "";
		return;
	}

	if ( !_property.empty() && !(hint() & XML_CDATA) ) {
		if ( hint() & XML_ELEMENT ) {
			xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL,
			                (const xmlChar*)_property.c_str(),
			                (const xmlChar*)(value.empty()?NULL:value.c_str()));
		}
		else
			xmlSetProp(static_cast<xmlNodePtr>(_current), (const xmlChar*)_property.c_str(), (const xmlChar*)value.c_str());
	}
	else if ( !value.empty() )
		xmlNodeSetContent(static_cast<xmlNodePtr>(_current), (const xmlChar*)value.c_str());

	_property = "";
}


void XMLArchive::setValidity(bool v) {
	if ( !v ) {
		if ( hint() & XML_ELEMENT ) {
			if ( _objectLocation != NULL ) {
				xmlNodePtr n = static_cast<xmlNodePtr>(_objectLocation);
				int lineNo = (int)xmlGetLineNo(n);
				std::string path;
				while ( n->parent != NULL ) {
					path.insert(0, (char*)n->name);
					path.insert(0, "/");
					n = n->parent;
				}

				SEISCOMP_ERROR("Invalid element content:%d: %s=%s", lineNo, path.c_str(), _property.c_str());
			}
			else
				SEISCOMP_ERROR("Invalid element content: %s", _property.c_str());
		}
		else if ( hint() & XML_CDATA ) {
			if ( _current != NULL ) {
				xmlNodePtr n = static_cast<xmlNodePtr>(_current);
				int lineNo = (int)xmlGetLineNo(n);
				std::string path;
				while ( n->parent != NULL ) {
					path.insert(0, (char*)n->name);
					path.insert(0, "/");
					n = n->parent;
				}

				SEISCOMP_ERROR("Invalid CDATA content:%d: %s=%s", lineNo, path.c_str(), _property.c_str());
			}
			else
				SEISCOMP_ERROR("Invalid CDATA content: %s", _property.c_str());
		}
		else {
			if ( _current != NULL ) {
				xmlNodePtr n = static_cast<xmlNodePtr>(_current);
				int lineNo = (int)xmlGetLineNo(n);
				std::string path;
				while ( n->parent != NULL ) {
					path.insert(0, (char*)n->name);
					path.insert(0, "/");
					n = n->parent;
				}

				if ( !_attribName.empty() ) {
					path += '.';
					path += _attribName;
				}

				SEISCOMP_ERROR("Invalid attribute content:%d: %s=%s", lineNo, path.c_str(), _property.c_str());
			}
			else
				SEISCOMP_ERROR("Invalid attribute content: %s", _property.c_str());
		}
	}

	Seiscomp::Core::Archive::setValidity(v);
}


void XMLArchive::addChild(const char* name, const char* type) const {
	if ( _current == NULL ) {
#ifdef CLASSNAME_AS_TAGNAME
		xmlNodePtr rootNode = (xmlNodePtr)addRootNode(type);
		if ( name != NULL && *name != '\0' )
			xmlSetProp(rootNode, (const xmlChar*)"role", (const xmlChar*)name);
#else
		xmlNodePtr rootNode = (xmlNodePtr)addRootNode(name);
#endif
		_objectLocation = rootNode;
	}
	else {
#ifdef CLASSNAME_AS_TAGNAME
		_objectLocation = xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL, (const xmlChar*)type, NULL);
		if ( name != NULL && *name != '\0' )
			xmlSetProp(static_cast<xmlNodePtr>(_objectLocation), (const xmlChar*)"role", (const xmlChar*)name);
#else
		_objectLocation = xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL, (const xmlChar*)name, NULL);
#endif
	}
}


void* XMLArchive::addRootNode(const char* name) const {
	xmlNodePtr rootNode = xmlNewDocRawNode((xmlDocPtr)_document, NULL, (const xmlChar*)name, NULL);

	if ( versionMajor() || versionMinor() )
		xmlSetProp(rootNode, (const xmlChar*)"version",
		                     (const xmlChar*)(Seiscomp::Core::toString(versionMajor()) + "." + Seiscomp::Core::toString(versionMinor())).c_str());
	
	if ( !_namespace.first.empty() || !_namespace.second.empty() )
		xmlNewNs(rootNode, _namespace.second.empty()?NULL:(const xmlChar*)_namespace.second.c_str(),
		                   _namespace.first.empty()?NULL:(const xmlChar*)_namespace.first.c_str());
	
	xmlDocSetRootElement((xmlDocPtr)_document, rootNode);

	return rootNode;
}


void XMLArchive::serialize(RootType* object) {
	void* backupCurrent = _current;
	void* backupLocation = _objectLocation;
	_current = _objectLocation;
	
	Seiscomp::Core::Archive::serialize(object);

	_current = backupCurrent;
	_objectLocation = backupLocation;
}


void XMLArchive::serialize(SerializeDispatcher& disp) {
	if ( !isReading() && (hint() & XML_ELEMENT) && !_property.empty() ) {
		_objectLocation = xmlNewTextChild(static_cast<xmlNodePtr>(_current), NULL, (const xmlChar*)_property.c_str(), NULL);
		setHint((hint() & ~XML_ELEMENT) | XML_CDATA);
	}

	void* backupCurrent = _current;
	void* backupLocation = _objectLocation;
	_current = _objectLocation;
	
	Seiscomp::Core::Archive::serialize(disp);

	_current = backupCurrent;
	_objectLocation = backupLocation;
}


}
}
