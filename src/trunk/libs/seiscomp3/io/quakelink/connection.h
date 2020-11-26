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
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#ifndef __SEISCOMP_IO_QUAKELINK_CONNECTION_H__
#define __SEISCOMP_IO_QUAKELINK_CONNECTION_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/io/socket.h>

#include <string>
#include <vector>


namespace Seiscomp {
namespace IO {
namespace QuakeLink {

extern const char *SummaryTimeFormat;
extern const char *RequestTimeFormat;

enum RequestFormat {
	rfSummary           = 0,
	rfXML               = 1,
	rfGZXML             = 2,
	rfNative            = 3,
	rfGZNative          = 4
};

enum ContentType {
	ctUndefined         = -1,
	ctXML               = 0,
	ctEvSum             = 1,
	ctEvLog             = 2,
	ctText              = 3
};

enum Options {
	opIgnore            = 0x0000,
	opDefaults          = 0x0001,
	opXMLIndent         = 0x0002,
	opDataPicks         = 0x0004,
	opDataAmplitudes    = 0x0008,
	opDataStaMags       = 0x0010,
	opDataArrivals      = 0x0020,
	opDataStaMts        = 0x0040,
	opDataPreferred     = 0x0080,
	opKeepAlive         = 0x8000, // since API 1.6.0
	opAll               = 0xFFFF
};

enum OrderBy {
	obUndefined         = -1,
	obOTimeAsc          = 0,
	obOTimeDesc         = 1
};

// API and format version type
typedef unsigned int Version;

// Maps a version of a specific RequestFormat to a minimum required API version
typedef std::vector<Version> APIList;

// Maps a RequestFormat to the list of required API versions
typedef std::map<RequestFormat, APIList> FormatAPIMap;

class RequestFormatVersion {
	public:
		RequestFormatVersion(RequestFormat format, Version version=1)
		    : _format(format), _version(version < 1 ? 1 : version) {}
		operator RequestFormat() const { return _format; }
		RequestFormat format() const { return _format; }
		Version version() const { return _version; }

	private:
		RequestFormat _format;
		Version       _version;
};


/**
 * @brief The Response class
 */
DEFINE_SMARTPOINTER(Response);
class Response : public Core::BaseObject {
	DECLARE_CASTS(Response);
	public:
		// Header
		ContentType          type;
		uint                 length;
		Core::Time           timestamp;
		std::string          format;
		bool                 gzip;
		OPT(int)             revision; // since API 1.6.0
		bool                 disposed; // since API 4.0.0

		// Payload
		std::string data;

		Response() { reset(); }
		Response(const Response &other) { *this = other; }

		const Response& operator=(const Response &other) {
			type        = other.type;
			length      = other.length;
			timestamp   = other.timestamp;
			format      = other.format;
			gzip        = other.gzip;
			data        = other.data;
			revision    = other.revision;
			disposed    = other.disposed;
			return *this;
		}

		void reset() {
			type        = ctUndefined;
			length      = 0;
			timestamp   = Core::Time::Null;
			gzip        = false;
			revision    = Core::None;
			disposed    = false;
			format.clear();
			data.clear();
	}
};
typedef std::vector<Response> Responses;
typedef std::vector<Response*> ResponsesPtr;

DEFINE_SMARTPOINTER(Connection);
class Connection : public Core::BaseObject {

	public:
		/** Default constructor */
		Connection();

		/** Destructor */
		virtual ~Connection();

		/**
		 * @brief Initializes client instance with connection URL and options
		 * @param url URL of the QuakeLink including user name and password if
		 * required
		 * @param options Bit mask of all options to enable
		 * @return
		 */
		bool init(const std::string &url, int options = opIgnore);

		/**
		 * @brief Returns connection state
		 * @return True if a connection to the QuakeLink server is established
		 */
		bool connected() const;

		/** Disconnects from QL server */
		void disconnect();

