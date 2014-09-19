/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#include "app.h"


using namespace std;


namespace {


template <typename T>
void readConfig(const Seiscomp::Client::Application *, T *storage, const char *name) {}

template <typename T>
void readCLI(const Seiscomp::Client::CommandLine *, T *storage, const char *name) {}


#define IMPL_READ_CONFIG(T, method)\
template <>\
void readConfig<T>(const Seiscomp::Client::Application *app, T *storage, const char *name) {\
	try { *storage = app->method(name); }\
	catch ( ... ) {}\
}

template <>
void readCLI<bool>(const Seiscomp::Client::CommandLine *cli, bool *storage, const char *name) {
	if ( cli->hasOption(name) )
		*storage = true;
}


template <typename T>
struct OptionImpl : Application::Option {
	OptionImpl(T *var, const char *cfgname,
	           const char *cligroup = NULL,
	           const char *cliparam = NULL,
	           const char *clidesc = NULL,
	           bool clidefault = false,
	           bool cliswitch = false)
	: Application::Option(cfgname, cligroup, cliparam,
	                      clidesc, clidefault, cliswitch), storage(var) {}

	void bind(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return;
		if ( cliGroup != NULL ) cli->addGroup(cliGroup);
		if ( cliSwitch )
			cli->addOption(cliGroup?cliGroup:"Generic",
			               cliParam, cliDesc);
		else
			cli->addOption(cliGroup?cliGroup:"Generic",
			               cliParam, cliDesc, storage, cliDefault);
	}

	bool get(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return true;
		if ( cliSwitch ) readCLI(cli, storage, cliParam);
		return true;
	}

	bool get(const Seiscomp::Client::Application *app) {
		readConfig(app, storage, cfgName);
		return true;
	}

	void printStorage(std::ostream &os) {
		os << *storage;
	}

	T *storage;
};


template <typename T>
struct OptionVecImpl : Application::Option {
	OptionVecImpl(std::vector<T> *var, const char *cfgname,
				  const char *cligroup = NULL,
				  const char *cliparam = NULL,
				  const char *clidesc = NULL,
				  bool clidefault = false,
				  bool cliswitch = false)
	: Application::Option(cfgname, cligroup, cliparam,
	                      clidesc, clidefault, cliswitch), storage(var) {}

	void bind(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return;

		if ( cliGroup != NULL ) cli->addGroup(cliGroup);

		cli->addOption(cliGroup?cliGroup:"Generic",
					   cliParam, cliDesc, storage);
	}

	bool get(Seiscomp::Client::CommandLine *cli) {
		if ( cliParam == NULL ) return true;
		if ( cliSwitch ) readCLI(cli, storage, cliParam);
		return true;
	}

	bool get(const Seiscomp::Client::Application *app) {
		readConfig(app, storage, cfgName);
		return true;
	}

	void printStorage(std::ostream &os) {
		bool first = true;
		for ( typename std::vector<T>::iterator it = storage->begin();
			  it != storage->end(); ++it ) {
			if ( !first ) os << ",";
			else first = false;
			os << *it;
		}
	}

	std::vector<T> *storage;
};


template <typename T>
Application::OptionPtr
bind(T *var,
     const char *cfgname, const char *cligroup = NULL,
     const char *cliparam = NULL, const char *clidesc = NULL,
     bool clidefault = false, bool cliswitch = false) {
	return new OptionImpl<T>(var, cfgname, cligroup, cliparam, clidesc,
                             clidefault, cliswitch);
}

template <typename T>
Application::OptionPtr
bind(std::vector<T> *var, const char *cfgname, const char *cligroup = NULL,
     const char *cliparam = NULL, const char *clidesc = NULL,
     bool clidefault = false, bool cliswitch = false) {
	return new OptionVecImpl<T>(var, cfgname, cligroup, cliparam, clidesc,
                                clidefault, cliswitch);
}


IMPL_READ_CONFIG(int, configGetInt)
IMPL_READ_CONFIG(vector<int>, configGetInts)
IMPL_READ_CONFIG(double, configGetDouble)
IMPL_READ_CONFIG(vector<double>, configGetDoubles)
IMPL_READ_CONFIG(bool, configGetBool)
IMPL_READ_CONFIG(vector<bool>, configGetBools)
IMPL_READ_CONFIG(string, configGetString)
IMPL_READ_CONFIG(vector<string>, configGetStrings)


}


Application::Application(int argc, char **argv)
: Seiscomp::Client::StreamApplication(argc, argv) {}


void Application::createCommandLineDescription() {
	Seiscomp::Client::StreamApplication::createCommandLineDescription();

	for ( Options::iterator it = _options.begin(); it != _options.end(); ++it )
		(*it)->bind(&commandline());
}


bool Application::validateParameters() {
	if ( !Seiscomp::Client::StreamApplication::validateParameters() ) return false;

	for ( Options::iterator it = _options.begin(); it != _options.end(); ++it )
		if ( !(*it)->get(&commandline()) ) return false;

	return true;
}


bool Application::initConfiguration() {
	if ( !Seiscomp::Client::StreamApplication::initConfiguration() ) return false;

	for ( Options::iterator it = _options.begin(); it != _options.end(); ++it )
		if ( !(*it)->get(this) ) return false;

	return true;
}


void Application::addOption(OptionPtr opt) {
	_options.push_back(opt);
}


const Application::Options &Application::options() const {
	return _options;
}


#define IMPL_ADD_OPTION(TYPE) \
void Application::addOption(TYPE *var, const char *cfgname,\
                            const char *cligroup, const char *cliparam,\
                            const char *clidesc, bool clidefault,\
                            bool cliswitch) {\
	addOption(bind(var, cfgname, cligroup, cliparam, clidesc, clidefault, cliswitch));\
}


IMPL_ADD_OPTION(int)
IMPL_ADD_OPTION(double)
IMPL_ADD_OPTION(bool)
IMPL_ADD_OPTION(string)

IMPL_ADD_OPTION(vector<int>)
IMPL_ADD_OPTION(vector<double>)
IMPL_ADD_OPTION(vector<bool>)
IMPL_ADD_OPTION(vector<string>)
