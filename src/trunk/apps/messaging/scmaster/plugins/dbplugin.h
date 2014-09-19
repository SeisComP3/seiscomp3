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

#ifndef __DBPLUGIN_H__
#define __DBPLUGIN_H__

#include <iostream>
#include <string>

#include <seiscomp3/utils/timer.h>
#include <seiscomp3/communication/masterplugininterface.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/datamodel/databasearchive.h>


namespace Seiscomp {
namespace Communication {


DEFINE_SMARTPOINTER(DbPlugin);


class DbPlugin : public MasterPluginInterface {
	DECLARE_SC_CLASS(DbPlugin);

	// --------------------------------------------------------------------------
	// CONSTRUCTION - DESTRUCTION
	// --------------------------------------------------------------------------
public:
	virtual ~DbPlugin();


	// --------------------------------------------------------------------------
	// PUBLIC INTERFACE
	// --------------------------------------------------------------------------
public:
	virtual bool process(NetworkMessage *nmsg, Core::Message *msg);
	virtual NetworkMessage* service();
	virtual bool init(Config::Config& conf, const std::string& configPrefix);
	virtual bool close();

	virtual void printStateOfHealthInformation(std::ostream &) const;

	// --------------------------------------------------------------------------
	// PRIVATE INTERFACE
	// --------------------------------------------------------------------------
private:
	bool connectToDb();
	void disconnectFromDb();


private:
	Seiscomp::IO::DatabaseInterfacePtr      _db;
	Seiscomp::DataModel::DatabaseArchivePtr _dbArchive;
	std::string                             _dbDriver;
	std::string                             _dbWriteConnection;
	std::string                             _dbReadConnection;
	bool                                    _strictVersionMatch;

	mutable Util::StopWatch                 _stopper;

	mutable size_t                          _addedObjects;
	mutable size_t                          _updatedObjects;
	mutable size_t                          _removedObjects;
	mutable size_t                          _errors;
};

} // namespace Communication
} // namespace Seiscomp

#endif
