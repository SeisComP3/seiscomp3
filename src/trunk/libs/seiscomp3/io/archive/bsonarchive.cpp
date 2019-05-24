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

#define SEISCOMP_COMPONENT BSONArchive
#include <seiscomp3/logging/log.h>
#include <seiscomp3/io/archive/bsonarchive.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/platform/platform.h>
#include <seiscomp3/datamodel/version.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>

#include "bson/bson.h"


namespace Seiscomp {
namespace IO {


namespace {

ssize_t streamBufReadCallback(void* context, uint8_t* buffer, size_t len) {
	std::streambuf* buf = static_cast<std::streambuf*>(context);
	if ( buf == NULL ) return -1;

	long count = 0;
	int ch = buf->sgetc();
	while ( ch != EOF && len-- ) {
		*buffer++ = (char)buf->sbumpc();
		ch = buf->sgetc();
		++count;
	}

	return count;
}

ssize_t streamBufReadCallback2(void* context, void* buffer, size_t len) {
	return streamBufReadCallback(context, (unsigned char *) buffer, len);
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class BSONArchive::BSONImpl : public Core::BaseObject {
	public:
		BSONImpl() {
			root = NULL;
			current = NULL;
			children = NULL;
			bsonReader = NULL;
			jsonReader = NULL;
			links = NULL;
		}

		const bson_t       *root;
		bson_t             *current;
		bson_t             *children;

		bson_reader_t      *bsonReader;
		bson_json_reader_t *jsonReader;

		bson_iter_t         iter;
		bson_iter_t         iterParent;
		bson_iter_t         iterChildren;

		std::list<std::pair<std::string, bson_t*> > *links;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BSONArchive::BSONArchive() : Seiscomp::Core::Archive() {
	_siblingCount = 0;
	_startSequence = false;
	_validObject = false;
	_buf = NULL;
	_deleteOnClose = false;
	_compression = false;
	_json = false;
	_forceWriteVersion = -1;

	_impl = new BSONImpl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BSONArchive::BSONArchive(std::streambuf* buf, bool isReading, int forceWriteVersion) {
	_siblingCount = 0;
	_startSequence = false;
	_validObject = false;
	_buf = NULL;
	_deleteOnClose = false;
	_compression = false;
	_json = false;
	_forceWriteVersion = forceWriteVersion;

	_impl = new BSONImpl;

	if ( isReading )
		open(buf);
	else
		create(buf);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
BSONArchive::~BSONArchive() {
	close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::open(std::streambuf* buf) {
	close();

	if ( buf == NULL ) return false;

	_buf = buf;
	_deleteOnClose = false;

	return open();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::open(const char* filename) {
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::open() {
	if ( !Seiscomp::Core::Archive::open(NULL) )
		return false;

	std::streambuf* buf = _buf;
	boost::iostreams::filtering_istreambuf filtered_buf;

	if ( _compression ) {
		filtered_buf.push(boost::iostreams::zlib_decompressor());
		filtered_buf.push(*_buf);
		buf = &filtered_buf;
	}

	if ( _json ) {
		_impl->jsonReader = bson_json_reader_new(
				buf,
				streamBufReadCallback,
				NULL,
				false,
				16384);

		if ( _impl->jsonReader == NULL )
			return false;

		_impl->root = bson_new();

		bson_error_t err;

		switch ( bson_json_reader_read(_impl->jsonReader, (bson_t *) _impl->root, &err) ) {
			case -1: // error set
				SEISCOMP_ERROR("%s", err.message);

			case 0: // no data was read
				bson_json_reader_destroy(_impl->jsonReader);
				_impl->jsonReader = NULL;
				return false;

			default:
				break;
		}
	}
	else {
		_impl->bsonReader = bson_reader_new_from_handle(
				buf,
				streamBufReadCallback2,
				NULL);

		if ( _impl->bsonReader == NULL )
			return false;

		_impl->root = bson_reader_read(_impl->bsonReader, NULL);

		if ( _impl->root == NULL ) {
			bson_reader_destroy(_impl->bsonReader);
			_impl->bsonReader = NULL;
			return false;
		}
	}

	bson_iter_t iter;
	const char *ver;
	uint32_t len;

	if ( bson_iter_init_find(&iter, _impl->root, "version") && ((ver = bson_iter_utf8(&iter, &len)) != NULL) ) {
		std::string version(ver, len);
		size_t pos = version.find(".");

		if ( pos != std::string::npos) {
			int major;
			int minor;

			if ( Core::fromString(major, version.substr(0, pos)) &&
					Core::fromString(minor, version.substr(pos + 1, std::string::npos)) )
				setVersion(Core::Version(major, minor));
			else
				setVersion(Core::Version(0,0));
		}
		else {
			int major;

			if ( Core::fromString(major, version.substr(0, pos)) )
				setVersion(Core::Version(major ,0));
			else
				setVersion(Core::Version(0,0));
		}

	}
	else {
		setVersion(Core::Version(0,0));
	}

	bson_iter_init(&_impl->iterParent, _impl->root);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::create(std::streambuf* buf, bool writeVersion) {
	close();

	_buf = buf;
	_deleteOnClose = false;

	return create(writeVersion);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::create(const char* filename, bool writeVersion) {
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

	return create(writeVersion);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::create(bool writeVersion) {
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

	_impl->current = bson_new();
	_impl->links = new std::list<std::pair<std::string, bson_t*> >;

	bson_append_utf8(_impl->current, "version", -1, (Seiscomp::Core::toString(versionMajor()) + "." + Seiscomp::Core::toString(versionMinor())).c_str(), -1);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::close() {
	if ( isReading() ) {
		if ( _impl->jsonReader != NULL ) {
			bson_destroy((bson_t *)_impl->root);
			bson_json_reader_destroy(_impl->jsonReader);
			_impl->root = NULL;
			_impl->jsonReader = NULL;
		}
		else if ( _impl->bsonReader != NULL ) {
			// _root points into reader
			bson_reader_destroy(_impl->bsonReader);
			_impl->root = NULL;
			_impl->bsonReader = NULL;
		}
	}
	else {
		if ( _impl->links != NULL ) {
			while ( _impl->links->size() > 0 ) {
				std::pair<std::string, bson_t*> link = _impl->links->front();
				_impl->links->pop_front();
				bson_append_array(_impl->current, link.first.c_str(), -1, link.second);
				bson_destroy(link.second);
			}

			delete _impl->links;
			_impl->links = NULL;
		}

		if ( _buf ) {
			std::streambuf *buf = _buf;
			boost::iostreams::filtering_ostreambuf filtered_buf;

			if ( _compression ) {
				filtered_buf.push(boost::iostreams::zlib_compressor());
				filtered_buf.push(*_buf);
				buf = &filtered_buf;
			}

			if ( _json ) {
				size_t len;
				char* str = bson_as_json(_impl->current, &len);
				buf->sputn(str, len);
				bson_free(str);
			}
			else {
				buf->sputn((const char *) bson_get_data(_impl->current), _impl->current->len);
			}
		}

		if ( _impl->current != NULL ) {
			bson_destroy(_impl->current);
			_impl->current = NULL;
		}
	}

	if ( _deleteOnClose && _buf )
		delete _buf;

	_deleteOnClose = false;
	_buf = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::setCompression(bool enable) {
	_compression = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::setJSON(bool enable) {
	_json = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(time_t& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_DATE_TIME:
			value = bson_iter_time_t(&_impl->iter);
			setValidity(true);
			break;

		default:
			SEISCOMP_ERROR("Invalid time_t value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(Seiscomp::Core::Time& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_NULL:
			value = Core::Time::Null;
			setValidity(true);
			break;

		case BSON_TYPE_UTF8:
			if ( Core::fromString(value, bson_iter_utf8(&_impl->iter, NULL)) ) {
				setValidity(true);
				break;
			}

		default:
			SEISCOMP_ERROR("Invalid Core::Time value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(int& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_INT32:
			value = bson_iter_int32(&_impl->iter);
			setValidity(true);
			break;

		case BSON_TYPE_INT64:
			value = bson_iter_int64(&_impl->iter);
			setValidity(true);
			break;

		default:
			SEISCOMP_ERROR("Invalid int value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(float& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_DOUBLE:
		{
			double tmp = bson_iter_double(&_impl->iter);
			if ( (tmp >= -std::numeric_limits<float>::max() && tmp <= std::numeric_limits<float>::max()) ||
			     tmp == std::numeric_limits<double>::infinity() || tmp == -std::numeric_limits<double>::infinity()) {
				value = tmp;
				setValidity(true);
				break;
			}
		}

		case BSON_TYPE_INT32:
		{
			double tmp = bson_iter_int32(&_impl->iter);
			if ( (tmp >= -std::numeric_limits<float>::max() && tmp <= std::numeric_limits<float>::max()) ||
			     tmp == std::numeric_limits<double>::infinity() || tmp == -std::numeric_limits<double>::infinity()) {
				value = tmp;
				setValidity(true);
				break;
			}
		}

		case BSON_TYPE_INT64:
		{
			double tmp = bson_iter_int64(&_impl->iter);
			if ( (tmp >= -std::numeric_limits<float>::max() && tmp <= std::numeric_limits<float>::max()) ||
			      tmp == std::numeric_limits<double>::infinity() || tmp == -std::numeric_limits<double>::infinity()) {
				value = tmp;
				setValidity(true);
				break;
			}
		}

		default:
			SEISCOMP_ERROR("Invalid float value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(double& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_DOUBLE:
			value = bson_iter_double(&_impl->iter);
			setValidity(true);
			break;

		case BSON_TYPE_INT32:
			value = bson_iter_int32(&_impl->iter);
			setValidity(true);
			break;

		case BSON_TYPE_INT64:
			value = bson_iter_int64(&_impl->iter);
			setValidity(true);
			break;

		default:
			SEISCOMP_ERROR("Invalid double value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(bool& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_BOOL:
			value = bson_iter_bool(&_impl->iter);
			setValidity(true);
			break;

		default:
			SEISCOMP_ERROR("Invalid boolean value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::string& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_UTF8:
		{
			uint32_t len;
			const char* tmp = bson_iter_utf8(&_impl->iter, &len);
			value = std::string(tmp, len);
			setValidity(true);
			break;
		}

		default:
			SEISCOMP_ERROR("Invalid string value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(char& value) {
	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_UTF8:
		{
			uint32_t len;
			const char* tmp = bson_iter_utf8(&_impl->iter, &len);

			if ( len == 1 ) {
				value = tmp[0];
				setValidity(true);
				break;
			}
		}

		default:
			SEISCOMP_ERROR("Invalid char value");
			setValidity(false);
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void BSONArchive::readVector(std::vector<T>& value) {
	bson_iter_t iter = _impl->iter;

	switch ( bson_iter_type (&_impl->iter) ) {
		case BSON_TYPE_ARRAY:
			if ( bson_iter_recurse(&iter, &_impl->iter) ) {
				std::vector<T> tmpvec;

				while ( bson_iter_next(&_impl->iter) ) {
					T tmpval;
					read(tmpval);

					if ( !_validObject ) {
						_impl->iter = iter;
						SEISCOMP_ERROR("Invalid vector element");
						return;
					}

					tmpvec.push_back(tmpval);
				}

				value = tmpvec;
				setValidity(true);
				break;
			}

		default:
			SEISCOMP_ERROR("Invalid vector");
			setValidity(false);
			break;
	}

	_impl->iter = iter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<int>& value) {
	readVector<int>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<float>& value) {
	readVector<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<double>& value) {
	readVector<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<std::string>& value) {
	readVector<std::string>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<Core::Time>& value) {
	readVector<Core::Time>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<char>& value) {
	readVector<char>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void BSONArchive::readComplex(std::complex<T>& value) {
	std::vector<T> v;

	readVector<T>(v);

	if ( _validObject && v.size() == 2 ) {
		value = std::complex<T>(v[0], v[1]);
		setValidity(true);
	}
	else {
		SEISCOMP_ERROR("Invalid complex value");
		setValidity(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::complex<float>& value) {
	readComplex<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::complex<double>& value) {
	readComplex<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::read(std::vector<std::complex<double> >& value) {
	readVector<std::complex<double> >(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(time_t value) {
	bson_append_time_t(_impl->current, _attribName.c_str(), -1, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(Seiscomp::Core::Time& value) {
	if ( value.valid() || (hint() & XML_MANDATORY) )
		bson_append_utf8(_impl->current, _attribName.c_str(), -1, Core::toString(value).c_str(), -1);
	else
		bson_append_null(_impl->current, _attribName.c_str(), -1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(int value) {
	if ( (value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max()) )
		bson_append_int32(_impl->current, _attribName.c_str(), -1, value);
	else
		bson_append_int64(_impl->current, _attribName.c_str(), -1, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(float value) {
	bson_append_double(_impl->current, _attribName.c_str(), -1, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(double value) {
	bson_append_double(_impl->current, _attribName.c_str(), -1, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(bool value) {
	bson_append_bool(_impl->current, _attribName.c_str(), -1, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::string& value) {
	bson_append_utf8(_impl->current, _attribName.c_str(), -1, value.c_str(), -1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(char& value) {
	bson_append_utf8(_impl->current, _attribName.c_str(), -1, &value, 1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void BSONArchive::writeVector(std::vector<T>& value) {
	bson_t b = BSON_INITIALIZER;
	bson_t* current = _impl->current;
	std::string attribName = _attribName;
	_impl->current = &b;

	int arrayIndex = 0;

	for ( typename std::vector<T>::iterator it = value.begin(); it != value.end(); ++it) {
		_attribName = Core::toString(arrayIndex);
		write(*it);
		++arrayIndex;
	}

	_attribName = attribName;
	_impl->current = current;

	bson_append_array(_impl->current, _attribName.c_str(), -1, &b);
	bson_destroy(&b);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<int>& value) {
	writeVector<int>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<float>& value) {
	writeVector<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<double>& value) {
	writeVector<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<std::string>& value) {
	writeVector<std::string>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<Core::Time>& value) {
	writeVector<Core::Time>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<char>& value) {
	writeVector<char>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template<typename T>
void BSONArchive::writeComplex(std::complex<T>& value) {
	std::vector<T> v(2);

	v[0] = value.real();
	v[1] = value.imag();

	writeVector<T>(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::complex<float>& value) {
	writeComplex<float>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::complex<double>& value) {
	writeComplex<double>(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::write(std::vector<std::complex<double> >& value) {
	writeVector<std::complex<double> >(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::locateObjectByName(const char* name, const char* targetClass, bool) {
	_className = (targetClass != NULL) ? targetClass : "";

	if ( targetClass != NULL ) {
		if ( hint() & STATIC_TYPE ) {
			if ( isReading() ) {
				_impl->iter = _impl->iterParent;

				if ( bson_iter_find(&_impl->iter, name) ) {
					if ( _startSequence ) {
						if ( bson_iter_type(&_impl->iter) == BSON_TYPE_ARRAY &&
								bson_iter_recurse(&_impl->iter, &_impl->iterChildren) &&
								bson_iter_next(&_impl->iterChildren) &&
								bson_iter_type(&_impl->iterChildren) == BSON_TYPE_DOCUMENT ) {
							_impl->iter = _impl->iterChildren;
							return true;
						}
					}
					else {
						if ( bson_iter_type(&_impl->iter) == BSON_TYPE_DOCUMENT )
							return true;
					}
				}

				return false;
			}
			else {
				_attribName = name;
				_impl->children = NULL;
				return true;
			}
		}
		else {
			if ( isReading() ) {
				while ( bson_iter_next(&_impl->iterParent) ) {
					if ( _startSequence ) {
						if ( bson_iter_type(&_impl->iterParent) == BSON_TYPE_ARRAY &&
								bson_iter_recurse(&_impl->iterParent, &_impl->iterChildren) &&
								bson_iter_next(&_impl->iterChildren) &&
								bson_iter_type(&_impl->iterChildren) == BSON_TYPE_DOCUMENT ) {
							_className = bson_iter_key(&_impl->iterParent);
							_impl->iter = _impl->iterChildren;
							return true;
						}
					}
					else {
						if ( bson_iter_type(&_impl->iterParent) == BSON_TYPE_DOCUMENT ) {
							_className = bson_iter_key(&_impl->iterParent);
							_impl->iter = _impl->iterParent;
							return true;
						}
					}
				}

				return false;
			}
			else {
				_attribName = targetClass;
				_impl->children = NULL;
				return true;
			}
		}
	}
	else {
		if ( isReading() ) {
			_impl->iter = _impl->iterParent;
			return bson_iter_find(&_impl->iter, name);
		}
		else {
			_attribName = name;
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool BSONArchive::locateNextObjectByName(const char* name, const char* targetClass) {
	if ( isReading() ) {
		if (  bson_iter_next(&_impl->iterChildren ) ) {
			_impl->iter = _impl->iterChildren;
			return true;
		}

		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::readSequence() {
	_startSequence = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::writeSequence(int size) {
	_startSequence = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string BSONArchive::determineClassName() {
	_startSequence = false;
	return _className;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::setClassName(const char* className) {
	if ( className != NULL ) {
		_className = className;
		_attribName = className;
	}

	if ( _className.length() > 0 && _startSequence ) {
		_impl->children = bson_new();
		_siblingCount = 0;
		_impl->links->push_back(make_pair(_attribName, _impl->children));
	}

	_startSequence = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::setValidity(bool v) {
	_validObject = v;
	Seiscomp::Core::Archive::setValidity(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::serialize(RootType* object) {
	bson_t* current = _impl->current;
	bson_t* children = _impl->children;
	bson_iter_t iterParent = _impl->iterParent;
	bson_iter_t iterChildren = _impl->iterChildren;
	std::list<std::pair<std::string, bson_t*> >* links = _impl->links;
	std::string attribName = _attribName;
	std::string className = _className;
	int siblingCount = _siblingCount;
	bool startSequence = _startSequence;

	if ( isReading() ) {
		if ( !bson_iter_recurse(&_impl->iter, &_impl->iterParent) )
			SEISCOMP_ERROR("Could not recurse into %s", attribName.c_str());
	}
	else {
		_impl->current = bson_new();
		_impl->children = NULL;
		_siblingCount = 0;
		_impl->links = new std::list<std::pair<std::string, bson_t*> >;
		_startSequence = false;
	}

	Seiscomp::Core::Archive::serialize(object);

	if ( !isReading() ) {
		while ( _impl->links->size() > 0 ) {
			std::pair<std::string, bson_t*> link = _impl->links->front();
			_impl->links->pop_front();
			bson_append_array(_impl->current, link.first.c_str(), -1, link.second);
			bson_destroy(link.second);
		}

		delete _impl->links;

		if ( children == NULL ) {
			bson_append_document(current, attribName.c_str(), -1, _impl->current);
		}
		else {
			bson_append_document(children, Core::toString(siblingCount).c_str(), -1, _impl->current);
			++siblingCount;
		}

		bson_destroy(_impl->current);
	}

	_impl->current = current;
	_impl->children = children;
	_impl->iterParent = iterParent;
	_impl->iterChildren = iterChildren;
	_impl->links = links;
	_attribName = attribName;
	_className = className;
	_siblingCount = siblingCount;
	_startSequence = startSequence;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void BSONArchive::serialize(SerializeDispatcher& disp) {
	bson_t* current = _impl->current;
	bson_t* children = _impl->children;
	bson_iter_t iterParent = _impl->iterParent;
	bson_iter_t iterChildren = _impl->iterChildren;
	std::list<std::pair<std::string, bson_t*> >* links = _impl->links;
	std::string attribName = _attribName;
	std::string className = _className;
	int siblingCount = _siblingCount;
	bool startSequence = _startSequence;

	Seiscomp::Core::Archive::serialize(disp);

	_impl->current = current;
	_impl->children = children;
	_impl->iterParent = iterParent;
	_impl->iterChildren = iterChildren;
	_impl->links = links;
	_attribName = attribName;
	_className = className;
	_siblingCount = siblingCount;
	_startSequence = startSequence;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