		/**
		 * @brief Executes the hello command which retrieves the server ID and
		 * API version.
		 * @param id Variable to store the server identifier including build
		 * version, e.g. QuakeLink (gempa GmbH) v2020.083#da048827b
		 * @param api Variable to store the server API version
		 * @return True on success. Note: The first QuakeLink server versions
		 * did not report a API version. If such a server is encountered the
		 * version is set to 0.
		 * @since SeisComP ABI version 13.0.0
		 */
		bool hello(std::string &id, Version &api);

		/**
		 * @brief Sets connection options
		 * @param options Bit mask of all options to enable
		 * @return True if the options could be set
		 */
		bool setOptions(int options);

		/**
		 * @brief Gets all updates of a particular event. The response format is
		 * restricted to 'summary'.
		 * @param resp Response object to store the result
		 * @param eventID Event ID to retrieve all updates from
		 * @param formatVersion Response format and version. Note currently only
		 * the rfSummary format is supported but in different versions.
		 * since API 13.0.0
		 * @return True if the updates of an event could be fetched
		 */
		bool getUpdates(Response &resp, const std::string &eventID,
		                const RequestFormatVersion &formatVersion = rfSummary);

		/**
		 * @brief Gets one particular event revision
		 * @param response Response object to store the result
		 * @param eventID Event ID to retrive the particular revision from
		 * @param revision Event revision, use a negative number to query the
		 * latest revision
		 * @param formatVersion Response format and version
		 * @return True if the event revision could be fetched
		 */
		bool get(Response &response, const std::string &eventID,
		         int revision = -1,
		         const RequestFormatVersion &formatVersion = rfSummary);

		/**
		 * @brief Selects archived events. Returns when all matching events have
		 *        been processed.
		 * @param responses List of Response objects to store the results
		 * @param from Begin of time window
		 * @param to End of time window
		 * @param formatVersion Response format
		 * will request the default version.
		 * @param where SQL like filter clause, of form
		 *   clause    := condition[ AND|OR [(]clause[)]]
		 *   condition := MAG|DEPTH|LAT|LON|PHASES op {float} |
		 *                UPDATED|OTIME op time |
		 *                MAG|DEPTH|LAT|LON|PHASES|OTIME|UPDATED IS [NOT] NULL
		 *   op        := =|>|>=|<|<=|eq|gt|ge|lt|ge
		 * @param orderBy sort order of the results,
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @param limit maximum number of results to return, a value of 0 will
		 * disable any limit,
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @param offset of the requested result set, a value of 0 will return
		 * the first item
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @return True if the query could be executed
		 */
		bool selectArchived(Responses &responses,
		                    const Core::Time &from = Core::Time(),
		                    const Core::Time &to = Core::Time(),
		                    const RequestFormatVersion &formatVersion = rfSummary,
		                    const std::string &where = "",
		                    OrderBy orderBy = obUndefined,
		                    unsigned long limit = 0,
		                    unsigned long offset = 0);

