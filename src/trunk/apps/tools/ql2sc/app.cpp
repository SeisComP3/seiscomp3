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


typedef Diff2::PropertyIndex PropertyIndex;


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
	createCreationInfoIndexRecursive(index, EventParameters::Meta());
	return index;
}


static PropertyIndex CreationInfoIndex = createCreationInfoIndex();


class MyDiff : public Diff2 {
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
				CreationInfo *ci = CreationInfo::Cast(bo);
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
bool resolveRouting(string &result, const Object *o, const RoutingTable &routing) {
	if ( !o ) return false;

	RoutingTable::const_iterator it = routing.find(o->typeInfo().className());
	if ( it != routing.end() ) {
		result = it->second;
		return !result.empty();
	}

	return resolveRouting(result, o->parent(), routing);
}

} // ns anonymous
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::App(int argc, char **argv) : Client::Application(argc, argv) {
	setDatabaseEnabled(true, true);
	setMessagingEnabled(true);
	setPrimaryMessagingGroup("EVENT");
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
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::run() {
	for ( QLClients::iterator it = _clients.begin();
	      it != _clients.end(); ++it )
		(*it)->run();

	return Client::Application::run();
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
	for ( QLClients::iterator it = _clients.begin();
	      it != _clients.end(); ++it )
		(*it)->join(until);

	SEISCOMP_INFO("application finished");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
		logNode = new LogNode(EventParameters::TypeInfo().className(),
		                      _verbosity > 3 ? LogNode::DIFFERENCES : LogNode::OPERATIONS);

	EventParametersPtr ep;

	// event remove message
	if ( msg->disposed ) {
		if ( msg->type != IO::QuakeLink::ctText ) {
			SEISCOMP_ERROR("Content-Type of message not set to text");
			return false;
		}
		// search event to delete in cache
		Event *event = Event::Cast(_cache.find(Event::TypeInfo(), msg->data));

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

		if ( !loadEventParam(ep, msg->data, msg->gzip) ) return false;

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
		Notifiers journals;
		if ( config->syncEventAttributes ){
			// No event routing, forward event attributes
			for ( size_t i = 0; i < ep->eventCount(); ++i )
				syncEvent(ep->event(i), journals);
		}

		sendJournals(journals);
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
	if ( po )
		_cache.feed(po);
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
void App::syncEvent(Event *event, Notifiers &notifiers) {
	if ( !query() ) {
		SEISCOMP_ERROR("No database query available for event attribute synchronization");
		return;
	}

	EventPtr targetEvent = query()->getEvent(event->preferredOriginID());
	if ( !targetEvent ) {
		SEISCOMP_WARNING("No event found for origin %s, skipping attribute sync",
		                 event->preferredOriginID().c_str());
		return;
	}

	query()->loadComments(targetEvent.get());
	query()->loadEventDescriptions(targetEvent.get());

	SEISCOMP_INFO("Sync with event %s", targetEvent->publicID().c_str());

	JournalEntryPtr entry;

	// Event type
	{
		OPT(EventType) et, targetEt;

		try { et = event->type(); } catch ( ... ) {}
		try { targetEt = targetEvent->type(); } catch ( ... ) {}

		if ( et != targetEt ) {
			entry = createJournalEntry(targetEvent->publicID(), "EvType", et ? et->toString() : "");
			notifiers.push_back(
				new Notifier("Journaling", OP_ADD, entry.get())
			);
		}
	}

	// Event type certainty
	{
		OPT(EventTypeCertainty) etc, targetEtc;

		try { etc = event->typeCertainty(); } catch ( ... ) {}
		try { targetEtc = targetEvent->typeCertainty(); } catch ( ... ) {}

		if ( etc != targetEtc ) {
			entry = createJournalEntry(targetEvent->publicID(), "EvTypeCertainty", etc ? etc->toString() : "");
			notifiers.push_back(
				new Notifier("Journaling", OP_ADD, entry.get())
			);
		}
	}

	// Event name
	{
		EventDescription *desc = event->eventDescription(EventDescriptionIndex(EARTHQUAKE_NAME));
		EventDescription *targetDesc = targetEvent->eventDescription(EventDescriptionIndex(EARTHQUAKE_NAME));

		if ( desc ) {
			if ( !targetDesc || desc->text() != targetDesc->text() ) {
				entry = createJournalEntry(targetEvent->publicID(), "EvName", desc->text());
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
		}
		else {
			if ( targetDesc && !targetDesc->text().empty() ) {
				entry = createJournalEntry(targetEvent->publicID(), "EvName", "");
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
		}
	}

	// Operator comment
	{
		Comment *cmt = event->comment(string("Operator"));
		Comment *targetCmt = targetEvent->comment(string("Operator"));
		if ( cmt ) {
			if ( !targetCmt || cmt->text() != targetCmt->text() ) {
				SEISCOMP_DEBUG("Update operator comment: %s", cmt->text().c_str());
				entry = createJournalEntry(targetEvent->publicID(), "EvOpComment", cmt->text());
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
		}
		else {
			if ( targetCmt && !targetCmt->text().empty() ) {
				entry = createJournalEntry(targetEvent->publicID(), "EvOpComment", "");
				notifiers.push_back(
					new Notifier("Journaling", OP_ADD, entry.get())
				);
			}
		}
	}

	// Comments in general
	for ( size_t i = 0; i < targetEvent->commentCount(); ++i ) {
		Comment *targetCmt = targetEvent->comment(i);
		if ( targetCmt->id() == "Operator" ) continue;
		SEISCOMP_DEBUG("> %s", targetCmt->id().c_str());

		Comment *cmt = event->comment(targetCmt->id());
		if ( !cmt ) {
			SEISCOMP_DEBUG("Remove comment '%s'", targetCmt->id().c_str());
			// Remove comment
			notifiers.push_back(
				new Notifier(targetEvent->publicID(), OP_REMOVE, targetCmt)
			);
		}
		else {
			if ( cmt->text() != targetCmt->text() ) {
				SEISCOMP_DEBUG("Update comment '%s'", targetCmt->id().c_str());
				*targetCmt = *cmt;
				// Update comment
				notifiers.push_back(
					new Notifier(targetEvent->publicID(), OP_UPDATE, targetCmt)
				);
			}
		}
	}

	for ( size_t i = 0; i < event->commentCount(); ++i ) {
		Comment *cmt = event->comment(i);
		if ( cmt->id() == "Operator" ) continue;
		SEISCOMP_DEBUG("< %s", cmt->id().c_str());

		Comment *targetCmt = targetEvent->comment(cmt->id());
		if ( !targetCmt ) {
			SEISCOMP_DEBUG("Add comment '%s'", cmt->id().c_str());
			// Add comment
			notifiers.push_back(
				new Notifier(targetEvent->publicID(), OP_ADD, cmt)
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
	              "to the message group:\nEVENT", int(nm->size()));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::applyNotifier(const Notifier *n) {
	bool enabled = Notifier::IsEnabled();
	Notifier::SetEnabled(false);

	// parent not cached but current object is a cached public object,
	// this should not happen because the parent must have been loaded to
	// detect the diff in the first place
	PublicObject* parent = PublicObject::Find(n->parentID());
	if ( parent == NULL ) {
		PublicObject *notifierPO = PublicObject::Cast(n->object());
		if ( notifierPO && n->operation() == OP_UPDATE ) {
			PublicObject *po = PublicObject::Find(notifierPO->publicID());
			if ( po ) po->assign(notifierPO);
		}
	}
	else {
		switch ( n->operation() ) {
			case OP_ADD:
			{
				Object *clone = n->object()->clone();
				clone->attachTo(parent);
				addObject(n->parentID(), clone);
			}
//				n->object()->setParent(NULL);
//				n->object()->attachTo(parent);
//				addObject(n->parentID(), n->object());
				break;
			case OP_REMOVE:
				n->object()->detachFrom(parent);
				removeObject(n->parentID(), n->object());
				break;
			case OP_UPDATE:
				parent->updateChild(n->object());
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
