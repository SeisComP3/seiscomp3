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


#define SEISCOMP_COMPONENT QL2SC

#include "app.h"

#include <seiscomp3/system/environment.h>

#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/journalentry.h>
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
using namespace Seiscomp::DataModel;


namespace al = boost::algorithm;
namespace io = boost::iostreams;


namespace Seiscomp {
namespace QL2SC {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


typedef Diff3::PropertyIndex PropertyIndex;


PropertyIndex CreationInfoIndex;


Core::Time getLastModificationTime(const CreationInfo &ci) {
	try {
		return ci.modificationTime();
	}
	catch ( ... ) {
		try {
			return ci.creationTime();
		}
		catch ( ... ) {}
	}

	return Core::Time();
}


template <typename T>
Core::Time getLastModificationTime(const T *o) {
	try {
		return getLastModificationTime(o->creationInfo());
	}
	catch ( ... ) {}

	return Core::Time();
}


class MyDiff : public Diff3 {
	public:
		MyDiff() {}

	protected:
		bool blocked(const Core::BaseObject *o, LogNode *node, bool local) {
			const Core::MetaProperty *prop = 0;
			PropertyIndex::const_iterator it = CreationInfoIndex.find(o->className());
			if ( it == CreationInfoIndex.end() ) {
				if ( o->meta() )
					prop = o->meta()->property("creationInfo");
			}
			else
				prop = it->second;

			if ( !prop ) return false;

			string agencyID;

			try {
				Core::MetaValue v = prop->read(o);
				Core::BaseObject *bo = boost::any_cast<Core::BaseObject*>(v);
				CreationInfo *ci = CreationInfo::Cast(bo);
				if ( ci ) {
					agencyID = ci->agencyID();

					if ( !SCCoreApp->isAgencyIDAllowed(agencyID) ) {
						if ( node && node->level() >= LogNode::DIFFERENCES )
							node->addChild(o2t(o), "SKIP (" + string(local?"local":"remote") +
							               " agency '" + agencyID + "' blocked)");
						return true;
					}
				}
			}
			catch ( ... ) {}

			return false;
		}

