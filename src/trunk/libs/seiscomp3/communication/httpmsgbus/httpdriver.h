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

#ifndef __SEISCOMP_COMMUNICATION_HTTPDRIVER_H__
#define __SEISCOMP_COMMUNICATION_HTTPDRIVER_H__

#include <seiscomp3/communication/networkinterface.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/io/httpsocket.h>

extern "C" {
	#include "bson/bson.h"
}

namespace Seiscomp {
namespace Communication {


template<typename SocketType>
class SC_SYSTEM_CLIENT_API HttpDriver : public NetworkInterface {

	public:
		HttpDriver();
		virtual ~HttpDriver();

		virtual int connect(const std::string& serverAddress,
		                    const std::string& clientName);
		virtual int disconnect();

		virtual NetworkMessage* receive(int* error = NULL);
		virtual int send(const std::string& group, int type, NetworkMessage* msg,
		                 bool selfDiscard = true);

		virtual int subscribe(const std::string& group);
		virtual int unsubscribe(const std::string& group);

		virtual bool poll(int* error = NULL);

		virtual bool isConnected();

	public:
		virtual std::string privateGroup() const;

		virtual std::string groupOfLastSender() const;

		virtual void setSequenceNumber(int64_t seq);

		virtual int64_t getSequenceNumber() const;

	private:
		IO::HttpSocket<SocketType> _sock;
		std::string _serverHost;
		std::string _serverPath;
		std::string _user;
		std::string _password;
		std::string _sid;
		std::string _cid;
		std::string _lastSender;
		std::set<std::string> _msgGroups;
		std::list<NetworkMessage*> _fakemsgs;
		int64_t _seq;
		bool _isConnected;

		std::string bsonGetString(const bson_t *bson, const char *key);
		int64_t bsonGetInt(const bson_t *bson, const char *key);
		void bsonGetBlob(const bson_t *bson, const char *key, const void **data, int *data_len);
		void initSession();
};

} // namespace Communication
} // namespace Seiscomp

#endif

