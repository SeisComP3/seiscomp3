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


#ifndef __COM_CLIENTDB_H__
#define __COM_CLIENTDB_H__

#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <cmath>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/communication/clientstatus.h>


namespace Seiscomp
{

namespace Communication
{

/** Class to store information about the connected clients.
 */
class ClientInfo
{

	// -----------------------------------------------------------------------
	// X'struction
	// -----------------------------------------------------------------------
public:
	/** Clones the passed message.
	 * @param sm Reference to a ServiceMessage 
	 */
	ClientInfo(const ServiceMessage& sm);
	~ClientInfo();


	// -----------------------------------------------------------------------
	// Public interface
	// -----------------------------------------------------------------------
public:
	const std::string& clientName() const;
	const std::string& privateGroup() const;
	Protocol::ClientType clientType() const;
	Protocol::ClientPriority clientPriority() const;
	
	/** Returns the time in seconds since the specific ClientInfo object was
	 * created.
	 * @return Elapsed time*/
	Seiscomp::Core::TimeSpan uptime() const;

	const Communication::ClientStatus* clientStatus() const;
		

private:
	//! Not implemented
	ClientInfo(const ClientInfo& info);
	
	
	// -----------------------------------------------------------------------
	// Private data members
	// -----------------------------------------------------------------------
private:
	std::string                _clientName;
	std::string                _privateGroup;
	Protocol::ClientType       _clientType;
	Protocol::ClientPriority   _clientPriority;
	
	Seiscomp::Util::StopWatch _timer;
	Communication::ClientStatus*       _clientStatus;
};



/** Stores information about the clients which are connected to the
 * master client 
 */
class ClientDB
{
	// -----------------------------------------------------------------------
	// Nested types
	// -----------------------------------------------------------------------
public:
	typedef std::vector<boost::shared_ptr<ClientInfo> >::iterator iterator;
	typedef std::vector<boost::shared_ptr<ClientInfo> >::const_iterator const_iterator;

private:
	typedef std::set<std::string> Cache;
	
	
	// -----------------------------------------------------------------------
	// X'struction
	// -----------------------------------------------------------------------
public:
	ClientDB();
	~ClientDB();

	// -----------------------------------------------------------------------
	// Public interface
	// -----------------------------------------------------------------------
public:

	//! Adds a new client to the client data base
	const ClientInfo* addClientToDB(const ServiceMessage& msg);

	//! Removes a client from the client db
	bool removeClientFromDB(const std::string& privateSenderGroup);

	//! Returns a pointer to the client with the given private client group from the DB
	const ClientInfo* getClientFromDB(const std::string& privateSenderGroup);

	//! Returns true if the client is already in the database
	bool isClientInDB(const std::string& privateSenderGroup);
			
	//! Returns the number of stored clients
	int size() const;

	//! Returns an iterator to the beginning of the container
	iterator begin() ;

	//! Returns and iterator one past the last element of the container
	iterator end();
	
	//! Removes all clients from the database
	void clear();

	// -----------------------------------------------------------------------
	// Private data members
	// -----------------------------------------------------------------------
private:

	//! Stores the client data wihich is sent by the clients connect call
	// std::vector<const ClientInfo*> _clientDB;
	std::vector<boost::shared_ptr<ClientInfo> > _clientDB;
	Cache                                       _cache;
};

// -----------------------------------------------------------------------
// Inline methods
// -----------------------------------------------------------------------
#include "clientdb.ipp"

} // namespace Communication
} // namespace Seiscomp

#endif
