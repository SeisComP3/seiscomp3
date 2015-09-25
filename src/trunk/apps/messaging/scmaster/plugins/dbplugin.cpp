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

#define SEISCOMP_COMPONENT DbPlugin
#include <seiscomp3/logging/log.h>

#include "dbplugin.h"

#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/core/plugin.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/version.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/system.h>


namespace Seiscomp {
namespace Communication {

IMPLEMENT_SC_CLASS_DERIVED(
		DbPlugin,
		MasterPluginInterface,
		"DbPlugin"
);

REGISTER_MASTER_PLUGIN_INTERFACE(DbPlugin, "dbplugin");

ADD_SC_PLUGIN(
		"Database plugin for scmaster",
		"GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 1, 0, 0
)


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DbPlugin::DbPlugin() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DbPlugin::~DbPlugin() {
	disconnectFromDb();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DbPlugin::process(NetworkMessage *nmsg, Core::Message *message) {
	if ( !message || !nmsg ) return true;

	SEISCOMP_DEBUG("Writing message to database");

	int error = 0;
	for ( Core::MessageIterator it = message->iter(); *it != NULL; ++it ) {
		DataModel::Notifier* notifier = DataModel::Notifier::Cast(*it);
		if ( notifier != NULL && notifier->object() != NULL ) {
			bool result = false;
			while ( !result ) {
				switch ( notifier->operation() ) {
					case DataModel::OP_ADD: {
						++_addedObjects;
						DataModel::DatabaseObjectWriter writer(*_dbArchive.get());
						result = writer(notifier->object(), notifier->parentID());
						if (!result) --error;
					}
						break;
					case DataModel::OP_REMOVE:
						++_removedObjects;
						result = _dbArchive->remove(notifier->object(), notifier->parentID());
						if (!result) --error;
						break;
					case DataModel::OP_UPDATE:
						++_updatedObjects;
						result = _dbArchive->update(notifier->object(), notifier->parentID());
						if (!result) --error;
						break;
					default:
						break;
				}

				if ( !result ) {
					if ( !_db->isConnected() ) {
						SEISCOMP_ERROR("Lost connection to database: %s", _dbWriteConnection.c_str());
						while ( !connectToDb() );
						if ( !operational() ) {
							SEISCOMP_INFO("Stopping database plugin");
							break;
						}
						else
							SEISCOMP_INFO("Reconnected to database: %s", _dbWriteConnection.c_str());
					}
					else {
						SEISCOMP_WARNING(
							"Error handling message from %s to %s",
							nmsg->clientName().c_str(),
							nmsg->destination().c_str()
						);

						// If no client connection error occurred -> go ahead because
						// wrong queries cannot be fixed here
						++_errors;
						result = true;
					}
				}
			}
		}
	}

	// For now we return true otherwise the master will stop because
	// e.g. an erroneous module sends the same notifier twice or more
	//return (error < 0) ? false : true;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DbPlugin::printStateOfHealthInformation(std::ostream &os) const {
	double elapsed = (double)_stopper.elapsed();
	if ( elapsed > 0.0 ) {
		double aa = _addedObjects / elapsed;
		double au = _updatedObjects / elapsed;
		double ar = _removedObjects / elapsed;
		double ae = _errors / elapsed;

		SEISCOMP_INFO("DBPLUGIN (aps,ups,dps,errors) %.2f %.2f %.2f %.2f",
		              aa, au, ar, ae);

		_stopper.restart();
		_addedObjects = _updatedObjects = _removedObjects = _errors = 0;

		os << "dbadds=" << aa << "&"
		   << "dbupdates=" << au << "&"
		   << "dbdeletes=" << ar << "&"
		   << "dberrors=" << ae << "&";
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NetworkMessage* DbPlugin::service() {
	if ( !_dbDriver.empty() && !_dbReadConnection.empty() ) {
		DatabaseProvideMessage dbMsg(_dbDriver.c_str(), _dbReadConnection.c_str());
		return NetworkMessage::Encode(&dbMsg, Protocol::CONTENT_BINARY);
	}
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DbPlugin::init(Config::Config& conf, const std::string& configPrefix) {
	try {
		_dbDriver = conf.getString(configPrefix + "dbPlugin.dbDriver");
	}
	catch ( Config::Exception& ) {
		SEISCOMP_ERROR("Configuration '%sdbPlugin.dbDriver' is not set", configPrefix.c_str());
		return operational();
	}

	try {
		_dbWriteConnection = conf.getString(configPrefix + "dbPlugin.writeConnection");
	}
	catch ( Config::Exception& ) {
		SEISCOMP_ERROR("Configuration '%sdbPlugin.writeConnection' is not set", configPrefix.c_str());
		return operational();
	}

	try {
		_dbReadConnection = conf.getString(configPrefix + "dbPlugin.readConnection");
	}
	catch ( Config::Exception& ) {
		SEISCOMP_WARNING("Configuration '%sdbPlugin.readConnection' is not set, "
		                 "no service will be provided", configPrefix.c_str());
	}

	try {
		_strictVersionMatch = conf.getBool(configPrefix + "dbPlugin.strictVersionCheck");
	}
	catch ( Config::Exception& ) {
		_strictVersionMatch = true;
	}

	SEISCOMP_DEBUG("Checking database '%s' and trying to connect with '%s'",
	               _dbDriver.c_str(), _dbWriteConnection.c_str());

	_db = IO::DatabaseInterface::Create(_dbDriver.c_str());
	if ( _db == NULL ) {
		SEISCOMP_ERROR("Could not get database driver '%s'", _dbDriver.c_str());
		return operational();
	}

	setOperational(true);
	bool res = connectToDb();

	_stopper.restart();
	_addedObjects = _updatedObjects = _removedObjects = _errors = 0;

	return res;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DbPlugin::close() {
	disconnectFromDb();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool DbPlugin::connectToDb() {
	int counter = 0;
	while ( operational() && !_db->connect(_dbWriteConnection.c_str()) ) {
		if ( counter == 0 )
			SEISCOMP_ERROR("Database check... connection refused, retry");
		++counter;
		Core::sleep(1);

		if ( counter > 10 ) {
			SEISCOMP_ERROR("Database check... connection not available, abort");
			return false;
		}
	}

	if ( !operational() ) return true;

	SEISCOMP_INFO("Database connection established");

	_dbArchive = new DataModel::DatabaseArchive(_db.get());

	if ( !_dbArchive ) {
		SEISCOMP_ERROR("DbPlugin: Could not create DBArchive");
		return false;
	}

	if ( _dbArchive->hasError() )
		return false;

	Core::Version localSchemaVersion = Core::Version(DataModel::Version::Major, DataModel::Version::Minor);
	if ( localSchemaVersion > _dbArchive->version() ) {
		SEISCOMP_WARNING("Database schema v%s is older than schema v%s "
		                 "currently supported. Information will be lost when "
		                 "saving objects to the database! This should be fixed!",
		                 _dbArchive->version().toString().c_str(),
		                 localSchemaVersion.toString().c_str());
		if ( _strictVersionMatch ) {
			SEISCOMP_ERROR("Strict version check is enabled and schema versions "
			               "do not match.");
			return false;
		}
		else
			SEISCOMP_INFO("Strict version check is disabled and different "
			              "schema versions are not treated as error");
	}
	else
		SEISCOMP_DEBUG("Database check... ok");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void DbPlugin::disconnectFromDb() {
	if ( _db && _db->isConnected() )
		_db->disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp
