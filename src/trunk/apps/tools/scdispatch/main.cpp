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

#define SEISCOMP_COMPONENT Dispatcher

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/utils/timer.h>

#include <seiscomp3/datamodel/databasearchive.h>
#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/datamodel/diff.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;


typedef map<string, string> RoutingTable;


class BaseObjectDispatcher : protected Visitor {
	// ----------------------------------------------------------------------
	//  X'struction
	// ----------------------------------------------------------------------
	public:
		BaseObjectDispatcher(Visitor::TraversalMode tm,
		                     Communication::Connection* connection, bool test)
		: Visitor(tm)
		, _connection(connection)
		, _errors(0)
		, _count(0)
		, _test(test) {}


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Does the actual writing
		virtual bool operator()(Object *object) {
			_errors = 0;
			_count = 0;
			_loggedObjects.clear();

			object->accept(this);

			return _errors == 0;
		}

		void setRoutingTable(const RoutingTable &table) {
			_routingTable.clear();
			_routingTable = table;
		}

		//! Returns the number of handled objects
		int count() const { return _count; }

		//! Returns the number of errors while writing
		int errors() const { return _errors; }


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		string indent(Object *object) {
			string tmp;
			while ( object->parent() != NULL ) {
				tmp += "  ";
				object = object->parent();
			}
			return tmp;
		}

		void logObject(Object *object, Operation op, const std::string &group,
		               const std::string &additionalIndent = "") {
			PublicObject *po = PublicObject::Cast(object);
			if ( po != NULL )
				_loggedObjects.insert(po->publicID());

			PublicObject *parent = object->parent();
			if ( parent != NULL ) {
				if ( _loggedObjects.find(parent->publicID()) == _loggedObjects.end() )
					logObject(parent, OP_UNDEFINED, "");
			}

			if ( op != OP_UNDEFINED )
				cout << char(toupper(op.toString()[0]));
			else
				cout << ' ';

			cout << "  " << additionalIndent << indent(object)
			     << object->className() << "(";

			if ( po != NULL )
				cout << "'" << po->publicID() << "'";
			else {
				const Core::MetaObject *meta = object->meta();
				bool first = true;
				for ( size_t i= 0; i < meta->propertyCount(); ++i ) {
					const Core::MetaProperty *prop = meta->property(i);
					if ( prop->isIndex() ) {
						if ( !first ) cout << ",";
						if ( prop->type() == "string" )
							cout << "'" << prop->readString(object) << "'";
						else
							cout << prop->readString(object);
						first = false;
					}
				}
			}

			cout << ")";
			if ( !group.empty() )
				cout << " : " << group;
			cout << endl;
		}


