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


#ifndef __SEISCOMP_CLIENT_APPLICATION_H__
#define __SEISCOMP_CLIENT_APPLICATION_H__

#include <boost/shared_ptr.hpp>

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/core/interruptible.h>
#include <seiscomp3/client/commandline.h>
#include <seiscomp3/client/queue.h>
#include <seiscomp3/client/monitor.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/client.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/communication/connection.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/configmodule.h>
#include <seiscomp3/math/coord.h>
#include <seiscomp3/utils/timer.h>

#include <set>

#define SCCoreApp (Seiscomp::Client::Application::Instance())


namespace Seiscomp {

namespace Logging {
	class Output;
}

namespace Client {


MAKEENUM(
	ApplicationStatus,
	EVALUES(
		STARTED,
		FINISHED
	),
	ENAMES(
		"started",
		"finished"
	)
);


class SC_SYSTEM_CLIENT_API ApplicationStatusMessage : public Core::Message {
	DECLARE_SC_CLASS(ApplicationStatusMessage);
	DECLARE_SERIALIZATION;

	public:
		ApplicationStatusMessage();
		ApplicationStatusMessage(const std::string &module,
		                         ApplicationStatus status);

		ApplicationStatusMessage(const std::string &module,
		                         const std::string &username,
		                         ApplicationStatus status);


	public:
		virtual bool empty() const;

		const std::string &module() const;
		const std::string &username() const;
		ApplicationStatus status() const;


	private:
		std::string _module;
		std::string _username;
		ApplicationStatus _status;
};


struct SC_SYSTEM_CLIENT_API Notification {
	//! Declares the application internal notification types.
	//! Custom types can be used with negative values.
	enum Type {
		Object,
		Disconnect,
		Reconnect,
		Close,
		Timeout,
		Sync,
		AcquisitionFinished
	};

	Notification() : object(NULL), type(Object) {}
	Notification(Core::BaseObject * o) : object(o), type(Object) {}
	Notification(int t) : object(NULL), type(t) {}
	Notification(int t, Core::BaseObject * o) : object(o), type(t) {}

	Core::BaseObject *object;
	int type;
};


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Application class to write commandline clients easily using
    \brief messaging, configuration, database access, commandline parameters
    \brief and so on.

    The class Application works as follows:

    \code
    exec()
    	init()
    	run()
    		handleMessage(msg)
    		idle()
    	done()
    \endcode

    All of the above methods are virtual. A derived class can reimplement
    each method to fit its needs.
    The Application class does all the administrative work:
    	- Reading from configuration files
    	- Connecting to a message server
    	- Connecting to a database
    	- Handling commandline parameters
    	- Process forking when creating daemons
    	- Signal handling

    So usually it is enough to reimplement the handleMessage() method to
    read messages and react on them.
  */
class SC_SYSTEM_CLIENT_API Application : public Seiscomp::Core::InterruptibleObject {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef std::vector<std::string> Arguments;
		typedef ObjectMonitor::Log       ObjectLog;

		//! Initialization stages used when reporting errors
		enum Stage {
			COMMANDLINE,
			CONFIGURATION,
			LOGGING,
			MESSAGING,
			DATABASE,
			ST_QUANTITY
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Application(int argc, char** argv);
		~Application();


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		int operator()();


	// ----------------------------------------------------------------------
	//  Public functions
	// ----------------------------------------------------------------------
	public:
		//! Returns a list of commandline parameters
		const Arguments& arguments() const;

		//! Returns the commandline interface to add groups and options
		CommandLine& commandline();
		const CommandLine& commandline() const;

		//! Returns the local configuration object
		const Config::Config &configuration() const;

		//! Returns the path of the application (arg[0])
		const char* path() const;

		/**
		 * Returns the name of the application used for section lookup
		 * in the configuration repository.
		 * @return The name of the application
		 */
		const std::string& name() const;

		/**
		 * Adds a pacakge search path to the pluginregistry. This call
		 * is equal to
		 * \code
		 * Seiscomp::Client::PluginRegistry::Instance()->addPackagePath(package);
		 * \endcode
		 * @param path
		 */
		void addPluginPackagePath(const std::string &package);

		//! Returns the version string
		const char *frameworkVersion() const;

