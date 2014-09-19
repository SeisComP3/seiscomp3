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

#include <seiscomp3/datamodel/eventparameters_package.h>


using namespace std;
using namespace Seiscomp;


typedef map<string, string> RoutingTable;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectWriter : protected DataModel::Visitor {
	// ----------------------------------------------------------------------
	//  X'struction
	// ----------------------------------------------------------------------
	public:
		ObjectWriter(Communication::Connection* connection,
		             DataModel::Operation op,
		             bool test)
		 : Visitor(op != DataModel::OP_REMOVE?DataModel::Visitor::TM_TOPDOWN:DataModel::Visitor::TM_BOTTOMUP),
		   _connection(connection),
		   _errors(0),
		   _count(0),
		   _operation(op),
		   _test(test) {}


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Does the actual writing
		bool operator()(DataModel::Object* object) {
			return (*this)(object, "");
		}

		bool operator()(DataModel::Object* object, const string& parentID) {
			cout << parentID << endl;
			_parentID = parentID;
			_errors = 0;
			_count = 0;

			object->accept(this);

			return _errors == 0;
		}

		//! Returns the number of handled objects
		int count() const { return _count; }

		//! Returns the number of errors while writing
		int errors() const { return _errors; }

		void setOperation(DataModel::Operation op) {
			_operation = op;
		}

		DataModel::Operation operation() const {
			return _operation;
		}

		void setRoutingTable(const RoutingTable &table) {
			_routingTable.clear();
			_routingTable = table;
			RoutingTable::const_iterator it = table.begin();
			for ( ; it != table.end(); it++ )
				cout << it->first << " : " << it->second << endl;
		}


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(DataModel::PublicObject* publicObject) {
			return write(publicObject);
		}

		void visit(DataModel::Object* object) {
			write(object);
		}


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		string indent(DataModel::Object *object) {
			string tmp;
			object = object->parent();
			while ( object->parent() != NULL ) {
				tmp += "  ";
				object = object->parent();
			}
			return tmp;
		}

		bool write(DataModel::Object *object) {
			if ( SCCoreApp->isExitRequested() ) return false;

			RoutingTable::iterator targetIt = _routingTable.find(object->className());
			if ( targetIt == _routingTable.end() )
				return false;

			DataModel::PublicObject *parent = object->parent();

			if ( !parent ) {
				cerr << "No parent found for object " << object->className() << endl;
				return false;
			}

			cout << char(toupper(_operation.toString()[0])) << "  "
			     << indent(object)
			     << object->className() << "(";

			DataModel::PublicObject *po = DataModel::PublicObject::Cast(object);
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

			cout << ")" << endl;

			_parentID = parent->publicID();

			++_count;

			if ( _test ) return true;

			DataModel::NotifierMessage notifierMessage;
			notifierMessage.attach(new DataModel::Notifier(_parentID, _operation, object));

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
		Communication::Connection* _connection;
		string                     _parentID;
		int                        _errors;
		int                        _count;
		DataModel::Operation       _operation;
		RoutingTable               _routingTable;
		bool                       _test;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectCounter : protected DataModel::Visitor {
	public:
		ObjectCounter(DataModel::Object* object) : DataModel::Visitor(), _count(0) {
			object->accept(this);
		}

	public:
		unsigned int count() const { return _count; }

	protected:
		bool visit(DataModel::PublicObject*) {
			++_count;
			return true;
		}

		virtual void visit(DataModel::Object*) {
			++_count;
		}

	private:
		unsigned int _count;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ObjectDispatcher : public ObjectWriter {

	// ----------------------------------------------------------------------
	//  X'struction
	// ----------------------------------------------------------------------
	public:
		ObjectDispatcher(Communication::Connection* connection,
		                 DataModel::Operation op,
		                 bool test,
		                 unsigned int total,
		                 unsigned int totalProgress)
		 : ObjectWriter(connection, op, test),
		   _total(total),
		   _totalProgress(totalProgress),
		   _lastStep(-1),
		   _failure(0) {}


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(DataModel::PublicObject* publicObject) {
			bool result = ObjectWriter::visit(publicObject);
			if ( !result )
				_failure += ObjectCounter(publicObject).count()-1;

			updateProgress();

			return result;
		}


		void visit(DataModel::Object* object) {
			ObjectWriter::visit(object);
			updateProgress();
		}


		void updateProgress() {
			unsigned int current = count() + _failure;
			unsigned int progress = current * _totalProgress / _total;
			if ( progress != _lastStep ) {
				_lastStep = progress;
				//cout << "." << flush;
			}
		}


	private:
		unsigned int _total;
		unsigned int _totalProgress;
		unsigned int _lastStep;
		unsigned int _failure;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class DispatchTool : public Seiscomp::Client::Application {
	public:
		DispatchTool(int argc, char** argv)
		 : Application(argc, argv),
		   _operation(DataModel::OP_ADD) {

			setAutoApplyNotifierEnabled(false);
			setInterpretNotifierEnabled(false);
			setMessagingEnabled(true);
			setDatabaseEnabled(false, false);
			setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
			addMessagingSubscription("");

			_routingTable.insert(make_pair(DataModel::Comment::ClassName(), "IMPORT_GROUP"));
			_routingTable.insert(make_pair(DataModel::Pick::ClassName(), "PICK"));
			_routingTable.insert(make_pair(DataModel::Amplitude::ClassName(), "AMPLITUDE"));
			_routingTable.insert(make_pair(DataModel::Arrival::ClassName(), "LOCATION"));
			_routingTable.insert(make_pair(DataModel::Origin::ClassName(), "LOCATION"));
			_routingTable.insert(make_pair(DataModel::StationMagnitude::ClassName(), "MAGNITUDE"));
			_routingTable.insert(make_pair(DataModel::StationMagnitudeContribution::ClassName(), "MAGNITUDE"));
			_routingTable.insert(make_pair(DataModel::Magnitude::ClassName(), "MAGNITUDE"));
			_routingTable.insert(make_pair(DataModel::FocalMechanism::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::MomentTensor::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::MomentTensorStationContribution::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::MomentTensorComponentContribution::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::MomentTensorPhaseSetting::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::DataUsed::ClassName(), "FOCMECH"));
			_routingTable.insert(make_pair(DataModel::EventDescription::ClassName(), "EVENT"));
			_routingTable.insert(make_pair(DataModel::FocalMechanismReference::ClassName(), "EVENT"));
			_routingTable.insert(make_pair(DataModel::OriginReference::ClassName(), "EVENT"));
			_routingTable.insert(make_pair(DataModel::Event::ClassName(), "EVENT"));
		}


	protected:
		void createCommandLineDescription() {
			commandline().addGroup("Dispatch");
			commandline().addOption("Dispatch", "input,i", "File to dispatch to messaging", &_inputFile, false);
			commandline().addOption(
				"Dispatch",
				"operation,O",
				"Notifier operation: add, update or remove",
				&_notifierOperation,
				false
			);
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
					_routingTable.insert(make_pair(entry[0], entry[1]));
				}

			}

			if ( commandline().hasOption("operation") ) {
				if ( !_operation.fromString(_notifierOperation) ) {
					cout << "Notifier operation " << _notifierOperation << " is not valid" << endl;
					cout << "Operations are add, update and remove" << endl;
					return false;
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


		virtual bool initConfiguration() {
			if ( !Application::initConfiguration() ) return false;
			return true;
		}


		virtual bool initSubscriptions() {
			return true;
		}


		bool run() {
			if ( commandline().hasOption("input") )
				return importDatabase();

			return false;
		}


		bool importDatabase() {
			IO::XMLArchive ar;
			if ( _inputFile == "-" )
				ar.open(cin.rdbuf());
			else if ( !ar.open(_inputFile.c_str()) ) {
				cout << "Error: could not open input file '" << _inputFile << "'" << endl;
				return false;
			}

			cout << "Parsing file '" << _inputFile << "'..." << endl;

			Util::StopWatch timer;
			DataModel::ObjectPtr doc;
			ar >> doc;
			ar.close();

			if ( doc == NULL ) {
				cerr << "Error: no valid object found in file '" << _inputFile << "'" << endl;
				return false;
			}

			ObjectDispatcher dispatcher(connection(), _operation, commandline().hasOption("test"), ObjectCounter(doc.get()).count(), 78);
			cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ROUTING TABLE" << endl;
			dispatcher.setRoutingTable(_routingTable);
			cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;

			unsigned int totalCount = ObjectCounter(doc.get()).count();

			cout << "Time needed to parse XML: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;
			cout << "Document object type: " << doc->className() << endl;
			cout << "Total number of objects: " << totalCount << endl;

			if ( connection() )
				cout << "Dispatching " << doc->className() << " to " << connection()->masterAddress() << endl;
			timer.restart();

			dispatcher(doc.get());
			sync();
			cout << endl;

			cout << "While dispatching " << dispatcher.count() << "/" << totalCount << " objects " << dispatcher.errors() << " errors occured" << endl;
			cout << "Time needed to dispatch " << dispatcher.count() << " objects: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;

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
		DataModel::Operation _operation;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char** argv) {
	DispatchTool app(argc, argv);
	return app.exec();
}
