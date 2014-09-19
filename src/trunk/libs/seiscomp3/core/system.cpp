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


#define SEISCOMP_COMPONENT System
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/platform/platform.h>
#include <stdlib.h>

#ifdef MACOSX
	#include <sys/param.h>
	#define HOST_NAME_MAX MAXHOSTNAMELEN
#endif


#ifndef WIN32
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif



#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

namespace Seiscomp {
namespace Core {

std::string getHostname() {
#ifdef WIN32
	static bool initialized = false;
	if ( !initialized ) {
		WSADATA wsaData;
		int wsaerr = WSAStartup(MAKEWORD(2, 0), &wsaData);
		if (wsaerr != 0) {
			SEISCOMP_ERROR("WSAStartup failed with error: %d", wsaerr);
		}
		initialized = true;
	}
#endif

	char hostname[HOST_NAME_MAX];
	if ( gethostname(hostname, HOST_NAME_MAX) != 0 ) {
		const char* name = NULL;
		name = getenv("HOSTNAME");
		return (name) ? name : "";
	}
	return hostname;
}




std::string getLogin() {
#ifdef WIN32
	const char *name = getenv("USERNAME");
	return (name) ? name : "";
#else
	char buf[100];
	if ( !getlogin_r(buf, 100) )
		return buf;
	return "";
#endif
}




void sleep(unsigned long seconds) {
#ifndef WIN32
	::sleep(seconds);
#else
	Sleep(seconds*1000);
#endif
}




void msleep(unsigned long milliseconds) {
#ifndef WIN32
	::usleep(milliseconds * 1000);
#else
	Sleep(milliseconds);
#endif
}




unsigned int pid() {
#if defined(_MSC_VER)
	return GetCurrentProcessId();
#else
	return (unsigned int)getpid();
#endif
}




bool system(const std::string& command) {
	return std::system(command.c_str()) == -1 ? false : true;
}


} // namespace Core
} // namespace Seiscomp

