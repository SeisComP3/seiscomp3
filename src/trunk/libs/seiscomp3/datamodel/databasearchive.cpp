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


#define SEISCOMP_COMPONENT DatabaseArchive
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/databasearchive.h>
#include <seiscomp3/datamodel/version.h>
#include <seiscomp3/datamodel/stream.h>
#include <seiscomp3/logging/log.h>

#include <iostream>
#include <stdlib.h>
#include <strings.h>


using namespace Seiscomp::Core;


#define ATTRIBUTE_SEPERATOR    "_"
#define MICROSECONDS_POSTFIX   ATTRIBUTE_SEPERATOR"ms"
#define OBJECT_USED_POSTFIX    "used"
#define CHILD_ID_POSTFIX       "oid"


namespace Seiscomp {
namespace DataModel {


class AttributeMapper {
	public:
		AttributeMapper(const DatabaseArchive::AttributeMap &map)
		  : _it(map.begin()), _end(map.end()) {}

		inline bool next() const {
			return _it != _end;
		}

		inline const std::string &attrib() const {
			return _it++->first;
		}

	private:
		mutable DatabaseArchive::AttributeMap::const_iterator _it;
		DatabaseArchive::AttributeMap::const_iterator _end;
};


std::ostream &operator<<(std::ostream &os, const AttributeMapper &m) {
	bool first = true;
	while ( m.next() ) {
		if ( !first ) os << ",";
		os << m.attrib();
		first = false;
	}
	return os;
}


class ValueMapper {
	public:
		ValueMapper(const DatabaseArchive::AttributeMap &map)
		  : _it(map.begin()), _end(map.end()) {}

		inline bool next() const {
			return _it != _end;
		}

		inline std::string value() const {
			static std::string null = "NULL";
			OPT_CR(std::string) v = _it++->second;
			//return v?'\'' + *v + '\'':null;
			return v?*v:null;
		}

