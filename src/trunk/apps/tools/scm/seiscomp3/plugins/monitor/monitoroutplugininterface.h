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


#ifndef __SEISCOMP_APPLICATIONS_MONITOROUTPLUGININTERFACE_H__
#define __SEISCOMP_APPLICATIONS_MONITOROUTPLUGININTERFACE_H__

#include <string>

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/plugins/monitor/api.h>
#include <seiscomp3/communication/connectioninfo.h>
#include <seiscomp3/plugins/monitor/types.h>


namespace Seiscomp {
namespace Applications {


DEFINE_SMARTPOINTER(MonitorOutPluginInterface);


class SC_MPLUGIN_API MonitorOutPluginInterface : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(MonitorOutPluginInterface);

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MonitorOutPluginInterface(const std::string& name);
		virtual ~MonitorOutPluginInterface() {}

	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual bool initOut(const Config::Config &cfg) = 0;
		virtual bool deinitOut() = 0;
		virtual bool print(const ClientTable& table) = 0;
		virtual bool refreshOut() = 0;
		virtual bool clearOut() = 0;

		const std::string& name() const;

		static MonitorOutPluginInterface* Create(const std::string& service);

	// ----------------------------------------------------------------------
	// Private data members
	// ---------------------------------------------------------------------
	private:
		std::string _name;

};


DEFINE_INTERFACE_FACTORY(MonitorOutPluginInterface);

#define REGISTER_MONITOR_OUT_PLUGIN_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Applications::MonitorOutPluginInterface, Class> __##Class##InterfaceFactory__(Service)

} // namespace Applications
} // namespace Seiscomp

#endif
