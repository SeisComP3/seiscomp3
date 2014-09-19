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

#ifndef __SEISCOMP_APPLICATIONS_IMEXIMPL_H__
#define __SEISCOMP_APPLICATIONS_IMEXIMPL_H__

#include <string>
#include <list>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <seiscomp3/communication/protocol.h>
#include <seiscomp3/communication/systemconnection.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/queue.h>

#include "imex.h"
#include "criterion.h"


namespace Seiscomp {
namespace Applications {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ImExImpl {

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	public:
		typedef std::list<DataModel::Origin*> SentOriginList;
		typedef std::map<std::string, std::string> RoutingTable;

	private:
		class NetworkMessageWrapper {
			public:
				NetworkMessageWrapper() :
					_networkMessage(NULL) {}
				NetworkMessageWrapper(
						const std::string& destination, Communication::NetworkMessage* message) :
					_destination(destination),
					_networkMessage(message) {}

				const std::string& destination() {
					return _destination;
				}
				Communication::NetworkMessage* networkMessage() {
					return _networkMessage.get();
				}

			private:
				std::string _destination;
				Communication::NetworkMessagePtr _networkMessage;
		};

		class EventWrapper {
			public:
				EventWrapper(DataModel::Event* event) :
					_sent(false),
					_publicID(event->publicID()),
					_preferredOriginID(event->preferredOriginID()),
					_preferredMagnitudeID(event->preferredMagnitudeID()),
					_event(event) {}

				EventWrapper& operator = (DataModel::Event* event) {
					_publicID             = event->publicID();
					_preferredOriginID    = event->preferredOriginID();
					_preferredMagnitudeID = event->preferredMagnitudeID();
					_event                = event;
					return *this;
				}

				DataModel::Event* event() const { return _event; }

				bool hasBeenSent() const { return _sent; }

				void setHasBeenSent(bool b) { _sent = b; }

				const std::string& preferredOriginID() const {
					return _preferredOriginID;
				}

				const std::string& publicID() const {
					return _publicID;
				}

				const std::string& preferredMagnitudeID() const {
					return _preferredMagnitudeID;
				}

				// Core::Time time() const throw (Core::ValueException) { return _event->created(); }

			private:
				bool              _sent;
				std::string       _publicID;
				std::string       _preferredOriginID;
				std::string       _preferredMagnitudeID;
				DataModel::Event* _event;
		};
		typedef std::list<EventWrapper> EventList;
		typedef std::vector<Criterion> Criteria;
		typedef std::vector<DataModel::Pick*> SentPicks;
		typedef std::vector<DataModel::Amplitude*> SentAmplitudes;


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		~ImExImpl();
		bool handleMessage(Core::Message* message);
		void cleanUp();
		void stop();
		void wait();
		std::string sinkName() const;
		static ImExImpl* Create(ImEx* imex, const std::string& sinkName);
		static RoutingTable CreateDefaultRoutingTable();


	// ------------------------------------------------------------------
	// Private interface
	// ------------------------------------------------------------------
	private:
		ImExImpl(ImEx* imex, const std::string& implName);

		bool init();

		int connectToSink();

		void cleanUp(EventList& eventList);

		bool buildRoutingTable();
		bool fillRoutingTable(std::vector<std::string>& source, RoutingTable& dest);

		void handleNotifierMessage(DataModel::NotifierMessage* notifierMessage);
		// void handleDataMessage(Core::DataMessage* dataMessage);
		// void handleEvent(DataModel::Event* event);

		void readSinkMessages();

		ImEx::OriginList::const_iterator findOrigin(const DataModel::Origin* origin);
		ImEx::OriginList::const_iterator findOrigin(const std::string& originID);

		bool hasOriginBeenSent(const std::string& originID);
		bool hasEventBeenSent(const DataModel::Origin* origin);

		bool isOriginEligibleImport(const DataModel::Origin* origin);
		bool isOriginEligibleExport(const DataModel::Origin* origin);
		bool isOriginEligible(const DataModel::Origin* origin);

		bool filter(DataModel::Origin* origin);
		bool filterMagnitudeImport(const DataModel::Origin* origin);
		bool filterMagnitudeExport(const DataModel::Origin* origin);
		bool filterMagnitude(const DataModel::Origin* origin);

		void serializeMessage(const std::string& destination, Core::Message* message);
		void sendOrigin(DataModel::Origin* origin, bool update = false);
		void sendEvent(EventWrapper& eventWrapper);
		void sendMessage(const std::string& destination, Core::Message* message);
		void sendMessageRaw();
		void sendMessageImport(const std::string& destination, Core::Message* message);
		void sendMessageExport(const std::string& destination, Core::Message* message);

		bool configGetRoutingTable(const std::string& prefix, const std::string& name, RoutingTable* routingTable);


	// ------------------------------------------------------------------
	// Private implementation
	// ------------------------------------------------------------------
	private:
		std::string                           _sinkName;
		std::string                           _sinkAddress;
		std::string                           _userName;
		Communication::SystemConnectionPtr    _sink;
		ImEx*                                 _imex;
		boost::shared_ptr<CriterionInterface> _criterion;
		boost::thread                        *_thread0;
		boost::thread                        *_thread1;

		int  _sleepDuration;

		bool _filter;
		bool _useDefinedRoutingTable;

		std::string _conversion;

		EventList      _eventList;
		SentOriginList _sentOrigins;
		SentPicks      _sentPicks;
		SentAmplitudes _sentAmplitudes;

		RoutingTable _routingTable;

		volatile bool _isRunning;
		size_t _maxQueueSize;
		Client::ThreadedQueue<NetworkMessageWrapper> _messageQueue;

		Communication::Protocol::MSG_CONTENT_TYPES _messageEncoding;

		bool (ImExImpl::*isOriginEligibleImpl)(const DataModel::Origin* origin);
		bool (ImExImpl::*filterMagnitudeImpl)(const DataModel::Origin* origin);
		void (ImExImpl::*sendMessageImpl)(const std::string& destination, Core::Message* message);

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namepsace Applications
} // namespace Seiscomp

#endif