		//! Returns the configured agencyID
		const std::string& agencyID() const;

		//! Returns the configured author
		const std::string& author() const;

		/**
		 * Returns according to the configured white- and blacklist of
		 * agencyID's whether the passed agencyID is allowed or not
		 * @param agencyID The agencyID to check
		 * @return The boolean result
		 */
		bool isAgencyIDAllowed(const std::string &agencyID) const;

		/**
		 * Returns !isAgencyIDAllowed(agencyID)
		 * @param agencyID The agencyID to check
		 * @return !isAgencyIDAllowed(agencyID)
		 */
		bool isAgencyIDBlocked(const std::string &agencyID) const;

		/**
		 * Enters the mainloop and waits until exit() is called
		 * or a appropriate signal has been fired (e.g. SIGTERM).
		 * @return The value that was set with to exit()
		 */
		int exec();

		/**
		 * Exit the application and set the returnCode.
		 * @param returnCode The value returned from exec()
		 */
		virtual void exit(int returnCode);

		/**
		 * Exit the application and set the returnCode to 0.
		 * This call is equivalent to exit(0).
		 */
		void quit();


		/**
		 * Returns whether exit has been requested or not.
		 * This query is important within own run loops to
		 * check for an abort criteria.
		 */
		bool isExitRequested() const;

		//! Prints the program usage. The default implementation
		//! prints the commandline options only
		virtual void printUsage() const;

		//! Returns the application's messaging connection interface
		Communication::Connection* connection() const;

		//! Returns the configured database type
		const std::string &databaseType() const;

		//! Returns the configured database connection parameters
		const std::string &databaseParameters() const;

		//! Returns the application's database interface
		IO::DatabaseInterface* database() const;

		//! Returns the application's database URI
		const std::string &databaseURI() const;

		//! Returns the application's database query interface
		DataModel::DatabaseQuery* query() const;

		//! Returns the configures recordstream URL to be used by
		//! RecordStream::Open()
		const std::string& recordStreamURL() const;

		//! Returns the path to the crashhandler
		const std::string&  crashHandler() const;

		//! Returns the list of configured points of interest
		const std::vector<Math::Geo::CityD>& cities() const;

		//! Returns the nearest city with respect to lat/lon and
		//! a given maximum distance and minimum population
		const Math::Geo::CityD *nearestCity(double lat, double lon,
		                                    double maxDist, double minPopulation,
		                                    double *dist, double *azi) const;

		//! Returns the config module object if available
		DataModel::ConfigModule *configModule() const;

		//! Returns the state of a station
		bool isStationEnabled(const std::string& networkCode,
		                      const std::string& stationCode);

		//! Returns the messaging-server
		const std::string& messagingHost() const;

		//! Enables a timer that calls every n seconds the
		//! handleTimeout() methods
		//! A value of 0 seconds disables the timer
		void enableTimer(unsigned int seconds);

		//! Disables the timer
		void disableTimer();

		//! Sends a notification to the application. If used in derived
		//! classes to send custom notifications use negative notification
		//! types and reimplement dispatchNotification(...).
		void sendNotification(const Notification &);

		/**
		 * If a connection is available a sync request
		 * message is sent. The application calls
		 * handleSync(ID) whenever a sync response is
		 * received. This method returns true if the
		 * sync request could be sent false otherwise.
		 */
		bool requestSync(const char *syncID);

		/**
		 * Requests a sync and waits until the sync request
		 * has been replied. If an error occured it returns
		 * false, true otherwise.
		 * If syncID is empty or NULL an ID is
		 * generated.
		 */
		bool sync(const char *syncID = NULL);


	// ----------------------------------------------------------------------
	//  Initialization configuration functions
	//  This functions have to be called before the init() method
	// ----------------------------------------------------------------------
	public:
		//! Enables the daemon mode to be selectable via commandline
		void setDaemonEnabled(bool enable);

		//! Sets the primary messaging group
		void setPrimaryMessagingGroup(const std::string&);

		//! Returns the set primary messaging group
		const std::string &primaryMessagingGroup() const;

		//! Sets the username used for the messaging connection
		void setMessagingUsername(const std::string&);

