/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include <gempa/caps/socket.h>
#include <gempa/caps/log.h>

#include <fcntl.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#else
#ifndef SHUT_RDWR
#define SHUT_RDWR SD_BOTH
#endif
#define _WIN32_WINNT 0x0501  // Older versions does not support getaddrinfo
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif



#include <cerrno>
#include <cstring>
#include <sstream>


using namespace std;


namespace {


inline string toString(unsigned short value) {
	stringstream ss;
	ss << value;
	return ss.str();
}


pthread_mutex_t *ssl_mutex_buffer = NULL;


unsigned long SSL_thread_id_function(void) {
	return ((unsigned long)pthread_self());
}


void SSL_locking_function(int mode, int id, const char *file, int line) {
	if ( mode & CRYPTO_LOCK )
		pthread_mutex_lock(&ssl_mutex_buffer[id]);
	else
		pthread_mutex_unlock(&ssl_mutex_buffer[id]);
}


void SSL_static_init() {
	if ( ssl_mutex_buffer == NULL )
		ssl_mutex_buffer = (pthread_mutex_t*)malloc(CRYPTO_num_locks()*sizeof(pthread_mutex_t));

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_init(&ssl_mutex_buffer[i], NULL);

	CRYPTO_set_id_callback(SSL_thread_id_function);
	CRYPTO_set_locking_callback(SSL_locking_function);
}


void SSL_static_cleanup() {
	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);

	if ( ssl_mutex_buffer == NULL )
		return;

	for ( int i = 0; i < CRYPTO_num_locks(); ++i )
		pthread_mutex_destroy(&ssl_mutex_buffer[i]);

	free(ssl_mutex_buffer);
	ssl_mutex_buffer = NULL;
}


struct SSLInitializer {
	SSLInitializer() {
		SSL_library_init();
		OpenSSL_add_all_algorithms();

		SSL_static_init();
	}

	~SSLInitializer() {
		SSL_static_cleanup();
	}
};


SSLInitializer __sslInitializer;


}


