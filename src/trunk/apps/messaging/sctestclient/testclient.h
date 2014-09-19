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

#ifndef __SEISCOMP_APPLICATIONS_TESTCLIENT_H__
#define __SEISCOMP_APPLICATIONS_TESTCLIENT_H__

#include <seiscomp3/client/application.h>

namespace Seiscomp {
namespace Applications {
	
class TestClient : public Client::Application {

	public:
		TestClient(int argc, char** argv);
		~TestClient() {}
		
	public:
		virtual bool init();
		virtual void handleMessage(Core::Message* msg);
		virtual void handleNetworkMessage(const Communication::NetworkMessage* msg);
		
};

} // namespace Application
} // namespace Seiscomp

#endif
