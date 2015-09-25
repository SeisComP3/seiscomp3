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


#ifndef __SEISCOMP_PROCESSING_PROCESSOR_H__
#define __SEISCOMP_PROCESSING_PROCESSOR_H__


#include <seiscomp3/config/exceptions.h>
#include <seiscomp3/utils/keyvalues.h>
#include <seiscomp3/client.h>


namespace Seiscomp {

namespace Config {
	class Config;
}

namespace Processing {


struct SC_SYSTEM_CLIENT_API Settings {
	Settings(const std::string &module,
	         const std::string &network,
	         const std::string &station,
	         const std::string &location,
	         const std::string &stream,
	         const Config::Config *config,
	         const Util::KeyValues *keys);

	//! Returns a parameter value for a station. The first
	//! lookup is in the global application configuration
	//! with name "key.module.network.station.parameter.
	//! If it is not found it tries to lookup the value in
	//! keyParameters. If no value is found an exception
	//! is thrown otherwise the value is returned.
	std::string getString(const std::string &parameter) const
	throw(Config::Exception);

	int getInt(const std::string &parameter) const
	throw(Config::Exception);

	double getDouble(const std::string &parameter) const
	throw(Config::Exception);

	bool getBool(const std::string &parameter) const
	throw(Config::Exception);

	//! Set the parameter value for a station. The first
	//! lookup is in the global application configuration
	//! with name "key.module.network.station.parameter.
	//! If it is not found it tries to lookup the value in
	//! keyParameters. If no value is found, false is returned,
	//! true otherwise
	bool getValue(std::string &value, const std::string &parameter) const;
	bool getValue(int &value, const std::string &parameter) const;
	bool getValue(double &value, const std::string &parameter) const;
	bool getValue(bool &value, const std::string &parameter) const;

	const std::string     &module;
	const std::string     &networkCode;
	const std::string     &stationCode;
	const std::string     &locationCode;
	const std::string     &channelCode;
	const Config::Config  *localConfiguration;
	const Util::KeyValues *keyParameters;
};


DEFINE_SMARTPOINTER(Processor);


class SC_SYSTEM_CLIENT_API Processor : public Core::BaseObject {
	DECLARE_SC_CLASS(Processor);

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Processor();

		//! D'tor
		virtual ~Processor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! This method can be called to initialize the processor.
		//! 'parameters' contains simple name-value pairs (strings).
		//! The default implementation does nothing.
		virtual bool setup(const Settings &settings);
};


}

}


#endif
