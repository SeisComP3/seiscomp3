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
	ctEvLog             = 2
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
		OPT(int)             revision;

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
			return *this;
		}

		void reset() {
			type        = ctUndefined;
			length      = 0;
			timestamp   = Core::Time::Null;
			gzip        = false;
			revision    = Core::None;
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
		 * @return True if the updates of an event could be fetched
		 */
		bool getUpdates(Response &resp, const std::string &eventID);

		/**
		 * @brief Gets one particular event revision
		 * @param response Response object to store the result
		 * @param eventID Event ID to retrive the particular revision from
		 * @param revision Event revision, use a negative number to query the
		 * latest revision
		 * @param format Response format
		 * @return True if the event revision could be fetched
		 */
		bool get(Response &response, const std::string &eventID,
		         int revision = -1, RequestFormat format = rfSummary);

		/**
		 * @brief Selects archived events. Returns when all matching events have
		 *        been processed.
		 * @param responses List of Response objects to store the results
		 * @param from Begin of time window
		 * @param to End of time window
		 * @param format Response format
		 * @param where SQL like filter clause, of form
		 *   clause    := condition[ AND|OR [(]clause[)]]
		 *   condition := MAG|DEPTH|LAT|LON|PHASES op {float} |
		 *                UPDATED|OTIME op time |
		 *                MAG|DEPTH|LAT|LON|PHASES|OTIME|UPDATED IS [NOT] NULL
		 *   op        := =|>|>=|<|<=|eq|gt|ge|lt|ge
		 * @return True if the query could be executed
		 */
		bool selectArchived(Responses &responses,
		                    const Core::Time &from = Core::Time(),
		                    const Core::Time &to = Core::Time(),
		                    RequestFormat format = rfSummary,
		                    const std::string &where = "");

		/**
		 * @brief Selects updated and (optional) archived events. This call
		 * blocks until 'disconnect()', or 'abort()' is called from a different
		 * thread or a connection failure occurs and automatic reconnection is
		 * disabled. The results are send as notifier objects.
		 * @param archived If enabled also archived events are returned
		 * @param from Begin of time window
		 * @param to End of time window
		 * @param format Response format
		 * @param where SQL like filter clause, of form
		 *   clause    := condition[ AND|OR [(]clause[)]]
		 *   condition := MAG|DEPTH|LAT|LON|PHASES op {float} |
		 *                UPDATED|OTIME op time |
		 *                MAG|DEPTH|LAT|LON|PHASES|OTIME|UPDATED IS [NOT] NULL
		 *   op        := =|>|>=|<|<=|eq|gt|ge|lt|ge
		 * @return True if the query could be executed
		 * @param updatedBufferSize Size of the buffer to store updated events
		 * until all archived events have been processed. If set to a negative
		 * value the buffer will be disabled. In this case archived and updated
		 * events may be mixed. If set to an insufficient positive value updated
		 * events may be lost if a buffer overflow occurs while archived events
		 * are still processed.
		 * @return True if the query could be executed and then was gracefully
		 * aborted.
		 */
		bool select(bool archived = false,
		            const Core::Time &from = Core::Time(),
		            const Core::Time &to = Core::Time(),
		            RequestFormat format = rfSummary,
		            const std::string &where = "",
		            int updatedBufferSize = 1000);

		/**
		 * @brief Gracefully aborts data transmission by sending an abort
		 * command to the server.
		 * @return True if the abort command could be sent
		 */
		bool abort();

		inline bool initialized() { return _sock; }
		inline bool interrupted() { return _sock && _sock->isInterrupted(); }

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

	private:
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

	protected:
		std::string             _logPrefix;

	private:
		std::string             _service;
		std::string             _user;
		std::string             _pass;
		Socket                 *_sock;

		int                     _options;

};

} // ns QuakeLink
} // ns IO
} // ns Seiscomp


#endif // __SEISCOMP_IO_QUAKELINK_CONNECTION_H__
