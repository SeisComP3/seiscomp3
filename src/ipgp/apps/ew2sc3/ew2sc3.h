/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/


#ifndef __IPGP_EARTHWORMTOSEISCOMP3_H__
#define __IPGP_EARTHWORMTOSEISCOMP3_H__


#include <string>
#include <vector>
#include <netdb.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/datamodel/publicobjectcache.h>
#include <seiscomp3/seismology/locatorinterface.h>


namespace Seiscomp {
namespace Logging {

class Output;
class Channel;

}
}

class EW2SC3 : public Seiscomp::Client::Application {

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit EW2SC3(int argc, char** argv);
		~EW2SC3();

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool initConfiguration();
		bool run();

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		/**
		 * @brief  Initializes connection to Earthworm socket
		 * @return EXIT_SUCCESS
		 */
		int ew_connect();

		/**
		 * Launched into boost thread!\n
		 * @brief  It endlessly listen to the socket in order to catch
		 *         incoming transmissions and after verifications, uses
		 *         extractOrigin() method to create a new origin.
		 * @return EXIT_SUCCESS
		 */
		int ew_read();

		/**
		 * Formats message before sending it to socket, by adding
		 * 'start character', 'logo' and 'end character' to it.\n
		 * Can also cut it into peaces if it's to big for the configured length.\n
		 * @param sock the socket to write into.
		 * @param msg the message
		 * @return 0/1 pending on writing success
		 */
		int ew_write(int sock, char* msg);

		/**
		 * @brief  Initializes the application variables by reading values
		 *         from configuration file.
		 * @return true/false pending on operation success
		 */
		bool config();

		/**
		 * @brief  Sends heartbeat to Earthworm server to let him know we
		 *         (as a client) are alive.
		 * @note   This method should be launched into a boost thread.
		 * @return EXIT_SUCCESS
		 */
		int sendHeartbeat();

		/**
		 * @brief  Filters the input message to extract the origin, the phases
		 *         and the others informations.
		 * @param  msg message to filter
		 * @return EXIT_SUCCESS
		 */
		int extractOrigin(char* msg);

		/**
		 * @brief  Sends a message into a socket.
		 * @param  sock the socket descriptor
		 * @param  buf message
		 * @param  buflen message length
		 * @param  flags message flag
		 * @param  timeout_msecs socket timeout
		 * @return number integer value of sent bytes
		 */
		int sendMessage(int sock, char* buf, int buflen, int flags, int timeout_msecs);

		double getResidual(const std::string& res);

		void runListenerThread();
		void runHeartbeatThread();
		void startListenerThread();
		void startHeartbeatThread();

	private:
		// ------------------------------------------------------------------
		//  Private methods
		// ------------------------------------------------------------------
		/**
		 * @brief  Converts std::string to double value
		 * @param  value std::string to convert
		 * @return double value of std::string
		 */
		const double to_double(const std::string& value);

		/**
		 * @brief  Converts std::string number to integer value
		 * @param  value std::string to convert
		 * @return integer value of std::string input
		 */
		const int to_int(const std::string& value);

		/**
		 * @brief  Replaces blank character into a string by another
		 *         string character
		 * @param  str reference string
		 * @param  str1 character of substitution
		 * @return string with replaces characters
		 */
		std::string blank_replace(std::string& str, std::string& str1);

		/**
		 * @brief  Converts sexagesimal value into decimal string
		 * @param  deg degree value
		 * @param  min minute value
		 * @param  pol polarity (S/W)
		 * @return decimal value of sexagesimal input
		 */
		const std::string sexagesimal_to_decimal(const double& deg,
		                                         const double& min,
		                                         const std::string& pol);

		/**
		 * @brief  Removes spaces from input string value
		 * @param  str input string
		 * @return spaces stripped value of input string
		 */
		std::string strip_space(std::string& str);

		/**
		 * @brief  Converts input value into a string
		 * @return string value of input value
		 */
		template<typename T> std::string to_string(const T& value);

		const double getSeiscompUncertainty(const double& value);

		void strExplode(const std::string& s, char c, std::vector<std::string>& v);

	private:
		// ------------------------------------------------------------------
		//  Private members
		// ------------------------------------------------------------------
		struct sockaddr_in serv_addr;
		struct hostent* server;

		int socketDescriptor;
		int senderPort;
		int senderTimeout;
		int messageReceiverStatus;
		int heartThreadStatus;
		int maxMsgSize;
		int myAliveInt;

		char* msgBuffer;
		char senderHeartText[256];
		char myAliveString[256];

		bool _connected;
		bool _archive;
		bool _enableUncertainties;

		int _instID;
		int _modID;

		std::string _configPath;
		std::string _instAuthor;
		std::string _senderHostName;
		std::string _defaultLatitude;
		std::string _defaultLongitude;
		std::string _locProfile;
		std::string _customAgencyID;
		std::string _myAliveString;
		std::string _senderAliveString;

		time_t _lastServerBeat;
		time_t _lastSocketBeat;

		double _uncertaintyMax;
		std::vector<double> _uncertainties;

		Seiscomp::Logging::Channel* _infoChannel;
		Seiscomp::Logging::Output* _infoOutput;
		Seiscomp::Logging::Channel* _errorChannel;
		Seiscomp::Logging::Output* _errorOutput;

	protected:
		// ------------------------------------------------------------------
		//  Protected members
		// ------------------------------------------------------------------
		boost::thread* _listenerThread;
		boost::thread* _heartbeatThread;
};

#endif
