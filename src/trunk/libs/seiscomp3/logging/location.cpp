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
#include <seiscomp3/logging/location.h>

#include <boost/thread/mutex.hpp>
#include <map>

using namespace std;

namespace Seiscomp {
namespace Logging {

struct FileNodeMap : public map<string, FileNode*> {
	~FileNodeMap() {
		for ( iterator it = begin(); it != end(); ++it )
			delete it->second;
	}
};

static FileNodeMap gFileMap;

/*
locks for global maps
*/
static boost::mutex gMapLock;

/*! @class Seiscomp::Logging::FileNode <seiscomp3/logging/location.h>
  @brief Provides filename based logging nodes.

  This allows subscribing to messages only from particular files.
  For example,
  @code

  int main()
  {
      // display some messages to stderr
      StdioNode std( STDERR_FILENO );

      // subscribe to everything from this file
      std.subscribeTo( FileNode::Lookup( __FILE__ ) );

      // and everything from "important.cpp"
      std.subscribeTo( FileNode::Lookup( "important.cpp" ));
  }
  @endcode

  Note that file names are not required to be unique across the entire
  program.  Different components may contain the same filename, which is
  why there is a second Lookup function which also takes the component
  name.

  @see Channel
  @author Valient Gough
*/


FileNode::FileNode(const char *_cn, const char *_fileName)
    : Node()
    , componentName( _cn )
    , fileName( _fileName )
{
}

FileNode::FileNode(const char *_fileName)
    : Node()
    , fileName( _fileName )
{
}

FileNode::~FileNode()
{
}

FileNode *
FileNode::Lookup( const char *fileName )
{
	boost::mutex::scoped_lock l(gMapLock);

	// no component specified, so look for componentless filename node
	FileNodeMap::const_iterator it = gFileMap.find( fileName );

	if ( it != gFileMap.end() ) {
		return it->second;
	}
	else {
		// create the componentless filename node.  We can't create a fully
		// componentized version because we don't have a component name..
		FileNode *node = new FileNode(fileName);
		gFileMap.insert( make_pair(std::string(fileName), node));

		return node;
    }
}

FileNode *
FileNode::Lookup(const char *componentName, const char *fileName)
{
    // do this first before we take out the lock
	FileNode *partial = Lookup( fileName );

	boost::mutex::scoped_lock l(gMapLock);

    // fullName is "[componentName]::[fileName]"
	string fullName = componentName;
	fullName += "::";
	fullName += fileName;

	FileNodeMap::const_iterator it = gFileMap.find( fullName );

	if ( it != gFileMap.end() ) {
		return it->second;
	}
	else {
		FileNode *node = new FileNode( componentName, fileName );
		gFileMap.insert( make_pair( fullName, node ));

		// partial node never publishes, but it can forward publications from
		// the fully specified nodes..
		partial->addPublisher( node );

		return node;
	}
}

}
}