		/**
		 * Adds a group to subscribe to. This is only a default group.
		 * If another group or groups are given via commandline or config
		 * file this subscription will be overriden completely.
		 */
		void addMessagingSubscription(const std::string&);

		//! Initialize the database, default = true, true
		void setDatabaseEnabled(bool enable, bool tryToFetch);
		bool isDatabaseEnabled() const;

		//! Returns whether the inventory should be loaded from a
		//! file (false) or from the database (true)
		bool isInventoryDatabaseEnabled() const;

		//! Returns whether the config module should be loaded from a
		//! file (false) or from the database (true)
		bool isConfigDatabaseEnabled() const;

		//! Initialize the messaging, default = true
		void setMessagingEnabled(bool enable);
		bool isMessagingEnabled() const;

		//! Enables/disables sending of start/stop messages.
		//! If enabled, a start message (at startup) and a
		//! stop message (at shutdown) will be sent to the
		//! STATUS group. Default = false
		void setStartStopMessagesEnabled(bool enable);
		bool areStartStopMessagesEnabled() const;

		//! Enables/disables auto shutdown caused by
		//! the shutdown of a definable master module or
		//! master username. If both values are set the
		//! one coming first is used.
		void setAutoShutdownEnabled(bool enable);
		bool isAutoShutdownEnabled() const;

		//! Enables recordstream URL option, default = true
		void setRecordStreamEnabled(bool enable);
		bool isRecordStreamEnabled() const;

		//! Load the stations from the inventory at startup, default = false
		void setLoadStationsEnabled(bool enable);
		bool isLoadStationsEnabled() const;

		//! Load the complete inventory at startup, default = false
		void setLoadInventoryEnabled(bool enable);
		bool isLoadInventoryEnabled() const;

		//! Load the configmodule from the database at startup, default = false
		void setLoadConfigModuleEnabled(bool enable);
		bool isLoadConfigModuleEnabled() const;

		//! Load the cities.xml file, default = false
		void setLoadCitiesEnabled(bool enable);
		bool isLoadCitiesEnabled() const;

		//! Load the custom defined fep regions in ~/.seiscomp3/fep or
		//! ~/seiscomp3/trunk/share/fep, default = false
		void setLoadRegionsEnabled(bool enable);
		bool isLoadRegionsEnabled() const;

		//! Sets whether the received notifier are applied automatically
		//! or not, default: true

		/**
		 * Sets whether the received notifier are applied automatically
		 * or not, default: true
		 * When AutoApplyNotifier is enabled a received message will
		 * be handled in two passes:
		 *  1. pass: Apply all attached notifier
		 *  2. pass: Interpret all notifier
		 *
		 * So when using an object in an interprete callback it is
		 * garantueed that all child objects that also has been sent
		 * inside the message are attached to it.
		 */
		void setAutoApplyNotifierEnabled(bool enable);
		bool isAutoApplyNotifierEnabled() const;

		/**
		 * Sets whether the received notifier will be interpreted or not.
		 * Default: true
		 * When this option is enabled, the callback methods
		 *  addObject(), updateObject() and removeObject() will be
		 * called after a notifier has been received.
		 */
		void setInterpretNotifierEnabled(bool enable);
		bool isInterpretNotifierEnabled() const;

		/** Returns whether a custom publicID pattern has been configured
		    or not */
		bool hasCustomPublicIDPattern() const;

		/**
		 * Sets the number of retries if a connection fails.
		 * The default value is 0xFFFFFFFF and should be understood
		 * as "keep on trying".
		 */
		void setConnectionRetries(unsigned int);

		//! Enables/disables logging of context (source file + line number)
		void setLoggingContext(bool);

		//! Enables/disables logging of component
		void setLoggingComponent(bool);

		//! Enables/disables logging to stderr
		void setLoggingToStdErr(bool);

		//! Adds a certain component to the logging output
		void addLoggingComponentSubscription(const std::string&);

		//! Sets the config module name to use when reading
		//! the database configuration. An empty module name
		//! means: read all available modules.
		//! The default module is "trunk".
		void setConfigModuleName(const std::string &module);
		const std::string &configModuleName() const;

		//! Sets the master module used when auto shutdown
		//! is activated.
		void setShutdownMasterModule(const std::string &module);

