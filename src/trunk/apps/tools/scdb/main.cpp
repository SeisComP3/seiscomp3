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


#define SEISCOMP_COMPONENT SCDBTool

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/utils/timer.h>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Communication;
using namespace Seiscomp::IO;


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


class ObjectWriter : public DataModel::DatabaseObjectWriter {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		ObjectWriter(DataModel::DatabaseArchive& archive,
		             int batchSize, unsigned int total,
		             unsigned int totalProgress)
		 : DataModel::DatabaseObjectWriter(archive, batchSize),
		   _total(total), _totalProgress(totalProgress),
		   _lastStep(0), _failure(0) {}


	// ----------------------------------------------------------------------
	//  Visitor interface
	// ----------------------------------------------------------------------
	protected:
		bool visit(DataModel::PublicObject* publicObject) {
			bool result = DataModel::DatabaseObjectWriter::visit(publicObject);
			if ( !result )
				_failure += ObjectCounter(publicObject).count()-1;

			updateProgress();

			return result;
		}

		void visit(DataModel::Object* object) {
			DataModel::DatabaseObjectWriter::visit(object);
			updateProgress();
		}

		void updateProgress() {
			unsigned int current = count() + _failure;
			unsigned int progress = current * _totalProgress / _total;
			if ( progress != _lastStep ) {
				_lastStep = progress;
				cout << "." << flush;
			}
		}

	private:
		unsigned int _total;
		unsigned int _totalProgress;
		unsigned int _lastStep;
		unsigned int _failure;
};



class DBTool : public Seiscomp::Client::Application {
	public:
		DBTool(int argc, char** argv) : Application(argc, argv) {
			setAutoApplyNotifierEnabled(false);
			setInterpretNotifierEnabled(false);
			setPrimaryMessagingGroup(Communication::Protocol::LISTENER_GROUP);
			addMessagingSubscription("*");
		}


	protected:
		void createCommandLineDescription() {
			commandline().addOption("Messaging", "mode,m", "listen mode [none, notifier, all]", &_listenMode, "notifier");
			commandline().addOption("Database", "output,o", "database connection to write to", &_databaseWriteConnection);

			commandline().addGroup("Import");
			commandline().addOption("Import", "input,i", "import file", &_importFile, false);
			commandline().addOption("Import", "batchsize,b", "batch size of database transactions [0..1000]", &_importBatchSize, 1);
		}


		bool validateParameters() {
			if ( _importBatchSize > 1000 ) {
				cout << "Error: batchsize '" << _importBatchSize << "' too large" << endl;
					return false;
			}

			if ( _listenMode != "none" ) {
				if ( _listenMode == "all" )
					_dbAllObjects = true;
				else if ( _listenMode == "notifier" )
					_dbAllObjects = false;
				else {
					cout << "Error: unknown listen mode '" << _listenMode << "'" << endl;
					return false;
				}
			}

			if ( commandline().hasOption("input") )
				setMessagingEnabled(false);

			return true;
		}


		virtual bool initConfiguration() {
			if ( !Application::initConfiguration() ) return false;

			string dbType, dbParams;
			try { _serviceRequestGroup = configGetString("connection.requestGroup"); } catch (...) {}
			try { _serviceProvideGroup = configGetString("connection.provideGroup"); } catch (...) {}
			try { dbType = configGetString("output.type"); } catch (...) {}
			try { dbParams = configGetString("output.parameters"); } catch (...) {}

			if ( !dbType.empty() && !dbParams.empty() )
				_databaseWriteConnection = dbType + "://" + dbParams;

			return true;
		}

		virtual bool initSubscriptions() {
			if ( _listenMode != "none" )
				return Application::initSubscriptions();

			return true;
		}

		virtual bool initDatabase() {
			int errorCode = -1;
			if ( _listenMode != "none" && !commandline().hasOption("input") ) {
				SEISCOMP_DEBUG("Checking database '%s'", _databaseWriteConnection.c_str());
	
				DatabaseInterfacePtr db = DatabaseInterface::Open(_databaseWriteConnection.c_str());
				if ( db == NULL ) {
					SEISCOMP_ERROR("Could not open output database '%s'", _databaseWriteConnection.c_str());
					Application::exit(errorCode);
					return false;
				}
				--errorCode;
		
				SEISCOMP_DEBUG("Database check...ok");
	
				setDatabase(db.get());
			}
			else if ( commandline().hasOption("input") )
				return Application::initDatabase();
			else {
				if ( connection() )
					connection()->subscribe(_serviceRequestGroup.c_str());
			}

			return true;
		}


