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


#ifndef __GEMPA_CAPS_SOCKET_H__
#define __GEMPA_CAPS_SOCKET_H__


#include <gempa/caps/packet.h>

#include <boost/shared_ptr.hpp>
#include <openssl/ssl.h>

#include <stdint.h>
#include <streambuf>
#include <iostream>
#include <fstream>


namespace Gempa {
namespace CAPS {


class Socket {
	public:
		typedef uint64_t count_t;

		enum Status {
			Success = 0,
			Error,
			AllocationError,
			InvalidDevice,
			ReuseAdressError,
			BindError,
			ListenError,
			AcceptError,
			ConnectError,
			AddrInfoError,
			Timeout,
			InvalidSocket,
			InvalidPort,
			InvalidAddressFamily,
			InvalidAddress,
			InvalidHostname
		};


	public:
		Socket();
		virtual ~Socket();


	public:
		static const char *toString(Status);

		int fd() { return _fd; }

		bool isValid();

		void shutdown();
		void close();


		int send(const char *data);

		virtual int write(const char *data, int len);
		virtual int read(char *data, int len);
		virtual int flush();

		//! Sets the socket timeout. This utilizes setsockopt which does not
		//! work in non blocking sockets.
		Status setSocketTimeout(int secs, int usecs);

		Status setNonBlocking(bool nb);

		virtual Status connect(const std::string &hostname, uint16_t port);

		count_t rx() const { return _bytesReceived; }
		count_t tx() const { return _bytesSent; }

	protected:
		Status applySocketTimeout(int secs, int usecs);


	protected:
		int         _fd;

		count_t     _bytesSent;
		count_t     _bytesReceived;

		int         _timeOutSecs;
		int         _timeOutUsecs;
};

typedef boost::shared_ptr<Socket> SocketPtr;

#if !defined(CAPS_FEATURES_SSL) || CAPS_FEATURES_SSL

class SSLSocket : public Socket {
	public:
		SSLSocket();
		SSLSocket(SSL_CTX *ctx);
		~SSLSocket();

	public:
		int write(const char *data, int len);
		int read(char *data, int len);

		Status connect(const std::string &hostname, uint16_t port);

		virtual const unsigned char *sessionID() const;
		virtual unsigned int sessionIDLength() const;

		X509 *peerCertificate();

	private:
		void cleanUp();

	private:
		SSL     *_ssl;
		SSL_CTX *_ctx;
};

typedef boost::shared_ptr<SSLSocket> SSLSocketPtr;

#endif

template <typename T, int N>
class socketbuf : public std::streambuf {
	public:
		socketbuf() {
			setsocket(NULL);
		}

		socketbuf(T *sock) {
			setsocket(sock);
		}

		void setsocket(T *sock) {
			_allowed_reads = -1;
			_real_buffer_size = 0;
			_block_write = false;
			setg(_in, _in, _in);
			setp(_out, _out + N);
			_sock = sock;
		}

		void settimeout(const struct timeval &tv) {
			_timeout = tv;
		}

		void set_read_limit(int bytes) {
			_allowed_reads = bytes;

			if ( _allowed_reads >= 0 ) {
				if ( egptr() - gptr() > _allowed_reads )
					setg(eback(), gptr(), gptr() + _allowed_reads);

				// Set the number of read bytes to the
				// remaining bytes in the buffer
				_allowed_reads -= egptr() - gptr();
			}
			else
				setg(eback(), gptr(), eback() + _real_buffer_size);

			//std::cout << "[" << (void*)eback() << ", " << (void*)gptr() << ", " << (void*)egptr() << "]" << " = " << (egptr() - gptr()) << std::endl;
		}

		int read_limit() const {
			if ( _allowed_reads < 0 ) return -1;
			return egptr() - gptr() + _allowed_reads;
		}


	protected:
		virtual int underflow() {
			// No more reads allowed?
			if ( !_allowed_reads )
				return traits_type::eof();

			// Read available data from socket
			int res = _sock->read(_in, N);
			if ( res <= 0 ) {
				set_read_limit(0);
				return traits_type::eof();
			}

			// Set input sequence pointers
			_real_buffer_size = res;
			setg(_in, _in, _in + _real_buffer_size);

			// clip to limit
			set_read_limit(_allowed_reads);

			return traits_type::to_int_type(*gptr());
		}

		virtual int overflow(int c) {
			if ( _block_write ) return traits_type::eof();

			if ( pptr() - pbase() == N ) {
				if ( sync() != 0 ) return traits_type::eof();
			}

			if ( !traits_type::eq_int_type(traits_type::eof(), c)) {
				   traits_type::assign(*pptr(), traits_type::to_char_type(c));

				   pbump(1);
			}

			return traits_type::not_eof(c);
		}

		virtual int sync() {
			if ( pbase() == pptr() ) return 0;

			int res = _sock->write(pbase(), pptr() - pbase());
			if ( res == pptr() - pbase() ) {
				setp(_out, _out + N);
				return 0;
			}

			return 1;
		}

		// Only forward seeking is supported
		virtual std::streampos
		seekoff(std::streamoff off, std::ios_base::seekdir way,
		        std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
			if ( way != std::ios_base::cur || which != std::ios_base::in || off < 0 )
				return -1;

			while ( off > 0 ) {
				int ch = sbumpc();
				if ( ch == traits_type::eof() )
					return -1;
				--off;
			}

			return 0;
		}

	private:
		T       *_sock;
		timeval  _timeout;
		char     _in[N];
		char     _out[N];
		bool     _block_write;
		int      _real_buffer_size;
		int      _allowed_reads;
};


}
}


#endif
