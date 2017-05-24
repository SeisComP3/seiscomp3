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


#define SEISCOMP_COMPONENT QL2SC

#include "app.h"

#include <seiscomp3/system/environment.h>

#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/files.h>

#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/version.hpp>

using namespace std;

namespace al = boost::algorithm;
namespace io = boost::iostreams;

namespace Seiscomp {
namespace QL2SC {


namespace {


typedef Seiscomp::DataModel::Diff2::PropertyIndex PropertyIndex;


// find classes providing 'creationInfo' information
void createCreationInfoIndexRecursive(PropertyIndex &index, const Core::MetaObject *meta) {
	bool found = false;
	for ( size_t i = 0; i < meta->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = meta->property(i);
		if ( !found && prop->name() == "creationInfo" ) {
			index[meta->rtti()->className()] = prop;
			found = true;
		}
		else if ( prop->isClass() ) {
			Core::BaseObject *bo = prop->createClass();
			if ( bo ) createCreationInfoIndexRecursive(index, bo->meta());
		}
	}
}

PropertyIndex createCreationInfoIndex() {
	PropertyIndex index;
	createCreationInfoIndexRecursive(index, DataModel::EventParameters::Meta());
	return index;
}


static PropertyIndex CreationInfoIndex = createCreationInfoIndex();


class MyDiff : public Seiscomp::DataModel::Diff2 {
	public:
		MyDiff() {}

	protected:
		bool blocked(const Core::BaseObject *o, LogNode *node, bool local) {
			PropertyIndex::const_iterator it = CreationInfoIndex.find(o->meta()->rtti()->className());
			if ( it == CreationInfoIndex.end() ) return false;

			string agencyID;
			try {
				Core::MetaValue v = it->second->read(o);
				Core::BaseObject *bo =boost::any_cast<Core::BaseObject*>(v);
				DataModel::CreationInfo *ci = DataModel::CreationInfo::Cast(bo);
				if ( ci ) agencyID = ci->agencyID();
			}
			catch ( ... ) {}

			if ( !SCCoreApp->isAgencyIDAllowed(agencyID) ) {
				if ( node && node->level() >= LogNode::DIFFERENCES)
					node->addChild(o2t(o), "SKIP (" + string(local?"local":"remote") +
					               " agency '" + agencyID + "' blocked)");
				return true;
			}

			return false;
		}
};



template<typename Container> class container_source {
	public:
		typedef typename Container::value_type  char_type;
		typedef io::source_tag                  category;
		container_source(const Container& container)
		 : _con(container), _pos(0) {}
		streamsize read(char_type* s, streamsize n) {
			streamsize amt = static_cast<streamsize>(_con.size() - _pos);
			streamsize result = (min)(n, amt);
			if (result != 0) {
				copy(_con.begin() + _pos, _con.begin() + _pos + result, s);
				_pos += result;
				return result;
			}
			return -1; // EOF
		}
		Container& container() { return _con; }
	private:
		typedef typename Container::size_type   size_type;
		const Container&  _con;
		size_type   _pos;
};

bool loadEventParam(DataModel::EventParametersPtr &ep, const string &data,
                    bool gzip = false) {
	bool retn = false;
	bool registrationEnabled = DataModel::PublicObject::IsRegistrationEnabled();
	DataModel::PublicObject::SetRegistrationEnabled(false);
	try {
		io::filtering_istreambuf buf;
		container_source<string> src(data);
		if ( gzip ) buf.push(io::gzip_decompressor());
		buf.push(src);

		IO::XMLArchive ar;
		if ( !ar.open(&buf) )
			SEISCOMP_ERROR("[xml] could not open stream buffer for reading");
		else {
			ar >> ep;
			ar.close();
			retn = true;
		}
	}
	catch (string &e) {
		SEISCOMP_ERROR("[xml] %s", e.c_str());
	}
	catch (exception &e) {
		SEISCOMP_ERROR("[xml] %s", e.what());
	}
	DataModel::PublicObject::SetRegistrationEnabled(registrationEnabled);
	return retn;
}


/** Adds all PublicObjects to a cache */
class SC_SYSTEM_CORE_API PublicObjectCacheFeeder : protected DataModel::Visitor {
	public:
		PublicObjectCacheFeeder(DataModel::PublicObjectCache &cache)
		 : _cache(cache), _root(NULL) {}

		void feed(DataModel::Object *o, bool skipRoot = false) {
			_root = skipRoot ? o : NULL;
			if ( o != NULL )
				o->accept(this);
		}

	private:
		bool visit(DataModel::PublicObject* po) {
			if ( _root && _root == po ) // skip root node
				return true;
			_cache.feed(po);
			return true;
		}

