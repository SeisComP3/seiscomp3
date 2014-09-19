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

#define SEISCOMP_COMPONENT Application
#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/protocol.h>

#include "testclient.h"

namespace Seiscomp {
namespace Applications {

/** \class TestClient is a simple client for networklibrary testing only.
 * The clients peergroup is Protocol::LISTENER_GROUP and it subscribes
 * to all available groups
 */
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TestClient::TestClient(int argc, char** argv) :
		Client::Application(argc, argv)
{
	setDatabaseEnabled(false, false);
	setMessagingUsername("sctest");
	addLoggingComponentSubscription("Application");
	setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
	addMessagingSubscription("*");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool TestClient::init()
{
	return Application::init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TestClient::handleMessage(Core::Message* msg)
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void TestClient::handleNetworkMessage(const Communication::NetworkMessage* msg)
{
	if (msg->type() >= 0)
	{
		SEISCOMP_INFO("Received new Message (type: %s)",
		              Communication::Protocol::MsgTypeToString(msg->type()));
	}
	else
	{
		SEISCOMP_INFO("Received new Systemmessage (type: %s)",
		              Communication::Protocol::MsgTypeToString(msg->type()));
	}

	SEISCOMP_INFO("Sender: %s Recipient: %s", msg->privateSenderGroup().c_str(), msg->destination().c_str());
	SEISCOMP_INFO("--");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		
} // namespace Applications
} // namespace Seiscomp
