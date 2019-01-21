/***************************************************************************
 * seedlink caps_plugin
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#ifndef GEMAP_CAPS_SEEDLINK_PLUGIN_H
#define GEMAP_CAPS_SEEDLINK_PLUGIN_H


#include <gempa/caps/packet.h>
#include <gempa/caps/rawpacket.h>
#include <gempa/caps/mseedpacket.h>
#include <gempa/caps/sessiontable.h>
#include <gempa/caps/socket.h>
#include <gempa/caps/application.h>

#include <boost/program_options.hpp>

#include <string>
#include <vector>
#include <list>
#include <map>


namespace Gempa {
namespace CAPS {


class App : public Application {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		App(int argc, char** argv);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setStreamState(const std::string &streamID,
		                    const CAPS::Time &time);


	// ----------------------------------------------------------------------
	//  Application interface
	// ----------------------------------------------------------------------
	public:
		virtual void exit(int returnCode);

	protected:
		virtual bool validateParameters();

		virtual bool init();
		virtual bool initCommandLine();
		virtual bool run();
		virtual void done();


	private:
		struct Packet {
			Packet(const std::string &streamID) : streamID(streamID),
			    record(NULL) {}
			~Packet() {
				if ( record ) delete record;
			}

			size_t size() const {
				return header.size() + record?record->dataSize(true):0;
			}

			std::string         streamID;
			std::vector<char>   header;
			CAPS::DataRecord   *record;
		};

		typedef boost::shared_ptr<Packet> PacketPtr;
		typedef std::list<PacketPtr> Packets;

		struct Request {
			Request(const std::string &streamID)
			: streamID(streamID)
			, receivedData(false) {}

			std::string  streamID;
			CAPS::Time   startTime;
			CAPS::Time   endTime;
			bool         receivedData;
		};

		typedef boost::shared_ptr<Request> RequestPtr;
		typedef std::map<std::string, RequestPtr> RequestByID;
		typedef std::list<Request*> Requests;

	private:
		bool parseURL(std::string url);

		bool addRequest(const std::string &net, const std::string &sta,
		                const std::string &loc, const std::string &cha,
		                const Time &stime,
		                const Time &etime,
		                bool receivedData = false);

		/**
		 * Finds a request by stream ID. To speed up the search the first stage
		 * uses a simple comparison to find the request. In contrast to the
		 * second stage takes also wild cards into account. Returns NULL if
		 * the stream ID does not match.
		*/
		Request *findRequest(const std::string &streamID);
		bool readStreams(const std::string &filename);

		void itemAdded(SessionTableItem *item);
		void itemAboutToBeRemoved(SessionTableItem *item);

		bool handshake();
		void disconnect();
		void close();
		void wait();

		bool storeRawPacket(RawDataRecord &rec);
		bool storeMSEEDPacket(MSEEDDataRecord &rec);


	private:
		typedef std::map<std::string, Time> StreamStates;

		std::string               _host;
		unsigned short            _port;
		std::string               _user;
		std::string               _password;
		bool                      _ssl;

		std::string               _address;
		StreamStates              _states;
		std::string               _logFile;
		std::vector<std::string>  _streams;
		SocketPtr                 _socket;
		SessionTable              _sessionTable;
		RequestByID               _requestByID;
		Requests                  _requests;
		bool                      _dump;
		bool                      _archive;
		socketbuf<Socket,512>     _socketBuf;
		char                      _lineBuf[201];
		int                       _currentID;
		SessionTableItem         *_currentItem;
		DataRecord::Header        _rawPacketHeader;
		RawResponseHeader         _rawResponseHeader;
		std::string               _streamsFile;
		CAPS::Time                _startTime;
		CAPS::Time                _endTime;
		std::string               _strStartTime;
		std::string               _strEndTime;
		unsigned int              _verbosity;

		typedef boost::shared_ptr<boost::program_options::options_description> OptionsPtr;
		OptionsPtr                              _options;
		boost::program_options::variables_map   _vm;
};


}
}


#endif
