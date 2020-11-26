/***************************************************************************
 *   Copyright (C) gempa GmbH                                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#ifndef __SEISCOMP_QL2SC_APP_H__
#define __SEISCOMP_QL2SC_APP_H__

#include "config.h"
#include "quakelink.h"

#include <seiscomp3/client/application.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/datamodel/diff.h>

#include <string>

namespace Seiscomp {

namespace DataModel {
	class FocalMechanism;
	class Magnitude;
	class Origin;
	class Event;
}

namespace IO {
	namespace QuakeLink {
		class Response;
	}
}

namespace QL2SC {


class SC_SYSTEM_CORE_API NoCache : public DataModel::PublicObjectCache {
	public:
		bool feed(DataModel::PublicObject* po) { return true; }
};


class App : public Client::Application {
	public:
		App(int argc, char **argv);
		~App();

	public:
		void feed(QLClient *client, IO::QuakeLink::Response *response);

	protected:
		typedef std::vector<QLClient*> QLClients;
		typedef DataModel::Diff2::Notifiers Notifiers;
		typedef DataModel::Diff2::LogNode LogNode;
		typedef DataModel::Diff2::LogNodePtr LogNodePtr;

		virtual void createCommandLineDescription();

		virtual bool init();
		virtual bool run();
		virtual void done();

		virtual bool dispatchNotification(int type, BaseObject *obj);
		virtual void addObject(const std::string& parentID, DataModel::Object *obj);
		virtual void updateObject(const std::string& parentID, DataModel::Object *obj);
		virtual void removeObject(const std::string& parentID, DataModel::Object *obj);
		virtual void handleTimeout();

		bool dispatchResponse(QLClient *client, const IO::QuakeLink::Response *response);

		template <class T>
		void diffPO(T *remotePO, const std::string &parentID,
		            Notifiers &notifiers, LogNode *logNode = NULL);

		void syncEvent(const DataModel::EventParameters *ep,
		               const DataModel::Event *event,
		               const RoutingTable *routing,
		               Notifiers &notifiers, bool syncPreferred);

		bool sendNotifiers(const Notifiers &notifiers, const RoutingTable &routing);
		bool sendJournals(const Notifiers &journals);
		void applyNotifier(const DataModel::Notifier *n);
		void readLastUpdates();
		void writeLastUpdates();

		DataModel::JournalEntry *createJournalEntry(const std::string &id,
		                                            const std::string &action,
		                                            const std::string &params);

		std::string waitForEventAssociation(const std::string &originID, int timeout);
		void originAssociatedWithEvent(const std::string &eventID,
		                               const std::string &originID);

	private:
		Config                   _config;
		QLClients                _clients;
		NoCache                  _cache;
		boost::mutex             _clientPublishMutex;
		std::string              _lastUpdateFile;
		std::string              _waitForEventIDOriginID;
		std::string              _waitForEventIDResult;
		int                      _waitForEventIDTimeout;
		std::vector<std::string> _ep;
		bool                     _test;
};

} // ns QL2SC
} // ns Seiscomp

#endif // __SEISCOMP_QL2SC_APP_H__
