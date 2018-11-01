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


#define SEISCOMP_COMPONENT Application

#include <seiscomp3/core/platform/platform.h>

#include <seiscomp3/logging/fd.h>
#include <seiscomp3/logging/filerotator.h>
#ifndef WIN32
#include <seiscomp3/logging/syslog.h>
#endif

#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/communication/connectioninfo.h>

#include <seiscomp3/math/geo.h>

#include <seiscomp3/utils/files.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/utils/replace.h>

#include <seiscomp3/seismology/regions.h>

#include <seiscomp3/client/application.h>
#include <seiscomp3/client/pluginregistry.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/client/configdb.h>

#include <seiscomp3/datamodel/config.h>
#include <seiscomp3/datamodel/configmodule.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/version.h>

#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/recordstream.h>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/client/queue.ipp>

#include <seiscomp3/utils/files.h>

#include <sstream>

#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <fcntl.h>

#ifdef __SUNPRO_CC
#include <sys/stat.h>
#endif

#include <boost/bind.hpp>

#ifdef WIN32
#define snprintf _snprintf
#define popen _popen
#define pclose _pclose
#define STDERR_FILENO 2
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::Communication;
using namespace Seiscomp::IO;


namespace {

class FlagCounter: public boost::program_options::untyped_value {
	public:
		FlagCounter(unsigned int* count);
		void xparse(boost::any&, const vector<string>&) const;

	private:
		unsigned int* _count;
};

FlagCounter::FlagCounter(unsigned int* count)
	: boost::program_options::untyped_value(true), _count(count) {
}

void FlagCounter::xparse(boost::any&, const vector<string>&) const {
	++(*_count);
}

struct AppResolver : public Util::VariableResolver {
	AppResolver(const std::string& name)
	 : _name(name) {}

	bool resolve(std::string& variable) const {
		if ( Util::VariableResolver::resolve(variable) )
			return true;

		if ( variable == "appname" )
			variable = _name;
		else
			return false;

		return true;
	}

	const std::string& _name;
};



/*
void printTraces() {
    #ifndef MACOSX
	void *array[20];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 20);
	strings = backtrace_symbols(array, size);

	SEISCOMP_ERROR(" Obtained %zd stack frames:", size);
	SEISCOMP_ERROR(" --------------------------");

	for (i = 0; i < size; i++)
		SEISCOMP_ERROR(" #%.2d %s", i, strings[i]);

	free(strings);
    #endif
}

int runGDB() {
	char tmp[]="/tmp/sc3-crash-XXXXXX";
	int fd;
	bool full_bt = false;

	fd = mkstemp(tmp);
	if( fd == -1 )
		return -1;
	else {
		char gdb_cmd[]="bt\nquit";
		char gdb_cmd_full[]="bt full\nquit";
		char cmd[128];
		FILE *fp;

		if( full_bt )
			write(fd, gdb_cmd_full, strlen(gdb_cmd_full));
		else
			write(fd, gdb_cmd, strlen(gdb_cmd));
		close(fd);

		snprintf(cmd, sizeof(cmd), "gdb -nw -n -batch -x \"%s\" --pid=%d",
		         tmp, getpid());
		cmd[sizeof(cmd)-1]=0;

		fflush(NULL);
		fp = popen(cmd, "r");
		if( !fp )
			return -1;
		else {
			char buff[4096];
			size_t len;

			while(fgets(buff, sizeof(buff), fp)) {
				len = strlen(buff);
				if( buff[len-1] == '\n')
					buff[len-1]=0;

				SEISCOMP_ERROR(" %s", buff);
			}
			pclose(fp);
		}
	}

	return 0;
}
*/

void crashHandler() {
	/*
	if ( runGDB() == -1 )
		printTraces();
	*/

	Application* app = Application::Instance();
	if ( !app ) return;

	const std::string& crash_handler = app->crashHandler();
	if ( crash_handler.empty() ) return;

	FILE* fp;/* = fopen(crash_handler, "rb");
	if ( fp == NULL ) {
		SEISCOMP_ERROR("crash handler executable '%s' not found", crash_handler);
		return;
	}

	fclose(fp);
	*/

	char buff[4096];
	size_t len;

	snprintf(buff, sizeof(buff), "%s %s %d", crash_handler.c_str(), app->path(), Core::pid());
	SEISCOMP_INFO("Running command: %s", buff);

	fp = popen(buff, "r");
	if ( ! fp ) return;

	while ( fgets(buff, sizeof(buff), fp) ) {
		len = strlen(buff);
		if( buff[len-1] == '\n')
			buff[len-1] = 0;

		SEISCOMP_INFO(" %s", buff);
	}
	pclose(fp);
}

void signalHandler(int signal) {
	// Prevent running the crashHandler again when the crashHandler crashes
	static bool signalCatched = false;

	// Logging disabled due to internal mutex locking that can lock the whole
	// application when interrupting a normal log operation and logging here again

	switch ( signal ) {
		case SIGABRT:
			//SEISCOMP_ERROR("ABORT");
			//SEISCOMP_ERROR("BACKTRACE:");
			//crashHandler();
			exit(-1);

		case SIGSEGV:
			//SEISCOMP_ERROR("SEGFAULT");
			if ( !signalCatched ) {
				signalCatched = true;
				//SEISCOMP_ERROR("BACKTRACE:");
				crashHandler();
			}
			exit(-1);

		default:
/*
#if !defined(WIN32) && !defined(__SUNPRO_CC) && !defined(sun)
			SEISCOMP_INFO("Receiving signal: %s", sys_siglist[signal]);
#else
			SEISCOMP_INFO("Receiving signal: %d", signal);
#endif
*/
			InterruptibleObject::Interrupt(signal);
	}
}

void registerSignalHandler(bool term, bool crash) {
	static volatile bool termRegistered = false;
	static volatile bool crashRegistered = false;

	SEISCOMP_DEBUG("Registering signal handler");

	if ( crash && crashRegistered )
		return;

	if ( crash ) {
#ifndef WIN32
		struct sigaction sa;
		sa.sa_handler = signalHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGSEGV, &sa, NULL);
		sigaction(SIGABRT, &sa, NULL);

		sa.sa_handler = SIG_IGN;
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGPIPE, &sa, NULL);
#else
		signal(SIGSEGV, signalHandler);
		signal(SIGABRT, signalHandler);
		signal(SIGPIPE, SIG_IGN);
#endif

		crashRegistered = true;
	}

	if ( term && termRegistered )
		return;

	if ( term ) {
#ifndef WIN32
		struct sigaction sa;
		sa.sa_handler = signalHandler;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGTERM, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);

		sa.sa_handler = SIG_IGN;
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGPIPE, &sa, NULL);
#else
		signal(SIGTERM, signalHandler);
		signal(SIGINT, signalHandler);
		signal(SIGPIPE, SIG_IGN);
#endif

		termRegistered = true;
	}
}


struct CityLessThan {
	bool operator()(const Math::Geo::CityD &x, const Math::Geo::CityD &y) {
		return x.population() < y.population();
	}
};

struct CityGreaterThan {
	bool operator()(const Math::Geo::CityD &x, const Math::Geo::CityD &y) {
		return x.population() > y.population();
	}
};

struct ParamRef {
	ParamRef() : param(NULL) {}
	ParamRef(System::SchemaParameter *param, const string &reference)
	  : param(param), reference(reference) {}
	System::SchemaParameter* param;
	string                   reference;
};

typedef map<string, ParamRef> ParamMap;
void mapSchemaParameters(ParamMap &map, System::SchemaParameters *params,
                         const string &reference, const string &path = "") {
	if ( params == NULL )
		return;

	string p = path.empty() ? "" : path + ".";
	for ( size_t i = 0; i < params->parameterCount(); ++i ) {
		System::SchemaParameter *param = params->parameter(i);
		map[p + param->name] = ParamRef(param, reference);
	}

	for ( size_t i = 0; i < params->groupCount(); ++i ) {
		System::SchemaGroup *group = params->group(i);
		mapSchemaParameters(map, group, reference, p + group->name);
	}
}

// comparison of strings separated into parts by '.' character
bool compare_string_toks(const std::string& a, const std::string& b) {
	vector<string> ta, tb;
	Core::split(ta, a.c_str(), ".", false);
	Core::split(tb, b.c_str(), ".", false);

	if ( ta.size() == 1 && tb.size() > 1 )
		return true;
	if ( ta.size() > 1 && tb.size() == 1 )
		return false;

	vector<string>::const_iterator a_it = ta.begin(), b_it = tb.begin();
	for ( ; a_it != ta.end() && b_it != tb.end(); ++a_it, ++b_it ) {
		int cmp = Core::compareNoCase(*a_it, *b_it);
		if ( cmp < 0 )
			return true;
		if ( cmp > 0 )
			return false;
	}

	return ta.size() <= tb.size();
}

// pad string to spefic length
inline string pad(const string &s, size_t len, char c = ' ') {
	return s.length() >= len ? s : (s + string(len-s.length(), c));
}

} // unnamed namespace


