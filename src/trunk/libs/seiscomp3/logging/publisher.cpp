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
#include <seiscomp3/logging/publisher.h>

#include <seiscomp3/logging/channel.h>
#include <seiscomp3/logging/location.h>
#include <seiscomp3/logging/log.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h> // in case we need memcpy

using namespace Seiscomp::Logging;


/*! @class Seiscomp::Logging::Publisher <seiscomp3/logging/publisher.h>
  @brief Publisher used by log macros.

  This derives from Node and interfaces to the static PublishLoc logging
  data allowing them to be enabled or disabled depending on subscriber
  status.

  An instance of this class is created for every error message location.
  Normally this class is not used directly.  

  For example, this
  @code
     rDebug( "hello world" );
  @endcode

  is turned approximatly into this:
  @code
     static PublishLoc _rl = {
	 publish:  & Seiscomp::Logging::Register ,
	 pub: 0,
	 component: "component",
	 fileName: "myfile.cpp",
	 functionName: "functionName()",
	 lineNum: __LINE__,
	 channel: 0
     };
     if(_rL.publish != 0)
	 (*_rl.publish)( &_rL, _RLDebugChannel, "hello world" );
  @endcode

  The Publisher instance manages the contents of the static structure
  _rL.  When someone subscribes to it's message, then _rL.publish is set to
  point to the publishing function, and when there are no subscribers then
  it is set to 0.

  The code produced contains one if statement, and with optimization comes
  out to about 3 instructions on an x86 computer (not including the
  function call).  If there are no subscribers to this message then that
  is all the overhead, plus the memory usage for the structures
  involved and the initial registration when the statement is first
  encountered..

  @see Channel
  @author Valient Gough
 */


Publisher::Publisher()
{
}

Publisher::Publisher(PublishLoc *loc) : Node(), src( loc )
{
	// link to channel for channel based subscriptions
	// Lookup the componentized version
	Node *channelNode = getComponentChannel( src->component,
	   src->channel->name().c_str(), src->channel->logLevel() );
	channelNode->addPublisher( this );

	// link to file
	Node *fileNode = FileNode::Lookup(src->component, src->fileName);
	fileNode->addPublisher(this);
}

Publisher::~Publisher()
{
	clear();
}

void Publisher::setEnabled(bool active)
{
	if ( src ) {
		if ( active )
			src->enable();
		else
			src->disable();
	}
}

void Publisher::Publish( PublishLoc *loc, Channel *channel,
	const char *format, ...)
{
	va_list args;
	va_start( args, format );
	PublishVA( loc, channel, format, args );
	va_end( args );
}

void Publisher::PublishVA( PublishLoc *loc, Channel *,
	const char *format, va_list ap )
{
	Data data;

	data.publisher = loc;
	data.time = time(0);
	data.msg = 0;

	char msgBuf[64];
	char *buf = msgBuf;
	int bufSize = sizeof(msgBuf);

	// loop until we have allocated enough space for the message
	for ( int numTries = 10; numTries; --numTries ) {
		va_list args;

		// va_copy() is defined in C99, __va_copy() in earlier definitions, and
		// automake says to fall back on memcpy if neither exist...
		//
		// FIXME: memcpy doesn't work for compilers which use array type for
		//        va_list such as Watcom
#if defined( va_copy )
		va_copy( args, ap );
#elif defined( __va_copy )
		__va_copy( args, ap );
#else
		memcpy( &args, &ap, sizeof(va_list) );
#endif

		int ncpy = vsnprintf( buf , bufSize, format, args );
		va_end( args );

		// if it worked, then return the buffer
		if ( ncpy > -1 && ncpy < bufSize ) {
			data.msg = buf;
			break;
		}
		else {
			// newer implementations of vsnprintf return # bytes needed..
			if ( ncpy > 0 )
				bufSize = ncpy + 1;
			else
				bufSize *= 2; // try twice as much space as before

			if ( buf != msgBuf ) delete[] buf;

			buf = new char[bufSize];
		}
	}

	loc->pub->publish( data );

	if ( buf != msgBuf ) delete[] buf;
}

