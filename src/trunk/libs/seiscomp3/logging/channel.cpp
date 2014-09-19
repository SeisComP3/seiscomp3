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
#include <boost/thread/mutex.hpp>
#include <string.h>

#include <seiscomp3/logging/channel.h>

using namespace std;

const char GlobalComponent[] = "/";

namespace Seiscomp {
namespace Logging {

static Channel *gRootChannel =0;

// big lock around channel lookups..
static boost::mutex gChannelLock;

// Use GetComponentChannel here because we want to reference the global
// versions, not the componentized versions..
// We'll use
SC_SYSTEM_CORE_API Channel *_SCDebugChannel = getGlobalChannel("debug", LL_DEBUG);
SC_SYSTEM_CORE_API Channel *_SCInfoChannel = getGlobalChannel("info", LL_INFO);
SC_SYSTEM_CORE_API Channel *_SCWarningChannel = getGlobalChannel("warning", LL_WARNING);
SC_SYSTEM_CORE_API Channel *_SCErrorChannel = getGlobalChannel("error", LL_ERROR);
SC_SYSTEM_CORE_API Channel *_SCNoticeChannel = getGlobalChannel("notice", LL_NOTICE);


/*! @class Seiscomp::Logging::Channel <seiscomp3/logging/channel.h>
  @brief Implements a hierarchical logging channel

  You should not need to use Channel directly under normal
  circumstances. See COMPONENT_CHANNEL() macro, GetComponentChannel() and
  getGlobalChannel()

  Channel implements channel logging support.  A channel is a named
  logging location which is global to the program.  Channels are
  hierarchically related.
  
  For example, if somewhere in your program a message is logged to
  "debug/foo/bar", then it will be delived to any subscribers to
  "debug/foo/bar", or subscribers to "debug/foo", or subscribers to
  "debug".   Subscribing to a channel means you will receive anything
  published on that channel or sub-channels.

  As a special case, subscribing to the channel "" means you will receive
  all messages - as every message has a channel and the empty string "" is
  considered to mean the root of the channel tree.

  In addition, componentized channels are all considered sub channels of
  the global channel hierarchy.  All rDebug(), rWarning(), and rError()
  macros publish to the componentized channels (component defined by
  SEISCOMP_COMPONENT).

  @code
      // get the "debug" channel for our component.  This is the same as
      // what rDebug() publishes to.
      Channel *node = COMPONENT_CHANNEL("debug", Log_Debug);
      // equivalent to
      Channel *node = GetComponentChannel(SEISCOMP_COMPONENT, "debug");

      // Or, get the global "debug" channel, which will have messages from
      // *all* component's "debug" channels.
      Channel *node = getGlobalChannel( "debug", Log_Debug );
  @endcode

  @author Valient Gough
  @see COMPONENT_CHANNEL()
  @see getComponentChannel()
  @see getGlobalChannel()
*/



Channel::Channel( const string &n, LogLevel level )
    : Node()
    , _name( n )
    , _level( level )
{
}

Channel::~Channel()
{
}

const std::string &Channel::name() const
{
	return _name;
}

LogLevel Channel::logLevel() const
{
	return _level;
}

void Channel::setLogLevel(LogLevel level)
{
	_level = level;
}

Channel *Channel::getComponent(Channel *parent, const char *component)
{
	ComponentMap::const_iterator it = components.find( component );

	if ( it == components.end() ) {
		Channel *ch = new Channel(_name, _level);
		components.insert(make_pair(std::string(component), ch));

		// connect to its parent
		if(parent)
	    	parent->addPublisher(ch);

		// connect to globalized version
		addPublisher(ch);

		return ch;
	}
	else {
		return it->second;
	}
}

/*! @relates Channel
  @brief Return the named channel across all components.

  Channels are hierarchical.  See Channel for more detail.
  The global channel contains messages for all component channels.

  For example, subscribing to the global "debug" means the subscriber would
  also get messages from <Component , "debug">, and <Component-B, "debug">, and
  <Component-C, "debug/foo">, etc.

  @author Valient Gough
*/
Channel *getGlobalChannel(const char *path, LogLevel level)
{
	return getComponentChannel(GlobalComponent, path, level);
}

/*!  @relates Channel
  @brief Return the named channel for a particular component.

  @author Valient Gough
*/
Channel *getComponentChannel(const char *component, const char *path, LogLevel level)
{
    // since we much with the globally visible channel tree, hold a lock..
	boost::mutex::scoped_lock l(gChannelLock);

	string currentPath;

	if ( !gRootChannel )
		gRootChannel = new Channel("", level);

	Channel *current = gRootChannel;
	Channel *currentComponent = 0;
	if ( strcmp(component, GlobalComponent) != 0 )
		currentComponent = gRootChannel->getComponent(0, component);

	while( *path ) {
		// if log level is currently undefined but we now know what it is, then
		// define it..
		if ( (current->logLevel() == LL_UNDEFINED) && (level != LL_UNDEFINED) )
			current->setLogLevel(level);

		const char *next = strchr( path , '/' );
		size_t len = next ? next - path : strlen( path );

		if ( len > 1 ) {
			string pathEl( path, len );

			if ( !currentPath.empty() )
				currentPath += '/';
			currentPath += pathEl;

			ComponentMap::const_iterator it = 
			current->subChannels.find( pathEl );

			if ( it != current->subChannels.end() ) {
				// found.  possibly creating sub-map
				current = it->second;
	    	}
			else {
				// create
				Channel *nm = new Channel( currentPath, level );
				current->subChannels.insert( make_pair(pathEl, nm) );

				current->addPublisher( nm );

				current = nm;
			}

			// track componentized version
			if ( currentComponent )
				currentComponent = current->getComponent(currentComponent,
				component);

			path += len;
		}
		else {
			// skip separator character
			++path;
		}
	}

	if ( currentComponent )
		return currentComponent;
	else
		return current;
}

void
Channel::publish(const Data &data) {
	set<Node*>::const_iterator it = data.seen.find(this);

	if ( it == data.seen.end() ) {
		const_cast<Data&>(data).seen.insert(this);
		Node::publish(data);
	}
}

}
}
