/***************************************************************************
 * libcapsclient
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


#include <gempa/caps/application.h>
#include <gempa/caps/log.h>

#include <csignal>
#include <locale.h>


namespace {


void signalHandler(int signal) {
	Gempa::CAPS::Application::Interrupt(signal);
}


void registerSignalHandler() {
	CAPS_DEBUG("Registering signal handler");

	signal(SIGTERM, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
}


}


namespace Gempa {
namespace CAPS {


Application *Application::_app = NULL;


Application::Application(int argc, char **argv) {
	_exitRequested = false;
	_argc = argc;
	_argv = argv;

	_app = this;

	registerSignalHandler();
}

void Application::done() {
	_exitRequested = true;
	 CAPS_DEBUG("leaving ::done");
}

int Application::exec() {
	_returnCode = 1;
	if ( init() ) {
		_returnCode = 0;

		if ( !run() && _returnCode == 0 )
			_returnCode = 1;

		done();
	}
	else
		done();

	return _returnCode;
}

void Application::exit(int returnCode) {
	_returnCode = returnCode;
	_exitRequested = true;
}

void Application::handleInterrupt(int signal) {
	switch ( signal ) {
		case SIGABRT:
			exit(-1);

		case SIGSEGV:
			exit(-1);

		default:
			this->exit(_returnCode);
	}
}

void Application::Interrupt(int signal) {
	if ( _app ) _app->handleInterrupt(signal);
}

bool Application::init() {
	setlocale(LC_ALL, "C");

	if ( !initCommandLine() ) {
		exit(1);
		return false;
	}

	if ( !initConfiguration() ) {
		exit(1);
		return false;
	}

	if ( !initCommandLine() ) {
		exit(1);
		return false;
	}

	if ( !validateParameters() ) {
		exit(1);
		return false;
	}

	return true;
}

bool Application::initCommandLine() {
	return true;
}

bool Application::initConfiguration() {
	return true;
}

bool Application::run() {
	return true;
}

bool Application::validateParameters() {
	return true;
}


}
}
