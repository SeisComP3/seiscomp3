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


#ifndef __SEISCOMP_IO_HTTPSOCKET_H__
#define __SEISCOMP_IO_HTTPSOCKET_H__


#include <boost/iostreams/filter/zlib.hpp>


namespace Seiscomp {
namespace IO {


template <typename SocketType>
class SC_SYSTEM_CORE_API HttpSocket : public SocketType {
	public:
		HttpSocket();
		virtual ~HttpSocket();
		virtual void open(const std::string& serverHost,
			const std::string& user = "", const std::string& password = "");
		void httpGet(const std::string &path);
		void httpPost(const std::string &path, const std::string &msg);
		std::string httpReadRaw(int size);
		std::string httpReadSome(int size);
		std::string httpRead(int size);

	private:
		std::string _serverHost;
		std::string _user;
		std::string _password;
		std::string _error;
		bool _chunkMode;
		int _remainingBytes;
		boost::iostreams::zlib_decompressor *_decomp;

		void httpReadResponse();
		void sendAuthorization();
};


}
}


#endif
