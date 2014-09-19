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


#ifndef __SC_LOGGING_LOGGING_H__
#define __SC_LOGGING_LOGGING_H__

#ifndef SEISCOMP_COMPONENT
#  ifndef WIN32
#    warning SEISCOMP_COMPONENT not defined -> setting to "Seiscomp"
#  endif
#  define SEISCOMP_COMPONENT "Seiscomp"
#endif

/* Define that SEISCOMP_* are also available as variadic versions, e.g.
   SEISCOMP_ERROR and SEISCOMP_VERROR whereas the latter takes an argument
   list (va_list), e.g. SEISCOMP_VERROR("%s:%d", args).
 */
#define SEISCOMP_LOG_VA

#include <seiscomp3/logging/publishloc.h>
#include <seiscomp3/logging/common.h>
#include <seiscomp3/logging/defs.h>


namespace Seiscomp {
namespace Logging {


class Channel;
class Output;


enum LogLevel {
	LL_UNDEFINED = 0,
	LL_CRITICAL,
	LL_ERROR,
	LL_WARNING,
	LL_NOTICE,
	LL_INFO,
	LL_DEBUG,
	LL_QUANTITY
};


/**
	\brief The following macros do the actual logging according
	\brief defined log levels
	The first parameter is the message including placeholders (printf style),
	the following parameters hold the values.
	\code
	SEISCOMP_[LEVEL]("Precondition not satisfied: i > %d", myVarHolding_i);
	\endcode
*/

#define STR(X) #X

/*! @def _scMessageDef
  Defines a static Publisher and points it to the registration function for
  the first call.
  @internal
*/
#ifdef __FUNCTION__
#define _scMessageDef(ID, COMPONENT) \
  static Seiscomp::Logging::PublishLoc ID SEISCOMP_SECTION = {& ID ## _enabled, \
      &Seiscomp::Logging::Register, &Seiscomp::Logging::RegisterVA, 0, STR(COMPONENT), __FILE__, \
      __FUNCTION__, __LINE__, 0};
#else
#define _scMessageDef(ID, COMPONENT) \
  static Seiscomp::Logging::PublishLoc ID SEISCOMP_SECTION = {& ID ## _enabled, \
      &Seiscomp::Logging::Register, &Seiscomp::Logging::RegisterVA, 0, STR(COMPONENT), __FILE__, \
      "[unknown]", __LINE__, 0};
#endif

/*! @def _scMessageCall
  Checks if the Publisher is enabled and publishes the message if so.
  @internal
*/
#if HAVE_PRINTF_FP || !HAVE_PRINTF_ATTR
# define _scMessageCall(ID, COMPONENT, CHANNEL, ...) \
  static bool ID ## _enabled = true; \
  if ( unlikely(ID ## _enabled) ) \
  { \
    _scMessageDef(ID, COMPONENT) \
    (*ID.publish)(&ID, CHANNEL, ##__VA_ARGS__); \
  }

# define _scvMessageCall(ID, COMPONENT, CHANNEL, format, args) \
  static bool ID ## _enabled = true; \
  if ( unlikely(ID ## _enabled) ) \
  { \
    _scMessageDef(ID, COMPONENT) \
    (*ID.publishVA)(&ID, CHANNEL, format, args); \
  }
#else // no PRINTF attributes..
# define _scMessageCall(ID, COMPONENT, CHANNEL, ...) \
  static bool ID ## _enabled = true; \
  if ( unlikely(ID ## _enabled) ) \
  { \
    _scMessageDef(ID, COMPONENT) \
    (*ID.publish)( &ID, CHANNEL, ##__VA_ARGS__ ); \
    Seiscomp::Logging::__checkArgs( 0, ##__VA_ARGS__ ); \
  }

# define _scvMessageCall(ID, COMPONENT, CHANNEL, format, args) \
  static bool ID ## _enabled = true; \
  if ( unlikely(ID ## _enabled) ) \
  { \
    _scMessageDef(ID, COMPONENT) \
    (*ID.publishVA)( &ID, CHANNEL, format, args); \
  }
#endif

/*! @def _scMessage(ID, CHANNEL, ... )

  Combines the publisher definition (_scMessageDef) and invokation
  (_scMessageCall)

  enclose in do{}while(0) to insure that it acts as a single statement even if
  placed in various if/else constructs..
  @internal
*/
#define _scMessage(ID, CHANNEL, ... ) \
  do { _scMessageCall(ID, SEISCOMP_COMPONENT, CHANNEL, ##__VA_ARGS__ ) } while(0)

#define _scvMessage(ID, CHANNEL, format, args) \
  do { _scvMessageCall(ID, SEISCOMP_COMPONENT, CHANNEL, format, args ) } while(0)

/*! @addtogroup LoggingMacros
  These macros are the primary interface for logging messages:
  - SEISCOMP_DEBUG(format, ...)
  - SEISCOMP_INFO(format, ...)
  - SEISCOMP_WARNING(format, ...)
  - SEISCOMP_ERROR(format, ...)
  - SEISCOMP_NOTICE(format, ...)
  - SEISCOMP_LOG(channel, format, ...)
  @{
*/

/*! @def SEISCOMP_DEBUG(format, ...)
    @brief Log a message to the "debug" channel.  Takes printf style arguments.

    Format is ala printf, eg:
    @code
    SEISCOMP_DEBUG("I'm sorry %s, I can't do %s", name, request);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_DEBUG(...) \
  _scMessage( _SCLOGID, Seiscomp::Logging::_SCDebugChannel, ##__VA_ARGS__ )

#define SEISCOMP_VDEBUG(format, args) \
  _scvMessage( _SCLOGID, Seiscomp::Logging::_SCDebugChannel, format, args )

/*! @def SEISCOMP_INFO(format, ...)
    @brief Log a message to the "info" channel.  Takes printf style arguments.

    Format is ala printf, eg:
    @code
    SEISCOMP_INFO("I'm sorry %s, I can't do %s", name, request);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_INFO(...) \
  _scMessage( _SCLOGID, Seiscomp::Logging::_SCInfoChannel, ##__VA_ARGS__ )

#define SEISCOMP_VINFO(format, args) \
  _scvMessage( _SCLOGID, Seiscomp::Logging::_SCInfoChannel, format, args )

/*! @def SEISCOMP_WARNING(format, ...)
    @brief Log a message to the "warning" channel.  Takes printf style
    arguments.

    Output a warning message - meant to indicate that something doesn't seem
    right.

    Format is ala printf, eg:
    @code
    SEISCOMP_WARNING("passed %i, expected %i, continuing", foo, bar);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_WARNING(...) \
  _scMessage( _SCLOGID, Seiscomp::Logging::_SCWarningChannel, ##__VA_ARGS__ )

#define SEISCOMP_VWARNING(format, args) \
  _scvMessage( _SCLOGID, Seiscomp::Logging::_SCWarningChannel, format, args )

/*! @def SEISCOMP_ERROR(...)
    @brief Log a message to the "error" channel. Takes printf style arguments.

    An error indicates that something has definately gone wrong.

    Format is ala printf, eg:
    @code
    SEISCOMP_ERROR("bad input %s, aborting request", input);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_ERROR(...) \
  _scMessage( _SCLOGID, Seiscomp::Logging::_SCErrorChannel, ##__VA_ARGS__ )

#define SEISCOMP_VERROR(format, args) \
  _scvMessage( _SCLOGID, Seiscomp::Logging::_SCErrorChannel, format, args )

/*! @def SEISCOMP_NOTICE(...)
    @brief Log a message to the "notice" channel. Takes printf style arguments.

    A notice indicates that something important happened.

    Format is ala printf, eg:
    @code
    SEISCOMP_NOTICE("bad input %s, aborting request", input);
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_NOTICE(...) \
  _scMessage( _SCLOGID, Seiscomp::Logging::_SCNoticeChannel, ##__VA_ARGS__ )

#define SEISCOMP_VNOTICE(format, args) \
  _scvMessage( _SCLOGID, Seiscomp::Logging::_SCNoticeChannel, format, args )

/*! @def SEISCOMP_LOG(channel,format,...)
    @brief Log a message to a user defined channel. Takes a channel and printf
    style arguments.

    An error indicates that something has definately gone wrong.

    Format is ala printf, eg:
    @code
    static Channel * MyChannel = SEISCOMP_CHANNEL("debug/mine");
    SEISCOMP_LOG(MyChannel, "happy happy, joy joy");
    @endcode

    When using a recent GNU compiler, it should automatically detect format
    string / argument mismatch just like it would with printf.

    Note that unless there are subscribers to this message, it will do nothing.
*/
#define SEISCOMP_LOG(channel, ...) \
  _scMessage( _SCLOGID, channel, ##__VA_ARGS__ )

#define SEISCOMP_VLOG(channel, format, args) \
  _scvMessage( _SCLOGID, channel, format, args )

/*! @def DEF_CHANNEL( const char *path, LogLevel level )
    @brief Returns pointer to Channel struct for the given path
    @param path The hierarchical path to the channel.  Elements in the path
    are separated by '/'.

	SEISCOMP_DEF_LOGCHANNEL gets an existing (or defines a new) log type.  For example
	"debug", "warning", "error" are predefined types.  You might define
	completely new types, like "timing", or perhaps sub-types like
	"debug/timing/foo", depending on your needs.

	Reporting paths do not need to be unique within a project (or even a
	file).

	Channels form a hierarchy.  If one subscribes to "debug", then you also
	get messages posted to more specific types such as "debug/foo".  But if
	you subscribe to a more specific type, such as "debug/foo", then you will
	not receive more general messages such as to "debug".

	Example:
	@code
	#include <seiscomp3/logging/log.h>
	#include <seiscomp3/logging/channel.h>

	static Channel *MyChannel = SEISCOMP_DEF_LOGCHANNEL("me/mine/allmine",Log_Info);

	func()
	{
		SEISCOMP_LOG( MyChannel, "this is being sent to my own channel" );
		SEISCOMP_LOG( MyChannel, "%s %s", "hello", "world" );
	}

	main()
	{
		// log all messages to the "me" channel to stderr
		StdioNode stdLog( STDERR_FILENO );
		stdLog.subscribeTo( SEISCOMP_CHANNEL ("me") );

		func();
	}
	@endcode

	@see test.cpp
 */

#define SEISCOMP_DEF_LOGCHANNEL(path,level) SEISCOMP_CHANNEL_IMPL(SEISCOMP_COMPONENT, path, level)
#define SEISCOMP_CHANNEL(path) SEISCOMP_CHANNEL_IMPL(SEISCOMP_COMPONENT, path, Seiscomp::Logging::UNDEFINED)
#define SEISCOMP_CHANNEL_IMPL(COMPONENT,path,level) \
    Seiscomp::Logging::getComponentChannel(LOG_STR(COMPONENT),path,level)


#define SEISCOMP_DEBUG_S(str) SEISCOMP_DEBUG("%s", (str).c_str())
#define SEISCOMP_INFO_S(str) SEISCOMP_INFO("%s", (str).c_str())
#define SEISCOMP_WARNING_S(str) SEISCOMP_WARNING("%s", (str).c_str())
#define SEISCOMP_ERROR_S(str) SEISCOMP_ERROR("%s", (str).c_str())
#define SEISCOMP_NOTICE_S(str) SEISCOMP_NOTICE("%s", (str).c_str())
#define SEISCOMP_LOG_S(str) SEISCOMP_LOG("%s", (str).c_str())

/**
	Loglevel "LOG" is a special level, which needs a channel, to
	log the message to.
	\code
	static Channel* MyChannel = SEISCOMP_DEF_LOGCHANNEL("myroot/mysub", Log_Error);
	SEISCOMP_LOG(myChannel, "Postcondition not satisfied: count2 [%d] <= count1 [%d]", count2, count1);
	\endcode
*/

SC_SYSTEM_CORE_API void debug(const char*);
SC_SYSTEM_CORE_API void info(const char*);
SC_SYSTEM_CORE_API void warning(const char*);
SC_SYSTEM_CORE_API void error(const char*);
SC_SYSTEM_CORE_API void notice(const char*);
SC_SYSTEM_CORE_API void log(Channel*, const char* format);


/** NOTE: All retrieved Channel pointer must not be deleted!!! */

/** Retrieve all messages from all components */
SC_SYSTEM_CORE_API Channel* getAll();

/** Retrieve messages from a channel of a all components */
SC_SYSTEM_CORE_API Channel* getGlobalChannel(const char* channel, LogLevel level = LL_UNDEFINED);

/** Retrieve messages from a channel of a given component */
SC_SYSTEM_CORE_API Channel* getComponentChannel(const char* component, const char* channel, LogLevel level = LL_UNDEFINED);

/** Retrieve defined messages from a component */
SC_SYSTEM_CORE_API Channel* getComponentAll(const char* component);
SC_SYSTEM_CORE_API Channel* getComponentDebugs(const char* component);
SC_SYSTEM_CORE_API Channel* getComponentInfos(const char* component);
SC_SYSTEM_CORE_API Channel* getComponentWarnings(const char* component);
SC_SYSTEM_CORE_API Channel* getComponentErrors(const char* component);
SC_SYSTEM_CORE_API Channel* getComponentNotices(const char* component);


/**
	\brief Example on how to use logging

	File: [name].cpp
	\code
	// NOTE: The definition of component has to be done before the include
	// of log.h !
	#define SEISCOMP_COMPONENT MyComponent
	#include <seiscomp3/logging/log.h>

	// include further header
	...

	void foo(int i) {
		if ( i < 0 ) {
			SEISCOMP_WARNING("precondition: i >= 0 not satisfied, i = %d", i);
			return;
		}

		SEISCOMP_INFO("postcondition satisfied");
	}
	\endcode

	To subscribe logging channels and to output logging messages,
	create a node and subscribe the channels of interest.
	\code
	int main(int argc, char** argv) {
		SeisComp::Logging::Init(argc, argv);
		...
		// create one default node and log to stderr
		Seiscomp::Logging::FdOutput stdLog(STDERR_FILENO);
		// lets receive all warnings from the component MyComponent (see above).
		stdLog.subscribe(Seiscomp::Logging::GetComponentWarnings("MyComponent"));
		...
		// dont catch any messages
		stdLog.clear();
		// lets receive all messages from the component MyComponent
		stdLog.subscribe(Seiscomp::Logging::GetComponentAll("MyComponent"));
		...
		// subscribe to all messages globally
		stdLog.subscribe(Seiscomp::Logging::GetAll());

	}
	\endcode
*/


SC_SYSTEM_CORE_API Output* consoleOutput();
SC_SYSTEM_CORE_API void enableConsoleLogging(Channel*);
SC_SYSTEM_CORE_API void disableConsoleLogging();

SC_SYSTEM_CORE_API void init(int &argc, char **argv);


}
}

#endif
