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

#ifndef __SEISCOMP_APPLICATIONS_MNCURSESPLUGIN_H__
#define __SEISCOMP_APPLICATIONS_MNCURSESPLUGIN_H__

#include <seiscomp3/plugins/monitor/monitoroutplugininterface.h>

namespace Seiscomp {
namespace Applications {

DEFINE_SMARTPOINTER(MNcursesPlugin);

class MNcursesPlugin : public MonitorOutPluginInterface {
	DECLARE_SC_CLASS(MNcursesPlugin);

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	public:
		enum TextAttrib { NORMAL = 0x0, HIGHLIGHT, TEXTATTRIB_QUNATITY };

	private:
		typedef std::map<Communication::ConnectionInfoTag, std::string> Header;
		typedef std::map<Communication::ConnectionInfoTag, int>         ColumnSizes;
		typedef std::vector<Communication::ConnectionInfoTag>           TagOrder;


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MNcursesPlugin();
		virtual ~MNcursesPlugin() {}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual bool print(const ClientTable& table);
		virtual bool initOut(const Config::Config& cfg);
		virtual bool deinitOut();
		virtual bool refreshOut();
		virtual bool clearOut();


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		bool print(const std::string& str, TextAttrib attrib = NORMAL);
		bool printTable(ClientTable& table);
		void readUserInput();
		bool init();
		void updateColumnSizes(const ClientTable& table);
		std::string formatLine(ClientInfoData& vec);
		void initDataStructures(Communication::ConnectionInfoTag key, const std::string& description);
		int findTag(Communication::ConnectionInfoTag tag) const;


	// ----------------------------------------------------------------------
	// Private data member
	// ----------------------------------------------------------------------
	private:
		void                              *_context;
		Header                             _header;
		ColumnSizes                        _columnSizes;
		TagOrder                           _tagOrder;
		unsigned int                       _currentLine;
		Communication::ConnectionInfoTag   _activeTag;
		bool                               _reverseSortOrder;
		ClientTable                        _clientTableCache;
		boost::mutex                       _dataStructureMutex;

};

} // namespace Application
} // namespace Seiscomp

#endif
