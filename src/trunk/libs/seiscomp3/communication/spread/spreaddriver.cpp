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

// logging
#define SEISCOMP_COMPONENT Communication
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/core/status.h>

#include "spreaddriver.h"
#include "sp.h"

#include <memory.h>


namespace Seiscomp {
namespace Communication {


namespace {

int SP_error(int error)
{
	switch( error )
	{
	case ILLEGAL_SPREAD:
		SEISCOMP_ERROR("SP_error: (%d) Illegal spread was provided", error);
		return Core::Status::SEISCOMP_CONNECT_ERROR;
	case COULD_NOT_CONNECT:
		SEISCOMP_ERROR("SP_error: (%d) Could not connect. Is Spread running?", error);
		return Core::Status::SEISCOMP_CONNECT_ERROR;
	case REJECT_QUOTA:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, to many users", error);
		return Core::Status::SEISCOMP_TOO_MANY_USERS;
	case REJECT_NO_NAME:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, no name was supplied", error);
		return Core::Status::SEISCOMP_CONNECT_ERROR;
	case REJECT_ILLEGAL_NAME:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, illegal name", error);
		return Core::Status::SEISCOMP_INVALID_CLIENT_NAME_ERROR;
	case REJECT_NOT_UNIQUE:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, name not unique", error);
		return Core::Status::SEISCOMP_CLIENT_NAME_NOT_UNIQUE;
	case REJECT_VERSION:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, library does not fit daemon", error);
		return Core::Status::SEISCOMP_WRONG_SERVER_VERSION;
	case CONNECTION_CLOSED:
		SEISCOMP_ERROR("SP_error: (%d) Connection closed by spread", error);
		break;
	case REJECT_AUTH:
		SEISCOMP_ERROR("SP_error: (%d) Connection rejected, authentication failed", error);
		break;
	case ILLEGAL_SESSION:
		SEISCOMP_ERROR("SP_error: (%d) Illegal session was supplied", error);
		break;
	case ILLEGAL_SERVICE:
		SEISCOMP_ERROR("SP_error: (%d) Illegal service request", error);
		break;
	case ILLEGAL_MESSAGE:
		SEISCOMP_ERROR("SP_error: (%d) Illegal message", error);
		break;
	case ILLEGAL_GROUP:
		SEISCOMP_ERROR("SP_error: (%d) Illegal group", error);
		break;
	case BUFFER_TOO_SHORT:
		SEISCOMP_ERROR("SP_error: (%d) The supplied buffer was too short", error);
		break;
	case GROUPS_TOO_SHORT:
		SEISCOMP_ERROR("SP_error: (%d) The supplied groups list was too short", error);
		break;
	case MESSAGE_TOO_LONG:
		SEISCOMP_ERROR("SP_error: (%d) The message body + group names was too large to fit in a message", error);
		break;
	case NET_ERROR_ON_SESSION:
		SEISCOMP_ERROR("SP_error: (%d) The network socket experienced an error. This Spread mailbox will no longer work until the connection is disconnected and then reconnected", error);
		break;
	default:
		SEISCOMP_ERROR("SP_error: (%d) unrecognized error", error);
	}

	return Core::Status::SEISCOMP_NETWORKING_ERROR;
}

}


REGISTER_NETWORK_INTERFACE(SpreadDriver, "spread");


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpreadDriver::SpreadDriver()
{
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpreadDriver::~SpreadDriver()
{
	// disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::connect(
		const std::string& serverAddress,
		const std::string& clientName) {
	if ( clientName.size() > MAX_PRIVATE_NAME ) {
		SEISCOMP_ERROR("The clientname %s exceeds the maximum length of: %d",
		               clientName.c_str(), MAX_PRIVATE_NAME);
		return Core::Status::SEISCOMP_INVALID_CLIENT_NAME_ERROR;
	}

	init();

	std::string connectionString(serverAddress);

	if ( !serverAddress.empty() ) {
		// Check if the address is already valid
		if ( serverAddress.find("@") == std::string::npos ) {
			std::vector<std::string> tokens;
			Core::split(tokens, serverAddress.c_str(), ":");
			if ( tokens.size() > 2 || tokens.size() == 0 ) {
				SEISCOMP_ERROR("Invalid host address: %s", serverAddress.c_str());
				return Core::Status::SEISCOMP_CONNECT_ERROR;
			}

			if ( tokens.size() > 1 ) {
				connectionString = tokens[1];
			}
			else {
				connectionString = "4803";
			}

			connectionString += "@";
			connectionString += tokens[0];
		}
	}
	else {
		connectionString = "4803@localhost";
	}

	int ret = SP_connect(connectionString.c_str(), clientName.c_str(), 0, 1, &_spMBox, _spPrivateGroup);
	if ( ret != ACCEPT_SESSION ) {
		return SP_error(ret);
	}

	_isConnected = true;
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::disconnect()
{
	mailbox mbox = _spMBox;
	// Reset mbox that receive does not throw an error after
	// a manually closed connection
	_spMBox = -1;
	int ret = SP_disconnect(mbox);
	if (ret < 0)
		SP_error(ret);
	init();
	return (ret < 0) ? Core::Status::SEISCOMP_NETWORKING_ERROR : Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* SpreadDriver::receive(int* error)
{
	_spMessageType = 0;
	int ret = SP_receive(_spMBox, &_spServiceType, _spSender, Protocol::MAX_GROUPS, &_spNumGroups, _spTargetGroups,
	                     &_spMessageType, &_spEndianMismatch, Protocol::STD_MSG_LEN, _spMessageReadBuffer);

	if ( ret < 0 ) {
		// Only print an error message when the mbox has not been released from another thread
		if (_spMBox != -1)
			SP_error(ret);
		if ( error )
			*error = handleError(ret);
		else
			handleError(ret);

		return NULL;
	}

	NetworkMessage* message = NULL;
	if (Is_regular_mess(_spServiceType))
	{
		if (_spMessageType > 0)
			message = new NetworkMessage;
		else if (_spMessageType < 0)
			message = new ServiceMessage;

		if (message)
		{
			if ( !message->read(_spMessageReadBuffer, Protocol::STD_MSG_LEN) ) {
				SEISCOMP_ERROR("Could not read regular message from %s to %s", _spSender, _spTargetGroups[0]) ;
				delete message;
				return NULL;
			}

			message->setSize(ret);
		}
	}
	else if (Is_membership_mess(_spServiceType))
	{
		membership_info membInfo ;
		int ret = SP_get_memb_info(_spMessageReadBuffer, _spServiceType, &membInfo) ;
		if (ret < 0)
		{
			SEISCOMP_ERROR("Could not read membership information") ;
			SP_error(ret);
		}

		if (Is_reg_memb_mess(_spServiceType))
		{
			if (Is_caused_disconnect_mess(_spServiceType))
			{
				message = new ServiceMessage(Protocol::CLIENT_DISCONNECTED_MSG);
				std::vector<std::string> tokens;
				int ret = Core::split(tokens, membInfo.changed_member, "#");
				if (ret == 3) {
					if (tokens[1] == Protocol::MASTER_CLIENT_NAME)
						message->setType(Protocol::MASTER_DISCONNECTED_MSG);
				}
				message->setPrivateSenderGroup(membInfo.changed_member);
				message->setData(membInfo.changed_member);
			}
			else if (Is_caused_join_mess(_spServiceType))
			{
				message = new ServiceMessage(Protocol::JOIN_GROUP_MSG);
				message->setPrivateSenderGroup(membInfo.changed_member);
				// Data has the form:
				// ?Group that has been joined&client that joined?member0 of the group&member1&...&memberN
				std::stringstream ss;
				ss << "?" << _spSender << "&" << membInfo.changed_member << "?";
				for (int i = 0; i < _spNumGroups; ++i)
				{
					if (i > 0)
						ss << "&";
					ss << _spTargetGroups[i];
				}
				message->setData(ss.str());
			}
			else if (Is_caused_leave_mess(_spServiceType))
			{
				message = new ServiceMessage(Protocol::LEAVE_GROUP_MSG);
				message->setPrivateSenderGroup(membInfo.changed_member);
				message->data() = "?";
				message->data() += _spSender;
				message->data() += "&";
				message->data() += membInfo.changed_member;
				message->data() += "?";
			}
		}
	}

	if (message)
		messageInfo(message);

	return message;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::send(const std::string& group, int type, NetworkMessage* msg, bool selfDiscard)
{
	int size;
	try {
		size = msg->write(_spMessageWriteBuffer, Protocol::STD_MSG_LEN);
	}
	catch ( const Seiscomp::Core::OverflowException &e ) {
		SEISCOMP_ERROR("Message size exceeds maximum limit %i",
		               Protocol::STD_MSG_LEN);
		return Core::Status::SEISCOMP_MESSAGE_SIZE_ERROR;
	}

	if (size < 0 || size > (int)Protocol::STD_MSG_LEN)
	{
		SEISCOMP_ERROR("Message size exceeds maximum limit %i : MESSAGE SIZE: %i" ,
		               Protocol::STD_MSG_LEN, size);
		return Core::Status::SEISCOMP_MESSAGE_SIZE_ERROR;
	}

	int service = FIFO_MESS;
	if ( selfDiscard )
		service = service | SELF_DISCARD;

	int ret = SP_multicast(_spMBox, service, group.c_str(),
	                       type, size, _spMessageWriteBuffer);

	if ( ret < 0 ) {
		SP_error(ret);
		return handleError(ret);
	}

	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::subscribe(const std::string& group)
{
	int ret = SP_join(_spMBox, group.c_str());
	if ( ret < 0 ) {
		SP_error(ret);
		return handleError(ret);
	}
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::unsubscribe(const std::string& group)
{
	int ret = SP_leave(_spMBox, group.c_str());
	if ( ret < 0 ) {
		SP_error(ret);
		return handleError(ret);
	}
	return Core::Status::SEISCOMP_SUCCESS;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpreadDriver::poll(int* error)
{
	int ret = SP_poll(_spMBox);
	if ( ret < 0 ) {
		SP_error(ret);
		if (error)
			*error = handleError(ret);
		else
			handleError(ret);
	}
	else {
		if ( error )
			*error = Core::Status::SEISCOMP_SUCCESS;
	}

	return ret > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpreadDriver::isConnected()
{
	return _isConnected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpreadDriver::messageInfo(const NetworkMessage* msg)
{
	const unsigned int MAX_MEMBERS = 100;
	const unsigned int MAX_VSSETS = 10;
	membership_info memb_info;
	vs_set_info vssets[10];
	unsigned int my_vsset_index;
	int num_vs_sets;
	char members[MAX_MEMBERS][MAX_GROUP_NAME];

	if (Is_regular_mess(_spServiceType))
	{
		/*
		SEISCOMP_DEBUG(" ");
		SEISCOMP_DEBUG("= regular spread message =");
		*/
		// Terminate the message with 0
		_spMessageReadBuffer[msg->size()] = 0;

		// Check spread message type
		std::string msgType;
		if (Is_unreliable_mess(_spServiceType))
			msgType = "UNRELIABLE ";
		else if (Is_reliable_mess(_spServiceType))
			msgType = "RELIABLE ";
		else if (Is_fifo_mess(_spServiceType))
			msgType = "FIFO ";
		else if (Is_causal_mess(_spServiceType))
			msgType = "CAUSAL ";
		else if (Is_agreed_mess(_spServiceType))
			msgType = "AGREED ";
		else if (Is_safe_mess(_spServiceType))
			msgType = "SAFE ";

		/*
		SEISCOMP_DEBUG("Spread message type: %s", msgType.c_str());
		SEISCOMP_DEBUG("Sequence number:     %d", msg->seqNum());
		SEISCOMP_DEBUG("Initial sender:      %s", msg->privateSenderGroup().c_str());
		SEISCOMP_DEBUG("Last sender:         %s", _spSender);
		SEISCOMP_DEBUG("Message id:          %d(%d)", _spMessageType, msg->type());
		SEISCOMP_DEBUG("Message name:        %s", Protocol::MsgTypeToString(_spMessageType));
		*/
		std::string targetStr;
		for (int i = 0; i < _spNumGroups; ++i)
		{
			if (i > 0)
				targetStr += ", ";
			targetStr += _spTargetGroups[i];
		}
		//SEISCOMP_DEBUG("Target groups:       %s", targetStr.c_str());
		//SEISCOMP_DEBUG("Message size:        %d", msg->size());
	}
	else if (Is_membership_mess(_spServiceType))
	{
		/*
		SEISCOMP_DEBUG(" ");
		SEISCOMP_DEBUG("= spread membership message =");
		*/

		int ret = SP_get_memb_info(_spMessageReadBuffer, _spServiceType, &memb_info);
		if (ret < 0)
		{
			SEISCOMP_DEBUG("Error: Membership message does not have valid body");
			SP_error(ret);
		}

		if (Is_reg_memb_mess(_spServiceType))
		{
			/*
			SEISCOMP_DEBUG("Received REGULAR membership message for group %s with %d members, where I am member %d:",
			              _spSender, _spNumGroups, _spMessageType);
			*/

			std::string targetStr;
			for (int i = 0; i < _spNumGroups; ++i)
			{
				if (i > 0)
					targetStr += ", ";
				targetStr += _spTargetGroups[i];
			}
			//SEISCOMP_DEBUG("Target groups: %s", targetStr.c_str());
			//SEISCOMP_DEBUG("Group id: %d %d %d", memb_info.gid.id[0], memb_info.gid.id[1], memb_info.gid.id[2]);

			/*
			if (Is_caused_join_mess(_spServiceType))
				SEISCOMP_DEBUG("Received message due to JOIN of %s", memb_info.changed_member);
			else if (Is_caused_leave_mess(_spServiceType))
				SEISCOMP_DEBUG("Received message due to LEAVE of %s", memb_info.changed_member);
			else if (Is_caused_disconnect_mess(_spServiceType))
				SEISCOMP_DEBUG("Received message due to DISCONNECT of %s", memb_info.changed_member);
			else */if (Is_caused_network_mess(_spServiceType))
			{
				//SEISCOMP_DEBUG("Received message due to NETWORK change with %u VS sets", memb_info.num_vs_sets);
				num_vs_sets = SP_get_vs_sets_info(_spMessageReadBuffer, &vssets[0], MAX_VSSETS, &my_vsset_index);

				if (num_vs_sets < 0)
				{
					SEISCOMP_DEBUG("WARNING: Membership message has more than %d vs sets. Recompile spread with larger MAX_VSSETS", MAX_VSSETS);
					SP_error(num_vs_sets);
				}

				for (int i = 0; i < num_vs_sets; ++i)
				{
					SEISCOMP_DEBUG( "%s VS set %d has %u members:",
					               (i == (int) my_vsset_index) ? ("LOCAL") : ("OTHER"), i, vssets[i].num_members);
					ret = SP_get_vs_set_members(_spMessageReadBuffer, &vssets[i], members, MAX_MEMBERS);
					if (ret < 0)
					{
						SEISCOMP_DEBUG("WARNING: VS Set has more than %d members. Recompile spread with larger MAX_MEMBERS", MAX_MEMBERS);
						SP_error(ret);
					}

					for (int j = 0; j < (int) vssets[i].num_members; ++j)
						SEISCOMP_DEBUG("%s", members[j]);
				}
			}
		}
		/*
		else if (Is_transition_mess(_spServiceType))
			SEISCOMP_DEBUG("Received TRANSITIONAL membership message for group %s", _spSender);
		else if (Is_caused_leave_mess(_spServiceType))
			SEISCOMP_DEBUG("Received membership message that %s left group", _spSender);
		else
			SEISCOMP_DEBUG("Received incorrectly membership message of type 0x%x", _spServiceType);
		*/
	}
	/*
	else if (Is_reject_mess(_spServiceType))
		SEISCOMP_DEBUG("REJECTED message from %s, of servicetype 0x%x messtype %d, (endian %d) to %d groups \n(%d bytes): %s",
		              _spSender, _spServiceType, _spMessageType, _spEndianMismatch, _spNumGroups, msg->size(), _spMessageReadBuffer);
	else
		SEISCOMP_DEBUG("Received message of unknown type 0x%x with return value %d", _spServiceType, msg->size());
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpreadDriver::init()
{
	_spMBox = -1;
	_spServiceType = DROP_RECV;
	_spMaxGroups = 0;
	_spNumGroups = 0;
	_spMessageType = 0;
	_spEndianMismatch = 0;
	_isConnected = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string SpreadDriver::privateGroup() const
{
	return _spPrivateGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string SpreadDriver::groupOfLastSender() const
{
	return _spSender;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SpreadDriver::handleError(int error) {
	switch ( error ) {
		case NET_ERROR_ON_SESSION:
			_isConnected = false;
			return Core::Status::SEISCOMP_NETWORKING_ERROR;

		case ILLEGAL_SESSION:
		case CONNECTION_CLOSED:
			_isConnected = false;
			return Core::Status::SEISCOMP_NOT_CONNECTED_ERROR;

		case MESSAGE_TOO_LONG:
			return Core::Status::SEISCOMP_MESSAGE_SIZE_ERROR;

		default:
			break;
	}

	_isConnected = false;
	return Core::Status::SEISCOMP_NETWORKING_ERROR;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp
