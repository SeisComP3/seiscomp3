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

#ifndef __SEISCOMP_IO_HTTPSOCKET_IPP__
#define __SEISCOMP_IO_HTTPSOCKET_IPP__


#include <boost/iostreams/concepts.hpp>


namespace Seiscomp {
namespace IO {


namespace {

template <typename SocketType>
class SC_SYSTEM_CORE_API HttpSource : public boost::iostreams::source {
	public:
		HttpSource(HttpSocket<SocketType> *sock);
		std::streamsize read(char* buf, std::streamsize size);

	private:
		HttpSocket<SocketType> *_sock;
};

template <typename SocketType>
HttpSource<SocketType>::HttpSource(HttpSocket<SocketType> *sock): _sock(sock)
{
}

template <typename SocketType>
std::streamsize HttpSource<SocketType>::read(char* buf, std::streamsize size) {
	std::string data = _sock->httpReadRaw(size);

	if ( data.size() > size ) {
		SEISCOMP_ERROR("impossible thing happened");
		memcpy(buf, data.data(), size);
		return size;
	}

	memcpy(buf, data.data(), data.size());
	return data.size();
}

}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HttpSocket<SocketType>::HttpSocket(): _chunkMode(false), _remainingBytes(0),
	_decomp(NULL)
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
HttpSocket<SocketType>::~HttpSocket()
{
	if ( _decomp )
		delete _decomp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpSocket<SocketType>::open(const std::string& serverHost,
	const std::string& user, const std::string& password)
{
	_serverHost = serverHost;
	_user = user;
	_password = password;
	_chunkMode = false;
	_remainingBytes = 0;
	SocketType::open(serverHost);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpSocket<SocketType>::httpReadResponse()
{
	if ( _decomp != NULL ) {
		delete _decomp;
		_decomp = NULL;
	}

	std::string line = this->readline();
	if ( line.compare(0, 7, "HTTP/1.") != 0 )
		throw Core::GeneralException(("server sent invalid response: " + line).c_str());

	size_t pos;
	pos = line.find(' ');
	if ( pos == std::string::npos )
		throw Core::GeneralException(("server sent invalid response: " + line).c_str());

	line.erase(0, pos+1);

	pos = line.find(' ');
	if ( pos == std::string::npos )
		throw Core::GeneralException(("server sent invalid response: " + line).c_str());

	int code;
	if ( !Core::fromString(code, line.substr(0, pos)) )
		throw Core::GeneralException(("server sent invalid status code: " + line.substr(0, pos)).c_str());

	if ( code != 200 && code != 204 )
		_error = "server request error: " + line;

	_remainingBytes = -1;

	int lc = 0;

	while ( !this->isInterrupted() ) {
		++lc;
		line = this->readline();
		if ( line.empty() ) break;

		SEISCOMP_DEBUG("[%02d] %s", lc, line.c_str());
		if ( line == "Transfer-Encoding: chunked" ) {
			_chunkMode = true;
			SEISCOMP_DEBUG(" -> enabled 'chunked' transfer");
		}
		else if ( line == "Content-Encoding: gzip" ) {
			_decomp = new boost::iostreams::zlib_decompressor(boost::iostreams::zlib::default_window_bits | 16);
			SEISCOMP_DEBUG(" -> enabled 'gzip' compression");
		}
		else if ( line == "Content-Encoding: deflate" ) {
			_decomp = new boost::iostreams::zlib_decompressor(boost::iostreams::zlib::default_window_bits);
			SEISCOMP_DEBUG(" -> enabled 'deflate' compression");
		}
		else if ( line.compare(0, 15, "Content-Length:") == 0 ) {
			if ( !Core::fromString(_remainingBytes, line.substr(15)) )
				throw Core::GeneralException("invalid Content-Length response");
			if ( _remainingBytes < 0 )
				throw Core::GeneralException("Content-Length must be positive");
		}
	}

	if ( _chunkMode ) {
		if ( _remainingBytes >= 0 )
			throw Core::GeneralException("protocol error: transfer encoding is chunked and content length given");
		_remainingBytes = 0;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpSocket<SocketType>::sendAuthorization()
{
	std::string auth = _user + ':' + _password;
	BIO* b64 = BIO_new(BIO_f_base64());
	BIO* bio = BIO_new(BIO_s_mem());
	BIO_push(b64, bio);
	BIO_write(b64, auth.c_str(), auth.length());
	BIO_flush(b64);
	BUF_MEM *mem;
	BIO_get_mem_ptr(b64, &mem);
	this->sendRequest("Authorization: Basic " + std::string(mem->data, mem->length - 1), false);
	BIO_free_all(b64);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpSocket<SocketType>::httpGet(const std::string &path)
{
	this->sendRequest(std::string("GET ") + path + " HTTP/1.1", false);
	this->sendRequest(std::string("Host: ") + _serverHost, false);
	this->sendRequest("User-Agent: Mosaic/1.0", false);
	this->sendRequest("Accept-Encoding: gzip, deflate", false);

	if ( _user.length() > 0 )
		sendAuthorization();

	this->sendRequest("", false);
	httpReadResponse();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
void HttpSocket<SocketType>::httpPost(const std::string &path, const std::string &msg)
{
	this->sendRequest(std::string("POST ") + path + " HTTP/1.1", false);
	this->sendRequest(std::string("Host: ") + _serverHost, false);
	this->sendRequest("User-Agent: Mosaic/1.0", false);
	this->sendRequest("Accept-Encoding: gzip, deflate", false);
	this->sendRequest("Content-Type: application/bson", false);
	this->sendRequest(std::string("Content-Length: ") + Core::toString(msg.size()), false);

	if ( _user.length() > 0 )
		sendAuthorization();

	this->sendRequest("", false);
	this->write(msg);
	httpReadResponse();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpSocket<SocketType>::httpReadRaw(int size)
{
	if ( _chunkMode && _remainingBytes <= 0 ) {
		std::string r = this->readline();
		size_t pos = r.find(' ');
		unsigned int remainingBytes;

		if ( sscanf(r.substr(0, pos).c_str(), "%X", &remainingBytes) !=  1 )
			throw Core::GeneralException((std::string("invalid chunk header: ") + r).c_str());

		_remainingBytes = remainingBytes;

		if ( _remainingBytes <= 0 ) {
			this->close();

			if ( _error.size() )
				throw Core::GeneralException(_error.c_str());
		}
	}

	if ( _remainingBytes <= 0 )
		return "";

	int toBeRead = _remainingBytes;
	if ( toBeRead > size ) toBeRead = size;

	// seiscomp3/io/socket.h defines BUFSIZE as the max read size
	std::string data = this->read(std::min(toBeRead, BUFSIZE));
	_remainingBytes -= data.size();

	if ( _chunkMode && _remainingBytes <= 0 )
		// Read trailing new line
		this->readline();

	if ( _error.size() ) {
		this->close();
		throw Core::GeneralException(_error.c_str());
	}

	if ( !_chunkMode && _remainingBytes <= 0 )
		this->close();

	return data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpSocket<SocketType>::httpReadSome(int size)
{
	if ( _decomp != NULL ) {
		HttpSource<SocketType> src(this);
		std::vector<char> tmp(size);
		std::streamsize bytesRead = _decomp->read(src, &tmp[0], size);
		return std::string(&tmp[0], bytesRead);
	}
	else {
		return httpReadRaw(size);
	}
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename SocketType>
std::string HttpSocket<SocketType>::httpRead(int size)
{
	std::string data;

	while ( data.size() < size ) {
		std::string::size_type bytesRead = data.size();
		data += httpReadSome(size - bytesRead);

		if ( data.size() == bytesRead )
			break;
	}

	return data;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


}
}

#endif
