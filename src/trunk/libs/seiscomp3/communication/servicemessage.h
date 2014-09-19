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


#include <seiscomp3/core/message.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/client.h>

#ifndef __SEISCOMP_COMMUNICATION_SERVICEREQUESTMESSAGE_H__
#define __SEISCOMP_COMMUNICATION_SERVICEREQUESTMESSAGE_H__


namespace Seiscomp {

namespace IO {

class DatabaseInterface;

}

namespace Communication {


DEFINE_SMARTPOINTER(SyncRequestMessage);

/**
 * \brief Message for requesting a synch
 * This message type requests a synch response
 * from a peer. This is not a service message and
 * thus enqueues into the user message flow.
 * Requesting a sync can be important to know when
 * a certain amount of messages has been processed by the
 * peer.
 */
class SC_SYSTEM_CLIENT_API SyncRequestMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(SyncRequestMessage);
	DECLARE_SERIALIZATION;
	
	public:
		//! Constructor
		SyncRequestMessage();

		/**
		 * Constructor
		 * @param ID The sync ID.
		 *           This ID can be used to validate a
		 *           sync request/response conversation.
		 */
		SyncRequestMessage(const char *ID);

		//! Implemented interface from Message
		virtual bool empty() const;


	public:
		/**
		 * @return The sync ID
		 */
		const char *ID() const;

	private:
		std::string _ID;
};


DEFINE_SMARTPOINTER(SyncResponseMessage);

/**
 * \brief Message to respond to a synch request
 */
class SC_SYSTEM_CLIENT_API SyncResponseMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(SyncResponseMessage);
	DECLARE_SERIALIZATION;
	
	public:
		//! Constructor
		SyncResponseMessage();

		/**
		 * Constructor
		 * @param ID The sync ID.
		 *           This ID can be used to validate a
		 *           sync request/response conversation.
		 */
		SyncResponseMessage(const char *ID);

		//! Implemented interface from Message
		virtual bool empty() const;


	public:
		/**
		 * @return The sync ID
		 */
		const char *ID() const;

	private:
		std::string _ID;
};



DEFINE_SMARTPOINTER(ServiceRequestMessage);

/**
 * \brief Message for requesting a service
 * This class is the base class for all kinds of service
 * requests being sent over the network.
 * It holds an optional servicename. If the servicename
 * is not set the request handler can choose a service
 * matching the servicetype defined by the classname.
 * A message of this type cannot not be sent. One has to
 * derive a class to define the type of service (database,
 * fileaccess, ...).
 */
class SC_SYSTEM_CLIENT_API ServiceRequestMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ServiceRequestMessage);
	DECLARE_SERIALIZATION;
	
	protected:
		//! Constructor
		ServiceRequestMessage();
		
		/**
		 * Constructor
		 * @param service The requested service name.
		 *                The name can be set NULL to let the
		 *                service request handler decide which
		 *                interface it will return.
		 */
		ServiceRequestMessage(const char* service);

		//! Implemented interface from Message
		virtual bool empty() const;

	public:
		/**
		 * @return The requested service name
		 */
		const char* service() const;

	private:
		std::string _serviceName;
};


DEFINE_SMARTPOINTER(ServiceProvideMessage);

/**
 * \brief Message for providing a service
 * This class is the base class for all kinds of service
 * providers.
 * It holds a servicename and the connection parameters.
 */
class SC_SYSTEM_CLIENT_API ServiceProvideMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ServiceProvideMessage);
	DECLARE_SERIALIZATION;

	protected:
		/**
		 * Constructor
		 * @param service The provided service name.
		 * @param params The connection parameters.
		 */
		ServiceProvideMessage(const char* service,
		                      const char* params);
	
	public:
		/**
		 * @return The provided service name.
		 */
		const char* service() const;

		/**
		 * @return The connection parameters.
		 */
		const char* parameters() const;

		//! Implemented interface from Message
		virtual bool empty() const;

	private:
		std::string _serviceName;
		std::string _parameters;
};


DEFINE_SMARTPOINTER(DatabaseRequestMessage);

/**
 * \brief Message for requesting a database service
 * This message type requests a databaseservice.
 * The servicename is optional. If the servicename is not
 * given the serviceprovider can choose between different
 * services it offers of this type.
 * So if the provider offers a mysql database and a postgresql
 * database it can select the published service by its own.
 * If the servicename is set the provider has to publish
 * the service matching this name if it is able to do so.
 */
class SC_SYSTEM_CLIENT_API DatabaseRequestMessage : public ServiceRequestMessage {
	DECLARE_SC_CLASS(DatabaseRequestMessage);
	
	public:
		//! Constructor
		DatabaseRequestMessage();
		
		/**
		 * Constructor
		 * @param service The requested service name.
		 *                The name can be set NULL to let the
		 *                service request handler decide which
		 *                interface it will return.
		 */
		DatabaseRequestMessage(const char* service);
};


DEFINE_SMARTPOINTER(DatabaseProvideMessage);

/**
 * \brief Message for providing a database service
 * When receiving this message a corresponding databaseinterface
 * can be created which one connects to using the provided
 * parameters.
 * \code
 * DatabaseProvideMessagePtr msg = DatabaseProvideMessage_Cast(con->read());
 * Seiscomp::IO::DatabaseInterfacePtr db = msg->interface();
 * if ( db != NULL ) {
 *   // do fancy things with the interface
 * }
 * \endcode
 */
class SC_SYSTEM_CLIENT_API DatabaseProvideMessage : public ServiceProvideMessage {
	DECLARE_SC_CLASS(DatabaseProvideMessage);

	public:
		//! Constructor
		DatabaseProvideMessage();
		
		/**
		 * Constructor
		 * @param service The provided service name.
		 * @param params The connection parameters.
		 */
		DatabaseProvideMessage(const char* service,
		                       const char* params);

	public:
		/**
		 * Returns a database interface for the provided service
		 * which is already connected to the database.
		 * @return The connected database interface. NULL, if the
		 *         databaseinterface cannot be created or if the
		 *         connection fails.
		 *         NOTE: The caller is reponsible for deleting the
		 *         returned object.
		 */
		Seiscomp::IO::DatabaseInterface* database() const;
};


}
}


#endif