		//! Sets the master username used when auto shutdown
		//! is activated.
		void setShutdownMasterUsername(const std::string &username);

		//! Closes the logging backend
		void closeLogging();

		/**
		 * Adds a logger for an input object flow.
		 * This method must be called after Application::init().
		 * The returned pointer is managed by the Application and must not
		 * be deleted.
		 */
		ObjectLog *
		addInputObjectLog(const std::string &name,
		                  const std::string &channel = "");

		/**
		 * Adds a logger for an output object flow.
		 * This method must be called after Application::init().
		 * The returned pointer is managed by the Application and must not
		 * be deleted.
		 */
		ObjectLog *
		addOutputObjectLog(const std::string &name,
		                   const std::string &channel = "");

		/**
		 * Logs input/output object throughput.
		 * @param log Pointer returned by addInputObjectLog or addOutputObjectLog
		 * @param timestamp The timestamp to be logged
		 */
		void logObject(ObjectLog *log, const Core::Time &timestamp,
		               size_t count = 1);

		/**
		 * Reloads the application inventory from either an XML file or
		 * the database.
		 */
		bool reloadInventory();


	// ----------------------------------------------------------------------
	//  Static public members
	// ----------------------------------------------------------------------
	public:
		//! Returns the pointer to the application's instance.
		static Application* Instance();

		/**
		 * Enabled/disables signal handling.
		 * It is enabled by default.
		 * NOTE: Call this method BEFORE construction when disabling signal
		 *       handling.
		 * @param termination enables/disables SIGTERM, SIGINT
		 * @param crash enables/disables SIGSEGV, SIGABRT
		 */
		static void HandleSignals(bool termination, bool crash);


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		//! Returns the applications version. The default implementation
		//! returns NULL and uses the global framework version instead.
		virtual const char *version();

		//! Reimplement this method to add additional commandline groups
		//! and/or options
		virtual void createCommandLineDescription();

		//! This method can be used to verify custom configuration or
		//! commandline parameters
		virtual bool validateParameters();

		//! Initialization method.
		virtual bool init();

		/**
		 * Starts the mainloop until exit() or quit() is called.
		 * The default implementation waits for messages in blocking mode
		 * and calls handleMessage() whenever a new message arrives.
		 */
		virtual bool run();

		//! This method gets called when all messages has been read or
		//! the connection is invalid
		virtual void idle();

		//! Cleanup method called before exec() returns.
		virtual void done();

		/**
		 * Forks the process.
		 * @return The result of forking
		 */
		virtual bool forkProcess();

		//! Opens the configuration file and reads the state variables
		virtual bool initConfiguration();

		//! Loads plugins
		virtual bool initPlugins();

		//! Initialized the database
		virtual bool initDatabase();

		//! Sets the database interface and creates a database query object
		void setDatabase(IO::DatabaseInterface* db);

		/**
		 * Reads the requested subscriptions from the configuration file
		 * and apply them to the messaging connection.
		 */
		virtual bool initSubscriptions();

		/**
		 * Prints the version information to stdout
		 */
		virtual void printVersion();

		//! Handles the interrupt request from outside
		void handleInterrupt(int) throw();

		/**
		 * Derived class can implement this method to react on
		 * errors while initialization. The default implementation
		 * does nothing.
		 * @param stage The stage where the error occured
		 */
		virtual bool handleInitializationError(Stage stage);

		const std::set<std::string>& subscribedGroups() const;

		/**
		 * Called when the application starts a sync request. This event can
		 * be implemented in derived classes to pause other services such
		 * as record acquisition. The default implementation is empty.
		 */
		virtual void handleStartSync();

		/**
		 * Called when the sync request is finished. This event can be
		 * implemented to resume paused services. The default implementation
		 * does nothing.
		 */
		virtual void handleEndSync();

		/**
		 * Called when the application received the AcquisitionFinished event.
		 * This is most likely send from the readRecords thread of the
		 * StreamApplication. The default implementation does nothing.
		 */
		virtual void handleEndAcquisition();


	// ----------------------------------------------------------------------
	//  Messaging handlers
	// ----------------------------------------------------------------------
	protected:
		virtual bool dispatch(Core::BaseObject*);