		void visit(DataModel::Object* o) {}

	private:
		DataModel::PublicObjectCache &_cache;
		DataModel::Object *_root;
};

/** Recursively resolves routing for a given object */
bool resolveRouting(string &result, const DataModel::Object *o, const RoutingTable &routing) {
	if ( !o ) return false;

	RoutingTable::const_iterator it = routing.find(o->typeInfo().className());
	if ( it != routing.end() ) {
		result = it->second;
		return !result.empty();
	}

	return resolveRouting(result, o->parent(), routing);
}

} // ns anonymous

App::App(int argc, char **argv) : Client::Application(argc, argv) {
	setDatabaseEnabled(true, true);
	setMessagingEnabled(true);
}

App::~App() {
	for ( QLClients::iterator it = _clients.begin(); it != _clients.end(); ++it ) {
		if ( *it != NULL ) {
			delete *it;
			*it = NULL;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Private
////////////////////////////////////////////////////////////////////////////////

bool App::init() {
	if ( !Client::Application::init() )
		return false;

	if ( !_config.init() )
		return false;

	int notificationID = -1;
	for ( HostConfigs::const_iterator it = _config.hosts.begin();
	      it != _config.hosts.end(); ++it, --notificationID ) {
		SEISCOMP_INFO("Initializing host '%s'", it->host.c_str());
		QLClient *client = new QLClient(notificationID, &*it, _config.backLog);
		_clients.push_back(client);
		if ( !client->init(it->url, it->options) ) {
			SEISCOMP_ERROR("Failed to initialize host '%s'", it->host.c_str());
			return false;
		}
	}

	// read previous update times
	string baseDir = Environment::Instance()->installDir() + "/var/lib";
	_lastUpdateFile = baseDir + "/" + name() + ".last_update";
	Util::createPath(baseDir);
	readLastUpdates();

	_cache.setBufferSize(_config.cacheSize);
	_cache.setDatabaseArchive(query());

	return true;
}

bool App::run() {
	for ( QLClients::iterator it = _clients.begin();
	      it != _clients.end(); ++it )
		(*it)->run();

	return Client::Application::run();
}

void App::done() {
	// Wait max. 10s for all threads to shutdown
	Core::Time until = Core::Time::GMT();
	until += 10.0;

//	// Request shutdown of clients
//	for ( QLClients::iterator it = _clients.begin();
//	      it != _clients.end(); ++it )
//		(*it)->abort();

	// Disconnect from messaging
	Client::Application::done();

	// Wait for threads to terminate
	for ( QLClients::iterator it = _clients.begin();
	      it != _clients.end(); ++it )
		(*it)->join(until);

	SEISCOMP_INFO("application finished");
}

bool App::dispatchNotification(int type, Core::BaseObject *obj) {
	if ( type >= 0 ) return false;
	size_t index = -type - 1;
	if ( index >= _clients.size() ) return false;
	QLClient *client = _clients[index];
	const HostConfig *config = client->config();
	const RoutingTable &routing = config->routingTable;
	RoutingTable::const_iterator rt_it;

	IO::QuakeLink::Response *msg = IO::QuakeLink::Response::Cast(obj);
	if ( msg == NULL ) {
		SEISCOMP_ERROR("received invalid message from host '%s'",
		               config->host.c_str());
		return true;
	}

	SEISCOMP_INFO("processing message from host '%s'", config->host.c_str());

	Notifiers notifiers;

	// log node is enabled for notice and debug level
	LogNodePtr logNode;
	if ( _verbosity > 2 )
		logNode = new LogNode(DataModel::EventParameters::TypeInfo().className(),
		                      _verbosity > 3 ? LogNode::DIFFERENCES : LogNode::OPERATIONS);

	// event remove message
	if ( msg->disposed ) {
		if ( msg->type != IO::QuakeLink::ctText ) {
			SEISCOMP_ERROR("Content-Type of message not set to text");
			return false;
		}
		// search event to delete in cache
		DataModel::Event *event = DataModel::Event::Cast(_cache.find(DataModel::Event::TypeInfo(), msg->data));

		// if event was not found in cache but loaded from database, all of its
		// child objects have to be loaded too
		if ( event && !_cache.cached() && query() ) {
			query()->load(event);
			PublicObjectCacheFeeder(_cache).feed(event, true);
		}

		MyDiff diff;
		diff.diff(event, NULL, "EventParameters", notifiers, logNode.get());
	}
	// event update
	else {
		if ( msg->type != IO::QuakeLink::ctXML ) {
			SEISCOMP_ERROR("Content-Type of message not set to XML");
			return false;
		}

		DataModel::EventParametersPtr ep;
		if ( !loadEventParam(ep, msg->data, msg->gzip) ) return false;

		const string &epID = ep->publicID();

		// check if routing for EventParameters exists
		string epRouting;
		rt_it = routing.find(ep->typeInfo().className());
		if ( rt_it != routing.end() ) epRouting = rt_it->second;

		// Picks
		if ( !epRouting.empty() ||
			 routing.find(DataModel::Pick::TypeInfo().className()) != routing.end() ) {
			for ( size_t i = 0; i < ep->pickCount(); ++i )
				diffPO(ep->pick(i), epID, notifiers, logNode.get());
		}

		// Amplitudes
		if ( !epRouting.empty() ||
			 routing.find(DataModel::Amplitude::TypeInfo().className()) != routing.end() ) {
			for ( size_t i = 0; i < ep->amplitudeCount(); ++i )
				diffPO(ep->amplitude(i), epID, notifiers, logNode.get());
		}

		// Origins
		if ( !epRouting.empty() ||
			 routing.find(DataModel::Origin::TypeInfo().className()) != routing.end() ) {
			for ( size_t i = 0; i < ep->originCount(); ++i )
				diffPO(ep->origin(i), epID, notifiers, logNode.get());
		}

		// FocalMechanisms
		if ( !epRouting.empty() ||
			 routing.find(DataModel::FocalMechanism::TypeInfo().className()) != routing.end() ) {
			for ( size_t i = 0; i < ep->focalMechanismCount(); ++i )
				diffPO(ep->focalMechanism(i), epID, notifiers, logNode.get());
		}

		// Events
		if ( !epRouting.empty() ||
			 routing.find(DataModel::Event::TypeInfo().className()) != routing.end() ) {
			for ( size_t i = 0; i < ep->eventCount(); ++i )
				diffPO(ep->event(i), epID, notifiers, logNode.get());
		}
	}

	// log diffs
	if ( logNode.get() && logNode->childCount() ) {
		stringstream ss;
		ss << endl;
		logNode->write(ss);
		if ( _verbosity > 3 )
			SEISCOMP_DEBUG_S(ss.str());
		else
			SEISCOMP_INFO_S(ss.str());
	}

	if ( sendNotifiers(notifiers, routing) ) {
		client->setLastUpdate(msg->timestamp);
		writeLastUpdates();
		return true;
	}
	return false;
}

void App::addObject(const string& parentID, DataModel::Object *obj) {
	DataModel::PublicObject *po = DataModel::PublicObject::Cast(obj);
	if ( po )
		_cache.feed(po);
}

void App::removeObject(const string& parentID, DataModel::Object *obj) {
	DataModel::PublicObject *po = DataModel::PublicObject::Cast(obj);
	if ( po )
		_cache.remove(po);
}

template <class T>
void App::diffPO(T *remotePO, const string &parentID, Notifiers &notifiers,
                 LogNode *logNode) {
	if ( remotePO == NULL ) return;

	// search corresponding object in cache
	T *localPO = T::Cast(_cache.find(remotePO->typeInfo(), remotePO->publicID()));

	// if object was not found in cache but loaded from database, all of its
	// child objects have to be loaded too
	if ( localPO && !_cache.cached() && query() ) {
		query()->load(localPO);
		PublicObjectCacheFeeder(_cache).feed(localPO, true);
	}

	MyDiff diff;
	diff.diff(localPO, remotePO, parentID, notifiers, logNode);
}

bool App::sendNotifiers(const Notifiers &notifiers, const RoutingTable &routing) {
	if ( notifiers.empty() ) {
		SEISCOMP_INFO("no modification required");
		return true;
	}

	// statistics
	int add = 0, update = 0, remove = 0, msgTotal = 0;
	map<string, int> groupMessages;

	string group, prevGroup;
	DataModel::NotifierMessagePtr nm = new DataModel::NotifierMessage();
	for ( Notifiers::const_iterator it = notifiers.begin();
	      it != notifiers.end(); ++it) {
		DataModel::Notifier *n = it->get();

		// check if a routing exists
		if ( !resolveRouting(group, n->object(), routing) ) continue;

		// the message has to be send if the batchSize is exceeded or the
		// message group changed
		if ( (nm->size() > 0 && group != prevGroup) ||
		     (_config.batchSize > 0 && nm->size() >= _config.batchSize) ) {
			SEISCOMP_DEBUG("sending notifier message (#%i) to group '%s'",
			               nm->size(), prevGroup.c_str());
			if ( !connection()->send(prevGroup, nm.get()) )
				return false;
			nm = new DataModel::NotifierMessage();
			++groupMessages[prevGroup];
		}

		// apply notifier locally
		if ( n->object() )
			applyNotifier(n);

		prevGroup = group;
		nm->attach(n);
		switch ( n->operation() ) {
			case DataModel::OP_ADD:    ++add;    break;
			case DataModel::OP_UPDATE: ++update; break;
			case DataModel::OP_REMOVE: ++remove; break;
			default: break;
		}
	}
	// send last message
	if ( nm->size() > 0 ) {
		if ( !connection()->send(group, nm.get()) )
			return false;
		++groupMessages[group];
	}

	if ( !groupMessages.empty() ) {
		stringstream ss;
		for ( map<string, int>::const_iterator it = groupMessages.begin();
		      it != groupMessages.end(); ++it ) {
			++msgTotal;
			ss << "  " << it->first << ": " << it->second << endl;
		}
		SEISCOMP_INFO("send %i notifer (ADD: %i, UPDATE: %i, REMOVE: %i) "
		              "to the following message groups:\n%s",
		              add + update + remove, add, update, remove,
		              ss.str().c_str());
	}
	return true;
}

void App::applyNotifier(const DataModel::Notifier *n) {
	bool enabled = DataModel::Notifier::IsEnabled();
	DataModel::Notifier::SetEnabled(false);

	// parent not cached but current object is a cached public object,
	// this should not happen because the parent must have been loaded to
	// detect the diff in the first place
	DataModel::PublicObject* parent = DataModel::PublicObject::Find(n->parentID());
	if ( parent == NULL ) {
		DataModel::PublicObject *notifierPO = DataModel::PublicObject::Cast(n->object());
		if ( notifierPO && n->operation() == DataModel::OP_UPDATE ) {
			DataModel::PublicObject *po = DataModel::PublicObject::Find(notifierPO->publicID());
			if ( po ) po->assign(notifierPO);
		}
	}
	else {
		switch ( n->operation() ) {
			case DataModel::OP_ADD:
			{
				DataModel::Object *clone = n->object()->clone();
				clone->attachTo(parent);
				addObject(n->parentID(), clone);
			}
//				n->object()->setParent(NULL);
//				n->object()->attachTo(parent);
//				addObject(n->parentID(), n->object());
				break;
			case DataModel::OP_REMOVE:
				n->object()->detachFrom(parent);
				removeObject(n->parentID(), n->object());
				break;
			case DataModel::OP_UPDATE:
				parent->updateChild(n->object());
				break;
			default:
				break;
		}
	}

	DataModel::Notifier::SetEnabled(enabled);
}

void App::readLastUpdates() {
	SEISCOMP_INFO("reading last update timestamps from file '%s'",
	              _lastUpdateFile.c_str());
	int i = 0;
	ifstream ifs(_lastUpdateFile.c_str());
	string line;
	Core::Time time;
	vector<string> toks;
	map<string, Core::Time> hostTimes;
	while ( ifs.good() && getline(ifs, line) ) {
		++i;
		if ( Core::split(toks, line.c_str(), " ") == 2 &&
		     time.fromString(toks[1].c_str(), "%FT%T.%fZ") )
			hostTimes[toks[0]] = time;
		else {
			SEISCOMP_ERROR("line %i of last update file '%s' invalid",
			               i, _lastUpdateFile.c_str());
			break;
		}
	}
	ifs.close();

	for ( QLClients::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		map<string, Core::Time>::const_iterator entry = hostTimes.find((*it)->config()->host);
		if ( entry != hostTimes.end() ) {
			SEISCOMP_DEBUG("setting last update time of host '%s' to %s",
			               entry->first.c_str(), entry->second.iso().c_str());
			(*it)->setLastUpdate(entry->second);
		}
	}
}

void App::writeLastUpdates() {
	SEISCOMP_DEBUG("writing last update times");

	// write times to temporary file
	string tmpFile = _lastUpdateFile + ".tmp";
	ofstream ofs(tmpFile.c_str(), ios::trunc);
	if ( !ofs.good() ) {
		SEISCOMP_ERROR("could not open file '%s' for writing", tmpFile.c_str());
		return;
	}

	for ( QLClients::iterator it = _clients.begin();
	      it != _clients.end() && ofs.good(); ++it )
		ofs << (*it)->config()->host << " " << (*it)->lastUpdate().iso() << endl;

	if ( !ofs.good() ) {
		SEISCOMP_ERROR("could not write to file '%s'", tmpFile.c_str());
		return;
	}
	ofs.close();

	// move temporary file
	if ( ::rename(tmpFile.c_str(), _lastUpdateFile.c_str()) )
		SEISCOMP_ERROR("Could not rename temporary file '%s' to '%s'",
		               tmpFile.c_str(), _lastUpdateFile.c_str());
}

} // ns QL2SC
} // ns Seiscomp