	// ----------------------------------------------------------------------
	//  Protected members
	// ----------------------------------------------------------------------
	protected:
		Communication::Connection *_connection;
		int                        _errors;
		int                        _count;
		RoutingTable               _routingTable;
		bool                       _test;
		std::set<std::string>      _loggedObjects;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectDispatcher : public BaseObjectDispatcher {
	// ----------------------------------------------------------------------
	//  X'struction
	// ----------------------------------------------------------------------
	public:
		ObjectDispatcher(Communication::Connection *connection,
		                 Operation op, bool test)
		: BaseObjectDispatcher(op != OP_REMOVE?Visitor::TM_TOPDOWN:Visitor::TM_BOTTOMUP,
		                       connection, test)
		, _operation(op) {}


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool operator()(Object *object) {
			_parentID = "";
			return BaseObjectDispatcher::operator()(object);
		}

		void setOperation(Operation op) {
			_operation = op;
		}

		Operation operation() const {
			return _operation;
		}


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(PublicObject* publicObject) {
			return write(publicObject);
		}

		void visit(Object* object) {
			write(object);
		}


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool write(Object *object) {
			if ( SCCoreApp->isExitRequested() ) return false;

			PublicObject *parent = object->parent();

			if ( !parent ) {
				cerr << "No parent found for object " << object->className() << endl;
				return false;
			}

			RoutingTable::iterator targetIt = _routingTable.find(object->className());
			PublicObject *p = parent;
			while ( (targetIt == _routingTable.end()) && (p != NULL) ) {
				targetIt = _routingTable.find(p->className());
				p = p->parent();
			}

			if ( targetIt == _routingTable.end() ) {
				cerr << "! No routing for " << object->className() << endl;
				return false;
			}

			logObject(object, _operation, targetIt->second);

			_parentID = parent->publicID();

			++_count;

			if ( _test ) return true;

			NotifierMessage notifierMessage;
			notifierMessage.attach(new Notifier(_parentID, _operation, object));

			unsigned int counter = 0;
			while ( counter <= 4 ) {
				if ( _connection->send(targetIt->second, &notifierMessage) ) {
					if ( _count % 100 == 0 ) SCCoreApp->sync();
					return true;
				}

				cerr << "Could not send object " << object->className()
				<< " to " << targetIt->second << "@" << _connection->masterAddress()
				<< endl;
				if ( _connection->isConnected() ) break;
				++counter;
				sleep(1);
			}

			++_errors;
			return false;
		}


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		string    _parentID;
		Operation _operation;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectMerger : public BaseObjectDispatcher {
	// ----------------------------------------------------------------------
	//  X'struction
	// ----------------------------------------------------------------------
	public:
		ObjectMerger(Communication::Connection *connection,
		             DatabaseReader *db, bool test, bool allowRemove)
		: BaseObjectDispatcher(Visitor::TM_TOPDOWN, connection, test)
		, _db(db)
		, _msgCount(0)
		, _allowRemove(allowRemove)
		{}


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		bool operator()(Object *object) {
			_msgCount = 0;

			bool ret = BaseObjectDispatcher::operator()(object);

			flush();
			return ret;
		}


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(PublicObject *po) {
			// Each PublicObject in a single message
			flush();

			PublicObject *parent = po->parent();

			if ( !parent ) {
				cerr << "! No parent found for object " << po->className() << endl;
				return false;
			}

			if ( SCCoreApp->isExitRequested() ) return false;

			RoutingTable::iterator targetIt = _routingTable.find(po->className());
			PublicObject *p = parent;
			while ( (targetIt == _routingTable.end()) && (p != NULL) ) {
				targetIt = _routingTable.find(p->className());
				p = p->parent();
			}

			if ( targetIt == _routingTable.end() ) {
				cerr << "! No routing for " << po->className() << endl;
				return false;
			}

			_targetGroup = targetIt->second;

			PublicObjectPtr stored = _db->loadObject(po->typeInfo(), po->publicID());

			if ( !stored ) {
				write(parent, po, OP_ADD);
				return true;
			}

			string storedParent = _db->parentPublicID(stored.get());
			if ( storedParent != parent->publicID() ) {
				// Instead of losing information due to a re-parent
				// we just create a new publicID and so a copy of the underlying
				// object
				PublicObject::GenerateId(po);
				write(parent, po, OP_ADD);
				return true;
			}


			std::vector<NotifierPtr> diffs;
			Diff2 diff;
			diff.diff(stored.get(), po, parent->publicID(), diffs);

			// All equal
			if ( diffs.empty() ) {
				return false;
			}

			_inputIndent = indent(po);

			for ( size_t i = 0; i < diffs.size(); ++i ) {
				Notifier *n = diffs[i].get();

				if ( !_allowRemove && (n->operation() == OP_REMOVE) )
					// Block removes if requested
					continue;

				po = PublicObject::Cast(n->object());
				if ( po != NULL ) flush();
				write(n->object()->parent(), n->object(), n->operation());
			}

			_inputIndent.clear();

			return false;
		}

		// Do nothing: objects are handled during diffing the public object
		// tree.
		void visit(Object *obj) {
			PublicObject *parent = obj->parent();

			if ( !parent ) {
				cerr << "! No parent found for object " << obj->className() << endl;
				return;
			}

			write(parent, obj, OP_ADD);
		}


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool write(PublicObject *parent, Object *object, Operation op) {
			++_count;

			logObject(object, op, _targetGroup);

			if ( !_msg ) _msg = new NotifierMessage;

			_msg->attach(new Notifier(parent->publicID(), op, object));

			return false;
		}

		void flush() {
			if ( !_msg ) return;
			if ( _test ) {
				SEISCOMP_DEBUG("Would send %d notifiers to %s group",
				               _msg->size(), _targetGroup.c_str());
				++_msgCount;
				_msg = NULL;
				return;
			}

			SEISCOMP_DEBUG("Send %d notifiers to %s group",
			               _msg->size(), _targetGroup.c_str());

			while ( !SCCoreApp->isExitRequested() ) {
				if ( _connection->send(_targetGroup.c_str(), _msg.get()) ) {
					_msg = NULL;
					++_msgCount;
					if ( _msgCount % 100 == 0 ) SCCoreApp->sync();
					return;
				}

				cerr << "Could not send message "
				     << " to " << _targetGroup << "@" << _connection->masterAddress()
				     << endl;

				sleep(1);
			}

			_msg = NULL;
			++_errors;
		}

		void logObject(Object *object, Operation op, const std::string &group) {
			if ( op != OP_REMOVE )
				BaseObjectDispatcher::logObject(object, op, group);
			else
				BaseObjectDispatcher::logObject(object, op, group, _inputIndent);
		}


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		DatabaseReader     *_db;
		NotifierMessagePtr  _msg;
		int                 _msgCount;
		string              _targetGroup;
		string              _inputIndent;
		bool                _allowRemove;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectCounter : protected Visitor {
	public:
		ObjectCounter(Object* object) : Visitor(), _count(0) {
			object->accept(this);
		}

	public:
		unsigned int count() const { return _count; }

	protected:
		bool visit(PublicObject*) {
			++_count;
			return true;
		}

		virtual void visit(Object*) {
			++_count;
		}

	private:
		unsigned int _count;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class DispatchTool : public Seiscomp::Client::Application {
	public:
		DispatchTool(int argc, char** argv)
		: Application(argc, argv)
		, _notifierOperation("merge")
		, _operation(OP_UNDEFINED) {

			setAutoApplyNotifierEnabled(false);
			setInterpretNotifierEnabled(false);
			setMessagingEnabled(true);
			setDatabaseEnabled(true, true);
			setLoggingToStdErr(true);
			setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
			addMessagingSubscription("");

			_routingTable.insert(make_pair(Pick::ClassName(), "PICK"));
			_routingTable.insert(make_pair(Amplitude::ClassName(), "AMPLITUDE"));
			_routingTable.insert(make_pair(Origin::ClassName(), "LOCATION"));
			_routingTable.insert(make_pair(StationMagnitude::ClassName(), "MAGNITUDE"));
			_routingTable.insert(make_pair(Magnitude::ClassName(), "MAGNITUDE"));
			_routingTable.insert(make_pair(FocalMechanism::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(Event::ClassName(), "EVENT"));
		}


	protected:
		void createCommandLineDescription() {
			commandline().addGroup("Dispatch");
			commandline().addOption("Dispatch", "input,i", "File to dispatch to messaging", &_inputFile, false);
			commandline().addOption("Dispatch", "operation,O", "Notifier operation: add, update, remove, merge or merge-without-remove",
			                        &_notifierOperation, true);
			commandline().addOption("Dispatch", "routingtable",
			                        "Specify routing table as list of object:group pairs",
			                        &_routingTableStr, false);

			commandline().addOption("Dispatch", "print-objects", "Print names of routable objects");
			commandline().addOption("Dispatch", "print-routingtable", "Print routing table");
			commandline().addOption("Dispatch", "test", "Do not send any object");
		}


		bool validateParameters() {
			if ( commandline().hasOption("routingtable") ) {
				vector<string> tableEntries;
				Core::split(tableEntries, _routingTableStr.c_str(), ",", false);

				vector<string>::iterator it = tableEntries.begin();
				_routingTable.clear();
				for ( ; it != tableEntries.end(); it++ ) {
					vector<string> entry;
					Core::split(entry, (*it).c_str(), ":", false);
					if ( entry.size() == 2 )
						_routingTable.insert(make_pair(entry[0], entry[1]));
				}
			}

			if ( _notifierOperation != "merge" && _notifierOperation != "merge-without-remove" )
				setDatabaseEnabled(false, false);

			if ( commandline().hasOption("operation") ) {
				if ( _notifierOperation != "merge" && _notifierOperation != "merge-without-remove" ) {
					if ( !_operation.fromString(_notifierOperation) ) {
						cout << "Notifier operation " << _notifierOperation << " is not valid" << endl;
						cout << "Operations are add, update, remove or merge" << endl;
						return false;
					}

					if ( _operation != OP_ADD && _operation != OP_REMOVE && _operation != OP_UPDATE ) {
						cout << "Notifier operation " << _notifierOperation << " is not valid" << endl;
						cout << "Operations are add, update, remove or merge" << endl;
						return false;
					}
				}
			}
			else if ( commandline().hasOption("print-objects") ) {
				RoutingTable::iterator it;
				for ( it = _routingTable.begin(); it != _routingTable.end(); ++it )
					cout << it->first << endl;
				return false;
			}
			else if ( commandline().hasOption("print-routingtable") ) {
				RoutingTable::iterator it;
				for ( it = _routingTable.begin(); it != _routingTable.end(); ++it )
					cout << it->first << ":" << it->second << endl;
				return false;
			}

			if ( !commandline().hasOption("input") ) {
				cerr << "No input given" << endl;
				return false;
			}

			if ( commandline().hasOption("test") )
				setMessagingEnabled(false);

			return true;
		}


		virtual bool initSubscriptions() {
			return true;
		}


		bool run() {
			if ( commandline().hasOption("input") )
				return processInput();

			return false;
		}


		bool processInput() {
			PublicObject::SetRegistrationEnabled(false);

			IO::XMLArchive ar;
			if ( !ar.open(_inputFile.c_str()) ) {
				cout << "Error: could not open input file '" << _inputFile << "'" << endl;
				return false;
			}

			cout << "Parsing file '" << _inputFile << "'..." << endl;

			Util::StopWatch timer;
			ObjectPtr doc;
			ar >> doc;
			ar.close();

			PublicObject::SetRegistrationEnabled(true);

			if ( doc == NULL ) {
				cerr << "Error: no valid object found in file '" << _inputFile << "'" << endl;
				return false;
			}

			BaseObjectDispatcher *dispatcher = NULL;
			if ( _notifierOperation == "merge" )
				dispatcher = new ObjectMerger(connection(), query(), commandline().hasOption("test"), true);
			else if ( _notifierOperation == "merge-without-remove" )
				dispatcher = new ObjectMerger(connection(), query(), commandline().hasOption("test"), false);
			else
				dispatcher = new ObjectDispatcher(connection(), _operation, commandline().hasOption("test"));

			dispatcher->setRoutingTable(_routingTable);

			unsigned int totalCount = ObjectCounter(doc.get()).count();

			cout << "Time needed to parse XML: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;
			cout << "Document object type: " << doc->className() << endl;
			cout << "Total number of objects: " << totalCount << endl;

			if ( connection() )
				cout << "Dispatching " << doc->className() << " to " << connection()->masterAddress() << endl;
			timer.restart();

			(*dispatcher)(doc.get());
			sync();
			cout << endl;

			cout << "While dispatching " << dispatcher->count() << "/" << totalCount << " objects " << dispatcher->errors() << " errors occured" << endl;
			cout << "Time needed to dispatch " << dispatcher->count() << " objects: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;

			delete dispatcher;

			return true;
		}

	// ----------------------------------------------------------------------
	// Private data member
	// ----------------------------------------------------------------------
	private:
		string               _inputFile;
		string               _notifierOperation;
		string               _routingTableStr;
		RoutingTable         _routingTable;
		Operation            _operation;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv) {
	DispatchTool app(argc, argv);
	return app.exec();
}
