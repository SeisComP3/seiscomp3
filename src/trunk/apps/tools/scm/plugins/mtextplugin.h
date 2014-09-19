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

#ifndef __SEISCOMP_APPLICATIONS_MTEXTPLUGIN_CPP___
#define __SEISCOMP_APPLICATIONS_MTEXTPLUGIN_CPP___

#include <map>
#include <string>

#include <seiscomp3/plugins/monitor/monitoroutplugininterface.h>


namespace Seiscomp {
namespace Applications {

DEFINE_SMARTPOINTER(MTextPlugin);

class MTextPlugin : public MonitorOutPluginInterface {
	DECLARE_SC_CLASS(MTextPlugin);

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, std::string> Clients;

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MTextPlugin();
		virtual ~MTextPlugin() {}

	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual bool print(const ClientTable& table);
		virtual bool initOut(const Config::Config& cfg);
		virtual bool deinitOut();
		virtual bool refreshOut();
		virtual bool clearOut();


	private:
		bool        _init;
		std::string _descrFileName;
		std::string _outputDir;
		Clients     _clients;
};

} // namespace Applications
} // namespace Seiscomp


#endif