		/**
		 * @brief Selects updated and (optional) archived events. This call
		 * blocks until 'disconnect()', or 'abort()' is called from a different
		 * thread or a connection failure occurs and automatic reconnection is
		 * disabled. The results are send as notifier objects.
		 * @param archived If enabled also archived events are returned
		 * @param from Begin of time window
		 * @param to End of time window
		 * @param formatVersion Response format
		 * @param where SQL like filter clause, of form
		 *   clause    := condition[ AND|OR [(]clause[)]]
		 *   condition := MAG|DEPTH|LAT|LON|PHASES op {float} |
		 *                UPDATED|OTIME op time |
		 *                MAG|DEPTH|LAT|LON|PHASES|OTIME|UPDATED IS [NOT] NULL
		 *   op        := =|>|>=|<|<=|eq|gt|ge|lt|ge
		 * @param updatedBufferSize Size of the buffer to store updated events
		 * until all archived events have been processed. If set to a negative
		 * value the buffer will be disabled. In this case archived and updated
		 * events may be mixed. If set to an insufficient positive value updated
		 * events may be lost if a buffer overflow occurs while archived events
		 * are still processed.
		 * @param orderBy sort order of the results,
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @param limit maximum number of results to return, a value of 0 will
		 * disable any limit,
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @param offset of the requested result set, a value of 0 will return
		 * the first item
		 * Note: Requires server API >= 1,
		 * Since: SeisComP ABI version 13.0.0
		 * @return True if the query could be executed and then was gracefully
		 * aborted.
		 */
		bool select(bool archived = false,
		            const Core::Time &from = Core::Time(),
		            const Core::Time &to = Core::Time(),
		            const RequestFormatVersion &formatVersion = rfSummary,
		            const std::string &where = "",
		            int updatedBufferSize = 1000,
		            OrderBy orderBy = obUndefined,
		            unsigned long limit = 0,
		            unsigned long offset = 0);

		/**
		 * @brief Gracefully aborts data transmission by sending an abort
		 * command to the server.
		 * @return True if the abort command could be sent
		 */
		bool abort();

		/**
		 * @brief serverID
		 * @return The server ID or an empty string if no connection could be
		 * established.
		 * @since SeisComP ABI version 13.0.0
		 */
		const std::string& serverID();

		/**
		 * @brief serverAPI
		 * @return The server API version or 0 if the version could not be
		 * obtained
		 * @since SeisComP ABI version 13.0.0
		 */
		Version serverAPI();

		/**
		 * @brief initialized
		 * @return True if this instance was sucessfully initialized via the
		 * init() call
		 */
		inline bool initialized() { return _sock; }

		/**
		 * @brief interrupted
		 * @return True if underlying socket of this instance was interrupted
		 */
		inline bool interrupted() { return _sock && _sock->isInterrupted(); }

		/**
		 * @brief isSupported
		 * @param formatVersion The format version to check
		 * @param log If enabled negative answers will be logged
		 * @return True if the format version combination is supported by the
		 * current server connection
		 * @since SeisComP ABI version 13.0.0
		 */
		bool isSupported(const RequestFormatVersion &formatVersion,
		                 bool log=false);

		/**
		 * @brief maximumSupportedVersion
		 * @param format The request format
		 * @return Maximum format version supported by the current server
		 * connection. Returns 0 if no connection could be established.
		 * @since SeisComP ABI version 13.0.0
		 */
		Version maximumSupportedVersion(RequestFormat format);

	protected:
		/**
		 * @brief Processes a response recevied by a select request.
		 * @param response Response object received,
		 * the ownership is transfered to the method
		 */
		virtual void processResponse(Response* response) { delete response; }

		inline void setLogPrefix(const std::string &prefix) {
			_logPrefix = prefix;
		}

		bool connect();
		bool sendRequest(const std::string &req, bool log = true);
		bool sendOptions(int changedOptions);
		bool updateOption(Options option, const char *cmd,
		                  int changedOptions = opAll);
		bool readResponse(Response &response);

		size_t readLine(std::string &line);
		void logAndDisconnect(const char *msg, const char *detail = 0);
		void logInvalidResp(const char *expected, const char *got);
		bool readResponseCode(std::string &code);
		bool assertResponseCode(const std::string &expected);
		bool assertLineBreak();
		bool readPayload(std::string &data, uint count);
		bool checkFormatVersion(std::string &error,
		                        const RequestFormatVersion &formatVerion);

	protected:
		std::string             _logPrefix;

		std::string             _service;
		std::string             _user;
		std::string             _pass;
		Socket                 *_sock;

		int                     _options;
		std::string             _serverID;
		Version                 _serverAPI;

};


} // ns QuakeLink
} // ns IO
} // ns Seiscomp


#endif // __SEISCOMP_IO_QUAKELINK_CONNECTION_H__
