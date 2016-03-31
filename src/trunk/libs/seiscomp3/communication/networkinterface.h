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

#ifndef __SEISCOMP_COMMUNICATION_NETWORKINTERFACE_H__
#define __SEISCOMP_COMMUNICATION_NETWORKINTERFACE_H__

#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/core/interfacefactory.h>

namespace Seiscomp {
namespace Communication {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DEFINE_SMARTPOINTER(NetworkInterface);
/** \brief An abstract network interface

	This class provides an interface to communication frameworks such as
	spread.
	Communication interface classes will be registered in their constructor in
	the global interface pool. They can only be created through the
	interface factory.

	Example to request a communication interface
	\code
	ClientInterfacePtr db = ClientInterface::Create("spread");
	\endcode

	To implement new interface, just derive from NetworkInterface
	and register the instance with REGISTER_COMMUNICATION_INTERFACE
	\code
	class MyCommunicationInterface : NetworkInterface {
		DECLARE_SC_CLASS(MyCommunicationInterface)
		
	public:
			MyCommunicationInterface(const char* serviceName)
	: CommunicationInterface(serviceName) {}

		// Implement all virtual methods
};

	REGISTER_COMMUNICATION_INTERFACE(MyCommunicationInterface, "MyServiceName");
	\endcode
 */
class SC_SYSTEM_CLIENT_API NetworkInterface : public Core::BaseObject {
	DECLARE_SC_CLASS(NetworkInterface);
	
	// ----------------------------------------------------------------------
	// X'STRUCTION
	// ----------------------------------------------------------------------
	public:
		virtual ~NetworkInterface();
		
	protected:
		NetworkInterface();


	// ----------------------------------------------------------------------
	// INTERFACE
	// ----------------------------------------------------------------------
	public:
		/** Connects to the messaging system with the given clientname. If
		 * this call succeedes, the connected client will be uniquely identified
		 * by the clientname. Therefore every follwing call with the same 
		 * clientname to the same server should fail. The Server in turn assigns the 
		 * connected client a private groupname which is unknown by the other clients
		 * and can be used for private communication.
		 * @param serverAddress The address of the server. The format is host:port
		 *                      Port is optional and defaults to 4803
		 * @param clientName name of the client
		 * @return SEISCOMP_SUCCESS on success
		*/
		virtual int connect(const std::string& serverAddress, 
		                    const std::string& clientName) = 0;
		
		/** Disconnects from the messaging system
		 * @return SEISCOMP_SUCCESS on success
		 */
		virtual int disconnect() = 0;
		
		/** Returns the received network message/service message or null. The ownership of the
		 * message goes to the calling program.
		 * @param error in/out parameter which holds the error code
		 * @param toPrivateAddress Receive messages which were sent to the clients private address
		 * @return the received message
		 */
		virtual NetworkMessage* receive(int* error = NULL) = 0;
	
		/** Sends a message
		 * @param group name of the destination group
		 * @param type the type of the message
		 * @param msg the message itself
		 * @param selfDiscard specifies if the client should receive its own messages
		 * @return SEISCOMP_SUCCESS on success
		 */
		virtual int send(const std::string& group, int type, NetworkMessage* msg, bool selfDiscard = true) = 0;
				
		/** Subscribes to a message group
		 * @param group name of the group
		 * @return SEISCOMP_SUCCESS on success
		 */
		virtual int subscribe(const std::string& group) = 0;
		
		/** Unsubscribes a message group
		 * @param group name of the group
		 * @return SEISCOMP_SUCCESS on success
		 */
		virtual int unsubscribe(const std::string& group) = 0;
		
		/** Returns true if there is a message waiting on the network connection
		 * @param error in/out parrameter which holds the error code
		 * @return true if ther is a new message
		 */
		virtual bool poll(int* error = NULL) = 0;
		
		/** Retrurs true if the client is connected
		 * @return connection state
		 */
		virtual bool isConnected() = 0;
		
		/** Returns a client interface for the given service
		 *  @return A pointer to the client interface
	 	 *          NOTE: The returned pointer has to be deleted by the
	 	 *                caller! 
		 */
		static NetworkInterface* Create(const char* service);
	
	
		// ------------------------------------------------------------------
		// GETTER AND SETTER
		// ------------------------------------------------------------------
	public:
		/** Returns the private group that has been assigne to the client
		 * by the server 
		 * @return name of the private group
		 */
		virtual std::string privateGroup() const = 0;
		
		/** Returns the private group of the last sender
		 * @return name of the group
		 */
		virtual std::string groupOfLastSender() const = 0;

		/** Sets the sequence number of the next message
		 * if implemented
		 */
		virtual void setSequenceNumber(int64_t seq) { }

		/** Gets the sequence number of the next message
		 * if implemented
		 * @return sequence number
		 */
		virtual int64_t getSequenceNumber() const { return -1; }

};	


DEFINE_INTERFACE_FACTORY(NetworkInterface);

#define REGISTER_NETWORK_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Communication::NetworkInterface, Class> __##Class##InterfaceFactory__(Service)

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace Communication
} // namespace Seiscomp

#endif
