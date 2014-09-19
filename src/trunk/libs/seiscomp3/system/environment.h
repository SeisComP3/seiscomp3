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


#ifndef __SEISCOMP_SEISCOMPINIT_H__
#define __SEISCOMP_SEISCOMPINIT_H__

#include <iostream>
#include <string>
#include <fstream>
#include <memory>

#include <seiscomp3/core.h>
#include <seiscomp3/config/config.h>

namespace Seiscomp {

class SC_SYSTEM_CORE_API Environment {
public:
	enum ConfigStage {
		CS_UNDEFINED       = -1,
		CS_FIRST           =  0,
		CS_DEFAULT_GLOBAL  =  0,
		CS_DEFAULT_APP     =  1,
		CS_CONFIG_GLOBAL   =  2,
		CS_CONFIG_APP      =  3,
		CS_USER_GLOBAL     =  4,
		CS_USER_APP        =  5,
		CS_LAST            =  5,
		CS_QUANTITY        =  6
	};


	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
private:
	Environment();

public:
	~Environment();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
public:
	//! Returns an instance of Environment
	static Environment* Instance();

	//! Returns the home directory path
	const std::string& homeDir() const;

	//! Returns the seiscomp user configuration path
	const std::string& configDir() const;

	//! Returns the 2nd level seiscomp configuration path
	const std::string &appConfigDir() const;

	//! Returns the global seiscomp configuration path
	const std::string &globalConfigDir() const;

	//! Returns the directory where the application, libraries and
	//! data files have been installed
	const std::string& installDir() const;

	//! Returns the default shared data directory
	const std::string& shareDir() const;

	//! Returns the logging directory
	const std::string& logDir() const;

	//! Returns the path to the file where the clienst store their read spread
	//! messages. This Archive is needed for an archive request.
	std::string archiveFileName( const std::string& clientname ) const;

	//! Returns the path to the users configuration for the given name
	std::string configFileName(const std::string& programname) const;

	//! Returns the path to the 2nd level configuration for the given name
	std::string appConfigFileName(const std::string& programname) const;

	//! Returns the path to the 2nd level configuration for the given name
	std::string globalConfigFileName(const std::string& programname) const;

	//! Returns a name for a logfile in the logging directory
	//! logDir + "/" + name + ".log"
	std::string logFile(const std::string& name) const;

	std::string absolutePath(const std::string& name) const;

	std::string configFileLocation(const std::string& name, int stage) const;

	/** Confenience method to fill the symboltable.
	 * Following files will be read:
	 * SEISCOMP_ROOT/etc/global.cfg
	 * SEISCOMP_ROOT/etc/<programname>.cfg
	 * SEISCOMP_ROOT/config/global.cfg
	 * SEISCOMP_ROOT/config/<programname>.cfg
	 * ~/.seiscomp3/global.cfg
	 * ~/.seiscomp3/<programname>.cfg
	 */
	bool initConfig(Config::Config *config, const std::string& name,
	                int fromStage = CS_FIRST,
	                int toStage = CS_LAST,
	                bool standalone = false) const;


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
private:
	bool init();

	//! Creates a directory if it is not still existing
	bool createDir( const std::string& dir ) const;


	// ----------------------------------------------------------------------
	// Implementation
	// ----------------------------------------------------------------------
private:
	//! Home directory path
	std::string _homeDir;

	//! Path of of the seiscomp home directory
	std::string _localConfigDir;

	//! Seiscomp log directory
	std::string _logDir;

	//! Seiscomp install directory
	std::string _installDir;

	//! Seiscomp plugin directory
	std::string _shareDir;

	//! Suffix of the clients archive file
	std::string _archiveFileName;

	std::string _appConfigDir;
	std::string _globalConfigDir;

	static std::auto_ptr<Environment> _instance;

};



// ------------------------------------------------------------------------
// Inline methods
//-------------------------------------------------------------------------

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string& Environment::configDir() const
{
	return _localConfigDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string &Environment::appConfigDir() const {
	return _appConfigDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string &Environment::globalConfigDir() const {
	return _globalConfigDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string& Environment::homeDir() const {
	return _homeDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string& Environment::installDir() const
{
	return _installDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string& Environment::shareDir() const
{
	return _shareDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const std::string& Environment::logDir() const {
	return _logDir;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Seiscomp

#endif