		bool run() {
			if ( commandline().hasOption("input") )
				return importDatabase();

			return Application::run();
		}


		ServiceProvideMessage* createServiceResponse(ServiceRequestMessage* msg) {
			DatabaseRequestMessage* dbRequest = DatabaseRequestMessage::Cast(msg);
			if ( dbRequest == NULL ) {
				SEISCOMP_DEBUG("Could not handle message of type '%s' -> ignoring", msg->className());
				return NULL;
			}
		
			if ( !Core::isEmpty(dbRequest->service()) && _dbType != dbRequest->service() )
				return NULL;
		
			if ( _dbType.empty() || _dbParameters.empty() )
				return NULL;
		
			return new DatabaseProvideMessage(_dbType.c_str(), _dbParameters.c_str());
		}


		void handleMessage(Message* msg) {
			ServiceRequestMessage* serviceRequestMsg = ServiceRequestMessage::Cast(msg);
			if ( serviceRequestMsg ) {
				ServiceProvideMessagePtr serviceMsg = createServiceResponse(serviceRequestMsg);
				if ( serviceMsg ) {
					connection()->send(_serviceProvideGroup.c_str(), serviceMsg.get());
				}
		
				return;
			}

			DataModel::DatabaseObjectWriter writer(*query());
			for ( MessageIterator it = msg->iter(); *it != NULL; ++it ) {
				if ( _dbAllObjects ) {
					DataModel::Object* object = DataModel::Object::Cast(*it);
					if ( object != NULL ) {
						writer(object);
						continue;
					}
				}
		
				DataModel::Notifier* notifier = DataModel::Notifier::Cast(*it);
				if ( notifier != NULL && notifier->object() != NULL ) {
					DataModel::PublicObject *po = DataModel::PublicObject::Cast(notifier->object());
					if ( po != NULL )
						SEISCOMP_DEBUG("%s %s", notifier->operation().toString(), po->publicID().data());

					switch ( notifier->operation() ) {
						case DataModel::OP_ADD:
							writer(notifier->object(), notifier->parentID());
							//dbArchive->write(notifier->object(), notifier->parentID());
							break;
						case DataModel::OP_REMOVE:
							query()->remove(notifier->object(), notifier->parentID());
							break;
						case DataModel::OP_UPDATE:
							query()->update(notifier->object(), notifier->parentID());
							break;
						default:
							break;
					}
				}
			}
		}


		bool importDatabase() {
			XMLArchive ar;
			if ( _importFile == "-" )
				ar.open(std::cin.rdbuf());
			else if ( !ar.open(_importFile.c_str()) ) {
				cout << "Error: could not open input file '" << _importFile << "'" << endl;
				return false;
			}
		
			/*
			DatabaseInterfacePtr db = DatabaseInterface::Open(_databaseWriteConnection.c_str());
			if ( db == NULL ) {
				cout << "Error: could not connect to database '" << _databaseWriteConnection << "'" << endl;
				return false;
			}
			*/
		
			cout << "Parsing file '" << _importFile << "'..." << endl;
		
			Util::StopWatch timer;
			DataModel::ObjectPtr doc;
			ar >> doc;
			ar.close();
		
			if ( doc == NULL ) {
				cout << "Error: no valid object found in file '" << _importFile << "'" << endl;
				return false;
			}
		
			ObjectWriter writer(*query(), _importBatchSize, ObjectCounter(doc.get()).count(), 78);
		
			cout << "Time needed to parse XML: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;
			cout << "Document object type: " << doc->className() << endl;
			cout << "Total number of objects: " << ObjectCounter(doc.get()).count() << endl;
		
			cout << "Writing " << doc->className() << " into database" << endl;
			timer.restart();
		
			writer(doc.get());
			cout << endl;
		
			cout << "While writing " << writer.count() << " objects " << writer.errors() << " errors occured" << endl;
			cout << "Time needed to write " << writer.count() << " objects: " << Core::Time(timer.elapsed()).toString("%T.%f") << endl;

			_returnCode = writer.errors() > 0?1:0;
			return true;
		}


	private:
		std::string _importFile;
		unsigned int _importBatchSize;

		std::string _listenMode;
		bool _dbAllObjects;

		std::string _databaseWriteConnection;
		std::string _serviceRequestGroup;
		std::string _serviceProvideGroup;
};


int main(int argc, char** argv) {
	DBTool app(argc, argv);
	return app.exec();
}

