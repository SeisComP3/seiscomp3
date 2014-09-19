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

#define SEISCOMP_COMPONENT admin

#include <iostream>
#include <iomanip>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/communication/clientstatus.h>
#include <seiscomp3/core/strings.h>

#include "admin.h"

using namespace Seiscomp;



template <typename Iterator>
void print(Iterator begin, Iterator end, const std::string& desc)
{
	std::cout << desc << std::endl;
	while (begin != end)
	{
		std::cout << *begin << std::endl;
		++begin;
	}
}



int main(int argc, char *argv[])
{
	// Command line parser
	std::string serverAddress;
	std::string password;
	std::string clientName;
	std::string group;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
	("help,h", "Show this help message")
	("verbose,v", "Show full output")
    ("serveraddress,H", boost::program_options::value<std::string>(&serverAddress),
	 "server (master client) address")
	("password", boost::program_options::value<std::string>(&password), "Password for server")
	("connectedclients,c", "Show connected clients")
	("availablegroups,g", "Show available groups")
	("groupmembers,m", boost::program_options::value<std::string>(&group), "Show member of given group")
	("disconnect,d", boost::program_options::value<std::string>(&clientName), "Disconnect client with given name")
	("status,s", boost::program_options::value<std::string>(&clientName), "Ask client/group for status message");

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	// Logging initialization
	Seiscomp::Logging::init(argc, argv);
	Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getComponentErrors("admin"));

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return EXIT_FAILURE;
	}

 	if (vm.count("verbose"))
 	{
 		Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getAll());
 	}

	Seiscomp::Admin admin;
	if (vm.count("serveraddress"))
	{
		if (!admin.connect(serverAddress, Communication::Protocol::DEFAULT_ADMIN_CLIENT_NAME, password))
		{
			std::cout << "Could not connect to address: " << serverAddress << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		std::cout << "Specifying a server address is mandatory" << std::endl;;
		std::cout << desc << "\n";
		return EXIT_FAILURE;
	}
	std::cout << "Successfully connected to address: " << serverAddress << std::endl;

	if (vm.count("connectedclients"))
	{
		std::vector<std::string> clients;
		if (admin.showConnectedClients(clients))
		{
			std::cout << "= Connected clients to master on " << serverAddress << " =" << std::endl;
			for (std::vector<std::string>::iterator it = clients.begin(); it != clients.end(); ++it)
			{
				std::vector<std::string> tokens;
				Core::split(tokens, (*it).c_str(), "?", false);
				std::cout << "Client Name: "     << tokens[0] << std::endl;
				if ( vm.count("verbose") ) {
					std::cout << "Client Type: "     << tokens[1] << std::endl;
					std::cout << "Client Priority: " << tokens[2] << std::endl;
					std::cout << "Program name: "    << tokens[3] << std::endl;
					std::cout << "IP(s): "           << tokens[5] << std::endl;
				}
				std::cout << "Hostname: "        << tokens[4] << std::endl;
				std::cout << "Client Uptime: "   << tokens[6] << std::endl;
				if (it + 1 != clients.end())
					std::cout << "==" << std::endl;
			}
		}
	}

	if (vm.count("availablegroups"))
	{
		std::vector<std::string> groups;
		if (admin.showAvailableGroups(groups))
			print(groups.begin(), groups.end(), "= Available Groups =");
	}

	if (vm.count("groupmembers"))
	{
		std::vector<std::string> members;
		if (admin.showGroupMembers(group, members))
		{
			std::string str = "= Members of group: " + group;
			print(members.begin(), members.end(), str);
		}
	}

	if (vm.count("disconnect"))
	{
		std::cout << "Disconnecting client: " << clientName << std::endl;
		admin.disconnectClient(clientName);
		std::cout << "Done" << std::endl;
	}

	if (vm.count("status"))
	{
		std::cout << "= Status for client: " << clientName << " =" << std::endl;
		Util::StopWatch timer;
		if ( !admin.requestHealthStatus(clientName) )
		{
			std::cout << "Client/Group " << clientName << " not listed in host database / error" << std::endl;
		}
		else {
			std::auto_ptr<Communication::ClientStatus> status = std::auto_ptr<Communication::ClientStatus>(admin.showHealthStatus());
			while ( status.get() )
			{
				std::cout << status->privateGroup() << std::endl;
				std::cout << "  response time (sec.):" << timer.elapsed() << std::endl;
				std::cout << "  hostname: " << status->hostname() << std::endl;
				std::cout << "  IP(s): " << status->ips() << std::endl;
				std::cout << "  program name: " << status->programName() << std::endl;
				std::cout << "  PID: " << status->pid() << std::endl;
				std::cout << "  CPU (%): " << status->cpuUsage() << std::endl;
				std::cout << "  total host memory (kB): " << status->totalMemory() << std::endl;
				std::cout << "  client memory usage (kB): " << status->clientMemoryUsage() << std::endl;
				std::cout << "  MEM (%): " << status->memoryUsage() << std::endl;
				std::cout << "  sent messages: "<< status->sentMessages() << std::endl;
				std::cout << "  received (queued) messages: " << status->receivedMessages() << std::endl;
				std::cout << "  current messagequeue size: " << status->messageQueueSize() << std::endl;
				std::cout << "  average messagequeue size: " << status->averageMessageQueueSize() << std::endl;
				std::cout << "  uptime: " << status->uptime() << std::endl;

				status = std::auto_ptr<Communication::ClientStatus>(admin.showHealthStatus());
			}
		}
	}

	admin.disconnect();

	return EXIT_SUCCESS;
}
