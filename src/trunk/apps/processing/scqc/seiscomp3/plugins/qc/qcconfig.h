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


#ifndef __SEISCOMP_QC_QCCONFIG_H__
#define __SEISCOMP_QC_QCCONFIG_H__

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/plugins/qc/api.h>
#include <seiscomp3/plugins/qc/qcplugin.h>

#include <string>
#include <vector>

namespace Seiscomp {
namespace Applications {
namespace Qc {


class SC_QCPLUGIN_API QcConfigException: public Core::GeneralException {
public:
	QcConfigException(): Core::GeneralException("Qc config exception") {}
	QcConfigException(const std::string &what): Core::GeneralException(what) {}
};


DEFINE_SMARTPOINTER(QcConfig);

class SC_QCPLUGIN_API QcConfig: public Core::BaseObject {
	DECLARE_SC_CLASS(QcConfig);

public:
	//! default Constructor
	QcConfig();

	//! initializing Constructor
	QcConfig(QcApp *app, const std::string &pluginName="default");
	
	//! destructor
	virtual ~QcConfig();
	
	//! Read plugin specific configuration from config files. If not found,
	//! use plugin.defaults. If not found, use compiled-in default.
	std::string readConfig(const std::string& pluginName, const std::string& keyWord,
						const std::string& defaultValue) const throw(QcConfigException);
		
	//! Returns true if configured in realtime only mode; false otherwise
	bool realtimeOnly() const;
	
	//! Returns the over all buffer length
	int buffer() const;
	
	//! Returns the archive interval time span
	int archiveInterval() const;
	
	//! Returns the configured archive buffer length
	//! if it is <= buffer length; buffer length otherwise
	int archiveBuffer() const;
	
	//! Returns the report interval time span
	int reportInterval() const;
	
	//! Returns the configured report buffer length
	//! if it is <= buffer length; buffer length otherwise
	int reportBuffer() const;
	
	//! Returns the configured timeout for realtime only processors
	//! throws exception in case of archive mode
	int reportTimeout() const throw (QcConfigException);
	
	//! Returns the alert interval time span if in realtime processing mode
	//! throws exception in case of archive mode
	int alertInterval() const throw (QcConfigException);
	
	//! Returns the configured report buffer length
	//! if it is <= buffer length; buffer length otherwise in realtime mode
	//! throws exception in case of archive mode
	int alertBuffer() const throw (QcConfigException);
	
	//! Returns the thresholds triggering alarm in realtime mode
	//! throws exception in case of archive mode
	std::vector<int> alertThresholds() const throw (QcConfigException);
		
	//! Returns true if configured in realtime only mode; false otherwise
	static bool RealtimeOnly(const QcApp *app, const std::string &pluginName);

private:
	QcApp* _app;
	bool _realtime;
	int _buffer;
	int _archiveInterval;
	int _archiveBuffer;
	int _reportInterval;
	int _reportBuffer;
	int _reportTimeout;
	int _alertInterval;
	int _alertBuffer;
	std::vector<int> _alertThresholds;

	void setQcConfig(const std::string &pluginName) throw(QcConfigException);
};



}
}
}

#endif