		//! Custom dispatch method for notifications with negative (< 0)
		//! types. The default implementation return false.
		virtual bool dispatchNotification(int type, Core::BaseObject*);

		//! Callback method to display a message regarding the current
		//! initialization state
		virtual void showMessage(const char*);

		//! Callback method to display a warning regarding the current
		//! initialization state
		virtual void showWarning(const char*);

		/**
		 * Reads messages from the connection.
		 * @return true, if successfull, false if not. When returning false,
		 *         the mainloop will stop and the program is going to
		 *         terminate.
		 */
		bool readMessages();

		/**
		 * This method gets called when a previously started timer timeout's.
		 * The timer has to be started by enableTimer(timeout).
		 */
		virtual void handleTimeout();

		/**
		 * This method is called when close event is sent to the application.
		 * The default handler returns true and causes the event queue to
		 * shutdown and to exit the application.
		 * It false is returned the close event is ignored.
		 */
		virtual bool handleClose();

		/**
		 * This methods gets called when an auto shutdown has been
		 * initiated. The default implementation just quits.
		 */
		virtual void handleAutoShutdown();

		/**
		 * This methods gets called when an the log interval is reached
		 * and the application should prepare its logging information. This
		 * method can be used to sync logs.
		 * The default implementation does nothing.
		 */
		virtual void handleMonitorLog(const Core::Time &timestamp);

		/**
		 * This methods gets called when a sync response has been
		 * received. It passes the sync ID received to the callee.
		 * The default implementation does nothing.
		 */
		virtual void handleSync(const char *ID);

		/**
		 * This method gets called after the connection got lost.
		 */
		virtual void handleDisconnect();

		/**
		 * This method gets called after the connection got reestablished.
		 */
		virtual void handleReconnect();

		/**
		 * This method gets called whenever a new message arrives. Derived
		 * classes has to implement this method to receive messages.
		 * To enable autoapplying and notifier interpreting call this method
		 * inside the reimplemented version.
		 * @param msg The message. A smartpointer may be stored for
		 *            future use. The pointer must not be deleted!
		 */
		virtual void handleMessage(Core::Message* msg);

		/**
		 * When a network message arrives that couldn't be decoded into a
		 * message this methods get called.
		 */
		virtual void handleNetworkMessage(const Communication::NetworkMessage* msg);

		//! Callback for interpret notifier
		virtual void addObject(const std::string& parentID, DataModel::Object*) {}

		//! Callback for interpret notifier
		virtual void removeObject(const std::string& parentID, DataModel::Object*) {}

		//! Callback for interpret notifier
		virtual void updateObject(const std::string& parentID, DataModel::Object*) {}


	// ----------------------------------------------------------------------
	//  Configuration query functions
	// ----------------------------------------------------------------------
	public:
		/**
		 * Read a single value from the application's configuration.
		 * All configuration query methods throw exceptions when
		 * the query could not be resolved or the requested format
		 * did not match.
		 * This documentation applies to all configGet* functions.
		 * @param query The query
		 * @return The requested value
		 */
		bool configGetBool(const std::string& query) const throw(Config::Exception);
		int configGetInt(const std::string& query) const throw(Config::Exception);
		double configGetDouble(const std::string& query) const throw(Config::Exception);
		std::string configGetString(const std::string& query) const throw(Config::Exception);

		/**
		 * @brief Convenience method that calls configGetString and resolves
		 *        variables such as @DATADIR@ and @ROOTDIR@ and produces a
		 *        canonicalized absolute pathname.
		 * @param query The query
		 * @return The path
		 */
		std::string configGetPath(const std::string& query) const throw(Config::Exception);

		std::vector<bool> configGetBools(const std::string& query) const throw(Config::Exception);
		std::vector<int> configGetInts(const std::string& query) const throw(Config::Exception);
		std::vector<double> configGetDoubles(const std::string& query) const throw(Config::Exception);
		std::vector<std::string> configGetStrings(const std::string& query) const throw(Config::Exception);

		/**
		 * Write a singel value to the local section of the clients
		 * configuration file.
		 */
		void configSetBool(const std::string& query, bool v);
		void configSetInt(const std::string& query, int v);
		void configSetDouble(const std::string& query, double v);
		void configSetString(const std::string& query, const std::string &v);