namespace Seiscomp {
namespace Client {


Application* Application::_instance = NULL;
bool Application::_handleTermination = true;
bool Application::_handleCrash = false;

IMPLEMENT_SC_CLASS_DERIVED(ApplicationStatusMessage, Message, "app_stat_msg");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage(const std::string &module,
                                                   ApplicationStatus status)
: Message(), _module(module), _status(status) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatusMessage::ApplicationStatusMessage(const std::string &module,
                                                   const std::string &username,
                                                   ApplicationStatus status)
: Message(), _module(module), _username(username), _status(status) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ApplicationStatusMessage::empty() const { return false; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &ApplicationStatusMessage::module() const { return _module; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &ApplicationStatusMessage::username() const { return _username; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatus ApplicationStatusMessage::status() const { return _status; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ApplicationStatusMessage::serialize(Archive& ar) {
	ar & TAGGED_MEMBER(module);
	ar & TAGGED_MEMBER(username);
	ar & TAGGED_MEMBER(status);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int argc, char** argv)
 : _messageThread(NULL) {
	_version = CurrentVersion.toString();

	_inputMonitor = _outputMonitor = NULL;
	_objectLogTimeWindow = 60;

	if ( _instance != this && _instance != NULL ) {
		SEISCOMP_WARNING("Another application object exists already. "
		                 "This usage is not intended. "
		                 "The Application::Instance() method will return "
		                 "the last created application.");
	}

	_instance = this;

	_commandline = boost::shared_ptr<CommandLine>(new CommandLine);

	prepare(argc, argv);

	registerSignalHandler(_handleTermination, _handleCrash);

	_enableDaemon = true;
	_enableMessaging = true;
	_enableDatabase = true;
	_enableRecordStream = false;
	_enableFetchDatabase = true;
	_enableLoadStations = false;
	_enableLoadInventory = false;
	_enableLoadConfigModule = false;
	_enableAutoApplyNotifier = true;
	_enableInterpretNotifier = true;
	_enableLoadCities = false;
	_enableLoadRegions = false;
	_enableStartStopMessages = false;
	_enableAutoShutdown = false;
	_customPublicIDPattern = false;

	_retryCount = 0xFFFFFFFF;

	_verbosity = 2;
	_logContext = false;
	_logComponent = -1; // -1=unset, 0=off, 1=on
	_logToStdout = false;
	_logUTC = false;
	_messagingTimeout = 3;
	_messagingHost = "localhost";
	_messagingPrimaryGroup = Communication::Protocol::LISTENER_GROUP;

	_configModuleName = "trunk";

	_returnCode = 0;
	_exitRequested = false;

	DataModel::Notifier::SetEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {
	closeLogging();

	if ( _inputMonitor ) delete _inputMonitor;
	if ( _outputMonitor ) delete _outputMonitor;

	if ( _instance == this )
		_instance = NULL;

	for ( int i = 0; i < _argc; ++i )
		delete[] _argv[i];

	delete[] _argv;

	// Remove all queued notifiers
	DataModel::Notifier::Clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application* Application::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::HandleSignals(bool termination, bool crash) {
	_handleTermination = termination;
	_handleCrash = crash;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::prepare(int argc, char** argv) {
	_logger = NULL;

	_argc = argc;
	_argv = new char*[argc];

	_arguments.clear();
	for ( int i = 0; i < argc; ++i ) {
		_arguments.push_back(argv[i]);
		_argv[i] = new char[strlen(argv[i]) + 1];
		strcpy(_argv[i], argv[i]);
	}

	if ( argc > 0 )
		_name = argv[0];
	else
		_name = "";

	size_t pos = _name.rfind('/');
	if ( pos != string::npos )
		_name.erase(0, pos+1);

	pos = _name.rfind('\\');
	if ( pos != string::npos )
		_name.erase(0, pos+1);

	pos = _name.rfind('.');
	if ( pos != string::npos )
		_name.erase(pos);

	if ( _messagingUser.empty() )
		_messagingUser = _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef WITH_SVN_REVISION
extern SC_SYSTEM_CLIENT_API const char* svn_revision();
#endif
#ifdef WITH_BUILD_INFOS
//extern const char* last_build();
extern SC_SYSTEM_CLIENT_API const char* build_system();
extern SC_SYSTEM_CLIENT_API const char* compiler_version();
#endif
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printVersion() {
	const char *appVersion = version();
	if ( appVersion == NULL )
		cout << name() << ": " << frameworkVersion() << endl;
	else {
		cout << name() << ": " << appVersion << endl;
		cout << "Framework: " << frameworkVersion() << endl;
	}
	cout << "API version: "
	     << SC_API_VERSION_MAJOR(SC_API_VERSION) << "."
	     << SC_API_VERSION_MINOR(SC_API_VERSION) << "."
	     << SC_API_VERSION_PATCH(SC_API_VERSION) << endl;
	cout << CurrentVersion.systemInfo() << endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printConfigVariables() {
	list<string> varList;
	Config::Variables::const_iterator cv_it = _configuration.getVariables().begin();
	for ( ; cv_it != _configuration.getVariables().end(); ++cv_it ) {
		varList.push_back(pad(cv_it->first, 50) + " " + cv_it->second);
	}

	varList.sort(compare_string_toks);
	cout << "available configuration variables:" << endl;
	list<string>::const_iterator it = varList.begin();
	for ( ; it != varList.end(); ++it ) {
		cout << "  " << *it << endl;
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::schemaValidationNames(vector<string> &modules,
                                        vector<string> &plugins) const {

	// process global and application secific modules
	modules.push_back("global");
	modules.push_back(name());

	// process all loaded plugins
	for ( PluginRegistry::iterator it = PluginRegistry::Instance()->begin();
		it != PluginRegistry::Instance()->end(); ++it ) {
		plugins.push_back(Util::removeExtension(Util::basename(it->filename)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateSchemaParameters() {
	System::SchemaDefinitions defs;
	string path = Environment::Instance()->appConfigDir() + "/descriptions";
	if ( !defs.load(path.c_str()) ) {
		cerr << "Could not load schema definitions from directory " << path << endl;
		return false;
	}

	vector<string> moduleNames;
	vector<string> pluginNames;
	schemaValidationNames(moduleNames, pluginNames);

	ParamMap paramMap;

	// map module parameter path to SchemaParameter instance
	for ( vector<string>::const_iterator mn_it = moduleNames.begin();
	      mn_it != moduleNames.end(); ++mn_it ) {
		System::SchemaModule *module = defs.module(*mn_it);
		cerr << "Schema module '" << *mn_it << "' ";
		if ( module == NULL ) {
			cerr << "not found" << endl;
		}
		else {
			cerr << "loaded" << endl;
			mapSchemaParameters(paramMap, module->parameters.get(),
			                    "module " + module->name);
		}

		System::SchemaDefinitions::PluginList plugins = defs.pluginsForModule(*mn_it);
		System::SchemaDefinitions::PluginList::const_iterator p_it = plugins.begin();
		for ( ; p_it != plugins.end(); ++p_it ) {
			vector<string>::iterator pn_it = find(pluginNames.begin(),
			                                      pluginNames.end(), (*p_it)->name);
			if ( pn_it != pluginNames.end() ) {
				mapSchemaParameters(paramMap, (*p_it)->parameters.get(),
				                    "plugin " + (*p_it)->name);
				cerr << "Schema plugin '" << *pn_it << "' loaded" << endl;
				pluginNames.erase(pn_it);
			}
		}
	}

	if ( paramMap.empty() ) {
		cerr << "No schema parameters found" << endl;
		return false;
	}

	map<string, string> typeMappings;
	typeMappings["color"] = "string";
	typeMappings["host-with-port"] = "string";
	typeMappings["gradient"] = "list:string";
	typeMappings["path"] = "string";

	list<string> missing;
	list<string> invalidType;
	list<string> emptyDesc;

	Config::Variables::const_iterator cv_it = _configuration.getVariables().begin();
	for ( ; cv_it != _configuration.getVariables().end(); ++cv_it ) {
		ParamMap::iterator pm_it = paramMap.find(cv_it->first);
		if ( pm_it == paramMap.end() ) {
			missing.push_back(cv_it->first);
			continue;
		}

		if ( cv_it->second != pm_it->second.param->type ) {
			map<string, string>::const_iterator tm_it =
			        typeMappings.find(pm_it->second.param->type);
			if ( tm_it == typeMappings.end() || tm_it->second != cv_it->second ) {
				string line = pad(cv_it->first, 40);
				line += " expected: " + cv_it->second + ", ";
				if ( pm_it->second.param->type.empty() )
					line += "undefined in schema";
				else
					line += "found in schema: " + pm_it->second.param->type;
				invalidType.push_back(line + " [" +
				                      pm_it->second.reference + "]");
			}
		}

		if ( pm_it->second.param->description.empty() ) {
			emptyDesc.push_back(pad(cv_it->first, 40) + " [" +
			                    pm_it->second.reference + "]");
		}

		paramMap.erase(pm_it);
	}

	if ( !missing.empty() ) {
		missing.sort(compare_string_toks);
		cout << endl
		     << "parameters missing in schema:" << endl;
		list<string>::const_iterator it = missing.begin();
		for ( ; it != missing.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !invalidType.empty() ) {
		invalidType.sort(compare_string_toks);
		cout << endl
		     << "parameters of invalid type:" << endl;
		list<string>::const_iterator it = invalidType.begin();
		for ( ; it != invalidType.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !emptyDesc.empty() ) {
		emptyDesc.sort(compare_string_toks);
		cout << endl
		     << "parameters without schema description:" << endl;
		list<string>::const_iterator it = emptyDesc.begin();
		for ( ; it != emptyDesc.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	if ( !paramMap.empty() ) {
		list<string> paramList;
		ParamMap::const_iterator pm_it = paramMap.begin();
		for ( ; pm_it != paramMap.end(); ++pm_it ) {
			string line = pad(pm_it->first, 40);
			paramList.push_back(line + " [" + pm_it->second.reference + "]");
		}
		paramList.sort(compare_string_toks);
		cout << endl
		     << "parameters found in schema but not read by application:" << endl;
		list<string>::const_iterator it = paramList.begin();
		for ( ; it != paramList.end(); ++it ) {
			cout << "  " << *it << endl;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::name() const {
	return _name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addPluginPackagePath(const std::string &package) {
	Seiscomp::Client::PluginRegistry::Instance()->addPackagePath(package);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Application::Arguments& Application::arguments() const {
	return _arguments;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Config::Config &Application::configuration() const {
	return _configuration;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* Application::path() const {
	return _argv[0];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDatabaseEnabled(bool enable, bool tryToFetch) {
	_enableDatabase = enable;
	_enableFetchDatabase = tryToFetch;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isDatabaseEnabled() const {
	return _enableDatabase;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isInventoryDatabaseEnabled() const {
	return _inventoryDB.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isConfigDatabaseEnabled() const {
	return _configDB.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addMessagingSubscription(const std::string& group) {
	_messagingSubscriptionRequests.push_back(group);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDaemonEnabled(bool enable) {
	_enableDaemon = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMessagingEnabled(bool enable) {
	_enableMessaging = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isMessagingEnabled() const {
	return _enableMessaging;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setStartStopMessagesEnabled(bool enable) {
	_enableStartStopMessages = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::areStartStopMessagesEnabled() const {
	return _enableStartStopMessages;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setAutoShutdownEnabled(bool enable) {
	_enableAutoShutdown = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAutoShutdownEnabled() const {
	return _enableAutoShutdown;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setRecordStreamEnabled(bool enable) {
	_enableRecordStream = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isRecordStreamEnabled() const {
	return _enableRecordStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadStationsEnabled(bool enable) {
	_enableLoadStations = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadStationsEnabled() const {
	return _enableLoadStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadInventoryEnabled(bool enable) {
	_enableLoadInventory = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadInventoryEnabled() const {
	return _enableLoadInventory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadConfigModuleEnabled(bool enable) {
	_enableLoadConfigModule = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadConfigModuleEnabled() const {
	return _enableLoadConfigModule;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadCitiesEnabled(bool enable) {
	_enableLoadCities = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadCitiesEnabled() const {
	return _enableLoadCities;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoadRegionsEnabled(bool enable) {
	_enableLoadRegions = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isLoadRegionsEnabled() const {
	return _enableLoadRegions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setAutoApplyNotifierEnabled(bool enable) {
	_enableAutoApplyNotifier = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAutoApplyNotifierEnabled() const {
	return _enableAutoApplyNotifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setInterpretNotifierEnabled(bool enable) {
	_enableInterpretNotifier = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isInterpretNotifierEnabled() const {
	return _enableInterpretNotifier;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::hasCustomPublicIDPattern() const {
	return _customPublicIDPattern;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setConnectionRetries(unsigned int r) {
	_retryCount = r;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingContext(bool e) {
	_logContext = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingComponent(bool e) {
	_logComponent = e ? 1 : 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setLoggingToStdErr(bool e) {
	_logToStdout = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::addLoggingComponentSubscription(const std::string& c) {
	_logComponents.push_back(c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setConfigModuleName(const std::string &module) {
	_configModuleName = module;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::configModuleName() const {
	return _configModuleName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setShutdownMasterModule(const std::string &module) {
	_shutdownMasterModule = module;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setShutdownMasterUsername(const std::string &name) {
	_shutdownMasterUsername = name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setPrimaryMessagingGroup(const std::string& group) {
	_messagingPrimaryGroup = group;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::primaryMessagingGroup() const {
	return _messagingPrimaryGroup;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMessagingUsername(const std::string& user) {
	_messagingUser = user;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isExitRequested() const {
	return _exitRequested;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::printUsage() const {
	commandline().printOptions();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closeLogging() {
	if ( _logger ) {
		delete _logger;
		_logger = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Communication::Connection* Application::connection() const {
	return _connection.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseType() const {
	return _dbType;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseParameters() const {
	return _dbParameters;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IO::DatabaseInterface* Application::database() const {
	return _database.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string &Application::databaseURI() const {
	return _db;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::DatabaseQuery* Application::query() const {
	return _query.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string&  Application::recordStreamURL() const {
	return _recordStream;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector<Math::Geo::CityD>& Application::cities() const {
	return _cities;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Math::Geo::CityD *Application::nearestCity(double lat, double lon,
                                                 double maxDist, double minPopulation,
                                                 double *dist, double *azi) const {
	return Math::Geo::nearestCity(lat, lon, maxDist, minPopulation, _cities, dist, azi);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ConfigModule *Application::configModule() const {
	return _configModule.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isStationEnabled(const std::string& networkCode,
                                   const std::string& stationCode) {
	if ( _configModule ) {
		for ( size_t i = 0; i < _configModule->configStationCount(); ++i ) {
			DataModel::ConfigStation* cs = _configModule->configStation(i);
			if ( cs->networkCode() == networkCode &&
			     cs->stationCode() == stationCode )
				return cs->enabled();
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::messagingHost() const {
	return _messagingHost;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::enableTimer(unsigned int seconds) {
	if ( !seconds ) {
		_userTimer.stop();
		return;
	}

	_userTimer.setTimeout(seconds);

	if ( _userTimer.isActive() ) return;

	_userTimer.setCallback(boost::bind(&Application::timeout, this));
	_userTimer.start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::disableTimer() {
	_userTimer.disable();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::crashHandler() const {
	return _crashHandler;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Application::argumentStr(const std::string& query) const {
	std::string param = "--" + query;
	for ( size_t i = 1; i < _arguments.size(); ++i ) {
		if ( !_arguments[i].compare(0, param.size(), param) ) {
			std::string value = _arguments[i].substr(param.size());
			if ( !value.empty() && value[0] == '=' ) {
				value.erase(0, 1);
				return value;
			}
		}
	}

	throw Seiscomp::Config::OptionNotFoundException(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::configGetInt(const std::string& query) const {
	try {
		return atoi(argumentStr(query).c_str());
	}
	catch ( ... ) {}

	return _configuration.getInt(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double Application::configGetDouble(const std::string& query) const {
	try {
		return atof(argumentStr(query).c_str());
	}
	catch ( ... ) {}

	return _configuration.getDouble(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::configGetBool(const std::string& query) const {
	try {
		return argumentStr(query) == "true";
	}
	catch ( ... ) {}

	return _configuration.getBool(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Application::configGetString(const std::string& query) const {
	try {
		return argumentStr(query);
	}
	catch ( ... ) {}

	return _configuration.getString(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Application::configGetPath(const std::string& query) const {
	try {
		return Environment::Instance()->absolutePath(argumentStr(query));
	}
	catch ( ... ) {}

	return Environment::Instance()->absolutePath(_configuration.getString(query));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<int> Application::configGetInts(const std::string& query) const {
	return _configuration.getInts(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<double> Application::configGetDoubles(const std::string& query) const {
	return _configuration.getDoubles(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<bool> Application::configGetBools(const std::string& query) const {
	return _configuration.getBools(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<std::string> Application::configGetStrings(const std::string& query) const {
	try {
		std::string param = argumentStr(query);
		std::vector<std::string> tmp;
		Core::split(tmp, param.c_str(), ",");
		for ( size_t i = 0; i < tmp.size(); ++i )
			Core::trim(tmp[i]);
		return tmp;
	}
	catch ( ... ) {}

	return _configuration.getStrings(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetBool(const std::string& query, bool v) {
	_configuration.setBool(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetInt(const std::string& query, int v) {
	_configuration.setInt(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetDouble(const std::string& query, double v) {
	_configuration.setDouble(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetString(const std::string& query, const std::string &v) {
	_configuration.setString(query, v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetBools(const std::string& query,
                                 const std::vector<bool> &values) {
	_configuration.setBools(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetInts(const std::string& query,
                                const std::vector<int> &values) {
	_configuration.setInts(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetDoubles(const std::string& query,
                                   const std::vector<double> &values) {
	_configuration.setDoubles(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetStrings(const std::string& query,
                                   const std::vector<std::string> &values) {
	_configuration.setStrings(query, values);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configUnset(const std::string& query) {
	_configuration.remove(query);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::saveConfiguration() {
	return _configuration.writeConfig(Environment::Instance()->configFileName(name()),
	                                  true, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::operator()() {
	return exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::exec() {
	_exitRequested = false;
	_returnCode = 1;

	if ( init() ) {
		_returnCode = 0;

		// If run return false and the returnCode still indicates no error
		// it is set to 1 to indicate wrong behaviour.
		if ( !run() && _returnCode == 0 )
			_returnCode = 1;

		done();
	}
	else
		done();

	if ( !_logToStdout ) SEISCOMP_NOTICE("Shutdown");

	return _returnCode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initPlugins() {
	PluginRegistry::Instance()->addPackagePath(name());

	if ( !_plugins.empty() ) {
		std::vector<std::string> tokens;
		Core::split(tokens, _plugins.c_str(), ",");
		std::vector<std::string>::iterator it = tokens.begin();
		for ( ; it != tokens.end(); ++it )
			PluginRegistry::Instance()->addPluginName(Core::trim(*it));

		if ( PluginRegistry::Instance()->loadPlugins() < 0 ) {
			SEISCOMP_ERROR("Failed to load all requested plugins, bailing out");
			return false;
		}
	}
	else {
		if ( PluginRegistry::Instance()->loadConfiguredPlugins(&_configuration) < 0 ) {
			SEISCOMP_ERROR("Failed to load all requested plugins, bailing out");
			return false;
		}
	}

	if ( PluginRegistry::Instance()->pluginCount() ) {
		std::string pluginList;
		pluginList = "\nPlugins:\n"
					  "--------\n";
		int idx = 1;
		for ( PluginRegistry::iterator it = PluginRegistry::Instance()->begin();
			it != PluginRegistry::Instance()->end(); ++it ) {
			pluginList += " [" + toString(idx) + "]\n";
			pluginList += "  description: " + (*it)->description().description + "\n";
			pluginList += "       author: " + (*it)->description().author + "\n";
			pluginList += "      version: " + toString((*it)->description().version.major)
			                                + "." + toString((*it)->description().version.minor)
			                                + "." + toString((*it)->description().version.revision)
			                                + "\n";
			pluginList += "          API: " + toString(SC_API_VERSION_MAJOR((*it)->description().apiVersion))
			                                + "." + toString(SC_API_VERSION_MINOR((*it)->description().apiVersion))
			                                + "." + toString(SC_API_VERSION_PATCH((*it)->description().apiVersion))
			                                + "\n";
			++idx;
		}

		SEISCOMP_INFO("%s", pluginList.c_str());
	}
	else
		SEISCOMP_INFO("No plugins loaded");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	setlocale(LC_ALL, "C");

	Logging::enableConsoleLogging(Logging::getGlobalChannel("error"));

	showMessage("Reading configuration");

	// First commandline parsing stage
	initCommandLine();
	if ( !commandline().parse(_argc, _argv) ) {
		exit(-1);
		return false;
	}

	// Enable tracking of configuration variables output or validation of those
	// is requested on commandline
	_configuration.trackVariables(commandline().hasOption("print-config-vars") ||
	                              commandline().hasOption("validate-schema-params"));

	if ( !initConfiguration() ) {
		exit(-1);
		return false;
	}

	_commandline.reset();
	_commandline = boost::shared_ptr<CommandLine>(new CommandLine);
	initCommandLine();
	if ( !parseCommandLine() ) {
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("help") ) {
		printUsage();
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("version") ) {
		printVersion();
		exit(-1);
		return false;
	}

	if ( !validateParameters() ) {
		cerr << "Try --help for help" << endl;
		exit(-1);
		return false;
	}

	_inputMonitor = new ObjectMonitor(_objectLogTimeWindow);
	_outputMonitor = new ObjectMonitor(_objectLogTimeWindow);

	_queue.resize(10);

	showMessage("Initialize logging");
	if ( !initLogging() ) {
		if ( !handleInitializationError(LOGGING) )
			return false;
	}

	showMessage("Loading plugins");
	if ( !initPlugins() ) {
		if ( !handleInitializationError(PLUGINS) )
			return false;
	}

	if ( commandline().hasOption("db-driver-list") ) {
		DatabaseInterfaceFactory::ServiceNames* services = DatabaseInterfaceFactory::Services();
		if ( services ) {
			cout << "Supported database drivers: ";
			for ( DatabaseInterfaceFactory::ServiceNames::iterator it = services->begin();
			      it != services->end(); ++it ) {
				if ( it != services->begin() )
					cout << ", ";
				cout << *it;
			}
			cout << endl;
			delete services;
			exit(-1);
			return false;
		}
	}

	if ( commandline().hasOption("record-driver-list") ) {
		RecordStreamFactory::ServiceNames* services = RecordStreamFactory::Services();
		if ( services ) {
			cout << "Supported recordstream drivers: ";
			for ( RecordStreamFactory::ServiceNames::iterator it = services->begin();
			      it != services->end(); ++it ) {
				if ( it != services->begin() )
					cout << ", ";
				cout << *it;
			}
			cout << endl;
			delete services;
			exit(-1);
			return false;
		}
	}

	if ( commandline().hasOption("print-config-vars") ) {
		printConfigVariables();
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("validate-schema-params") ) {
		validateSchemaParameters();
		exit(-1);
		return false;
	}

	if ( commandline().hasOption("daemon") ) {
		if ( !forkProcess() ) {
			cerr << "FATAL: Process forking failed" << endl;
			this->exit(-1);
			return false;
		}
	}

	if ( _lockfile.length() > 0 ) {
		int r = acquireLockfile(_lockfile);
		if ( r < 0 ) {
			// Error should have been reported by acquireLockfile()
			exit(-1);
			return false;
		}
		else if ( r == 0 ) {
			SEISCOMP_ERROR("Already running");
			exit(-1);
			return false;
		}
	}

	if ( _enableMessaging && !_connection ) {
		SEISCOMP_INFO("Connect to messaging");
		showMessage("Initialize messaging");
		if ( !initMessaging() ) {
			if ( !handleInitializationError(MESSAGING) )
			//if ( !_connection )
				return false;
		}
	}

	if ( _enableDatabase && !_database ) {
		SEISCOMP_INFO("Connect to database");
		showMessage("Initialize database");
		if ( !initDatabase() ) {
			if ( !handleInitializationError(DATABASE) )
				return false;
		}

		if ( _query && _connection && _connection->schemaVersion() > _query->version() ) {
			stringstream ss;
			ss << "The schema v" << _query->version().toString() << " of the "
			      "database is older than the one the server is using (v" <<
			      _connection->schemaVersion().toString() << ") , not all "
			      "information will be stored in the database." << endl <<
			      "This should be fixed!";
			showWarning(ss.str().c_str());
			SEISCOMP_WARNING("%s", ss.str().c_str());
		}
	}

	if ( !reloadInventory() )
		return false;

	if ( _exitRequested )
		return false;

	if ( !reloadBindings() )
		return false;

	if ( _exitRequested )
		return false;

	if ( isLoadRegionsEnabled() ) {
		showMessage("Reading custom regions");
		Regions regions;
		regions.load();
	}

	if ( isLoadCitiesEnabled() ) {
		showMessage("Reading city data");

		IO::XMLArchive ar;
		bool foundCity;

		if ( _cityDB.empty() ) {
			foundCity = ar.open((Environment::Instance()->configDir() + "/cities.xml").c_str());
			if ( !foundCity )
				foundCity = ar.open((Environment::Instance()->shareDir() + "/cities.xml").c_str());
		}
		else
			foundCity = ar.open(_cityDB.c_str());

		if ( foundCity ) {
			ar >> NAMED_OBJECT("City", _cities);

			SEISCOMP_INFO("Found cities.xml and read %lu entries", (unsigned long)_cities.size());

			// Sort the cities descending
			std::sort(_cities.begin(), _cities.end(), CityGreaterThan());

			ar.close();
		}
	}

	showMessage("");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleInitializationError(Stage) { return false; }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::timeout() {
	sendNotification(Notification::Timeout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendNotification(const Notification &n) {
	_queue.push(n);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::requestSync(const char *syncID) {
	if ( !_connection ) return false;
	Communication::SyncRequestMessage req(syncID);
	return _connection->send(Communication::Protocol::STATUS_GROUP, &req);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::sync(const char *syncID) {
	if ( syncID == NULL || *syncID == '\0' ) {
		_currentSyncID = "sync_";
		_currentSyncID += name() + "_" + Core::Time::GMT().iso();
	}
	else
		_currentSyncID = syncID;

	// Safety first
	if ( _currentSyncID.empty() ) return false;

	handleStartSync();

	if ( !requestSync(_currentSyncID.c_str()) ) {
		handleEndSync();
		SEISCOMP_DEBUG("End sync");
		return false;
	}

	// Start listening to messages if not done yet
	if ( !_messageThread )
		startMessageThread();

	// Loop until the internal syncID has been resettet
	// within processEvent
	while ( !_currentSyncID.empty() ) {
		if ( !processEvent() ) break;
		idle();
	}

	handleEndSync();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleStartSync() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleEndSync() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleEndAcquisition() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::reloadInventory() {
	if ( _enableLoadInventory ) {
		if ( !_inventoryDB.empty() ) {
			if ( !loadInventory(_inventoryDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading complete inventory");
				showMessage("Loading inventory");
				Inventory::Instance()->load(_query.get());
				SEISCOMP_INFO("Finished loading complete inventory");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}

		int filtered = Inventory::Instance()->filter(&_networkTypeFirewall,
		                                             &_stationTypeFirewall);
		if ( filtered > 0 )
			SEISCOMP_INFO("Filtered %d stations by type", filtered);
	}
	else if ( _enableLoadStations ) {
		if ( !_inventoryDB.empty() ) {
			if ( !loadInventory(_inventoryDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading inventory (stations only)");
				showMessage("Loading stations");
				Inventory::Instance()->loadStations(_query.get());
				SEISCOMP_INFO("Finished loading inventory (stations only)");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}

		int filtered = Inventory::Instance()->filter(&_networkTypeFirewall,
		                                             &_stationTypeFirewall);
		if ( filtered > 0 )
			SEISCOMP_INFO("Filtered %d stations by type", filtered);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::reloadBindings() {
	_configModule = NULL;

	if ( _enableLoadConfigModule ) {
		std::set<std::string> params;

		if ( !_configDB.empty() ) {
			if ( !loadConfig(_configDB) ) return false;
		}
		else if ( _database ) {
			if ( _query ) {
				SEISCOMP_INFO("Loading configuration module");
				showMessage("Reading station config");
				if ( !_configModuleName.empty() )
					ConfigDB::Instance()->load(query(), _configModuleName, Core::None, Core::None, Core::None, params);
				else
					ConfigDB::Instance()->load(query(), Core::None, Core::None, Core::None, Core::None, params);
				SEISCOMP_INFO("Finished loading configuration module");
			}
			else {
				SEISCOMP_ERROR("No database query object");
				return false;
			}
		}

		DataModel::Config* config = ConfigDB::Instance()->config();
		for ( size_t i = 0; i < config->configModuleCount(); ++i ) {
			if ( config->configModule(i)->name() == _configModuleName ) {
				_configModule = config->configModule(i);
				break;
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	if ( _connection )
		startMessageThread();

	while ( !_exitRequested ) {
		if ( !processEvent() ) break;
		idle();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::processEvent() {
	try {
		Notification evt = _queue.pop();
		BaseObjectPtr obj = evt.object;
		switch ( evt.type ) {
			case Notification::Object:
				if ( !obj ) {
					SEISCOMP_ERROR("Got NULL object");
					return true;
				}

				//SEISCOMP_DEBUG("Received object: %s, refCount: %d", obj->className(), obj->referenceCount());
				if ( !dispatch(obj.get()) ) {
					SEISCOMP_WARNING("Could not dispatch objects");
				}
				break;

			case Notification::Reconnect:
				handleReconnect();
				break;

			case Notification::Disconnect:
				// Reset sync request because server is not going to respond
				// to our initial sync message anymore
				_currentSyncID.clear();
				handleDisconnect();
				break;

			case Notification::Timeout:
				handleTimeout();
				break;

			case Notification::Close:
				if ( handleClose() ) {
					SEISCOMP_INFO("Close event received, returning");
					return false;
				}
				else
					SEISCOMP_INFO("Close event received but ignored");
				break;

			case Notification::Sync:
				sync();
				break;

			case Notification::AcquisitionFinished:
				handleEndAcquisition();
				break;

			default:
				if ( !dispatchNotification(evt.type, obj.get()) )
					SEISCOMP_WARNING("Wrong eventtype in queue: %d", evt.type);
				break;
		}
	}
	catch ( QueueClosedException& ex ) {
		SEISCOMP_INFO("%s, returning", ex.what());
		return false;
	}
	catch ( GeneralException& ex ) {
		SEISCOMP_INFO("Exception: %s, returning", ex.what());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::dispatch(Core::BaseObject* obj) {
	//SEISCOMP_DEBUG("dispatch %s (refCount = %d)", obj->className(), obj->referenceCount());
	Message *msg = Message::Cast(obj);
	if ( msg ) {
		if ( _enableAutoShutdown ) {
			// Filter application status messages
			ApplicationStatusMessage *as = ApplicationStatusMessage::Cast(msg);
			if ( as ) {
				SEISCOMP_DEBUG("Received application status: module=%s, username=%s: %s",
				               as->module().c_str(), as->username().c_str(),
				               as->status().toString());
				if ( as->status() == FINISHED ) {
					if ( !_shutdownMasterModule.empty() && as->module() == _shutdownMasterModule ) {
						SEISCOMP_INFO("Initiate self shutdown because of module %s shutdown",
						              as->module().c_str());
						handleAutoShutdown();
					}
					else if ( !_shutdownMasterUsername.empty() && as->username() == _shutdownMasterUsername ) {
						SEISCOMP_INFO("Initiate self shutdown because of user %s shutdown",
						              as->username().c_str());
						handleAutoShutdown();
					}
				}
			}
		}

		Communication::SyncResponseMessage *sync_resp =
			Communication::SyncResponseMessage::Cast(msg);
		if ( sync_resp ) {
			if ( !_currentSyncID.empty() && (_currentSyncID == sync_resp->ID()) )
				_currentSyncID.clear();
			else
				handleSync(sync_resp->ID());
		}
		else
			handleMessage(msg);
		return true;
	}
	else {
		NetworkMessage *nmsg = NetworkMessage::Cast(obj);
		if ( nmsg ) {
			handleNetworkMessage(nmsg);
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::dispatchNotification(int, Core::BaseObject*) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::idle() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	_exitRequested = true;

	if ( _connection && _connection->isConnected() ) {
		if ( _enableStartStopMessages ) {
			ApplicationStatusMessage stat(name(), _messagingUser, FINISHED);
			_connection->send(Communication::Protocol::STATUS_GROUP, &stat);
		}
		_connection->disconnect();
	}

	_queue.close();

	if ( _userTimer.isActive() ) {
		SEISCOMP_INFO("Disable timer");
		disableTimer();
	}

	if ( _messageThread ) {
		SEISCOMP_INFO("Waiting for message thread");
		_messageThread->join();
		delete _messageThread;
		_messageThread = NULL;
		SEISCOMP_INFO("Message thread finished");
	}

	_connection = NULL;
	_query = NULL;
	_database = NULL;

	SEISCOMP_DEBUG("Leaving ::done");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::initCommandLine() {
	commandline().addGroup("Generic");
	//commandline().addOption("Generic", "name,N", "set application name used for configuration file lookup", &_name);
	commandline().addOption("Generic", "help,h", "produce help message");
	commandline().addOption("Generic", "version,V", "show version information");
	commandline().addOption("Generic", "config-file", "Use alternative configuration file", &_alternativeConfigFile);
	commandline().addOption("Generic", "print-config-vars", "Print all available configuration variables and exit");
	commandline().addOption("Generic", "validate-schema-params", "Validates the applications description xml and exit");
	commandline().addOption("Generic", "plugins", "Load given plugins", &_plugins);
	//commandline().addOption("Generic", "crash-handler", "path to crash handler script", &_crashHandler);

	if ( _enableDaemon )
		commandline().addOption("Generic", "daemon,D", "run as daemon");

	if ( _enableMessaging ) {
		commandline().addOption("Generic", "auto-shutdown", "enables automatic application shutdown triggered by a status message", &_enableAutoShutdown);
		commandline().addOption("Generic", "shutdown-master-module", "triggers shutdown if the module name of the received messages match", &_shutdownMasterModule, false);
		commandline().addOption("Generic", "shutdown-master-username", "triggers shutdown if the user name of the received messages match", &_shutdownMasterUsername, false);
	}

	commandline().addGroup("Verbose");
	commandline().addOption("Verbose", "verbosity", "verbosity level [0..4]", &_verbosity, false);
	commandline().addCustomOption("Verbose", "v,v", "increase verbosity level (may be repeated, eg. -vv)", new FlagCounter(&_verbosity));
	commandline().addOption("Verbose", "quiet,q", "quiet mode: no logging output");
	commandline().addOption("Verbose", "print-component", "print the log component (default: file:1, stdout:0)", &_logComponent, false);
	commandline().addOption("Verbose", "print-context", "print source file and line number", &_logContext);
	commandline().addOption("Verbose", "component", "limits the logging to a certain component. this option can be given more than once", &_logComponents);
#ifndef WIN32
	commandline().addOption("Verbose", "syslog,s", "use syslog");
#endif
	commandline().addOption("Verbose", "lockfile,l", "path to lock file", &_lockfile);
	commandline().addOption("Verbose", "console", "send log output to stdout", &_logToStdout);
	commandline().addOption("Verbose", "debug", "debug mode: --verbosity=4 --console=1");
	commandline().addOption("Verbose", "trace", "trace mode: --verbosity=4 --console=1 --print-component=1 --print-context=1");
	commandline().addOption("Verbose", "log-file", "Use alternative log file", &_alternativeLogFile);
	commandline().addOption("Verbose", "log-utc", "Use UTC instead of local timezone", &_logUTC);


	if ( _enableMessaging ) {
		commandline().addGroup("Messaging");
		commandline().addOption("Messaging", "user,u", "client name used when connecting to the messaging", &_messagingUser);
		commandline().addOption("Messaging", "host,H", "messaging host (host[:port])", &_messagingHost);
		commandline().addOption("Messaging", "timeout,t", "connection timeout in seconds", &_messagingTimeout);
		commandline().addOption("Messaging", "primary-group,g", "the primary message group of the client", &_messagingPrimaryGroup);
		commandline().addOption("Messaging", "subscribe-group,S", "a group to subscribe to. this option can be given more than once", &_messagingSubscriptionRequests);
		commandline().addOption("Messaging", "encoding", "sets the message encoding (binary or xml)", &_messagingEncoding);
		commandline().addOption("Messaging", "start-stop-msg", "sets sending of a start- and a stop message", &_enableStartStopMessages);
	}


	if ( _enableDatabase ) {
		commandline().addGroup("Database");
		commandline().addOption("Database", "db-driver-list", "list all supported database drivers");
		commandline().addOption("Database", "database,d", "the database connection string, format: service://user:pwd@host/database", &_db, false);
		commandline().addOption("Database", "config-module", "the configmodule to use", &_configModuleName);
		commandline().addOption("Database", "inventory-db", "load the inventory from the given database or file, format: [service://]location", &_inventoryDB, false);
		commandline().addOption("Database", "config-db", "load the configuration from the given database or file, format: [service://]location", &_configDB, false);
	}


	if ( _enableRecordStream ) {
		commandline().addGroup("Records");
		commandline().addOption("Records", "record-driver-list", "list all supported record stream drivers");
		commandline().addOption("Records", "record-url,I", "the recordstream source URL, format: [service://]location[#type]", &_recordStream, false);
		commandline().addOption("Records", "record-file", "specify a file as recordsource", (std::string*)NULL);
		commandline().addOption("Records", "record-type", "specify a type for the records being read", (std::string*)NULL);
	}

	if ( _enableLoadCities ) {
		commandline().addGroup("Cities");
		commandline().addOption("Cities", "city-xml", "load the cities from the given XML file", &_cityDB, false);
	}

	createCommandLineDescription();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Application::version() {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createCommandLineDescription() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateParameters() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::parseCommandLine() {
	if ( !commandline().parse(_argc, _argv) )
		return false;

	_messagingUser = Util::replace(_messagingUser, AppResolver(_name));

	//if ( commandline().hasOption("subscription-list") )
	//	split(_messagingSubscriptionRequests, _messagingStrSubscriptions.c_str(), ",");

	const char* tmp = strstr(_db.c_str(), "://");
	if ( tmp ) {
		std::copy(_db.c_str(), tmp, std::back_inserter(_dbType));
		_dbParameters = tmp + 3;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {
//	if ( !_configuration.readConfig(Environment::Instance()->configFileName()) ) {
//		std::cerr << "An error occured while reading configuration file: "
//		          << Environment::Instance()->configFileName()
//		          << std::endl;
//		return false;
//	}

	if ( _alternativeConfigFile.empty() ) {
		if ( !Environment::Instance()->initConfig(&_configuration, name()) ) {
			SEISCOMP_ERROR("Configuration file errors found, abort");
			return false;
		}
	}
	else {
		_alternativeConfigFile = Environment::Instance()->absolutePath(_alternativeConfigFile);
		if ( !Util::fileExists(_alternativeConfigFile) ) {
			SEISCOMP_ERROR("Could not find alternative configuration file %s, abort", _alternativeConfigFile.c_str());
			return false;
		}
		if ( !_configuration.readConfig(_alternativeConfigFile) ) {
			SEISCOMP_ERROR("Error found in alternative configuration file %s, abort", _alternativeConfigFile.c_str());
			return false;
		}
	}

	try { _verbosity = configGetInt("logging.level"); } catch (...) {}
	try { _logToStdout = !configGetBool("logging.file"); } catch (...) {}
	try { _logContext = configGetBool("logging.context"); } catch (...) {}
	try { _logComponent = configGetBool("logging.component") ? 1 : 0; } catch (...) {}
	try { _logComponents = configGetStrings("logging.components"); } catch (...) {}
	try { _logUTC = configGetBool("logging.utc"); } catch (...) {}

	try { _objectLogTimeWindow = configGetInt("logging.objects.timeSpan"); } catch (...) {}

	try { _crashHandler = configGetString("scripts.crashHandler"); } catch ( ... ) {}
	try { _messagingHost = configGetString("connection.server"); } catch ( ... ) {}
	try { _messagingUser = Util::replace(configGetString("connection.username"), AppResolver(_name)); } catch ( ... ) {}
	try { _messagingTimeout = configGetInt("connection.timeout"); } catch ( ... ) {}
	try { _messagingPrimaryGroup = configGetString("connection.primaryGroup"); } catch ( ... ) {}
	try { _messagingEncoding = configGetString("connection.encoding"); } catch ( ... ) {}

	try { _enableStartStopMessages = configGetBool("client.startStopMessage"); } catch ( ... ) {}
	try { _enableAutoShutdown = configGetBool("client.autoShutdown"); } catch ( ... ) {}
	try { _shutdownMasterModule = configGetString("client.shutdownMasterModule"); } catch ( ... ) {}
	try { _shutdownMasterUsername = configGetString("client.shutdownMasterUsername"); } catch ( ... ) {}

	try {
		_db = configGetString("database");
	}
	catch ( ... ) {
		std::string dbType, dbParams;
		try { dbType = configGetString("database.type"); } catch ( ... ) {}
		try { dbParams = configGetString("database.parameters"); } catch ( ... ) {}
		if ( !dbType.empty() && !dbParams.empty() )
			_db = dbType + "://" + dbParams;
	}

	try { _configModuleName = configGetString("configModule"); } catch ( ... ) {}

	std::vector<string> groups;
	bool hasGroups = false;
	try {
		groups = _configuration.getStrings("connection.subscriptions");
		hasGroups = true;
	}
	catch (...) {}

	if ( hasGroups )
		_messagingSubscriptionRequests = groups;

	try {
		_recordStream = configGetString("recordstream");
	}
	catch (...) {
		try {
			_recordStream = configGetString("recordstream.service") + "://" +
			                configGetString("recordstream.source");
		}
		catch (...) {}
	}

	try { _inventoryDB = configGetPath("database.inventory"); }
	catch ( ... ) {}

	try { _configDB = configGetPath("database.config"); }
	catch ( ... ) {}

	try { _cityDB = configGetPath("cityXML"); }
	catch ( ... ) {}

	try { _agencyID = Util::replace(configGetString("agencyID"), AppResolver(_name)); }
	catch (...) { _agencyID = "UNSET"; }

	try { _author = Util::replace(configGetString("author"), AppResolver(_name)); }
	catch (...) { _author = Util::replace("@appname@@@@hostname@", AppResolver(_name)); }

	try {
		std::vector<std::string> whiteList = configGetStrings("processing.whitelist.agencies");
		std::copy(whiteList.begin(), whiteList.end(), std::inserter(_procFirewall.allow, _procFirewall.allow.end()));
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> blackList = configGetStrings("processing.blacklist.agencies");
		std::copy(blackList.begin(), blackList.end(), std::inserter(_procFirewall.deny, _procFirewall.deny.end()));
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> whiteList = configGetStrings("inventory.whitelist.nettype");
		std::copy(whiteList.begin(), whiteList.end(), std::inserter(_networkTypeFirewall.allow, _networkTypeFirewall.allow.end()));
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> blackList = configGetStrings("inventory.blacklist.nettype");
		std::copy(blackList.begin(), blackList.end(), std::inserter(_networkTypeFirewall.deny, _networkTypeFirewall.deny.end()));
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> whiteList = configGetStrings("inventory.whitelist.statype");
		std::copy(whiteList.begin(), whiteList.end(), std::inserter(_stationTypeFirewall.allow, _stationTypeFirewall.allow.end()));
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> blackList = configGetStrings("inventory.blacklist.statype");
		std::copy(blackList.begin(), blackList.end(), std::inserter(_stationTypeFirewall.deny, _stationTypeFirewall.deny.end()));
	}
	catch ( ... ) {}

	try { _enableLoadCities = configGetBool("loadCities"); } catch ( ... ) {}
	try { _enableLoadRegions = configGetBool("loadRegions"); } catch ( ... ) {}

	try {
		DataModel::PublicObject::SetIdPattern(configGetString("publicIDPattern"));
		_customPublicIDPattern = true;
	}
	catch ( ... ) {
		_customPublicIDPattern = false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initLogging() {
	bool logRotator = true;
	int logRotateTime = 60*60*24; /* one day*/
	int logRotateArchiveSize = 7; /* one week archive */
	int logRotateMaxFileSize = 100*1024*1024; /* max 100MB per logfile */

	Logging::disableConsoleLogging();

	if ( commandline().hasOption("quiet") )
		return true;

	try { logRotator = configGetBool("logging.file.rotator"); } catch (...) {}
	try { logRotateTime = configGetInt("logging.file.rotator.timeSpan"); } catch (...) {}
	try { logRotateArchiveSize = configGetInt("logging.file.rotator.archiveSize"); } catch (...) {}
	try { logRotateMaxFileSize = configGetInt("logging.file.rotator.maxFileSize"); } catch (...) {}

	bool enableLogging = _verbosity > 0;
	bool syslog = false;

	try { syslog = configGetBool("logging.syslog"); } catch ( ... ) {}
	if ( commandline().hasOption("syslog") ) syslog = true;

	bool trace = commandline().hasOption("trace");
	if ( trace || commandline().hasOption("debug") ) {
		enableLogging = true;
		_verbosity = 4;
		_logToStdout = true;
		if ( trace ) {
			_logContext = true;
			_logComponent = 1;
		}
	}

	if ( enableLogging ) {
		//std::cerr << "using loglevel " << _verbosity << std::endl;
#ifndef WIN32
		if ( syslog ) {
			Logging::SyslogOutput* syslogOutput = new Logging::SyslogOutput();
			const char *facility = NULL;
			string tmp_facility;

			try {
				tmp_facility = configGetString("logging.syslog.facility");
				facility = tmp_facility.c_str();
			}
			catch ( ... ) {}

			if ( syslogOutput->open(_name.c_str(), facility) ) {
				std::cerr << "using syslog: " << _name << ", "
				          << (facility?facility:"default") << "(code="
				          << syslogOutput->facility() << ")" << std::endl;
				_logger = syslogOutput;
			}
			else {
				std::cerr << "failed to open syslog: " << _name << std::endl;
				delete syslogOutput;
				syslogOutput = NULL;
				return false;
			}
		}
		else
#endif
		if ( !_logToStdout ) {
			std::string logFile = Environment::Instance()->absolutePath(_alternativeLogFile);
			if ( logFile.empty() )
				logFile = Environment::Instance()->logFile(_name.c_str());

			Logging::FileOutput* logger;
			if ( logRotator )
				logger = new Logging::FileRotatorOutput(logRotateTime, logRotateArchiveSize, logRotateMaxFileSize);
			else
				logger = new Logging::FileOutput();

			if ( logger->open(logFile.c_str()) ) {
				//std::cerr << "using logfile: " << logFile << std::endl;
				_logger = logger;
			}
			else {
				std::cerr << "failed to open logfile: " << logFile << std::endl;
				delete logger;
				logger = NULL;
			}
		}
		else
			_logger = new Logging::FdOutput(STDERR_FILENO);

		if ( _logger ) {
			_logger->setUTCEnabled(_logUTC);
			_logger->logComponent(_logComponent < 0 ? !_logToStdout : _logComponent);
			_logger->logContext(_logContext);
			if ( !_logComponents.empty() ) {
				for ( ComponentList::iterator it = _logComponents.begin();
				      it != _logComponents.end(); ++it ) {
					_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "notice"));
					switch ( _verbosity ) {
						default:
						case 4:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "debug"));
						case 3:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "info"));
						case 2:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "warning"));
						case 1:
							_logger->subscribe(Logging::getComponentChannel((*it).c_str(), "error"));
					}
				}
			}
			else {
				_logger->subscribe(Logging::getGlobalChannel("notice"));
				switch ( _verbosity ) {
					default:
					case 4:
						_logger->subscribe(Logging::getGlobalChannel("debug"));
					case 3:
						_logger->subscribe(Logging::getGlobalChannel("info"));
					case 2:
						_logger->subscribe(Logging::getGlobalChannel("warning"));
					case 1:
						_logger->subscribe(Logging::getGlobalChannel("error"));
				}
			}
		}
		else
			return false;
	}

	if ( !_logToStdout ) {
		const char *appVersion = version();
		SEISCOMP_NOTICE("Starting %s %s", name().c_str(), appVersion?appVersion:"");
		SEISCOMP_NOTICE("  Framework   : %s", frameworkVersion());
		SEISCOMP_NOTICE("  API Version : %d.%d.%d",
		                SC_API_VERSION_MAJOR(SC_API_VERSION),
		                SC_API_VERSION_MINOR(SC_API_VERSION),
		                SC_API_VERSION_PATCH(SC_API_VERSION));
		SEISCOMP_NOTICE("  Version     : %s", CurrentVersion.systemInfo().c_str());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initMessaging() {
	int status = 0;

	_messagingSubscriptions.clear();
	for ( size_t i = 0; i < _messagingSubscriptionRequests.size(); ++i )
		_messagingSubscriptions.insert(_messagingSubscriptionRequests[i]);

	while ( !_exitRequested && !_connection ) {
		SEISCOMP_DEBUG("Trying to connect to %s@%s with primary group = %s",
		               _messagingUser.c_str(), _messagingHost.c_str(), _messagingPrimaryGroup.c_str());
		_connection = Connection::Create(_messagingHost, _messagingUser,
		                                 _messagingPrimaryGroup, Protocol::PRIORITY_DEFAULT,
		                                 _messagingTimeout*1000, &status);
		if ( _connection ) break;

		if ( status == Status::SEISCOMP_WRONG_SERVER_VERSION )
			break;

		if ( _retryCount )
			--_retryCount;

		if ( !_retryCount )
			break;

		SEISCOMP_DEBUG("Connection error: %s -> trying again after 2 secs", Core::Status::StatusToStr(status));
		Core::sleep(2);
	}

	if ( !_connection ) {
		SEISCOMP_ERROR("Could not connect to message system");
		return false;
	}

	// Register monitor logging callback
	Communication::ConnectionInfo::Instance()->
		registerInfoCallback(boost::bind(&Application::monitorLog, this, _1, _2, _3));

	if ( !_logToStdout ) SEISCOMP_NOTICE("Connection to %s established", _messagingHost.c_str());

	Version localSchemaVersion = Version(DataModel::Version::Major, DataModel::Version::Minor);
	if ( _connection->schemaVersion() > localSchemaVersion ) {
		stringstream ss;
		ss << "Local schema v" << localSchemaVersion.toString() << " is "
		      "older than the one the server supports (v" <<
		      _connection->schemaVersion().toString() << ") , incoming messages "
		      "will not be readable but sending will work.";
		showWarning(ss.str().c_str());
		SEISCOMP_WARNING("%s", ss.str().c_str());
	}
	else if ( _connection->schemaVersion() < localSchemaVersion ) {
		stringstream ss;
		ss << "Local schema v" << localSchemaVersion.toString() << " is "
		      "more recent than the one the server supports (v" <<
		      _connection->schemaVersion().toString() << ") , not all "
		      "information can be handled by the server and will be ignored.";
		showWarning(ss.str().c_str());
		SEISCOMP_WARNING("%s", ss.str().c_str());
	}

	MessageEncoding enc;
	if ( enc.fromString(_messagingEncoding.c_str()) ) {
		SEISCOMP_INFO("Setting message encoding to %s", _messagingEncoding.c_str());
		_connection->setEncoding(enc);
	}

	if ( _enableStartStopMessages ) {
		SEISCOMP_DEBUG("Send START message to group %s",
		               Communication::Protocol::STATUS_GROUP.c_str());
		ApplicationStatusMessage stat(name(), _messagingUser, STARTED);
		_connection->send(Communication::Protocol::STATUS_GROUP, &stat);
	}

	return initSubscriptions();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initSubscriptions() {
	bool requestAllGroups = false;

	for ( set<string>::iterator it = _messagingSubscriptions.begin();
	      it != _messagingSubscriptions.end(); ++it ) {
		if ( (*it) == "*" || (*it) == "...") {
			requestAllGroups = true;
			break;
		}
	}

	if ( requestAllGroups ) {
		for ( int i = 0; i < _connection->groupCount(); ++i ) {
			if ( _connection->subscribe(_connection->group(i)) != Core::Status::SEISCOMP_SUCCESS ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
				return false;
			}
		}
	}
	else {
		for ( set<string>::iterator it = _messagingSubscriptions.begin();
		      it != _messagingSubscriptions.end(); ++it ) {
			if ( _connection->subscribe(it->c_str()) != Core::Status::SEISCOMP_SUCCESS ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", it->c_str());
				return false;
			}
		}
	}

	/*
	for ( int i = 0; i < _connection->groupCount(); ++i ) {
		if ( requestAllGroups ) {
			if ( !_connection->subscribe(_connection->group(i)) ) {
				SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
				return false;
			}
		}
		else {
			if ( _messagingSubscriptions.find(_connection->group(i)) != _messagingSubscriptions.end() ) {
				if ( !_connection->subscribe(_connection->group(i)) ) {
					SEISCOMP_ERROR("Could not subscribe to group '%s'", _connection->group(i));
					return false;
				}
				else {
					SEISCOMP_DEBUG("Subscribe to group '%s'", _connection->group(i));
				}
			}
		}
	}
	*/

	if ( _enableAutoShutdown )
		_connection->subscribe(Communication::Protocol::STATUS_GROUP);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::runMessageThread() {
	SEISCOMP_INFO("Starting message thread");
	while ( readMessages() ) {}
	SEISCOMP_INFO("Leaving message thread");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::startMessageThread() {
	_messageThread = new boost::thread(boost::bind(&Application::runMessageThread, this));
	//boost::thread(boost::bind(&Application::runMessageThread, this));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDatabase(IO::DatabaseInterface* db) {
	_database = db;
	if ( !_query )
		_query = new DataModel::DatabaseQuery(_database.get());
	else
		_query->setDriver(_database.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initDatabase() {
	setDatabase(NULL);

	if ( !_db.empty() ) {
		SEISCOMP_INFO("Read database service parameters from configfile");
		SEISCOMP_INFO("Trying to connect to %s", _db.c_str());

		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(_db.c_str());
		if (db) {
			SEISCOMP_INFO("Connected successfully");
			setDatabase(db.get());
			return !_query->hasError();
		}
		else {
			if ( _enableFetchDatabase )
				SEISCOMP_WARNING("Database connection to %s failed, trying to fetch the service message",
				                 _db.c_str());
			else {
				SEISCOMP_WARNING("Database connection to %s failed", _db.c_str());
				return false;
			}
		}
	}

	// Try to fetch here
	if ( !_connection ) {
		SEISCOMP_ERROR("Fetching database parameters failed, no messaging connection");
		return false;
	}

	Util::StopWatch fetchTimeout;

	// Poll for 5 seconds for a valid database provide message
	while ( fetchTimeout.elapsed() < TimeSpan(5.0) ) {
		while ( _connection->readNetworkMessage(false) == Core::Status::SEISCOMP_SUCCESS ) {
			MessagePtr msg = _connection->readMessage(false);

			Communication::DatabaseProvideMessage* dbrmsg = Communication::DatabaseProvideMessage::Cast(msg);
			if ( dbrmsg ) {
				std::string dbType = dbrmsg->service();
				std::string dbParameters = dbrmsg->parameters();
				_db = dbType + "://" + dbParameters;

				SEISCOMP_INFO("Received database service parameters");
				SEISCOMP_INFO("Trying to connect to %s database", dbrmsg->service());
				IO::DatabaseInterfacePtr db = dbrmsg->database();
				if (db) {
					setDatabase(db.get());
					SEISCOMP_INFO("Connected successfully");
					return !_query->hasError();
				}
				else
					SEISCOMP_WARNING("Database connection to %s://%s failed",
					                 dbrmsg->service(), dbrmsg->parameters());
				continue;
			}

			if ( fetchTimeout.elapsed() >= TimeSpan(5.0) ) break;
		}
	}

	SEISCOMP_ERROR("Timeout while waiting for database provide message");

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::set<std::string>& Application::subscribedGroups() const {
	return _messagingSubscriptions;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectMonitor::Log *
Application::addInputObjectLog(const std::string &name, const std::string &channel) {
	if ( !_inputMonitor ) return NULL;
	return _inputMonitor->add(name, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ObjectMonitor::Log *
Application::addOutputObjectLog(const std::string &name, const std::string &channel) {
	if ( !_outputMonitor ) return NULL;
	return _outputMonitor->add(name, channel);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::loadConfig(const std::string &configDB) {
	SEISCOMP_INFO("Loading configuration module %s", configDB.c_str());
	showMessage("Reading station config from");

	if ( configDB.find("://") == string::npos ) {
		try { ConfigDB::Instance()->load(configDB.c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else if ( configDB.find("file://") == 0 ) {
		try { ConfigDB::Instance()->load(configDB.substr(7).c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else {
		SEISCOMP_INFO("Trying to connect to %s", configDB.c_str());
		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(configDB.c_str());
		if ( db ) {
			SEISCOMP_INFO("Connected successfully");
			DataModel::DatabaseQueryPtr query = new DataModel::DatabaseQuery(db.get());
			ConfigDB::Instance()->load(query.get());
		}
		else {
			SEISCOMP_WARNING("Database connection to %s failed", configDB.c_str());
			return false;
		}
	}

	SEISCOMP_INFO("Finished loading configuration module");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::loadInventory(const std::string &inventoryDB) {
	SEISCOMP_INFO("Loading complete inventory from %s", inventoryDB.c_str());
	showMessage("Loading inventory");
	if ( inventoryDB.find("://") == string::npos ) {
		try { Inventory::Instance()->load(inventoryDB.c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else if ( inventoryDB.find("file://") == 0 ) {
		try { Inventory::Instance()->load(inventoryDB.substr(7).c_str()); }
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			return false;
		}
	}
	else {
		SEISCOMP_INFO("Trying to connect to %s", inventoryDB.c_str());
		IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(inventoryDB.c_str());
		if ( db ) {
			SEISCOMP_INFO("Connected successfully");
			DataModel::DatabaseQueryPtr query = new DataModel::DatabaseQuery(db.get());
			Inventory::Instance()->load(query.get());
		}
		else {
			SEISCOMP_WARNING("Database connection to %s failed", inventoryDB.c_str());
			return false;
		}
	}

	SEISCOMP_INFO("Finished loading complete inventory");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::logObject(ObjectMonitor::Log *log, const Core::Time &timestamp,
                            size_t count) {
	boost::mutex::scoped_lock l(_objectLogMutex);
	log->push(timestamp, count);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::forkProcess() {
#ifndef WIN32
	pid_t pid;

	// Become a session leader to lose controlling TTY.
	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return false;
	}
	else if ( pid != 0 ) // parent
		::exit(0);

	if ( setsid() < 0 ) {
		SEISCOMP_ERROR("setsid: %s", strerror(errno));
		return false;
	}

	// Ensure future opens won't allocate controlling TTYs.
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if ( sigaction(SIGHUP, &sa, NULL) < 0 ) {
		SEISCOMP_ERROR("can't ignore SIGHUP: %s", strerror(errno));
		return false;
	}

	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return false;
	}
	else if ( pid != 0 ) // parent
		::exit(0);

	// Attach file descriptors 0, 1, and 2 to /dev/null.
	close(0);
	close(1);
	close(2);
	int fd0 = open("/dev/null", O_RDWR);
	int fd1 = dup(0);
	int fd2 = dup(0);

	if ( fd0 != 0 || fd1 != 1 || fd2 != 2 ) {
		SEISCOMP_ERROR("forkProcess: unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		return false;
	}

	return true;
#else
	return false;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int Application::acquireLockfile(const std::string &lockfile) {
#ifndef WIN32
	int fd = open(lockfile.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if( fd < 0 ) {
		SEISCOMP_ERROR("could not open %s: %s", lockfile.c_str(), strerror(errno));
		return -1;
	}
	else if ( fd <= 2 ) {
		SEISCOMP_ERROR("acquireLockfile: unexpected file descriptor %d", fd);
		return -1;
	}

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if ( fcntl(fd, F_SETLK, &lock ) < 0 ) {
		close(fd);
		if(errno == EACCES || errno == EAGAIN) return 0; // already locked

		SEISCOMP_ERROR("could not lock %s: %s\n", lockfile.c_str(), strerror(errno));
		return -1;
	}

	if ( ftruncate(fd, 0) < 0 ) {
		SEISCOMP_ERROR("ftruncate: %s", strerror(errno));
		return -1;
	}

	char buf[30];
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", getpid());
	ssize_t pid_len = strlen(buf);

	if ( write(fd, buf, pid_len) != pid_len ) {
		SEISCOMP_ERROR("could not write pid file at %s: %s\n", lockfile.c_str(), strerror(errno));
		return -1;
	}

	int val;
	if ( (val = fcntl(fd, F_GETFD,0)) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	val |= FD_CLOEXEC;
	if ( fcntl(fd, F_SETFD, val) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	// locking successful
	return fd;
#else
	return -1;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showMessage(const char*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarning(const char*) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::readMessages() {
	if ( !_connection ) return true;

	int error;

	// We store a plain C-pointer here because SmartPointers are not
	// thread-safe. So the message has to be deleted manually when enqueueing
	// fails. Otherwise the referenceCount will be decremented when leaving
	// the method which can result in race conditions and much more important
	// in a segfault.
	NetworkMessage *nmsg = NULL;
	Message *msg = _connection->readMessage(true, Connection::READ_ALL, &nmsg, &error);
	if ( error == Core::Status::SEISCOMP_SUCCESS ) {
		if ( msg ) {
			if ( nmsg ) {
				if ( !_queue.push(nmsg) ) {
					delete nmsg;
					delete msg;
					return false;
				}
			}

			if ( _queue.push(msg) )
				return true;

			delete msg;
			return false;
		}
		else if ( nmsg ) {
			if ( !_queue.push(nmsg) ) {
				delete nmsg;
				return false;
			}

			return true;
		}
		else {
			// We read nothing ???
		}
	}
	else if ( !_exitRequested ) {
		if ( msg ) delete msg;

		// We should never step into this case
		if ( _connection->isConnected() )
			return true;

		SEISCOMP_WARNING("Connection lost, trying to reconnect");
		if ( !_queue.push(Notification::Disconnect) )
			return false;

		bool first = true;
		while ( !_exitRequested ) {
			_connection->reconnect();
			if ( _connection->isConnected() ) {
				SEISCOMP_INFO("Reconnected successfully");
				if ( _database ) {
					while ( !_database->isConnected() ) {
						SEISCOMP_WARNING("Connection lost to database %s, trying to reconnect", _db.c_str());
						if ( _database->connect(_db.c_str()) )
							SEISCOMP_INFO("Reconnected successfully to %s", _db.c_str());
						else
							Core::sleep(2);
					}
				}
				_queue.push(Notification::Reconnect);
				break;
			}
			else {
				if ( first ) {
					first = false;
					SEISCOMP_INFO("Reconnecting failed, trying again every 2 seconds");
				}
				Core::sleep(2);
			}
		}

		if ( _exitRequested )
			return false;
	}
	else {
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::monitorLog(const Communication::SystemConnection *con,
                             const Core::Time &now, std::ostream &os) {
	if ( _connection.get() != con ) return;

	handleMonitorLog(now);

	// Append app name
	os << "&app=" << name() << "&";

	boost::mutex::scoped_lock l(_objectLogMutex);

	ObjectMonitor::const_iterator it;

	_inputMonitor->update(now);
	_outputMonitor->update(now);

	for ( it = _inputMonitor->begin(); it != _inputMonitor->end(); ++it ) {
		os << "in(";
		if ( !it->name.empty() )
			os << "name:" << it->name << ",";
		if ( !it->channel.empty() )
			os << "chan:" << it->channel << ",";

		os << "cnt:" << it->count << ",";
		os << "avg:" << ((float)it->count / (float)it->test->timeSpan()) << ",";
		os << "tw:" << it->test->timeSpan();

		if ( it->test->last() )
			os << ",last:" << it->test->last().iso();
		os << /*"utime:" << now.iso() <<*/ ")&";
	}

	for ( it = _outputMonitor->begin(); it != _outputMonitor->end(); ++it ) {
		os << "out(";
		if ( !it->name.empty() )
			os << "name:" << it->name << ",";
		if ( !it->channel.empty() )
			os << "chan:" << it->channel << ",";

		os << "cnt:" << it->count << ",";
		os << "avg:" << ((float)it->count / (float)it->test->timeSpan()) << ",";
		os << "tw:" << it->test->timeSpan();

		if ( it->test->last() )
			os << ",last:" << it->test->last().iso();
		os << /*"utime:" << now.iso() <<*/ ")&";
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleTimeout() {
	std::cerr << "Unhandled Application::Timeout" << std::endl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleClose() {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleAutoShutdown() {
	SEISCOMP_DEBUG("Handling auto shutdown: quit");
	quit();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleMonitorLog(const Core::Time &timestamp) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleSync(const char *ID) {
	SEISCOMP_DEBUG("Sync response received: %s", ID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleReconnect() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleDisconnect() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleMessage(Core::Message* msg) {
	DataModel::NotifierMessage* nm;

	if ( _enableAutoApplyNotifier || _enableInterpretNotifier )
		nm = DataModel::NotifierMessage::Cast(msg);

	if ( _enableAutoApplyNotifier ) {
		if ( !nm ) {
			for ( MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) n->apply();
			}
		}
		else {
			for ( DataModel::NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				(*it)->apply();
		}
	}

	if ( _enableInterpretNotifier ) {
		if ( !nm ) {
			for ( MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) handleNotifier(n);
			}
		}
		else {
			for ( DataModel::NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				handleNotifier(it->get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleNetworkMessage(const Communication::NetworkMessage* msg) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleNotifier(DataModel::Notifier* n) {
	switch ( n->operation() ) {
		case DataModel::OP_ADD:
			addObject(n->parentID(), n->object());
			break;
		case DataModel::OP_REMOVE:
			removeObject(n->parentID(), n->object());
			break;
		case DataModel::OP_UPDATE:
			updateObject(n->parentID(), n->object());
			break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	_returnCode = returnCode;
	_exitRequested = true;

	_queue.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::quit() {
	this->exit(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleInterrupt(int s) throw() {
	this->exit(_returnCode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char *Application::frameworkVersion() const {
	return _version.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::agencyID() const {
	return _agencyID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::author() const {
	return _author;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAgencyIDAllowed(const std::string &agencyID) const {
	return _procFirewall.isAllowed(agencyID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::isAgencyIDBlocked(const std::string &agencyID) const {
	return !isAgencyIDAllowed(agencyID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
