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

#ifndef __SEISCOMP_APPLICATIONS_MONITORPLUGININTERFACE_H__
#define __SEISCOMP_APPLICATIONS_MONITORPLUGININTERFACE_H__

#include <map>
#include <vector>
#include <string>

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/plugins/monitor/api.h>
#include <seiscomp3/plugins/monitor/types.h>

namespace Seiscomp {
namespace Applications {


SC_MPLUGIN_API bool findName(ClientInfoData clientData, std::string name);


DEFINE_SMARTPOINTER(MonitorPluginInterface);

class MFilterParser;
class MFilterInterface;


class SC_MPLUGIN_API MonitorPluginInterface : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(MonitorPluginInterface);

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MonitorPluginInterface(const std::string& name);
		virtual ~MonitorPluginInterface();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		static MonitorPluginInterface* Create(const std::string& service);

		virtual bool init(const Config::Config &cfg);

		virtual void process(const ClientTable& clientTable) = 0;

		bool initFilter(const Config::Config &cfg);
		bool operational() const;
		bool isFilteringEnabled() const;
		void setOperational(bool val);

		const std::string& filterString() const;
		const ClientTable* filter(const ClientTable& clientTable);
		const ClientTable* filterMean(const ClientTable& clientTable);
		void setFilterMeanInterval(double interval);

		const ClientTable* match() const;

		const std::string& name() const;


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		template <Communication::EConnectionInfoTag tag>
		void sumData(ClientInfoData& lhs, const ClientInfoData& rhs);

		template <Communication::EConnectionInfoTag tag>
		void calculateMean(ClientInfoData& lhs, size_t count);

	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		Core::TimeSpan                _filterMeanInterval;
		Core::Time                    _filterMeanTimeMark;
		ClientTable                   _filterMeanClientTable;
		std::map<std::string, size_t> _filterMeanMessageCount;

		ClientTable       _match;
		std::string       _name;
		bool              _operational;
		bool              _isFilteringEnabled;
		std::string       _filterStr;
		MFilterParser*    _mFilterParser;
		MFilterInterface* _filter;

};


DEFINE_INTERFACE_FACTORY(MonitorPluginInterface);


#define REGISTER_MONITOR_PLUGIN_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Applications::MonitorPluginInterface, Class> __##Class##InterfaceFactory__(Service)


} // namespace Applications
} // namespace Seiscomp

#endif