		void configSetBools(const std::string& query, const std::vector<bool>&);
		void configSetInts(const std::string& query, const std::vector<int>&);
		void configSetDoubles(const std::string& query, const std::vector<double>&);
		void configSetStrings(const std::string& query, const std::vector<std::string>&);

		void configUnset(const std::string& query);

		bool saveConfiguration();


	// ----------------------------------------------------------------------
	//  Private functions
	// ----------------------------------------------------------------------
	private:
		void prepare(int argc, char** argv);

		std::string argumentStr(const std::string& query) const;

		bool parseCommandLine();
		int acquireLockfile(const std::string& lockfile);

		void initCommandLine();
		bool initLogging();
		bool initMessaging();

		bool loadConfig(const std::string &configDB);
		bool loadInventory(const std::string &inventoryDB);

		void startMessageThread();
		void runMessageThread();

		bool processEvent();

		void handleNotifier(DataModel::Notifier*);

		void timeout();

		void monitorLog(const Communication::SystemConnection*,
		                const Core::Time &,std::ostream&);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		static Application* _instance;
		static bool _handleCrash;
		static bool _handleTermination;

		int _argc;
		char** _argv;

		std::string _name;

		Arguments _arguments;
		boost::shared_ptr<CommandLine> _commandline;

		std::string _agencyID;
		std::string _author;
		std::set<std::string> _procWhiteList;
		std::set<std::string> _procBlackList;

		Inventory::TypeWhiteList _networkTypeWhiteList;
		Inventory::TypeBlackList _networkTypeBlackList;

		Inventory::TypeWhiteList _stationTypeWhiteList;
		Inventory::TypeBlackList _stationTypeBlackList;

		Logging::Output* _logger;
		DataModel::DatabaseQueryPtr _query;

		std::string _configModuleName;
		DataModel::ConfigModulePtr _configModule;

		// Initialization configuration
		bool _enableDaemon;
		bool _enableMessaging;
		bool _enableStartStopMessages;
		bool _enableAutoShutdown;
		bool _enableDatabase;
		bool _enableRecordStream;
		bool _enableFetchDatabase;
		bool _enableLoadStations;
		bool _enableLoadInventory;
		bool _enableLoadConfigModule;
		bool _enableAutoApplyNotifier;
		bool _enableInterpretNotifier;
		bool _enableLoadCities;
		bool _enableLoadRegions;

		unsigned int _retryCount;

		std::vector<Math::Geo::CityD> _cities;

		std::vector<std::string> _messagingSubscriptionRequests;
		std::set<std::string> _messagingSubscriptions;
		std::string _crashHandler;
		std::string _shutdownMasterModule;
		std::string _shutdownMasterUsername;

		bool _customPublicIDPattern;
		int _objectLogTimeWindow;


	protected:
		Config::Config _configuration;

		ObjectMonitor *_inputMonitor;
		ObjectMonitor *_outputMonitor;

		int _returnCode;
		bool _exitRequested;

		ThreadedQueue<Notification> _queue;
		boost::thread *_messageThread;

		Communication::ConnectionPtr _connection;
		IO::DatabaseInterfacePtr _database;

		std::string _version;

		// State variables
		std::string _messagingUser;
		std::string _messagingHost;
		std::string _messagingPrimaryGroup;
		std::string _messagingEncoding;
		unsigned int _messagingTimeout;

		std::string _inventoryDB;
		std::string _configDB;

		std::string _dbType;
		std::string _dbParameters;
		std::string _db;

		std::string _recordStream;

		std::string _alternativeConfigFile;

		std::string _alternativeLogFile;

		unsigned int _verbosity;
		bool _logContext;
		int _logComponent;
		bool _logToStdout;

		std::string _plugins;
		std::string _currentSyncID;

		typedef std::vector<std::string> ComponentList;
		ComponentList _logComponents;
		std::string _lockfile;

		Util::Timer _userTimer;

		boost::mutex _objectLogMutex;
};


inline CommandLine& Application::commandline() {
	return *_commandline;
}

inline const CommandLine& Application::commandline() const {
	return *_commandline;
}





}
}


#endif