namespace Gempa {
namespace CAPS {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Socket() {
	_fd = -1;
	_bytesSent = _bytesReceived = 0;
	_timeOutSecs = _timeOutUsecs = 0;

#ifdef WIN32
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,0),&wsa);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::~Socket() {
	close();
#ifdef WIN32
	WSACleanup();
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Socket::toString(Status stat) {
	switch ( stat ) {
		case Success:
			return "success";
		default:
		case Error:
			return "error";
		case AllocationError:
			return "allocation error";
		case ReuseAdressError:
			return "reusing address failed";
		case BindError:
			return "bind error";
		case ListenError:
			return "listen error";
		case AcceptError:
			return "accept error";
		case ConnectError:
			return "connect error";
		case AddrInfoError:
			return "address info error";
		case Timeout:
			return "timeout";
		case InvalidSocket:
			return "invalid socket";
		case InvalidPort:
			return "invalid port";
		case InvalidAddressFamily:
			return "invalid address family";
		case InvalidAddress:
			return "invalid address";
		case InvalidHostname:
			return "invalid hostname";
	}

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::shutdown() {
	if ( _fd == -1 ) return;
	//CAPS_DEBUG("Socket::shutdown");
	::shutdown(_fd, SHUT_RDWR);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Socket::close() {
	if ( _fd != -1 ) {
		//CAPS_DEBUG("[socket] close %lX with fd = %d", (long int)this, _fd);
		int fd = _fd;
		_fd = -1;
		::close(fd);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setSocketTimeout(int secs, int usecs) {
	_timeOutSecs = secs;
	_timeOutUsecs = usecs;

	if ( _fd != -1 )
		return applySocketTimeout(_timeOutSecs, _timeOutUsecs);

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::applySocketTimeout(int secs, int usecs) {
	if ( _fd != -1 ) {
		struct timeval timeout;
		void *opt;
		int optlen;

		if ( secs >= 0 ) {
			timeout.tv_sec = secs;
			timeout.tv_usec = usecs;
			opt = &timeout;
			optlen = sizeof(timeout);
		}
		else {
			opt = NULL;
			optlen = 0;
		}

		CAPS_DEBUG("set socket timeout to %d.%ds", secs, usecs);

		if ( setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, opt, optlen) )
			return Error;

		if ( setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, opt, optlen) )
			return Error;
	}
	else
		return InvalidSocket;

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::setNonBlocking(bool nb) {
	if ( !isValid() )
		return InvalidDevice;

#ifndef WIN32
	int flags = fcntl(_fd, F_GETFL, 0);

	if ( nb )
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if ( fcntl(_fd, F_SETFL, flags) == -1 )
		return Error;
#else
	u_long arg = nb?1:0;
	if ( ioctlsocket(_fd, FIONBIO, &arg) != 0 )
		return Device::Error;
#endif

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status Socket::connect(const std::string &hostname, uint16_t port) {
	if ( _fd != -1 ) {
		//CAPS_WARNING("closing stale socket");
		close();
	}

	struct sockaddr addr;
	size_t addrlen;

	struct addrinfo *res;
	struct addrinfo hints;

	memset (&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	string strPort = ::toString(port);

	int ret = getaddrinfo(hostname.c_str(), strPort.c_str(), &hints, &res);
	if ( ret ) {
		CAPS_DEBUG("Test3 Socket::connect(%s:%d): %s",
			       hostname.c_str(), port,
#ifndef WIN32
		           strerror(errno));
#else
		           gai_strerror(ret));
#endif
		return AddrInfoError;
	}

	addr = *(res->ai_addr);
	addrlen = res->ai_addrlen;
	freeaddrinfo(res);

	if ( (_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		/*CAPS_DEBUG("Socket::connect(%s:%d): %s",
		               hostname.c_str(), port, strerror(errno));*/
		return AllocationError;
	}

#ifndef WIN32
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == -1 ) {
		if ( errno != EINPROGRESS ) {
			/*CAPS_DEBUG("Socket::connect(%s:%d): %s",
			               hostname.c_str(), port, strerror(errno));*/
			close();
			return errno == ETIMEDOUT?Timeout:ConnectError;
		}
	}
#else
	if ( ::connect(_fd, (struct sockaddr *)&addr, addrlen) == SOCKET_ERROR ) {
		int err = WSAGetLastError();
		if (err != WSAEINPROGRESS && err != WSAEWOULDBLOCK) {
			CAPS_DEBUG("Socket::connect(%s:%d): %s",
			            hostname.c_str(), port, gai_strerror(err));
			close();
			return ConnectError;
		}
	}
#endif

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::send(const char *data) {
	return write(data, strlen(data));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::write(const char *data, int len) {
#if !defined(MACOSX) && !defined(WIN32)
	int sent = (int)::send(_fd, data, len, MSG_NOSIGNAL);
#else
	int sent = (int)::send(_fd, data, len, 0);
#endif
	if ( sent > 0 ) {
		_bytesSent += sent;
	}
	return sent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::read(char *data, int len) {
	int recvd = (int)::recv(_fd, data, len, 0);
	if ( recvd > 0 ) _bytesReceived += recvd;
	return recvd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Socket::flush() { return 1; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Socket::isValid() {
	return _fd != -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




#if !defined(CAPS_FEATURES_SSL) || CAPS_FEATURES_SSL
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket() : _ssl(NULL), _ctx(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::SSLSocket(SSL_CTX *ctx) : _ssl(NULL), _ctx(ctx) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SSLSocket::~SSLSocket() {
	close();
	cleanUp();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SSLSocket::write(const char *data, int len) {
	int ret = SSL_write(_ssl, data, len);
	if ( ret > 0 ) {
		_bytesSent += ret;
		return ret;
	}

	int err = SSL_get_error(_ssl, ret);

	switch ( err ) {
		case SSL_ERROR_WANT_X509_LOOKUP:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_READ:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_WRITE:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_ZERO_RETURN:
			errno = EINVAL;
			return 0;
		default:
			break;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SSLSocket::read(char *data, int len) {
	int ret = SSL_read(_ssl, data, len);
	if ( ret > 0 ) {
		_bytesReceived += ret;
		return ret;
	}

	int err = SSL_get_error(_ssl, ret);

	switch ( err ) {
		case SSL_ERROR_WANT_X509_LOOKUP:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_READ:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_WANT_WRITE:
			errno = EAGAIN;
			return -1;
		case SSL_ERROR_ZERO_RETURN:
			errno = EINVAL;
			return 0;
		default:
			break;
	}

	return ret;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Socket::Status SSLSocket::connect(const std::string &hostname, uint16_t port) {
	cleanUp();

	_ctx = SSL_CTX_new(SSLv23_client_method());
	if ( _ctx == NULL ) {
		CAPS_DEBUG("Invalid SSL context");
		return ConnectError;
	}

	SSL_CTX_set_mode(_ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

	Status s = Socket::connect(hostname, port);
	if ( s != Success )
		return s;

	_ssl = SSL_new(_ctx);
	if ( _ssl == NULL ) {
		CAPS_DEBUG("Failed to create SSL context");
		return ConnectError;
	}

	SSL_set_fd(_ssl, _fd);
	SSL_set_shutdown(_ssl, 0);
	SSL_set_connect_state(_ssl);
	int err = SSL_connect(_ssl);
	if ( err < 0 ) {
		CAPS_ERROR("Failed to connect with SSL, error %d",
		           SSL_get_error(_ssl, err));
		close();
		return ConnectError;
	}

	return Success;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const unsigned char *SSLSocket::sessionID() const {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return _ssl?_ssl->session->session_id:NULL;
#else
	return _ssl?SSL_SESSION_get0_id_context(SSL_get0_session(_ssl), NULL):NULL;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned int SSLSocket::sessionIDLength() const {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	return _ssl?_ssl->session->session_id_length:0;
#else
	unsigned int len;
	if ( !_ssl ) return 0;
	SSL_SESSION_get0_id_context(SSL_get0_session(_ssl), &len);
	return len;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
X509 *SSLSocket::peerCertificate() {
	if ( _ssl == NULL ) return NULL;
	return SSL_get_peer_certificate(_ssl);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SSLSocket::cleanUp() {
	if ( _ssl ) {
		SSL_free(_ssl);
		_ssl = NULL;
	}

	if ( _ctx ) {
		SSL_CTX_free(_ctx);
		_ctx = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#endif



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
