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

#ifndef __SEISCOMP_ADMIN_H__
#define __SEISCOMP_ADMIN_H__

#include <seiscomp3/communication/systemconnection.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/communication/clientstatus.h>


namespace Seiscomp {

class Admin
{
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		Admin() {}
		~Admin();


	// ----------------------------------------------------------------------
	// Public Interface
	// ----------------------------------------------------------------------
	public:
		bool connect(const std::string& serverAddress,
					const std::string& clientName = Communication::Protocol::DEFAULT_ADMIN_CLIENT_NAME,
		            const std::string& password = "");

		bool disconnect();

		bool showGroupMembers(const std::string& group, std::vector<std::string>& members);
		bool showAvailableGroups(std::vector<std::string>& groups);
		bool showConnectedClients(std::vector<std::string>& clients);
		bool disconnectClient(const std::string& clientName);
		bool requestHealthStatus(const std::string& groupOrClient);
		Communication::ClientStatus* showHealthStatus();

		// ----------------------------------------------------------------------
		// Private Interface
		// ----------------------------------------------------------------------
	private:
		int sendCommand(Communication::Protocol::SERVICE_MSG_TYPES type,
						const std::string& data = "");
		Communication::NetworkMessage* readResponse();
		std::string extractClientName(const std::string& str);

		// ----------------------------------------------------------------------
		// Private Members
		// ----------------------------------------------------------------------
	private:
		Communication::SystemConnectionPtr _client;

};

} // namespace Seiscomp

#endif
