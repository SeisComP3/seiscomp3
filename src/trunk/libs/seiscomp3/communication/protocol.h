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

#ifndef __SEISCOMP_COMMUNICATION_PROTOCOL_H__
#define __SEISCOMP_COMMUNICATION_PROTOCOL_H__

#include <seiscomp3/client.h>

#include <string>
#include <map>
#include <cmath>
#include <stdlib.h>

namespace Seiscomp {
namespace Communication {

class SC_SYSTEM_CLIENT_API Protocol {
	// ------------------------------------------------------------------------
	// Nested types
	// ------------------------------------------------------------------------
	public:
		//! Enumeration of client priorities
		enum ClientPriority {
			PRIORITY_DEFAULT,
			PRIORITY_HIGH,
			PRIORITY_MEDIUM,
			PRIORITY_LOW,
			CP_QUANTITY
		};

		//! Definition of the client types
		enum ClientType {
			TYPE_DEFAULT,
			TYPE_ONE,
			TYPE_TWO,
			TYPE_THREE,
			CT_QUANTITY
		};

		/**
		 * MESSAGE TYPES: Regular messages are positive. Service messsages are
		 * negative.
		 */
		enum SERVICE_MSG_TYPES {
			UNDEFINED_SERVICE_MSG               =  0,
			CONNECT_GROUP_MSG                   = -1,
			CONNECT_GROUP_REJECT_MSG            = -2,
			ADMIN_REJECT_MSG                    = -3,
			CONNECT_GROUP_OK_MSG                = -4,
			JOIN_GROUP_MSG                      = -5,
			LEAVE_GROUP_MSG                     = -6,
			CLIENT_DISCONNECTED_MSG             = -7,
			INVALID_GROUP_MSG                   = -8,
			MASTER_DISCONNECTED_MSG             = -9,
			ARCHIVE_SERVICE_MSG                 = -10,
			ARCHIVE_REQUEST_MSG                 = -11,
			INVAlID_ARCHIVE_REQUEST_MSG         = -12,
			INVALID_PROTOCOL_MSG                = -13,
			// COMMAND MESSAGES
			STATE_OF_HEALTH_CMD_MSG             = -14,
			STATE_OF_HEALTH_RESPONSE_MSG        = -15,
			REJECTED_CMD_MSG                    = -16,
			LIST_CONNECTED_CLIENTS_CMD_MSG      = -17,
			LIST_CONNECTED_CLIENTS_RESPONSE_MSG = -18,
			CLIENT_DISCONNECT_CMD_MSG           = -19,
			// ALWAYS UPDATE QUANTITY
			EMT_QUANTITY                        = 20
		};

		// message type and message content type
		// share the same flag
		// The lower 8 bits encode the message type
		// while the upper 7 bits encode the message content type

		// regular messages
		enum MSG_TYPES {
			UNDEFINED_MSG        = 0,
			DATA_MSG             = 1,
			ARCHIVE_MSG          = 2,
			MT_QUANTITY          = 3
		};

		// message content format
		enum MSG_CONTENT_TYPES {
			CONTENT_BINARY            = 0,
			CONTENT_XML               = 1,
			// XML (incl. seiscomp3 header) stream
			CONTENT_UNCOMPRESSED_XML  = 2,
			// XML stream
			CONTENT_IMPORTED_XML      = 3,
			MCT_QUANTITY              = 4
		};


	// ------------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------------
	private:
		Protocol() {}


	// ------------------------------------------------------------------------
	// Public Interface
	// ------------------------------------------------------------------------
	public:
		/** Helper function to return a char representation from the numerical
		 * message id.
		 * @return String representation or NULL if index is out of bounds
		 */
		static const char* MsgTypeToString(const int msgType)	{
			if ( msgType < 0 && abs( msgType ) < EMT_QUANTITY )
				return SERVICE_MSG_GROUP_NAMES[ abs( msgType ) ];
			else if ( msgType >= 0 && msgType < MT_QUANTITY )
				return MSG_GROUP_NAMES[ msgType ];
			return MSG_GROUP_NAMES[ 0 ];
		}

		//! Returns the string representation of the given numerical client type
		static const char* ClientTypeToString(ClientType type) {
			return CLIENT_TYPE_NAMES[ type ];
		}

		//! Returns the string representation of the given numerical client type
		static const char* ClientPriorityToString( ClientPriority priority ) {
			return CLIENT_PRIORITY_NAMES[ priority ];
		}
	

	// ------------------------------------------------------------------------
	// Data member
	// ------------------------------------------------------------------------
	public:
		//! Protocol version of the messaging system
		static const char* const PROTOCOL_VERSION;

		//! Older protocol version of the messaging system
		static const char* const PROTOCOL_VERSION_V1_0;
		static const char* const PROTOCOL_VERSION_V1_1;

		static const char *const HEADER_GROUP_TAG;
		static const char *const HEADER_SERVER_VERSION_TAG;
		static const char *const HEADER_SCHEMA_VERSION_TAG;

		/** Group name used for the service communication. Note: every client is a member
		 * of this group per default and cannot be used for regular data communication. */
		static const char* const MASTER_CLIENT_NAME;

		//! Maximal allowed sequence number (currently 2^16)
		static const int MAX_SEQ_NUM = 65536;

		/** Specifies the size of the masters message archive. The archive can hold the messages
		 * for a half an hour under the assumtion that 5 messages will arrive per second.
		 */
		static const int MASTER_ARCHIVE_SIZE = 9000;

		//! Maximal number of available groupsscmm
		static const int MAX_GROUPS = 100;

		//! Maximal size of a message
		static const unsigned int STD_MSG_LEN = 1048576;

		//! Definition of the predefined groups
		static const std::string MASTER_GROUP;

		/** Definition of a listener group. Clients with a primary listener group can subscribe
		 * to all available groups but cannot send to them. This is also true for every other client
		 * that wants to send to the lister group.
		 */
		static const std::string LISTENER_GROUP;

		/** All messages which are imported from another master (source) that do not have
		 * a respective group on the sink master should be send to this group.
		 */
		static const std::string IMPORT_GROUP;

		//! Name of the predfined administration group. Solely one client can join this group.
		static const std::string ADMIN_GROUP;

		//! Group the clients send their status messages to
		static const std::string STATUS_GROUP;

		/** Default name of the only administraion client. This name should be redefineable
		 * by a configuration file.
		 */
		static const std::string DEFAULT_ADMIN_CLIENT_NAME;

	
	private:
		//! String representation of the client priorities
		static const char* const CLIENT_PRIORITY_NAMES[];

		//! String representation of the client types
		static const char* const CLIENT_TYPE_NAMES[];

		//! String representation of the available message groups
		static const char* const MSG_GROUP_NAMES[];

		//! String representation of the available message groups
		static const char* const SERVICE_MSG_GROUP_NAMES[];
};


} // namespace Client
} // namespace Seiscomp

#endif
