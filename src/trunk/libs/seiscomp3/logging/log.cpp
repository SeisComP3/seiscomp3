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


#define SEISCOMP_COMPONENT log
#include <seiscomp3/logging/log.h>
#include <seiscomp3/logging/channel.h>
#include <seiscomp3/logging/publisher.h>
#include <seiscomp3/logging/fd.h>

#include <boost/thread/mutex.hpp>


namespace Seiscomp {
namespace Logging {

#ifdef WIN32
#define STDERR_FILENO 2
#endif

void RegisterVA(PublishLoc *loc, Channel *channel, const char *format, va_list args ) {
	static boost::mutex registrationLock;
	boost::mutex::scoped_lock lock(registrationLock);

	loc->channel = channel;

	Publisher *pub = new Publisher(loc);

	loc->pub = pub;
	loc->publish = Publisher::Publish;
	loc->publishVA = Publisher::PublishVA;

	if ( pub->enabled() ) {
		loc->enable();

		// pass through to the publication function since it is active at
		// birth.
		Publisher::PublishVA(loc, channel, format, args);
	}
	else
		loc->disable();
}


void Register(PublishLoc *loc, Channel *channel, const char *format, ... ) {
	static boost::mutex registrationLock;
	boost::mutex::scoped_lock lock(registrationLock);

	loc->channel = channel;

	Publisher *pub = new Publisher(loc);

	loc->pub = pub;
	loc->publish = Publisher::Publish;
	loc->publishVA = Publisher::PublishVA;

	if ( pub->enabled() ) {
		loc->enable();

		// pass through to the publication function since it is active at
		// birth.
		va_list args;
		va_start (args, format);
		Publisher::PublishVA(loc, channel, format, args);
		va_end( args );
	}
	else
		loc->disable();
}


PublishLoc::~PublishLoc() {
	disable();
	if ( pub != NULL ) {
		delete pub;
		pub = NULL;
	}
}


static FdOutput __consoleLogger(STDERR_FILENO);


Channel* getAll() {
	return getGlobalChannel("*");
}


Channel* getComponentAll(const char* component) {
	return getComponentChannel(component, "*");
}


Channel* getComponentDebugs(const char* component) {
	return getComponentChannel(component, "debug");
}


Channel* getComponentInfos(const char* component) {
	return getComponentChannel(component, "info");
}


Channel* getComponentWarnings(const char* component) {
	return getComponentChannel(component, "warning");
}


Channel* getComponentErrors(const char* component) {
	return getComponentChannel(component, "error");
}


Channel* getComponentNotices(const char* component) {
	return getComponentChannel(component, "notice");
}


Output* consoleOutput() {
	return &__consoleLogger;
}


void enableConsoleLogging(Channel* channel) {
	__consoleLogger.subscribe(channel);
}


void disableConsoleLogging() {
	__consoleLogger.clear();
}


#define DO_LOG(CHANNEL) \
	SEISCOMP_##CHANNEL("%s", msg)

#define DO_CHANNEL(CHANNEL) \
	SEISCOMP_LOG(CHANNEL, "%s", msg)


void debug(const char* msg) {
	DO_LOG(DEBUG);
}


void info(const char* msg) {
	DO_LOG(INFO);
}


void warning(const char* msg) {
	DO_LOG(WARNING);
}


void error(const char* msg) {
	DO_LOG(ERROR);
}


void notice(const char* msg) {
	DO_LOG(NOTICE);
}


void log(Channel* ch, const char* msg) {
	DO_CHANNEL(ch);
}


}
}