	private:
		mutable DatabaseArchive::AttributeMap::const_iterator _it;
		DatabaseArchive::AttributeMap::const_iterator _end;
};


namespace {


std::ostream &operator<<(std::ostream &os, const ValueMapper &m) {
	bool first = true;
	while ( m.next() ) {
		if ( !first ) os << ",";
		os << m.value();
		first = false;
	}
	return os;
}


const std::string &toSQL(const std::string &str) {
	static std::string converted;
	std::string::size_type pos = str.find('\'');
	if ( pos == std::string::npos )
		return str;

	converted.clear();

	if ( pos > 0 )
		converted.append(str, 0, pos);

	++pos;
	converted.append("''");
	while ( pos < str.size() ) {
		if ( str[pos] == '\'' )
			converted.push_back('\'');
		converted.push_back(str[pos]);
		++pos;
	}

	SEISCOMP_DEBUG("converted string: %s -> %s", str.c_str(), converted.c_str());
	return converted;
}


bool strtobool(bool &val, const char *str) {
	int v;
	if ( fromString(v, str) ) {
		val = v != 0;
		return true;
	}

	// Check for all 'true' options
	if ( strcasecmp(str, "t") == 0 || strcasecmp(str, "true") == 0 ||
	     strcasecmp(str, "y") == 0 || strcasecmp(str, "yes") == 0 ) {
		val = true;
		return true;
	}

	// Check for all 'false' options
	if ( strcasecmp(str, "f") == 0 || strcasecmp(str, "false") == 0 ||
	     strcasecmp(str, "n") == 0 || strcasecmp(str, "no") == 0 ) {
		val = false;
		return true;
	}

	return false;
}


}


DatabaseIterator::DatabaseIterator(DatabaseArchive *database, const RTTI *rtti)
: _rtti(rtti), _reader(database), _count(0), _oid(-1), _parent_oid(-1), _cached(false) {
	_object = fetch();
	if ( !_object && _reader ) operator++();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::DatabaseIterator()
: _rtti(NULL), _reader(NULL), _count(0), _object(NULL), _oid(-1), _parent_oid(-1), _cached(false) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::DatabaseIterator(const DatabaseIterator &iter)
: Seiscomp::Core::BaseObject(), _rtti(iter._rtti), _reader(iter._reader),
  _count(iter._count), _oid(iter._oid),
  _parent_oid(iter._parent_oid), _cached(iter._cached),
  _lastModified(iter._lastModified) {
	_object = iter._object;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator::~DatabaseIterator() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *DatabaseIterator::fetch() const {
	if ( _rtti == NULL || _reader == NULL ) return NULL;

	_cached = false;

	int col;
	_parent_oid = _oid = -1;
	if ( (col = _reader->_db->findColumn("_oid")) != -1)
		Core::fromString(_oid, std::string(static_cast<const char*>(_reader->_db->getRowField(col))));
		
	if ( (col = _reader->_db->findColumn("_parent_oid")) != -1)
		Core::fromString(_parent_oid, std::string(static_cast<const char*>(_reader->_db->getRowField(col))));

	if ( (col = _reader->_db->findColumn("_last_modified")) != -1)
		_lastModified = _reader->_db->stringToTime(static_cast<const char*>(_reader->_db->getRowField(col)));
	else
		_lastModified = Core::None;

	if ( _reader->_checkForCached ) {
		if ( _rtti->isTypeOf(PublicObject::TypeInfo()) ) {
			int col = _reader->_db->findColumn(_reader->_publicIDColumn.c_str());
			if ( col != -1 ) {
				const char* readPublicID = static_cast<const char*>(_reader->_db->getRowField(col));
				PublicObject *cached = PublicObject::Find(readPublicID);
				if ( cached != NULL && cached->typeInfo().isTypeOf(*_rtti) ) {
					_cached = true;
					return cached;
				}
			}
		}
	}

	BaseObject *bobj = ClassFactory::Create(_rtti->className());
	if ( bobj == NULL ) {
		SEISCOMP_ERROR("DatabaseIterator: object of type '%s' could not be created",
		               _rtti->className());
		_reader->_db->endQuery();
		return NULL;
	}

	Object* obj = Object::Cast(bobj);
	if ( obj == NULL ) {
		delete bobj;
		SEISCOMP_ERROR("DatabaseIterator: object of type '%s' could not be created",
		               _rtti->className());
		_reader->_db->endQuery();
		return NULL;
	}

	if ( _lastModified )
		obj->setLastModifiedInArchive(*_lastModified);

	_reader->serializeObject(obj);

	if ( !_reader->success() ) {
		SEISCOMP_WARNING("DatabaseIterator: error while reading object of type '%s': ignoring it",
		                 _rtti->className());
		delete obj;
		obj = NULL;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator=(const DatabaseIterator &it) {
	_rtti = it._rtti;
	_reader = it._reader;
	_count = it._count;
	_object = it._object;
	_oid = it._oid;
	_parent_oid = it._parent_oid;
	_lastModified = it._lastModified;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseIterator::valid() const {
	return _reader != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseIterator::next() {
	++(*this);
	return valid();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* DatabaseIterator::get() const {
	return _object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseIterator::fieldCount() const {
	return _reader?_reader->_db->getRowFieldCount():0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *DatabaseIterator::field(size_t index) const {
	return _reader?(const char*)_reader->_db->getRowField(index):NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* DatabaseIterator::operator*() const {
	return _object.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator++() {
	while ( _reader->_db->fetchRow() ) {
		_object = fetch();
		if ( !_object ) continue;
		++_count;
		return *this;
	}

	close();
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator& DatabaseIterator::operator++(int) {
	return operator++();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseIterator::close() {
	if ( _reader ) {
		_reader->_db->endQuery();
		_reader = NULL;
		_rtti = NULL;
	}

	_object = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseIterator::count() const {
	return _count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseObjectWriter::DatabaseObjectWriter(DatabaseArchive &archive,
                                           bool addToDatabase,
                                           int batchSize)
  : Visitor(addToDatabase?TM_TOPDOWN:TM_BOTTOMUP),
    _archive(archive), _addObjects(addToDatabase), _errors(0),
    _count(0), _batchSize(batchSize) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::operator()(Object *object) {
	return (*this)(object, "");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::operator()(Object *object, const std::string &parentID) {
	if ( _archive.driver() == NULL )
		return false;

	_parentID = parentID;

	_errors = 0;
	_count = 0;

	if ( _batchSize > 1 )
		_archive.driver()->start();
	object->accept(this);
	if ( _batchSize > 1 )
		_archive.driver()->commit();

	return _errors == 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::visit(PublicObject *publicObject) {
	return write(publicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseObjectWriter::visit(Object *object) {
	write(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseObjectWriter::write(Object *object) {
	++_count;

	if ( _batchSize <= 1 )
		_archive.driver()->start();
	bool result = _addObjects?_archive.write(object, _parentID):_archive.remove(object, _parentID);

	if ( !result ) {
		++_errors;
		if ( _batchSize <= 1 )
			_archive.driver()->rollback();
		return false;
	}

	if ( _batchSize <= 1 )
		_archive.driver()->commit();
	else if ( (_count % _batchSize) == 0 ) {
		_archive.driver()->commit();
		_archive.driver()->start();
	}

	_parentID = "";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::IO::DatabaseInterface *DatabaseArchive::driver() const {
	return _db.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::setDriver(Seiscomp::IO::DatabaseInterface *db) {
	_objectIdCache.clear();
	_db = db;
	_errorMsg = "";

	if ( !fetchVersion() ) close();

	if ( _db )
		_publicIDColumn = _db->convertColumnName("publicID");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::DatabaseArchive(Seiscomp::IO::DatabaseInterface *i)
  : _db(i), _objectAttributes(NULL) {
	setHint(IGNORE_CHILDS);
	Object::RegisterObserver(this);
	_allowDbClose = false;
	_checkForCached = true;

	if ( !fetchVersion() ) DatabaseArchive::close();

	if ( _db )
		_publicIDColumn = _db->convertColumnName("publicID");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseArchive::~DatabaseArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::open(const char *dataSource) {
	_errorMsg = "";

	if ( _db == NULL ) return false;
	if ( _db->isConnected() ) return false;

	if ( _db->connect(dataSource) ) {
		if ( !fetchVersion() ) {
			close();
			return false;
		}

		SEISCOMP_INFO("Connect to %s succeeded",dataSource);
		_allowDbClose = true;
		return true;
	}

	fetchVersion();

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::fetchVersion() {
	setVersion(Core::Version(0,0));

	if ( _db == NULL ) return false;

	if ( !_db->beginQuery("select value from Meta where name='Schema-Version'") ) {
		SEISCOMP_WARNING("Unable to read schema version from database, "
		                 "assuming v0.0");
		_db->endQuery();
		return true;
	}

	if ( !_db->fetchRow() ) {
		_errorMsg = "Unable to read schema version from database (empty result set)";
		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	Core::Version v;
	if ( !v.fromString((const char*)_db->getRowField(0)) ) {
		_errorMsg = "Invalid schema version in database: ";
		_errorMsg += (const char*)_db->getRowField(0);

		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	_db->endQuery();

	setVersion(v);

	if ( v > Core::Version(Version::Major, Version::Minor) ) {
		_errorMsg = "Database version v";
		_errorMsg += toString(v.majorTag()) + "." + toString(v.minorTag());
		_errorMsg += " not supported by client";
		SEISCOMP_ERROR("%s", _errorMsg.c_str());
		_db->endQuery();
		return false;
	}

	SEISCOMP_DEBUG("Found database version v%d.%d",
	               versionMajor(), versionMinor());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::hasError() const {
	return !_errorMsg.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string DatabaseArchive::errorMsg() const {
	return _errorMsg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::create(const char* /*dataSource*/) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::close() {
	if ( _db != NULL && _allowDbClose )
		_db->disconnect();
	_db = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::benchmarkQueries(int count) {
	for ( int i = 0; i < count; ++i ) {
		_db->beginQuery("select * from station where _network_oid=1");
		while ( _db->fetchRow() );
		_db->endQuery();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::setPublicObjectCacheLookupEnabled(bool e) {
	_checkForCached = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::isPublicObjectCacheLookupEnabled() const {
	return _checkForCached;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object *DatabaseArchive::queryObject(const Seiscomp::Core::RTTI &classType,
                                     const std::string &query) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return NULL;
	}

	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("query [%s] failed", query.c_str());
		return NULL;
	}

	if ( !_db->fetchRow() ) {
		_db->endQuery();
		return NULL;
	}

	if ( _checkForCached ) {
		if ( classType.isTypeOf(PublicObject::TypeInfo()) ) {
			int col = _db->findColumn(_publicIDColumn.c_str());
			if ( col != -1 ) {
				const char* readPublicID = static_cast<const char*>(_db->getRowField(col));
				PublicObject *cached = PublicObject::Find(readPublicID);
				if ( cached != NULL && cached->typeInfo().isTypeOf(classType) ) {
					_db->endQuery();
					return cached;
				}
			}
		}
	}

	BaseObject *bobj = ClassFactory::Create(classType.className());
	if ( bobj == NULL ) {
		SEISCOMP_ERROR("unable to create class of type '%s'", classType.className());
		_db->endQuery();
		return NULL;
	}

	Object* obj = Object::Cast(bobj);
	if ( obj == NULL ) {
		delete bobj;
		SEISCOMP_ERROR("unable to create class of type '%s'", classType.className());
		_db->endQuery();
		return NULL;
	}

	serializeObject(obj);

	_db->endQuery();

	if ( !success() ) {
		delete obj;
		obj = NULL;
	}

	return obj;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PublicObject *DatabaseArchive::getObject(const RTTI &classType,
                                         const std::string &publicID) {
	if ( !classType.isTypeOf(PublicObject::TypeInfo()) )
		return NULL;

	/* That has be to part of another piece of code, called 'cache'
	PublicObject *cached;
	if ( (cached = PublicObject::Find(publicID)) != NULL ) {
		if ( cached->typeInfo().isTypeOf(classType) )
			return cached;
	}
	*/

	std::stringstream ss;
	ss << "select " << PublicObject::ClassName() << "." << _publicIDColumn << "," <<
	        classType.className() << ".*"
	   << " from " << PublicObject::ClassName() << "," << classType.className()
	   << " where " << PublicObject::ClassName() << "._oid="
	                << classType.className() << "._oid"
	   << " and " << PublicObject::ClassName() << "." << _publicIDColumn << "='" << publicID << "'";

	Object *obj = queryObject(classType, ss.str());
	PublicObject *po = PublicObject::Cast(obj);
	if ( !po ) {
		delete obj;
		return NULL;
	}

	return po;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjects(const std::string &parentID,
                                             const RTTI &classType,
                                             bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	if ( !parentID.empty() ) {
		unsigned long long parentID_ = publicObjectId(parentID);
		if ( !parentID_ ) {
			SEISCOMP_INFO("parent object with id '%s' not found in database", parentID.c_str());
			return DatabaseIterator();
		}

		return getObjectIterator(parentID_, classType, ignorePublicObject);
	}

	return getObjectIterator(0, classType, ignorePublicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjects(const PublicObject *parent,
                                             const Seiscomp::Core::RTTI &classType,
                                             bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	unsigned long long parentID = getCachedId(parent);
	if ( !parentID && parent ) {
		parentID = publicObjectId(parent->publicID());
		if ( !parentID ) {
			SEISCOMP_INFO("parent object with id '%s' not found in database", parent->publicID().c_str());
			return DatabaseIterator();
		}
		registerId(parent, parentID);
	}

	return getObjectIterator(parentID, classType, ignorePublicObject);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseArchive::getObjectCount(const std::string &parentID,
                                       const Seiscomp::Core::RTTI &classType) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return 0;
	}

	std::stringstream ss;
	ss << "select count(*) from " << classType.className();
	if ( !parentID.empty() ) {
		ss << ",PublicObject where PublicObject._oid="
		   << classType.className() << "._parent_oid "
		      "and PublicObject." << _publicIDColumn << "='" << parentID << "'";
	}

	if ( !_db->beginQuery(ss.str().c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", ss.str().c_str());
		return 0;
	}

	size_t ret = 0;

	if ( _db->fetchRow() ) {
		ret = atoi((const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::parentPublicID(const PublicObject *object) {
	std::string query;
	query = "select Parent." + _publicIDColumn +
	        " from PublicObject as Parent, PublicObject as Child, " +
	        object->className() +
	        " where Child._oid=" +
	        object->className() + "._oid and Parent._oid=" +
	        object->className() + "._parent_oid and Child." + _publicIDColumn + "='" +
	        toSQL(object->publicID()) + "'";

	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", query.c_str());
		return "";
	}

	if ( _db->fetchRow() )
		query = (const char*)_db->getRowField(0);
	else
		query = std::string();

	_db->endQuery();

	return query;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t DatabaseArchive::getObjectCount(const PublicObject *parent,
                                       const Seiscomp::Core::RTTI &classType) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return 0;
	}

	std::stringstream ss;
	ss << "select count(*) from " << classType.className();
	if ( parent ) {
		ss << ",PublicObject where PublicObject._oid="
		   << classType.className() << "._parent_oid "
		      "and PublicObject." << _publicIDColumn<< "='" << parent->publicID() << "'";
	}

	if ( !_db->beginQuery(ss.str().c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", ss.str().c_str());
		return 0;
	}

	size_t ret = 0;

	if ( _db->fetchRow() ) {
		ret = atoi((const char*)_db->getRowField(0));
	}

	_db->endQuery();

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(unsigned long long parentID,
                                                    const RTTI &classType,
                                                    bool ignorePublicObject) {
	if ( !validInterface() ) {
		SEISCOMP_ERROR("no valid database interface");
		return DatabaseIterator();
	}

	std::string query;

	if ( ignorePublicObject || !classType.isTypeOf(PublicObject::TypeInfo()) )
		query = std::string("select * from ") + classType.className();
	else {
		std::stringstream ss;
		ss << "select " << PublicObject::ClassName() << "." << _publicIDColumn << ","
		   << classType.className() << ".* from "
		   << PublicObject::ClassName() << "," << classType.className()
		   << " where " << PublicObject::ClassName() << "._oid="
		   << classType.className() << "._oid";

		query = ss.str();
	}

	if ( parentID > 0 ) {
		if ( classType.isTypeOf(PublicObject::TypeInfo()) && !ignorePublicObject )
			query += " and ";
		else
			query += " where ";

		query += classType.className();
		query += "._parent_oid='" + toString(parentID) + "'";
	}

	return getObjectIterator(query, classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(const std::string &query,
                                                    const Seiscomp::Core::RTTI &classType) {
	return getObjectIterator(query, &classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DatabaseIterator DatabaseArchive::getObjectIterator(const std::string &query,
                                                    const Seiscomp::Core::RTTI *classType) {
	if ( !_db->beginQuery(query.c_str()) ) {
		SEISCOMP_ERROR("starting query '%s' failed", query.c_str());
		return DatabaseIterator();
	}

	if ( !_db->fetchRow() ) {
		_db->endQuery();
		return DatabaseIterator();
	}

	return DatabaseIterator(this, classType);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const Core::Time &value) const {
	return _db->timeToString(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const std::string &value) const {
	return toSQL(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::toString(const char *value) const {
	return toSQL(std::string(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(int &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(long long &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(float &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(double &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::complex<float> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::complex<double> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(bool &value) {
	if ( !strtobool(value, field()) ) {
		SEISCOMP_ERROR("DB: error in result field %d: could not cast '%s' to bool",
		               _fieldIndex, field());
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<char> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<int> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<float> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<double> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<std::string> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<Core::Time> &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::vector<std::complex<double> > &value) {
	fromString(value, field());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(std::string &value) {
	value = field();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(time_t &value) {
	Time t = _db->stringToTime(field());
	value = t.seconds();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::read(Time &value) {
	value = _db->stringToTime(field());
	if ( hint() & SPLIT_TIME ) {
		int microSeconds;
		_currentAttributeName += MICROSECONDS_POSTFIX;
		readAttrib();
		if ( field() != NULL ) {
			if ( fromString(microSeconds, field()) )
				value.setUSecs(microSeconds);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(int value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(long long value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(float value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(double value) {
	writeAttrib(toString(value));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::complex<float> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::complex<double> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(bool value) {
	writeAttrib(std::string(value?"'1'":"'0'"));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<char> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<int> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<float> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<double> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<std::string> &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<Core::Time> &value) {
	writeAttrib("'" + Core::toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::vector<std::complex<double> > &value) {
	writeAttrib("'" + toString(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(std::string &value) {
	writeAttrib("'" + toSQL(value) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(time_t value) {
	writeAttrib("'" + toString(Time(value)) + "'");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::write(Time &value) {
	writeAttrib("'" + toString(value) + "'");
	if ( hint() & SPLIT_TIME ) {
		std::string backupName = _currentAttributeName;
		_currentAttributeName += MICROSECONDS_POSTFIX;
		write((int)value.microseconds());
		_currentAttributeName = backupName;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::writeAttrib(OPT_CR(std::string) value) const {
	std::string indexName;
	std::string& index = indexName;

	if ( _currentAttributePrefix.empty() ) {
		if ( _currentAttributeName == "publicID" )
			return;
		index = _currentAttributeName;
	}
	else if ( _currentAttributeName.empty() )
		index = _currentAttributePrefix;
	else
		indexName = _currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName;

	AttributeMap* map = _objectAttributes;
	if ( (hint() & INDEX_ATTRIBUTE) && _ignoreIndexAttributes )
		map = &_indexAttributes;

	if ( value )
		(*map)[_db->convertColumnName(index)] = *value;
	else
		(*map)[_db->convertColumnName(index)] = None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::renderAttributes(const AttributeMap &attributes) {
	SEISCOMP_DEBUG("collected attributes -- list:");
	std::cout << AttributeMapper(attributes) << std::endl;
	SEISCOMP_DEBUG("collected attributes -- end list");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::renderValues(const AttributeMap &attributes) {
	SEISCOMP_DEBUG("collected values -- list:");
	std::cout << ValueMapper(attributes) << std::endl;
	SEISCOMP_DEBUG("collected values -- end list");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long long DatabaseArchive::publicObjectId(const std::string &publicId) {
	unsigned long long id = 0;
	std::stringstream ss;
	ss << "select _oid from " << PublicObject::ClassName()
	   << " where " << _publicIDColumn << "='" << toSQL(publicId) << "'";
	if ( !_db->beginQuery(ss.str().c_str()) )
		return id;

	if ( _db->fetchRow() )
		fromString(id, (const char*)_db->getRowField(0));

	_db->endQuery();

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long long DatabaseArchive::objectId(Object *object, const std::string &parentID) {
	PublicObject* publicObject = PublicObject::Cast(object);
	if ( publicObject )
		return publicObjectId(publicObject->publicID());

	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_indexAttributes.clear();
	_childTables.clear();
	_childDepth = 0;
	
	_ignoreIndexAttributes = true;

	resetAttributePrefix();

	unsigned long long iParentID = 0;

	PublicObject* parentObject = object->parent();
	if ( parentObject != NULL ) {
		iParentID = getCachedId(parentObject);
		if ( iParentID == 0 ) {
			iParentID = publicObjectId(parentObject->publicID());
			if ( iParentID )
				registerId(parentObject, iParentID);
			else {
				SEISCOMP_ERROR("objectID: parent object with publicID '%s' has not been "
				               "found in the database", parentObject->publicID().c_str());
				return (unsigned long long)-1;
			}
		}
	}
	else if ( !parentID.empty() ) {
		iParentID = publicObjectId(parentID);
		if ( !iParentID ) {
			SEISCOMP_ERROR("objectID: parent object with publicID '%s' has not been "
			               "found in the database", parentID.c_str());
			return (unsigned long long)-1;
		}
	}
	else {
		SEISCOMP_ERROR("objectID: no parent object given");
		return (unsigned long long)-1;
	}

	_isReading = false;

	_validObject = true;
	object->serialize(*this);
	if ( !_validObject ) {
		SEISCOMP_ERROR("failed to query for object");
		return (unsigned long long)-1;
	}

	if ( _indexAttributes.empty() ) {
		SEISCOMP_WARNING("objectID: index is empty");
		_indexAttributes = *_objectAttributes;
		//_isReading = true;
		//return (long unsigned int)-1;
	}

	_indexAttributes["_parent_oid"] = toString(iParentID);

	std::stringstream ss;
	ss << "select _oid from " << object->className() << " where ";

	bool first = true;
	for ( AttributeMap::iterator it = _indexAttributes.begin();
	      it != _indexAttributes.end(); ++it ) {
		if ( !first )
			ss << " and ";
		ss << it->first;
		if ( it->second )
			ss << "=" << *it->second;
		else
			ss << " is null";
		first = false;
	}

	_isReading = true;

	if ( !_db->beginQuery(ss.str().c_str()) )
		return (unsigned long long)-1;

	unsigned long long id = (unsigned long long)-1;

	if ( _db->fetchRow() )
		fromString(id, (const char*)_db->getRowField(0));

	_db->endQuery();

	return id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long long DatabaseArchive::insertObject() {
	std::stringstream ss;
	ss << "insert into " << Object::ClassName() << "(_oid) values("
	   << _db->defaultValue() << ")";

	if ( !_db->execute(ss.str().c_str()) )
		return 0;

	return _db->lastInsertId(Object::ClassName());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long long DatabaseArchive::insertPublicObject(const std::string &publicId) {
	if ( publicId.empty() ) return 0;

	unsigned long long objectId = insertObject();
	if ( objectId == 0 )
		return 0;

	std::stringstream ss;
	ss << "insert into " << PublicObject::ClassName()
	   << "(_oid," << _publicIDColumn << ") values("
	   << objectId << ",'" << toSQL(publicId) << "')";

	if ( !_db->execute(ss.str().c_str()) ) {
		deleteObject(objectId);
		return 0;
	}

	return objectId;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::insertRow(const std::string &table,
                                const AttributeMap &attribs,
                                const std::string &parentId) {
	std::stringstream ss;
	ss.precision(12);
	ss << "insert into " << table << "(";

	ss << AttributeMapper(attribs);

	ss << ") ";

	if ( parentId.empty() )
		ss << "values (";
	else
		ss << "select ";

	ss << ValueMapper(attribs);

	if ( parentId.empty() )
		ss << ")";
	else
		ss << " from " << PublicObject::ClassName()
		   << " where " << PublicObject::ClassName()
		   << "." << _publicIDColumn << "='" << toSQL(parentId) << "'";

	//SEISCOMP_DEBUG(ss.str().c_str());
	return _db->execute(ss.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::deleteObject(unsigned long long id) {
	std::stringstream ss;
	ss << "delete from " << Object::ClassName()
	   << " where _oid=" << id;
	SEISCOMP_DEBUG("deleting object with id %lld", id);
	return _db->execute(ss.str().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::write(Object* object, const std::string &parentId) {
	if ( object == NULL ) return false;

	if ( !validInterface() ) {
		setValidity(false);
		return false;
	}

	_validObject = true;

	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_childTables.clear();
	_childDepth = 0;
	_ignoreIndexAttributes = false;

	resetAttributePrefix();

	unsigned long long objectId;
	PublicObject* publicObject = PublicObject::Cast(object);
	if ( publicObject != NULL ) {
		if ( publicObjectId(publicObject->publicID()) > 0 ) {
			SEISCOMP_ERROR("object with publicID '%s' exists already",
			               publicObject->publicID().c_str());
			_isReading = true;
			return false;
		}

		objectId = insertPublicObject(publicObject->publicID());
		if ( !objectId ) {
			SEISCOMP_ERROR("writing object with publicID '%s' failed",
			               publicObject->publicID().c_str());
			_isReading = true;
			return false;
		}
	}
	else {
		objectId = insertObject();
		if ( !objectId ) {
			SEISCOMP_ERROR("writing object failed");
			setValidity(false);
			return false;
		}
	}

	_isReading = false;

	object->serialize(*this);
	if ( !_validObject ) {
		SEISCOMP_ERROR("serializing object with type '%s' failed", object->className());
		deleteObject(objectId);
		return false;
	}

	_rootAttributes["_oid"] = toString(objectId);
	bool success = false;

	PublicObject* parentObject = object->parent();
	if ( parentObject != NULL ) {
		unsigned long long iParentId = getCachedId(parentObject);
		if ( iParentId == 0 ) {
			iParentId = publicObjectId(parentObject->publicID());
			if ( iParentId )
				registerId(parentObject, iParentId);
		}
		/*
		if ( parentId == 0 )
			_rootAttributes["_parent_oid"] = "(select _oid from " + std::string(PublicObject::ClassName()) + " where publicID='" + parentObject->publicID() + "')";
		else
			_rootAttributes["_parent_oid"] = toString(parentId);
		*/
		if ( iParentId ) {
			_rootAttributes["_parent_oid"] = toString(iParentId);
			success = insertRow(object->className(), *_objectAttributes);
		}
	}
	else if ( !parentId.empty() ) {
		//_rootAttributes["_parent_oid"] = "(select _oid from " + std::string(PublicObject::ClassName()) + " where publicID='" + parentId + "')";
		unsigned long long iParentId = publicObjectId(parentId);
		if ( iParentId ) {
			_rootAttributes["_parent_oid"] = toString(iParentId);
			success = insertRow(object->className(), *_objectAttributes);
		}
		else
			SEISCOMP_ERROR("failed to get oid for object '%s'", parentId.c_str());
	}
	else
		success = insertRow(object->className(), *_objectAttributes);

	if ( success )
		registerId(object, objectId);
	else {
		SEISCOMP_ERROR("writing object with type '%s' failed",
		                object->className());
		deleteObject(objectId);
	}

	_isReading = true;
	_validObject = success;
	return success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::update(Object *object, const std::string &parentID) {
	if ( object == NULL ) return false;

	if ( !validInterface() ) {
		setValidity(false);
		return false;
	}

	_validObject = true;

	_objectAttributes = &_rootAttributes;
	_objectAttributes->clear();
	_indexAttributes.clear();
	_childTables.clear();
	_childDepth = 0;

	PublicObject* publicObject = PublicObject::Cast(object);
	
	_ignoreIndexAttributes = (publicObject == NULL);

	resetAttributePrefix();

	unsigned long long iParentID = 0;
	unsigned long long iPublicID = 0;

	PublicObject* parentObject = object->parent();
	if ( parentObject != NULL ) {
		iParentID = getCachedId(parentObject);
		if ( iParentID == 0 ) {
			iParentID = publicObjectId(parentObject->publicID());
			if ( iParentID )
				registerId(parentObject, iParentID);
			else {
				SEISCOMP_ERROR("update: parent object with publicID '%s' has not been "
				               "found in the database", parentObject->publicID().c_str());
				setValidity(false);
				return false;
			}
		}
	}
	else if ( !parentID.empty() ) {
		iParentID = publicObjectId(parentID);
		if ( !iParentID ) {
			SEISCOMP_ERROR("update: parent object with publicID '%s' has not been "
			               "found in the database", parentID.c_str());
			setValidity(false);
			return false;
		}
	}
	else {
		SEISCOMP_ERROR("update: no parent object given, aborting update");
		setValidity(false);
		return false;
	}

	if ( publicObject ) {
		iPublicID = getCachedId(publicObject);
		if ( iPublicID == 0 ) {
			iPublicID = publicObjectId(publicObject->publicID());
			if ( iPublicID )
				registerId(publicObject, iPublicID);
		}

		if ( iPublicID == 0 ) {
			SEISCOMP_ERROR("update: object with publicID '%s' has not been found in the database",
			               publicObject->publicID().c_str());
			setValidity(false);
			return false;
		}
	}

	_isReading = false;

	object->serialize(*this);
	if ( !_validObject ) {
		SEISCOMP_ERROR("serializing updated object with type '%s' failed", object->className());
		return false;
	}

	if ( _objectAttributes->empty() ) {
		SEISCOMP_DEBUG("no update for object type '%s' possible, empty list of non-index attributes", object->className());
		return true;
	}

	if ( iPublicID )
		_indexAttributes["_oid"] = toString(iPublicID);

	if ( _indexAttributes.empty() ) {
		SEISCOMP_ERROR("update: index is empty, no update possible");
		_isReading = true;
		return false;
	}

	_indexAttributes["_parent_oid"] = toString(iParentID);

	std::stringstream ss;
	ss << "update " << object->className() << " set ";

	bool first = true;
	for ( AttributeMap::iterator it = _objectAttributes->begin();
	      it != _objectAttributes->end(); ++it ) {
		if ( !first ) ss << ",";
		ss << it->first << "=" << (it->second?*it->second:"NULL");
		first = false;
	}

	ss << " where ";

	first = true;
	for ( AttributeMap::iterator it = _indexAttributes.begin();
	      it != _indexAttributes.end(); ++it ) {
		if ( !first ) ss << " and ";
		ss << it->first;
		if ( it->second )
			ss << "=" << *it->second;
		else
			ss << " is null";
		first = false;
	}

	_isReading = true;

	_validObject = _db->execute(ss.str().c_str());

	return success();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::remove(Object *object, const std::string &parentID) {
	if ( object == NULL ) return false;

	if ( !validInterface() ) {
		setValidity(false);
		return false;
	}

	unsigned long long objectID = getCachedId(object);
	if ( objectID == 0 )
		objectID = objectId(object, parentID);

	if ( objectID == 0 || objectID == (unsigned long long)-1 ) {
		SEISCOMP_WARNING("remove: object '%s' has not been found in database",
		                 object->className());
		return true;
	}

	_db->execute((std::string("delete from ") + object->className() +
		             " where _oid=" + toString(objectID)).c_str());
	if ( PublicObject::Cast(object) )
		_db->execute((std::string("delete from ") + PublicObject::ClassName() +
		             " where _oid=" + toString(objectID)).c_str());

	deleteObject(objectID);
	removeId(object);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::addTree(Object *object, const std::string &parentID,
                              int *objectsHandled) {
	DatabaseObjectWriter writer(*this, true);
	bool res = writer(object, parentID);
	if ( objectsHandled ) *objectsHandled = writer.count();
	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::removeTree(Object *object, const std::string &parentID,
                                 int *objectsHandled) {
	DatabaseObjectWriter writer(*this, false);
	bool res = writer(object, parentID);
	if ( objectsHandled ) *objectsHandled = writer.count();
	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::readAttrib() const {
	if ( _currentAttributePrefix.empty() ) {
		_fieldIndex = _db->findColumn(_db->convertColumnName(_currentAttributeName).c_str());
		//SEISCOMP_DEBUG("looking up column '%s' %s", _currentAttributeName.c_str(), _fieldIndex != -1?"succeeded":"failed");
	}
	else {
		_fieldIndex = _db->findColumn(_db->convertColumnName(_currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName).c_str());
		//SEISCOMP_DEBUG("looking up column '%s' %s", (_currentAttributePrefix + ATTRIBUTE_SEPERATOR + _currentAttributeName).c_str(), _fieldIndex != -1?"succeeded":"failed");
	}

	if ( _fieldIndex != -1 ) {
		_field = (const char*)_db->getRowField(_fieldIndex);
		_fieldSize = _db->getRowFieldSize(_fieldIndex);
	}
	else {
		_field = NULL;
		_fieldSize = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::locateObjectByName(const char *name, const char *targetClass, bool nullable) {
	if ( !isReading() ) {
		if ( !isEmpty(targetClass) ) {
			if ( !isEmpty(name) )
				pushAttributePrefix(name);

			if ( hint() & DB_TABLE ) {
				_currentAttributeName = "";

				_currentChildTable = _childTables.insert(_childTables.end(), ChildTable());
				_currentChildTable->first = targetClass;
				_objectAttributes = &_currentChildTable->second;

				++_childDepth;
			}
			else if ( nullable ) {
				_currentAttributeName = OBJECT_USED_POSTFIX;
				write((bool)true);
			}
		}
		else {
			if ( !isEmpty(name) )
				_currentAttributeName = name;
			else
				_currentAttributeName.clear();
		}
		return true;
	}

	if ( !isEmpty(targetClass) && (hint() & STATIC_TYPE) ) {
		if ( !isEmpty(name) ) {
			pushAttributePrefix(name);
			_currentAttributeName = name;
		}

		if ( !(hint() & DB_TABLE) ) {
			// When the object is nullable a special column has been added
			// to signal whether the complete type is set or not.
			// This column is named '[attributeName]_used' and contains
			// either 0 or 1.
			if ( nullable ) {
				bool used = false;
				_currentAttributeName = OBJECT_USED_POSTFIX;
				readAttrib();

				if ( _field )
					strtobool(used, _field);

				if ( !used ) {
					popAttribPrefix();
					return false;
				}
			}
			return true;
		}

		_currentAttributeName = CHILD_ID_POSTFIX;
	}
	else
		_currentAttributeName = name;

	readAttrib();

	if ( hint() & DB_TABLE ) {
		if ( _field == NULL ) {
			popAttribPrefix();
			return false;
		}

		unsigned long long childId;
		fromString(childId, _field);
		SEISCOMP_DEBUG("should read child table '%s' with _oid=%lld", targetClass, childId);
	}
	
	return _field != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::locateNextObjectByName(const char *name, const char *targetClass) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::locateNullObjectByName(const char *name, const char *targetClass, bool /*first*/) {
	if ( !isEmpty(name) ) {
		if ( !isEmpty(targetClass) ) {
			if ( !(hint() & DB_TABLE) ) {
				_currentAttributeName = std::string(name) + ATTRIBUTE_SEPERATOR "" OBJECT_USED_POSTFIX;
				write((bool)false);
				return;
			}
			else
				return;
		}
		_currentAttributeName = name;
	}
	else {
		if ( isEmpty(targetClass) )
			_currentAttributeName.clear();
		else
			_currentAttributeName = targetClass;
	}

	writeAttrib(None);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::resetAttributePrefix() const {
	_prefixPos = 0;
	_currentAttributePrefix = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::pushAttributePrefix(const char *name) const {
	if ( _prefixPos >= 64 ) {
		throw OverflowException("The attribute prefix cannot be pushed, stack overflow (more than 64 elements)");
		return;
	}

	_prefixOffset[_prefixPos++] = _currentAttributePrefix.size();
	if ( !name ) return;

	if ( _currentAttributePrefix.empty() )
		_currentAttributePrefix = name;
	else {
		_currentAttributePrefix += ATTRIBUTE_SEPERATOR;
		_currentAttributePrefix += name;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void DatabaseArchive::popAttribPrefix() const {
	--_prefixPos;
	if ( _prefixPos < 0 ) {
		_prefixPos = 0;
		return;
	}

	_currentAttributePrefix.erase(_prefixOffset[_prefixPos]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serialize(RootType *object) {
	BaseObject::Archive::serialize(object);
	popAttribPrefix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serialize(SerializeDispatcher &disp) {
	if ( hint() & DB_TABLE ) {
		if ( !isReading() ) {
			std::string backupPrefix(_currentAttributePrefix);
			_currentAttributePrefix = "";
			BaseObject::Archive::serialize(disp);
			_currentAttributePrefix = backupPrefix;
			_currentAttributeName = CHILD_ID_POSTFIX;
			--_childDepth;
	
			if ( !_childDepth )
				_objectAttributes = &_rootAttributes;
			else {
				--_currentChildTable;
				_objectAttributes = &_currentChildTable->second;
			}
	
			if ( !insertRow(_currentChildTable->first,
			                _currentChildTable->second) )
				return;
	
			write((long long)_db->lastInsertId(Object::ClassName()));
		}
		else {
			std::string backupPrefix(_currentAttributePrefix);
			_currentAttributePrefix = "";
			_currentAttributePrefix = backupPrefix;
		}
	}
	else
		BaseObject::Archive::serialize(disp);

	if ( hint() & STATIC_TYPE )
		popAttribPrefix();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::determineClassName() {
	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::setClassName(const char *) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::buildQuery(const std::string &table,
                                        const std::string &filter) {
	if ( filter.empty() )
		return std::string("select * from ") + table;
	else
		return std::string("select * from ") + table + " where " + filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string DatabaseArchive::buildExtendedQuery(const std::string &what,
                                                   const std::string &tables,
                                                   const std::string &filter)
{
	if ( filter.empty() )
		return std::string("select ") + what + " from " + tables;
	else
		return std::string("select ") + what + " from " + tables +
		       " where " + filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DatabaseArchive::validInterface() const {
	return _db != NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::onObjectDestroyed(Object *object) {
	removeId(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long long DatabaseArchive::getCachedId(const Object *o) const {
	ObjectIdMap::const_iterator it = _objectIdCache.find(o);
	if ( it == _objectIdCache.end() )
		return 0;
	return (*it).second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::registerId(const Object *o, unsigned long long id) {
	_objectIdCache[o] = id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int DatabaseArchive::getCacheSize() const {
	return _objectIdCache.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::serializeObject(Object *obj) {
	if ( obj == NULL ) return;

	resetAttributePrefix();

	_validObject = true;
	obj->serialize(*this);

	if ( _db != NULL && isReading() ) {
		int idId = _db->findColumn("_oid");
		if ( idId != -1 ) {
			fromString(idId, (const char*)_db->getRowField(idId));
			registerId(obj, idId);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DatabaseArchive::removeId(Object *o) {
	static int debug = 3;
	ObjectIdMap::iterator it = _objectIdCache.find(o);
	if ( it != _objectIdCache.end() ) {
		if ( debug ) {
			SEISCOMP_DEBUG("Object removed from cache with id: %lld", (*it).second);
			if ( debug == 1 )
				SEISCOMP_DEBUG("This message will not be repeated for other objects");
			--debug;
		}
		_objectIdCache.erase(it);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
