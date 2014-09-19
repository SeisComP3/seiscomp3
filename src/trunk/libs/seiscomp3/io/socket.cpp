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


#define SEISCOMP_COMPONENT Socket

#include <string.h>
#include <set>
#include <utility>
#include <limits>
#include <cerrno>
#include <fcntl.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include "socket.h"

#ifdef WIN32
#undef min
#undef max
#define posix_read _read
typedef int ssize_t;
#else
#define posix_read read
#endif

namespace Seiscomp {
namespace IO {

using namespace std;
using namespace Seiscomp::Core;


Socket::Socket()
    : _sockfd(-1), _rp(0), _wp(0), _timeout(0),
      _interrupt(false), _reconnect(false), _eol("\r\n") {
#ifdef WIN32
	//if (_pipe(_pipefd, 2, O_BINARY) < 0) {
#else
	if (pipe(_pipefd) < 0) {
		SEISCOMP_ERROR("pipe: %s", strerror(errno));
		throw SocketException("error creating signal pipe");
	}
#endif

#ifdef WIN32
	/*
	u_long arg = 1;
	ioctlsocket(_pipefd[READ], FIONBIO, &arg);
	ioctlsocket(_pipefd[WRITE], FIONBIO, &arg);
	*/
#else
	fcntl(_pipefd[READ], F_SETFL, O_NONBLOCK);
	fcntl(_pipefd[WRITE], F_SETFL, O_NONBLOCK);
#endif
}

Socket::~Socket() {
	Socket::close();
#ifndef WIN32
	::close(_pipefd[READ]);
	::close(_pipefd[WRITE]);
#endif
}

void Socket::setTimeout(int seconds) {
	if ( seconds <= 0 )
		_timeout = 0;
	else
		_timeout = seconds;
	SEISCOMP_DEBUG("Set timeout=%d", seconds);
}

void Socket::startTimer() {
	_timer.restart();
}

void Socket::open(const string& serverLocation) {
#if WIN32
	WSADATA wsaData;
	int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if ( wsaerr != 0 ) {
		SEISCOMP_ERROR("WSAStartup failed with error: %d", wsaerr);
		throw SocketException("WSAStartup failed");
	}
#endif

	if ( _sockfd != -1 ) {
		SEISCOMP_WARNING("closing stale socket");
		Socket::close();
	}

	string::size_type sep = serverLocation.find(':');
	if ( sep == string::npos ) {
		SEISCOMP_ERROR("Invalid Server address: %s", serverLocation.c_str());
		throw SocketException("invalid server address");
	}

	string hostname(serverLocation, 0, sep);
	char* chostname = strdup(hostname.c_str());
	if ( !chostname )
		throw bad_alloc();

	string portstr(serverLocation, sep + 1, string::npos);
	char* cportstr = strdup(portstr.c_str());
	if ( !cportstr )
		throw bad_alloc();

	struct sockaddr addr;
	size_t addrlen;
	int err = addrSocket(chostname, cportstr, &addr, &addrlen);
	if ( err != 0 ) {
		free(chostname);
		free(cportstr);
		/*
		if (err < 0)
			SEISCOMP_ERROR("Cannot resolve %s", hostname.c_str());
		else
			SEISCOMP_ERROR("Cannot resolve %s -> %s", hostname.c_str(),strerror(err));
		*/
		throw SocketResolveError(string("Cannot resolve ") + hostname);
	}

	free(chostname);
	free(cportstr);

	if ( (_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		SEISCOMP_ERROR("socket: %s",strerror(errno));
		throw SocketException("Socket error");
	}

	if ( nonblockSocket() < 0 ) {
		SEISCOMP_ERROR("Error setting socket to non-blocking (%s)",strerror(errno));
		throw SocketException("Error setting socket to non-blocking");
	}

	if ( connectSocket((struct sockaddr *) &addr, addrlen) < 0 ) {
		_reconnect = true;
		throw SocketException("Socket connect error");
	}

	int sockstat = checkSocket(10,0);
	if ( sockstat < 0 ) {
		_reconnect = true;
		throw SocketException("Socket connect error");
	}
	else if ( sockstat == 0 ) {
		_reconnect = true;
		throw SocketException("Socket connect timeout");
	}

	SEISCOMP_DEBUG("%s connected", serverLocation.c_str());
}

void Socket::close() {
	if ( _sockfd != -1 ) {
#ifndef WIN32
		::close(_sockfd);
#else
		closesocket(_sockfd);
		WSACleanup();
#endif
		_sockfd = -1;
	}

	_interrupt = false;
	_reconnect = false;
}

bool Socket::isOpen() {
	return _sockfd != -1;
}

bool Socket::tryReconnect() {
	return _reconnect;
}

void Socket::fillbuf() {
	if ( _rp > 0 ) {
		if ( _wp > _rp ) {
			memmove(_buf, _buf + _rp, _wp - _rp);
			_wp -= _rp;
			_rp = 0;
		}
		else {
			_wp = 0;
			_rp = 0;
		}
	}

	while ( 1 ) {
		if ( _interrupt )
			throw OperationInterrupted();

		int remaining = _timeout?_timeout - int(_timer.elapsed()):1;
		if ( remaining <= 0 ) {
			SEISCOMP_DEBUG("Timeout");
			_reconnect = true;
			throw SocketTimeout();
		}

		struct timeval tv;
		tv.tv_sec = remaining;
		tv.tv_usec = 0;

#ifndef WIN32
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(_sockfd, &read_fds);
		FD_SET(_pipefd[READ], &read_fds);
		//SEISCOMP_DEBUG("Going into select with timeout of %d seconds",
		//               _timeout?(int)tv.tv_sec:0);
		int r = select(max(_pipefd[READ], _sockfd) + 1, &read_fds, NULL, NULL, _timeout?&tv:NULL);
#else
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(_sockfd, &read_fds);

		int r = select(_sockfd + 1, &read_fds, NULL, NULL, _timeout?&tv:NULL);
#endif

		if ( r < 0 ) {
#ifndef WIN32
			if ( errno == EINTR )
#else
			int wsaerr = WSAGetLastError();
			if ( wsaerr != WSAEINTR )
#endif
				continue;

			SEISCOMP_ERROR("socket select: %s", strerror(errno));
			throw SocketException("socket select error");
		}
		else if ( r == 0 ) {
			SEISCOMP_DEBUG("Timeout");
			_reconnect = true;
			throw SocketTimeout();
		}
		else
			break;
	}

	if ( _interrupt )
		throw OperationInterrupted();

	int byteCount = readImpl(_buf + _wp, BUFSIZE - _wp);
	if ( byteCount < 0 ) {
		_reconnect = true;
		SEISCOMP_ERROR("socket read: %s",strerror(errno));
		throw SocketException("socket read error");
	}

	if ( byteCount == 0 ) {
		_reconnect = true;
		throw SocketException("unexpected EOF while reading from socket");
	}

	_wp += byteCount;
}

void Socket::write(const string& s) {
	ssize_t nleft = s.length();
	const char *ptr = (const char *) s.c_str();

	while ( nleft > 0 ) {
		if ( _interrupt )
			throw OperationInterrupted();

		int remaining = _timeout?_timeout - int(_timer.elapsed()):1;
		if ( remaining <= 0 ) {
			SEISCOMP_DEBUG("Timeout");
			throw SocketTimeout();
		}

		struct timeval tv;
		tv.tv_sec = remaining;
		tv.tv_usec = 0;

#ifndef WIN32
		fd_set read_fds, write_fds;
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_SET(_pipefd[READ], &read_fds);
		FD_SET(_sockfd, &write_fds);
//		SEISCOMP_DEBUG("Going into select with timeout of %d seconds",
//		               _timeout?(int)tv.tv_sec:0);
		int r = select(max(_pipefd[READ], _sockfd) + 1, &read_fds, &write_fds, NULL, _timeout?&tv:NULL);
#else
		fd_set write_fds;
		FD_ZERO(&write_fds);
		FD_SET(_sockfd, &write_fds);

		int r = select(_sockfd + 1, NULL, &write_fds, NULL, _timeout?&tv:NULL);
#endif
		if ( r < 0 ) {
#ifndef WIN32
			if ( errno == EINTR )
#else
			int wsaerr = WSAGetLastError();
			if ( wsaerr != WSAEINTR )
#endif
				continue;

			SEISCOMP_ERROR("select: %s", strerror(errno));
			throw SocketException("socket select error");
		}
		else if ( r == 0 ) {
			SEISCOMP_DEBUG("Timeout");
			throw SocketTimeout();
		}

		if ( _interrupt )
			throw OperationInterrupted();

		int byteCount = writeImpl(ptr, nleft);
		if ( byteCount < 0 ) {
#ifndef WIN32
			if ( errno == EINTR )
				continue;
			if ( errno == EPIPE )
				_reconnect = true;

			SEISCOMP_ERROR("write: %s", strerror(errno));
#else
			int wsaerr = WSAGetLastError();
			if ( wsaerr == WSAEINTR )
				continue;

			_reconnect = true;

			SEISCOMP_ERROR("write: %d", wsaerr);
#endif

			throw SocketException("socket write error");
		}

		nleft -= byteCount;
		ptr += byteCount;
	}
}

string Socket::read(int size) {
	if ( size > BUFSIZE ) {
		SEISCOMP_ERROR("Socket read: size > BUFSIZE");
		size = BUFSIZE;
	}

	while ( 1 ) {
		if ( _wp - _rp >= size ) {
			string s(_buf + _rp, size);
			_rp += size;
			return s;
		}

		fillbuf();
	}
}

string Socket::readline() {
	while ( 1 ) {
		if ( _wp > _rp ) {
			_buf[_wp] = 0;
			char *tail = _buf + _rp;
			int a = strcspn(tail, _eol.c_str());
			tail += a;
			int b = strspn(tail, _eol.c_str());
			for ( int i = 0; i < b; ++i ) {
				if ( *tail == '\n' ) {
					string s(_buf + _rp, a);
					_rp += (a + i + 1);
					return s;
				}
				++tail;
			}
		}

		if ( _wp - _rp < BUFSIZE ) {
			fillbuf();
			continue;
		}
		else {
			SEISCOMP_ERROR("Socket readline: line is too long");
			return string(_buf, 0, BUFSIZE);
		}
	}
}

string Socket::sendRequest(const string& request, bool waitResponse) {
	write(request + _eol);

	if ( !waitResponse )
	return string();

	string resp = readline();
	if ( resp == "ERROR" ) {
		SEISCOMP_ERROR("Command failed: %s",request.c_str());
		throw SocketCommandException(request);
	} else
		return resp;
}

bool Socket::isInterrupted() {
	return _interrupt;
}

void Socket::interrupt() {
	_interrupt = true;

#ifndef WIN32
	ssize_t r;

	do {
		char c;
		r = ::posix_read(_pipefd[READ], &c, 1);
	}
	while ( r > 0 || (r < 0 && errno == EINTR) );

	if ( r < 0 && errno != EAGAIN )
		SEISCOMP_ERROR("error reading signal pipe: %s", strerror(errno));

	else if ( r == 0 )
		SEISCOMP_ERROR("unexpected EOF while reading signal pipe");

	do {
		r = ::write(_pipefd[WRITE], "", 1);
	}
	while( r < 0 && errno == EINTR );

	if ( r < 0 )
		SEISCOMP_ERROR("error writing signal pipe: %s", strerror(errno));
#else
	Socket::close();
#endif
}

void Socket::handleInterrupt(int) throw() {
	interrupt();
}

int Socket::readImpl(char *buf, int count) {
#ifndef WIN32
	return ::posix_read(_sockfd, buf, count);
#else
	return recv(_sockfd, buf, count, 0);
#endif
}

int Socket::writeImpl(const char *buf, int count) {
#ifndef WIN32
		return ::write(_sockfd, buf, count);
#else
		return send(_sockfd, buf, count, MSG_OOB);
#endif
}

int Socket::connectSocket(struct sockaddr *addr, int len) {
#ifndef WIN32
	if ( connect(_sockfd,addr,len) == -1 )
		if ( errno != EINPROGRESS )
			return -1;
#else
	if ( connect(_sockfd,addr,len) == SOCKET_ERROR ) {
		int err = WSAGetLastError();
		if ( err != WSAEINPROGRESS && err != WSAEWOULDBLOCK )
			return -1;
	}
#endif
	return 0;
}

int Socket::nonblockSocket() {
#ifndef WIN32
	int flags = fcntl(_sockfd, F_GETFL, 0);

	flags |= O_NONBLOCK;
	if ( fcntl(_sockfd, F_SETFL, flags) == -1 )
		return -1;
#else
	u_long arg = 1;
	if ( ioctlsocket(_sockfd, FIONBIO, &arg) != 0 )
		return -1;
#endif

	return 0;
}

int Socket::addrSocket(char *hostname, char *port, struct sockaddr *addr, size_t *len) {
	struct addrinfo *res;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ( getaddrinfo(hostname,port,&hints,&res) )
		return -1;

	*addr = *(res->ai_addr);
	*len = res->ai_addrlen;
	freeaddrinfo(res);

	return 0;
}

int Socket::checkSocket(int secs, int usecs) {
	int ret;
	char testbuf[1];
	fd_set fds;
	struct timeval to;

	FD_ZERO (&fds);
	FD_SET ((unsigned int)_sockfd, &fds);

	to.tv_sec = secs;
	to.tv_usec = usecs;

	/* Check write ability with select() */
#ifndef WIN32
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(_pipefd[READ], &read_fds);
	ret = select(max(_pipefd[READ], _sockfd) + 1, &read_fds, &fds, NULL, &to);
#else
	ret = select(_sockfd + 1, NULL, &fds, NULL, &to);
#endif

	// Socket interrupted?
	if ( _interrupt )
		throw OperationInterrupted();

	if ( ret >= 0 )
		ret = ret > 0 ? 1 : 0;
	else
		ret = -1;

	/* Check read ability with recv() and
	   MSG_PEEK -> leaving the character to read in the socket buffer */
	if ( ret && (recv (_sockfd, testbuf, sizeof (char), MSG_PEEK)) <= 0 )
#ifndef WIN32
		ret = (errno == EWOULDBLOCK)?1:-1;
#else
		ret = (WSAGetLastError() == WSAEWOULDBLOCK)?1:-1;
#endif

	return ret;
}






SSLSocket::SSLSocket() : Socket(), _bio(NULL), _ssl(NULL), _ctx(NULL) {
	SSL_library_init();
	OpenSSL_add_all_algorithms();
}

SSLSocket::~SSLSocket() {
	close();
	cleanUp();
}

void SSLSocket::open(const std::string &serverLocation) {
#if WIN32
	WSADATA wsaData;
	int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if ( wsaerr != 0 ) {
		SEISCOMP_ERROR("WSAStartup failed with error: %d", wsaerr);
		throw SocketException("WSAStartup failed");
	}
#endif

	if ( _sockfd != -1 ) {
		SEISCOMP_WARNING("closing stale socket");
		close();
	}
	cleanUp();

	vector<string> toks;
	if ( Core::split(toks, serverLocation.c_str(), ":") != 2 )
		throw SocketException("invalid server address");

	string &host = toks[0];
	int port;
	if ( !Seiscomp::Core::fromString(port, toks[1]) )
		throw SocketException("invalid port number");

	_ctx = SSL_CTX_new(SSLv23_client_method());
	if ( _ctx == NULL )
		throw SocketException("invalid SSL context");

	_bio = BIO_new_ssl_connect(_ctx);
	if ( _bio == NULL )
		throw SocketException("invalid bio");

	BIO_get_ssl(_bio, &_ssl);

	char buf[6];
	snprintf(buf, 6, "%d", port);
	BIO_set_conn_hostname(_bio, host.c_str());
	BIO_set_conn_int_port(_bio, (char*)&port);

	if ( BIO_do_connect(_bio) <= 0 )
		throw SocketException("error establishing secure socket connection");

	if ( BIO_do_handshake(_bio) <= 0 )
		throw SocketException("error performing SSL handshake");

	BIO_get_fd(_bio, &_sockfd);
}

void SSLSocket::close() {
	Socket::close();
	if ( _bio )
		(void)BIO_reset(_bio);

	_ssl = NULL;
}

int SSLSocket::readImpl(char *buf, int count) {
	return BIO_read(_bio, buf, count);
}

int SSLSocket::writeImpl(const char *buf, int count) {
	return BIO_write(_bio, buf, count);
}


void SSLSocket::cleanUp() {
	if ( _bio ) {
		BIO_free_all(_bio);
		_bio = NULL;
	}
	_ssl = NULL;

	if ( _ctx ) {
		SSL_CTX_free(_ctx);
		_ctx = NULL;
	}
}

}
}
