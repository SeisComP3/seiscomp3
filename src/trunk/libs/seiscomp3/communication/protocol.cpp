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


#include <seiscomp3/communication/protocol.h>


namespace Seiscomp {
namespace Communication {

const char* const Protocol::PROTOCOL_VERSION_V1_0 = "1.0";
const char* const Protocol::PROTOCOL_VERSION_V1_1 = "1.1";
const char* const Protocol::PROTOCOL_VERSION = PROTOCOL_VERSION_V1_1;
const char* const Protocol::HEADER_GROUP_TAG = "Groups";
const char* const Protocol::HEADER_SERVER_VERSION_TAG = "Server-Version";
const char* const Protocol::HEADER_SCHEMA_VERSION_TAG = "Schema-Version";
const char* const Protocol::MASTER_CLIENT_NAME = "_MASTER_";

const char* const Protocol::CLIENT_PRIORITY_NAMES[Protocol::CP_QUANTITY] =
{
	"CLIENT_PRIORITY_DEFAULT",
	"CLIENT_PRIORITY_HIGH",
	"CLIENT_PRIORITY_MEDIUM",
	"CLIENT_PRIORITY_LOW"
};


const char* const Protocol::CLIENT_TYPE_NAMES[Protocol::CT_QUANTITY] =
{
	"CLIENT_TYPE_DEFAULT",
	"CLIENT_TYPE_ONE",
	"CLIENT_TYPE_TWO",
	"CLIENT_TYPE_THREE"
};


const std::string Protocol::MASTER_GROUP              = "MASTER_GROUP";
const std::string Protocol::LISTENER_GROUP            = "LISTENER_GROUP";
const std::string Protocol::IMPORT_GROUP              = "IMPORT_GROUP";
const std::string Protocol::ADMIN_GROUP               = "ADMIN_GROUP";
const std::string Protocol::STATUS_GROUP              = "STATUS_GROUP";

const std::string Protocol::DEFAULT_ADMIN_CLIENT_NAME = "ADMIN";


const char* const Protocol::MSG_GROUP_NAMES[Protocol::MT_QUANTITY] =
{
	"UNDEFINED_MSG",
	"DATA_MSG",
	"ARCHIVE_MSG"
};


const char* const Protocol::SERVICE_MSG_GROUP_NAMES[Protocol::EMT_QUANTITY] =
{
	"__DUMMY__",
	"CONNECT_GROUP_MSG",
	"CONNECT_GROUP_REJECT_MSG",
	"ADMIN_REJECT_MSG",
	"CONNECT_GROUP_OK_MSG",
	"JOIN_GROUP_MSG",
	"LEAVE_GROUP_MSG",
	"CLIENT_DISCONNECTED_MSG",
	"INVALID_GROUP_MSG",
	"MASTER_DISCONNECTED_MSG",
	"ARCHIVE_SERVICE_MSG",
	"ARCHIVE_REQUEST_MSG",
	"INVAlID_ARCHIVE_REQUEST_MSG",
	"INVALID_PROTOCOL_MSG",
	"STATE_OF_HEALTH_CMD_MSG",
	"STATE_OF_HEALTH_RESPONSE_MSG",
	"REJECTED_CMD_MSG",
	"LIST_CONNECTED_CLIENTS_CMD_MSG",
	"LIST_CONNECTED_CLIENTS_RESPONSE_MSG",
	"CLIENT_DISCONNECT_CMD_MSG",
	
};


} // namespace Client
} // namespace Seiscomp
