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
}

namespace QL2SC {

class App : public Seiscomp::Client::Application {

	public:
		App(int argc, char **argv);
		~App();
		static App* Instance();


	private:
		typedef std::vector<QLClient*> QLClients;
		typedef Seiscomp::DataModel::Diff2::Notifiers Notifiers;
		typedef Seiscomp::DataModel::Diff2::LogNode LogNode;
		typedef Seiscomp::DataModel::Diff2::LogNodePtr LogNodePtr;

		bool init();
		bool run();
		void done();

		bool dispatchNotification(int type, BaseObject *obj);
		void addObject(const std::string& parentID, Seiscomp::DataModel::Object *obj);
		void removeObject(const std::string& parentID, Seiscomp::DataModel::Object *obj);

		template <class T>
		void diffPO(T *remotePO, const std::string &parentID,
		            Notifiers &notifiers, LogNode *logNode = NULL);

		bool sendNotifiers(const Notifiers &notifiers, const RoutingTable &routing);
		void applyNotifier(const DataModel::Notifier *n);
		void readLastUpdates();
		void writeLastUpdates();

	private:
		Config                                      _config;
		QLClients                                   _clients;
		Seiscomp::DataModel::PublicObjectRingBuffer	_cache;
		std::string                                 _lastUpdateFile;
};

} // ns QL2SC
} // ns Seiscomp

#endif // __SEISCOMP_QL2SC_APP_H__
