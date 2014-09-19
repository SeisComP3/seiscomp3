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
#define SEISCOMP_COMPONENT MASTER_COM_MODULE
#include <seiscomp3/logging/log.h>

#include "clientdb.h"


namespace Seiscomp
{

namespace Communication
{

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClientDB::ClientDB()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ClientDB::~ClientDB()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClientInfo* ClientDB::addClientToDB(const ServiceMessage& msg)
{
	ClientInfo* ci = new ClientInfo(msg);
	_clientDB.push_back((boost::shared_ptr<ClientInfo>(ci)));
	_cache.insert(msg.privateSenderGroup());
	
	return ci;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const ClientInfo* ClientDB::getClientFromDB(const std::string& privateSenderGroup)
{
	const ClientInfo* found = NULL;
	for (iterator it = _clientDB.begin(); it != _clientDB.end(); ++it)
	{
		if (privateSenderGroup == (*it)->privateGroup())
		{
			found = (*it).get();
			break;
		}
	}
	return found;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ClientDB::isClientInDB(const std::string& privateSenderGroup)
{
	Cache::iterator it = _cache.find(privateSenderGroup);
	if (it != _cache.end())
		return true;
	return false;
	
	//return ((getClientFromDB(privateSenderGroup) != NULL) ? true : false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ClientDB::removeClientFromDB(const std::string& privateSenderGroup)
{
	for (iterator it = _clientDB.begin(); it != _clientDB.end(); ++it)
	{
		if (privateSenderGroup == (*it)->privateGroup())
		{
			SEISCOMP_INFO("Removing client %s from client database", privateSenderGroup.c_str());
			//delete *it;
			_clientDB.erase(it);
			_cache.erase(privateSenderGroup);
			return true;
		}
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ClientDB::clear()
{
	_clientDB.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Communication
} // namespace Seiscomp
