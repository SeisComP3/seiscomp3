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

#ifndef __SEISCOMP_COMMUNICATION_SPREADDRIVER_H__
#define __SEISCOMP_COMMUNICATION_SPREADDRIVER_H__

#include "sp.h"

#include <seiscomp3/communication/networkinterface.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/client.h>

namespace Seiscomp {
namespace Communication {


class SC_SYSTEM_CLIENT_API SpreadDriver : public NetworkInterface {
	
	public:
		SpreadDriver();
		virtual ~SpreadDriver();
		
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
		
		
	// ----------------------------------------------------------------------
	// Getter and Setter
	// ----------------------------------------------------------------------
	public:
		virtual std::string privateGroup() const;
		
		virtual std::string groupOfLastSender() const;
		
	
	// ----------------------------------------------------------------------
	// Private auxillary methods
	// ----------------------------------------------------------------------
	private:
		void messageInfo(const NetworkMessage* msg);
		
		void init();
		
		// Handles errors which eventually occure during spread calls. Ensures that the
		// driver will be in a valid state.
		int handleError(int error);
		
		
	// ----------------------------------------------------------------------
	// Network API specific members
	// ----------------------------------------------------------------------
	private:
		//! Filedescriptor for server communication
		mailbox     _spMBox;
		service     _spServiceType;
		char        _spSender[MAX_GROUP_NAME];
			
		int  _spMaxGroups;
		int  _spNumGroups;
		char _spTargetGroups[Protocol::MAX_GROUPS][MAX_GROUP_NAME];

		short _spMessageType;
		int   _spEndianMismatch;
	
		//! stores the actual messages received from the messaging system
		char _spMessageReadBuffer[Protocol::STD_MSG_LEN];
		char _spMessageWriteBuffer[Protocol::STD_MSG_LEN];

		//! Groupname which can be used for private communication
		char _spPrivateGroup[MAX_GROUP_NAME];
		
		
	// ----------------------------------------------------------------------
	// Implementation specific members
	// ----------------------------------------------------------------------
	private:
		bool _isConnected;
};

} // namespace Communication
} // namespace Seiscomp

#endif