		bool confirmUpdate(const Core::BaseObject *localO,
		                   const Core::BaseObject *remoteO,
		                   LogNode *node) {
			const Core::MetaProperty *prop = 0;
			PropertyIndex::const_iterator it = CreationInfoIndex.find(localO->className());
			if ( it == CreationInfoIndex.end() ) {
				if ( localO->meta() ) {
					prop = localO->meta()->property("creationInfo");
					CreationInfoIndex[localO->className()] = prop;
				}
			}
			else
				prop = it->second;

			// No creationInfo, no creationTime check possible
			if ( !prop ) return true;

			try {
				Core::MetaValue v;
				CreationInfo *ciLocal, *ciRemote;

				v = prop->read(localO);
				ciLocal = CreationInfo::Cast(boost::any_cast<Core::BaseObject*>(v));

				v = prop->read(remoteO);
				ciRemote = CreationInfo::Cast(boost::any_cast<Core::BaseObject*>(v));

				if ( ciLocal && ciRemote ) {
					Core::Time localMTime = getLastModificationTime(*ciLocal);
					Core::Time remoteMTime = getLastModificationTime(*ciRemote);

					if ( remoteMTime < localMTime ) {
						if ( node && node->level() >= LogNode::OPERATIONS )
							node->setMessage("SKIP UPDATE due to modification time");
						return false;
					}
				}
			}
			catch ( ... ) {}

			return true;
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


bool loadEventParam(EventParametersPtr &ep, const string &data,
                    bool gzip = false) {
	bool retn = false;
	bool registrationEnabled = PublicObject::IsRegistrationEnabled();
	PublicObject::SetRegistrationEnabled(false);
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
	PublicObject::SetRegistrationEnabled(registrationEnabled);
	return retn;
}


/** Adds all PublicObjects to a cache */
class SC_SYSTEM_CORE_API PublicObjectCacheFeeder : protected Visitor {
	public:
		PublicObjectCacheFeeder(PublicObjectCache &cache)
		 : _cache(cache), _root(NULL) {}

		void feed(Object *o, bool skipRoot = false) {
			_root = skipRoot ? o : NULL;
			if ( o != NULL )
				o->accept(this);
		}

	private:
		bool visit(PublicObject* po) {
			if ( _root && _root == po ) // skip root node
				return true;
			_cache.feed(po);
			return true;
		}

		void visit(Object* o) {}

	private:
		PublicObjectCache &_cache;
		Object *_root;
};


/** Recursively resolves routing for a given object */
bool checkRouting(const Object *o, const RoutingTable &routing) {
	if ( !o ) return false;

	RoutingTable::const_iterator it = routing.find(o->typeInfo().className());
	if ( it != routing.end() )
		return !it->second.empty();

	return checkRouting(o->parent(), routing);
}


bool resolveRouting(string &result, const Object *o, const RoutingTable &routing) {
	if ( !o ) return false;

	RoutingTable::const_iterator it = routing.find(o->typeInfo().className());
	if ( it != routing.end() ) {
		result = it->second;
		return !result.empty();
	}

	return resolveRouting(result, o->parent(), routing);
}


JournalEntryPtr getLastJournalEntry(DatabaseQuery &query, const string &eventID,
                                    const string &action) {
	JournalEntryPtr journalEntry;
	DatabaseIterator it = query.getJournalAction(eventID, action);
	for ( ; it.valid(); ++it ) {
		JournalEntryPtr tmp = JournalEntry::Cast(it.get());
		if ( !journalEntry )
			journalEntry = tmp;
		else {
			try {
				if ( journalEntry->created() < tmp->created() )
					journalEntry = tmp;
			}
			catch ( ... ) {
				// No creation time information, take the latest with respect
				// to database order
				journalEntry = tmp;
			}
		}
	}
	it.close();
	return journalEntry;
}


template <typename T>
bool checkUpdateOnTimeStamps(const T *remoteO, const T *localO) {
	Core::Time remoteTS = getLastModificationTime(remoteO);
	Core::Time localTS = getLastModificationTime(localO);

	if ( remoteTS.valid() && localTS.valid() ) {
		return remoteTS > localTS;
	}

	if ( remoteTS.valid() )
		return true;

	return false;
}


} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::App(int argc, char **argv)
: Client::Application(argc, argv)
, _waitForEventIDTimeout(0)
, _test(false) {
	setDatabaseEnabled(true, true);
	setMessagingEnabled(true);
	setPrimaryMessagingGroup("EVENT");
	addMessagingSubscription("EVENT");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::~App() {
	for ( QLClients::iterator it = _clients.begin(); it != _clients.end(); ++it ) {
		if ( *it != NULL ) {
			delete *it;
			*it = NULL;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::createCommandLineDescription() {
	commandline().addGroup("Mode");
	commandline().addOption("Mode", "test", "Do not send messages, just log output");
	commandline().addOption("Mode", "ep", "Check differences with given XML file and exit, can be given more than once", &_ep);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::init() {
	if ( !Client::Application::init() )
		return false;

	_test = commandline().hasOption("test");

	if ( !_config.init() )
		return false;

	int notificationID = -2;
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

	_cache.setDatabaseArchive(query());

	enableTimer(1);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::run() {
	if ( _ep.empty() ) {
		for ( QLClients::iterator it = _clients.begin();
		      it != _clients.end(); ++it )
			(*it)->run();

		return Client::Application::run();
	}
	else {
		for ( size_t ei = 0; ei < _ep.size(); ++ei ) {
			SEISCOMP_INFO("---- %s", _ep[ei].c_str());

			// Offline processing
			EventParametersPtr ep;
			Notifiers notifiers, journals;
			LogNodePtr logNode;

			bool registrationEnabled = PublicObject::IsRegistrationEnabled();
			PublicObject::SetRegistrationEnabled(false);

			IO::XMLArchive ar;
			if ( !ar.open(_ep[ei].c_str()) ) {
				cerr << "Failed to open " << _ep[ei] << endl;
				return false;
			}

			ar >> ep;
			ar.close();

			PublicObject::SetRegistrationEnabled(registrationEnabled);

			// log node is enabled for notice and debug level
			if ( _verbosity > 2 )
				logNode = new LogNode(EventParameters::TypeInfo().className(),
				                      _verbosity > 3 ? LogNode::DIFFERENCES : LogNode::OPERATIONS);

			const string &epID = ep->publicID();

			for ( size_t i = 0; i < ep->pickCount(); ++i )
				diffPO(ep->pick(i), epID, notifiers, logNode.get());

			// Amplitudes
			for ( size_t i = 0; i < ep->amplitudeCount(); ++i )
				diffPO(ep->amplitude(i), epID, notifiers, logNode.get());

			// Origins
			for ( size_t i = 0; i < ep->originCount(); ++i )
				diffPO(ep->origin(i), epID, notifiers, logNode.get());

			// FocalMechanisms
			for ( size_t i = 0; i < ep->focalMechanismCount(); ++i )
				diffPO(ep->focalMechanism(i), epID, notifiers, logNode.get());

			// Events
			for ( size_t i = 0; i < ep->eventCount(); ++i )
				diffPO(ep->event(i), epID, notifiers, logNode.get());

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

			// No event routing, forward event attributes
			for ( size_t i = 0; i < ep->eventCount(); ++i )
				syncEvent(ep.get(), ep->event(i), NULL, journals, true);

			cerr << "Notifiers: " << notifiers.size() << endl;
			cerr << "Journals: " << journals.size() << endl;

			for ( size_t i = 0; i < notifiers.size(); ++i )
				applyNotifier(notifiers[i].get());

			for ( size_t i = 0; i < journals.size(); ++i )
				applyNotifier(journals[i].get());

			ar.create("-");
			ar.setFormattedOutput(true);
			ar & NAMED_OBJECT("", notifiers);
			ar & NAMED_OBJECT("", journals);
			ar.close();
		}

		return true;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
	if ( _ep.empty() ) {
		for ( QLClients::iterator it = _clients.begin();
		      it != _clients.end(); ++it )
			(*it)->join(until);
	}

	SEISCOMP_INFO("application finished");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::feed(QLClient *client, IO::QuakeLink::Response *response) {
	_clientPublishMutex.lock();

	Client::Notification n(client->notificationID(), response);
	if ( !_queue.push(n) ) {
		_clientPublishMutex.unlock();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::dispatchNotification(int type, Core::BaseObject *obj) {
	if ( type >= 0 ) return false;
	if ( type == -1 ) return true;

	size_t index = -type - 2;
	if ( index >= _clients.size() ) return false;

	QLClient *client = _clients[index];

	IO::QuakeLink::Response *msg = IO::QuakeLink::Response::Cast(obj);
	if ( msg == NULL ) {
		SEISCOMP_ERROR("received invalid message from host '%s'",
		               client->config()->host.c_str());
		return true;
	}

	bool res = dispatchResponse(client, msg);

	// Allow new responses
	_clientPublishMutex.unlock();

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::dispatchResponse(QLClient *client, const IO::QuakeLink::Response *msg) {
	const HostConfig *config = client->config();
	const RoutingTable &routing = config->routingTable;
	RoutingTable::const_iterator rt_it;

	SEISCOMP_INFO("processing message from host '%s'", config->host.c_str());

	Notifiers notifiers;

	// log node is enabled for notice and debug level
	LogNodePtr logNode;
	if ( _verbosity > 2 )
		logNode = new LogNode(EventParameters::TypeInfo().className(),
		                      _verbosity > 3 ? LogNode::DIFFERENCES : LogNode::OPERATIONS);

	EventParametersPtr ep;

	// event remove message
	if ( msg->disposed ) {
		if ( msg->type != IO::QuakeLink::ctText ) {
			SEISCOMP_ERROR("Content-Type of message not set to text");
			return false;
		}

		// Event removal will be ignored
		return true;
	}

	// event update
	if ( msg->type != IO::QuakeLink::ctXML ) {
		SEISCOMP_ERROR("Content-Type of message not set to XML");
		return false;
	}

	if ( !loadEventParam(ep, msg->data, msg->gzip) )
		return false;

	const string &epID = ep->publicID();

	// check if routing for EventParameters exists
	string epRouting;
	rt_it = routing.find(ep->typeInfo().className());
	if ( rt_it != routing.end() ) epRouting = rt_it->second;

	// Picks
	if ( !epRouting.empty() ||
		 routing.find(Pick::TypeInfo().className()) != routing.end() ) {
		for ( size_t i = 0; i < ep->pickCount(); ++i )
			diffPO(ep->pick(i), epID, notifiers, logNode.get());
	}

	// Amplitudes
	if ( !epRouting.empty() ||
		 routing.find(Amplitude::TypeInfo().className()) != routing.end() ) {
		for ( size_t i = 0; i < ep->amplitudeCount(); ++i )
			diffPO(ep->amplitude(i), epID, notifiers, logNode.get());
	}

	// Origins
	if ( !epRouting.empty() ||
		 routing.find(Origin::TypeInfo().className()) != routing.end() ) {
		for ( size_t i = 0; i < ep->originCount(); ++i )
			diffPO(ep->origin(i), epID, notifiers, logNode.get());
	}

	// FocalMechanisms
	if ( !epRouting.empty() ||
		 routing.find(FocalMechanism::TypeInfo().className()) != routing.end() ) {
		for ( size_t i = 0; i < ep->focalMechanismCount(); ++i )
			diffPO(ep->focalMechanism(i), epID, notifiers, logNode.get());
	}

	// Events
	if ( !epRouting.empty() ||
		 routing.find(Event::TypeInfo().className()) != routing.end() ) {
		RoutingTable::const_iterator it;
		it = routing.find(Event::TypeInfo().className());
		if ( it != routing.end() && !it->second.empty() ) {
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

	if ( !_test ) {
		if ( sendNotifiers(notifiers, routing) ) {
			if ( config->syncEventAttributes ){
				Notifiers journals;
				// No event routing, forward event attributes
				for ( size_t i = 0; i < ep->eventCount(); ++i )
					syncEvent(ep.get(), ep->event(i), &routing,
					          journals, config->syncPreferred);
				sendJournals(journals);
			}
			client->setLastUpdate(msg->timestamp);
			writeLastUpdates();
			return true;
		}
	}
	else {
		for ( Notifiers::const_iterator it = notifiers.begin();
		      it != notifiers.end(); ++it) {
			Notifier *n = it->get();
			if ( n->object() )
				applyNotifier(n);
		}
		client->setLastUpdate(msg->timestamp);
		writeLastUpdates();
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::addObject(const string& parentID, Object *obj) {
	PublicObject *po = PublicObject::Cast(obj);
	if ( po ) {
		Event *event = Event::Cast(po);
		if ( !event )
			_cache.feed(po);
	}
	else if ( !_waitForEventIDOriginID.empty() ) {
		OriginReference *ref = OriginReference::Cast(obj);
		if ( ref ) {
			if ( ref->originID() == _waitForEventIDOriginID )
				originAssociatedWithEvent(parentID, ref->originID());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::updateObject(const std::string& parentID, DataModel::Object *obj) {
	addObject(parentID, obj);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::removeObject(const string& parentID, Object *obj) {
	PublicObject *po = PublicObject::Cast(obj);
	if ( po )
		_cache.remove(po);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::handleTimeout() {
	if ( _waitForEventIDTimeout > 0 ) {
		--_waitForEventIDTimeout;
		if ( _waitForEventIDTimeout <= 0 ) {
			_waitForEventIDOriginID = string();
			// Just wake up the event queue
			sendNotification(Client::Notification(-1));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <class T>
void App::diffPO(T *remotePO, const string &parentID, Notifiers &notifiers,
                 LogNode *logNode) {
	if ( remotePO == NULL ) return;

	// search corresponding object in cache
	typename Core::SmartPointer<T>::Impl localPO;
	localPO = T::Cast(_cache.find(remotePO->typeInfo(), remotePO->publicID()));

	// if object was not found in cache but loaded from database, all of its
	// child objects have to be loaded too
	if ( localPO && !_cache.cached() && query() ) {
		query()->load(localPO.get());
		PublicObjectCacheFeeder(_cache).feed(localPO.get(), true);
	}

	MyDiff diff;
	diff.diff(localPO.get(), remotePO, parentID, notifiers, logNode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::originAssociatedWithEvent(const std::string &eventID,
                                    const std::string &originID) {
	_waitForEventIDOriginID = string();
	_waitForEventIDTimeout = 0;
	_waitForEventIDResult = eventID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string App::waitForEventAssociation(const std::string &originID, int timeout) {
	// Setup state variables
	_waitForEventIDOriginID = originID;
	_waitForEventIDResult = string();
	// Wait a least 10 seconds for event association
	_waitForEventIDTimeout = timeout;

	while ( !_waitForEventIDOriginID.empty() ) {
		if ( !waitEvent() ) break;
		if ( !_waitForEventIDResult.empty() ) {
			return _waitForEventIDResult;
		}

		idle();
	}

	return string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
JournalEntry *App::createJournalEntry(const string &id, const string &action, const string &params ) {
	JournalEntry *entry = new JournalEntry;
	entry->setCreated(Core::Time::GMT());
	entry->setObjectID(id);
	entry->setSender(author());
	entry->setAction(action);
	entry->setParameters(params);
	return entry;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::syncEvent(const EventParameters *ep, const Event *event,
                    const RoutingTable *routing, Notifiers &notifiers,
                    bool syncPreferred) {
	if ( !query() ) {
		SEISCOMP_ERROR("No database query available for event attribute synchronization");
		return;
	}

	EventPtr targetEvent = query()->getEvent(event->preferredOriginID());
	if ( !targetEvent ) {
		SEISCOMP_DEBUG("No event found for origin %s, need to wait",
		               event->preferredOriginID().c_str());
		string eventID = waitForEventAssociation(event->preferredOriginID(),
		                                         _config.maxWaitForEventIDTimeout);
		if ( eventID.empty() ) {
			SEISCOMP_ERROR("Event association timeout reached, skipping event synchronisation for input event %s",
			               event->publicID().c_str());
			return;
		}

		SEISCOMP_DEBUG("Origin %s has been associated with event %s",
		               event->preferredOriginID().c_str(), eventID.c_str());

		targetEvent = static_cast<Event*>(query()->getObject(Event::TypeInfo(), eventID));
		if ( !targetEvent ) {
			SEISCOMP_ERROR("Failed to read target event %s from database, skipping event synchronisation for input event %s",
			               eventID.c_str(), event->publicID().c_str());
			return;
		}
	}

	query()->loadComments(targetEvent.get());
	query()->loadEventDescriptions(targetEvent.get());

	SEISCOMP_INFO("Sync with event %s", targetEvent->publicID().c_str());

	JournalEntryPtr entry;

	// Preferred origin
	if ( syncPreferred ) {
		if ( targetEvent->preferredOriginID() != event->preferredOriginID() ) {
			Origin *org = ep->findOrigin(event->preferredOriginID());
			if ( org ) {
				if ( !routing || checkRouting(org, *routing) ) {
					SEISCOMP_DEBUG("* check update of preferred origin: '%s'",
					               org->publicID().c_str());

					entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvPrefOrgID");
					if ( !entry || entry->sender() == author() ) {
						entry = createJournalEntry(targetEvent->publicID(), "EvPrefOrgID", org->publicID());
						notifiers.push_back(
							new Notifier("Journaling", OP_ADD, entry.get())
						);
					}
					else {
						SEISCOMP_INFO("* skipping fixing preferred origin because it "
						              "has been fixed already by %s",
						              entry->sender().c_str());
					}
				}
				else
					SEISCOMP_DEBUG("* origins are not being routed, skip fixing it");
			}
			else
				SEISCOMP_WARNING("* preferred origin not found in input document, skip fixing it");
		}

		if ( targetEvent->preferredFocalMechanismID() != event->preferredFocalMechanismID() ) {
			FocalMechanism *fm = ep->findFocalMechanism(event->preferredFocalMechanismID());
			if ( fm ) {
				if ( !routing || checkRouting(fm, *routing) ) {
					SEISCOMP_DEBUG("* check update of preferred focal mechanism: '%s'",
					               fm->publicID().c_str());
					entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvPrefFocMecID");
					if ( !entry || entry->sender() == author() ) {
						entry = createJournalEntry(targetEvent->publicID(), "EvPrefFocMecID", fm->publicID());
						notifiers.push_back(
							new Notifier("Journaling", OP_ADD, entry.get())
						);
					}
					else {
						SEISCOMP_INFO("* skipping fixing preferred focal mechanism because it "
						              "has been fixed already by %s",
						              entry->sender().c_str());
					}
				}
				else
					SEISCOMP_DEBUG("* focalMechanisms are not being routed, skip fixing it");
			}
			else
				SEISCOMP_WARNING("* preferred origin not found in input document, skip fixing it");
		}

		if ( targetEvent->preferredMagnitudeID() != event->preferredMagnitudeID() ) {
			Magnitude *prefMag = 0;
			for ( size_t i = 0; i < ep->originCount(); ++i ) {
				Origin *org = ep->origin(i);
				for ( size_t j = 0; j < org->magnitudeCount(); ++j ) {
					Magnitude *mag = org->magnitude(j);
					if ( mag->publicID() == event->preferredMagnitudeID() ) {
						prefMag = mag;
						i = ep->originCount();
						break;
					}
				}
			}

			if ( prefMag ) {
				SEISCOMP_DEBUG("* check update of preferred magnitude type: '%s'",
				               prefMag->type().c_str());
				entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvPrefMagType");
				if ( !entry || entry->sender() == author() ) {
					entry = createJournalEntry(targetEvent->publicID(), "EvPrefMagType", prefMag->type());
					notifiers.push_back(
						new Notifier("Journaling", OP_ADD, entry.get())
					);
				}
				else {
					SEISCOMP_INFO("* skipping setting preferred magnitude type because it "
					              "has been set already by %s",
					              entry->sender().c_str());
				}
			}
			else {
				SEISCOMP_WARNING("* preferred magnitude not found in input document, skip fixing the type");
			}
		}
	}

	// Event type
	{
		OPT(EventType) et, targetEt;

		try { et = event->type(); } catch ( ... ) {}
		try { targetEt = targetEvent->type(); } catch ( ... ) {}

		if ( et != targetEt ) {
			SEISCOMP_DEBUG("* check update of event type: '%s'", et ? et->toString() : "");

			entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvType");
			if ( !entry || entry->sender() == author() ) {
				entry = createJournalEntry(targetEvent->publicID(), "EvType", et ? et->toString() : "");
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
			else {
				SEISCOMP_INFO("* skipping event type update because it "
				              "has been set already by %s",
				              entry->sender().c_str());
			}
		}
	}

	// Event type certainty
	{
		OPT(EventTypeCertainty) etc, targetEtc;

		try { etc = event->typeCertainty(); } catch ( ... ) {}
		try { targetEtc = targetEvent->typeCertainty(); } catch ( ... ) {}

		if ( etc != targetEtc ) {
			SEISCOMP_DEBUG("* check update of event type certainty: '%s'", etc ? etc->toString() : "");

			entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvTypeCertainty");
			if ( !entry || entry->sender() == author() ) {
				entry = createJournalEntry(targetEvent->publicID(), "EvTypeCertainty", etc ? etc->toString() : "");
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
			else {
				SEISCOMP_INFO("* skipping event type certainty update because it "
				              "has been set already by %s",
				              entry->sender().c_str());
			}
		}
	}

	// Event name
	{
		EventDescription *desc = event->eventDescription(EventDescriptionIndex(EARTHQUAKE_NAME));
		EventDescription *targetDesc = targetEvent->eventDescription(EventDescriptionIndex(EARTHQUAKE_NAME));

		if ( desc ) {
			if ( !targetDesc || desc->text() != targetDesc->text() ) {
				SEISCOMP_DEBUG("* check update of event name: '%s'", desc->text().c_str());

				JournalEntryPtr entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvName");
				if ( !entry || entry->sender() == author() ) {
					entry = createJournalEntry(targetEvent->publicID(), "EvName", desc->text());
					notifiers.push_back(
						new Notifier("Journaling", OP_ADD, entry.get())
					);
				}
				else {
					SEISCOMP_INFO("* skipping event name update because it "
					              "has been set already by %s",
					              entry->sender().c_str());
				}
			}
		}
		else {
			if ( targetDesc && !targetDesc->text().empty() ) {
				SEISCOMP_DEBUG("* check remove of event name");
				JournalEntryPtr entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvName");
				if ( !entry || entry->sender() == author() ) {
					entry = createJournalEntry(targetEvent->publicID(), "EvName", "");
					notifiers.push_back(
						new Notifier("Journaling", OP_ADD, entry.get())
					);
				}
				else {
					SEISCOMP_INFO("* skipping event name removal because it "
					              "has been set already by %s",
					              entry->sender().c_str());
				}
			}
		}
	}

	// Operator comment
	{
		Comment *cmt = event->comment(string("Operator"));
		Comment *targetCmt = targetEvent->comment(string("Operator"));
		if ( cmt ) {
			if ( !targetCmt || cmt->text() != targetCmt->text() ) {
				SEISCOMP_DEBUG("* check update of operator comment: %s", cmt->text().c_str());

				JournalEntryPtr entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvOpComment");
				if ( !entry || entry->sender() == author() ) {
					entry = createJournalEntry(targetEvent->publicID(), "EvOpComment", cmt->text());
					notifiers.push_back(
						new Notifier("Journaling", OP_ADD, entry.get())
					);
				}
				else {
					SEISCOMP_INFO("* skipping event opertor comment update because it "
					              "has been set already by %s",
					              entry->sender().c_str());
				}
			}
		}
		else {
			if ( targetCmt && !targetCmt->text().empty() ) {
				JournalEntryPtr entry = getLastJournalEntry(*query(), targetEvent->publicID(), "EvOpComment");
				if ( !entry || entry->sender() == author() ) {
					entry = createJournalEntry(targetEvent->publicID(), "EvOpComment", "");
					notifiers.push_back(
						new Notifier("Journaling", OP_ADD, entry.get())
					);
				}
				else {
					SEISCOMP_INFO("* skipping event operator comment removal because it "
					              "has been set already by %s",
					              entry->sender().c_str());
				}
			}
		}
	}

	// Comments in general
	for ( size_t i = 0; i < targetEvent->commentCount(); ++i ) {
		Comment *localCmt = targetEvent->comment(i);
		if ( localCmt->id() == "Operator" ) continue;

		SEISCOMP_DEBUG("> %s", localCmt->id().c_str());

		Comment *remoteCmt = event->comment(localCmt->id());
		if ( !remoteCmt ) {
			SEISCOMP_DEBUG("* remove comment '%s'", localCmt->id().c_str());
			// Remove comment
			notifiers.push_back(
				new Notifier(targetEvent->publicID(), OP_REMOVE, localCmt)
			);
		}
		else {
			if ( remoteCmt->text() != localCmt->text() ) {
				if ( checkUpdateOnTimeStamps(remoteCmt, localCmt) ) {
					SEISCOMP_DEBUG("* update comment '%s'", localCmt->id().c_str());
					*localCmt = *remoteCmt;
					// Update comment
					notifiers.push_back(
						new Notifier(targetEvent->publicID(), OP_UPDATE, localCmt)
					);
				}
				else {
					SEISCOMP_INFO("* skipping update for comment '%s' because the "
					              "local counterpart was updated later",
					              localCmt->id().c_str());
				}
			}
		}
	}

	for ( size_t i = 0; i < event->commentCount(); ++i ) {
		Comment *remoteCmt = event->comment(i);
		if ( remoteCmt->id() == "Operator" ) continue;

		SEISCOMP_DEBUG("< %s", remoteCmt->id().c_str());

		Comment *localCmt = targetEvent->comment(remoteCmt->id());
		if ( !localCmt ) {
			SEISCOMP_DEBUG("* add comment '%s'", remoteCmt->id().c_str());
			// Add comment
			notifiers.push_back(
				new Notifier(targetEvent->publicID(), OP_ADD, remoteCmt)
			);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::sendNotifiers(const Notifiers &notifiers, const RoutingTable &routing) {
	if ( notifiers.empty() ) {
		SEISCOMP_INFO("no modification required");
		return true;
	}

	// statistics
	int add = 0, update = 0, remove = 0, msgTotal = 0;
	map<string, int> groupMessages;

	string group, prevGroup;
	NotifierMessagePtr nm = new NotifierMessage();
	for ( Notifiers::const_iterator it = notifiers.begin();
	      it != notifiers.end(); ++it) {
		Notifier *n = it->get();

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
			nm = new NotifierMessage();
			++groupMessages[prevGroup];
		}

		// apply notifier locally
		if ( n->object() )
			applyNotifier(n);

		prevGroup = group;
		nm->attach(n);
		switch ( n->operation() ) {
			case OP_ADD:    ++add;    break;
			case OP_UPDATE: ++update; break;
			case OP_REMOVE: ++remove; break;
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

	// Sync with messaging
	sync();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::sendJournals(const Notifiers &journals) {
	if ( journals.empty() ) {
		SEISCOMP_INFO("no journal entries queued");
		return true;
	}

	NotifierMessagePtr nm = new NotifierMessage();
	for ( Notifiers::const_iterator it = journals.begin();
	      it != journals.end(); ++it) {
		nm->attach(it->get());
	}

	if ( !connection()->send(nm.get()) )
		return false;

	SEISCOMP_INFO("send %i journal entries "
	              "to the message group: %s", int(nm->size()),
	              primaryMessagingGroup().c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::applyNotifier(const Notifier *n) {
	bool enabled = Notifier::IsEnabled();
	Notifier::SetEnabled(false);

	/*
	cerr << "Apply " << n->parentID()
	     << "  " << n->object()->className()
	     << "  " << n->operation().toString()
	     << endl;
	*/

	// parent not cached but current object is a cached public object,
	// this should not happen because the parent must have been loaded to
	// detect the diff in the first place
	PublicObject *parent = PublicObject::Find(n->parentID());

	if ( parent == NULL ) {
		PublicObject *notifierPO = PublicObject::Cast(n->object());
		if ( notifierPO && n->operation() == OP_UPDATE ) {
			PublicObject *po = PublicObject::Find(notifierPO->publicID());
			if ( po ) po->assign(notifierPO);
		}
	}
	else {
		Object *obj = n->object();

		switch ( n->operation() ) {
			case OP_ADD:
			{
				obj->detach();
				PublicObject *po = PublicObject::Cast(obj);
				if ( po && !po->registered() )
					po->registerMe();
				obj->attachTo(parent);
				addObject(n->parentID(), obj);
				break;
			}
			case OP_REMOVE:
				obj->detachFrom(parent);
				removeObject(n->parentID(), obj);
				break;
			case OP_UPDATE:
				parent->updateChild(obj);
				break;
			default:
				break;
		}
	}

	Notifier::SetEnabled(enabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns QL2SC
} // ns Seiscomp
