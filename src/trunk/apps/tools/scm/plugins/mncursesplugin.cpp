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

#define SEISCOMP_COMPONENT ScMonitor
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>

#include "mncursesplugin.h"

#if defined(__SUNPRO_CC) || defined(__sun__)
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

#include <iomanip>
#include <cstring>
#include <algorithm>
#include <functional>


#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <seiscomp3/core/plugin.h>

namespace Seiscomp {
namespace Applications {


IMPLEMENT_SC_CLASS_DERIVED(MNcursesPlugin,
                           MonitorOutPluginInterface,
                           "MNcursesPlugin");

REGISTER_MONITOR_OUT_PLUGIN_INTERFACE(MNcursesPlugin, "mncursesplugin");
ADD_SC_PLUGIN(
		"Plugin for ncurces output",
		"GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 1, 0, 0)




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SortClients : public std::binary_function<ClientInfoData, ClientInfoData, bool> {
	public:
		SortClients(Communication::ConnectionInfoTag key)
		 : _key(key) {}

		result_type operator() (const first_argument_type& map0, const second_argument_type& map1) const {
			bool result = false;
			try {
				first_argument_type::const_iterator it0  = map0.find(_key);
				second_argument_type::const_iterator it1 = map1.find(_key);
				switch ( _key )
				{
					case Communication::PROGRAMNAME_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::PROGRAMNAME_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::PROGRAMNAME_TAG>::Type>(it1->second);
						break;
					case Communication::CLIENTNAME_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::CLIENTNAME_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::CLIENTNAME_TAG>::Type>(it1->second);
						break;
					case Communication::HOSTNAME_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::HOSTNAME_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::HOSTNAME_TAG>::Type>(it1->second);
						break;
					case Communication::TOTAL_MEMORY_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::TOTAL_MEMORY_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::TOTAL_MEMORY_TAG>::Type>(it1->second);
						break;
					case Communication::CLIENT_MEMORY_USAGE_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::CLIENT_MEMORY_USAGE_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::CLIENT_MEMORY_USAGE_TAG>::Type>(it1->second);
						break;
					case Communication::MEMORY_USAGE_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::MEMORY_USAGE_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::MEMORY_USAGE_TAG>::Type>(it1->second);
						break;
					case Communication::CPU_USAGE_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::CPU_USAGE_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::CPU_USAGE_TAG>::Type>(it1->second);
						break;
					case Communication::MESSAGE_QUEUE_SIZE_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::MESSAGE_QUEUE_SIZE_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::MESSAGE_QUEUE_SIZE_TAG>::Type>(it1->second);
						break;
					case Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>::Type>(it1->second);
						break;
					case Communication::UPTIME_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::UPTIME_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::UPTIME_TAG>::Type>(it1->second);
						break;
					case Communication::RESPONSE_TIME_TAG:
						result = boost::lexical_cast<Communication::ConnectionInfoT<Communication::RESPONSE_TIME_TAG>::Type>(
								it0->second) < boost::lexical_cast<Communication::ConnectionInfoT<Communication::RESPONSE_TIME_TAG>::Type>(it1->second);
						break;
					default:
						result = it0->second < it1->second;
				}
			}
			catch(boost::bad_lexical_cast &) {}
			return result;
		}

	private:
		Communication::ConnectionInfoTag _key;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MNcursesPlugin::MNcursesPlugin()
: MonitorOutPluginInterface("mncursesplugin")
, _context(NULL)
, _currentLine(0)
, _reverseSortOrder(false) {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::print(const ClientTable& table) {
	{
		// A copy is needed for refreshing the screen
		boost::mutex::scoped_lock lock(_dataStructureMutex);
		_clientTableCache = table;
	}
	return printTable(_clientTableCache);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::print(const std::string& str, TextAttrib attrib) {
	if ( attrib == HIGHLIGHT )
		attron(A_STANDOUT);

	// updateColumnSizes();
	move(_currentLine, 0);
	addstr(str.c_str());
	++_currentLine;
	refresh();

	if ( attrib == HIGHLIGHT )
		attroff(A_STANDOUT);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::printTable(ClientTable& table) {
	clearOut(); // Clear the screen
	updateColumnSizes(_clientTableCache);
	std::string masterAddress;
	if ( SCCoreApp )
		masterAddress = SCCoreApp->connection()->masterAddress();
	std::string str = "[ Connected to master@" + masterAddress + " ] sorting: " + _header[_activeTag];
	print(str);
	print(formatLine(_header), HIGHLIGHT);

	boost::mutex::scoped_lock lock(_dataStructureMutex);
	// std::stable_sort(_clientTableCache.begin(), _clientTableCache.end(), std::not2(SortClients(_activeTag)));
	_clientTableCache.sort(std::not2(SortClients(_activeTag)));

	// Print clients
	if ( _reverseSortOrder ) {
		for ( ClientTable::reverse_iterator it = _clientTableCache.rbegin(); it != _clientTableCache.rend(); ++it )
			print(formatLine(*it));
	}
	else {
		for (ClientTable::iterator it = _clientTableCache.begin(); it != _clientTableCache.end(); ++it)
			print(formatLine(*it));
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::initOut(const Config::Config&) {
	_context = initscr();
	cbreak();
	noecho();
	keypad((WINDOW*)_context, TRUE);
	scrollok((WINDOW*)_context, TRUE);
	idlok((WINDOW*)_context, TRUE);
	_currentLine = 0;
	move(_currentLine, 0);
	curs_set(0); // Hide the cursor
	clearOut();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::deinitOut() {
	endwin();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::refreshOut() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::clearOut() {
	clear();
	refresh();
	_currentLine = 0;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MNcursesPlugin::init() {
	_activeTag = Communication::CLIENTNAME_TAG;

	initDataStructures(Communication::PROGRAMNAME_TAG, "prog");
	initDataStructures(Communication::CLIENTNAME_TAG, "name");
	initDataStructures(Communication::HOSTNAME_TAG, "host");
	initDataStructures(Communication::TOTAL_MEMORY_TAG, "hmem");
	initDataStructures(Communication::CLIENT_MEMORY_USAGE_TAG, "cmem");
	initDataStructures(Communication::MEMORY_USAGE_TAG, "mem");
	initDataStructures(Communication::CPU_USAGE_TAG, "cpu");
	initDataStructures(Communication::MESSAGE_QUEUE_SIZE_TAG, "q");
	initDataStructures(Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG, "mq");
	initDataStructures(Communication::UPTIME_TAG, "uptime");
	initDataStructures(Communication::RESPONSE_TIME_TAG, "resp");

	boost::thread thread1(boost::bind(&MNcursesPlugin::readUserInput, this));
	thread1.yield();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string MNcursesPlugin::formatLine(ClientInfoData& clientInfoData) {
	std::ostringstream os;
	TagOrder::iterator it = _tagOrder.begin();
	for ( ; it != _tagOrder.end(); ++it )
		os << std::setw(0) << "| " << std::setw(_columnSizes[*it]) << std::right << clientInfoData[*it] << " ";
	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNcursesPlugin::initDataStructures(Communication::ConnectionInfoTag tag,
                                        const std::string& description) {
	_tagOrder.push_back(tag);
	_header.insert(make_pair(tag, description));
	_columnSizes.insert(std::make_pair(tag, description.size()));
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MNcursesPlugin::findTag(Communication::ConnectionInfoTag tag) const {
	for ( size_t i = 0; i < _tagOrder.size(); ++i ) {
		if ( _tagOrder[i] == tag )
			return i;
	}
	return -1;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNcursesPlugin::updateColumnSizes(const ClientTable& table) {
	for ( ClientTable::const_iterator it = table.begin(); it != table.end(); ++it ) {
		for ( TagOrder::iterator tagIt = _tagOrder.begin(); tagIt != _tagOrder.end(); ++tagIt ) {
			int size = static_cast<int>(it->info.find(*tagIt)->second.size());
			_columnSizes[*tagIt] = std::max(_columnSizes[*tagIt], size);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MNcursesPlugin::readUserInput() {
	while ( true ) {
		int ch = wgetch((WINDOW*)_context);
		if ( ch == '<' ) {
			int idx = findTag(_activeTag);
			if ( idx >= 0 ) {
				if ( --idx < 0 ) idx = _tagOrder.size() - 1;
				_activeTag = _tagOrder[idx];
				printTable(_clientTableCache);
			}
		}
		else if ( ch == '>' ) {
			int idx = findTag(_activeTag);
			if ( idx >= 0 )  {
				idx = (idx + 1) % _tagOrder.size();
				_activeTag = _tagOrder[idx];
				printTable(_clientTableCache);
			}
		}
		else if (  ch == 'r' ) {
			_reverseSortOrder = !_reverseSortOrder;
			printTable(_clientTableCache);
		}
	}
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




} // namespace Application
} // namespace Seiscomp

