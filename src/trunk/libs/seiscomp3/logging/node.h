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


#ifndef __SC_LOGGING_NODE_H__
#define __SC_LOGGING_NODE_H__

#include <list>
#include <set>
#include <time.h>
#include <string>
#include <boost/thread/mutex.hpp>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Logging {

class SC_SYSTEM_CORE_API Node;

/*! @struct Data <seiscomp3/logging/node.h>
    @brief Data published through Node

	Data is the data which is published from SEISCOMP_DEBUG(), SEISCOMP_WARNING(),
	SEISCOMP_ERROR() and SEISCOMP_LOG() macros.  It contains a link to the
	publisher, the (approximate) time of the publication, and the formatted message.

	Note that none of the data in the publication is considered to be static.
	Once control has returned to the publisher, the data may be destroyed. If
	it is necessary to hang on to any of the data, a deep copy must be made.
*/

struct SC_SYSTEM_CORE_API Data {
	struct PublishLoc *publisher;
	//! time of publication
	time_t time;
	//! formatted msg - gets destroyed when publish() call returns.
	const char *msg;

	// track which nodes have seen this message, to avoid getting
	// duplicates.  It would be nice if we could enforce this via the node
	// structure instead, but that is much harder.
	std::set<Node*> seen;
};

// documentation in implementation file
class SC_SYSTEM_CORE_API Node {
	public:
		Node();
		virtual ~Node();

		// remove this node from the subscription network
		virtual void clear();

		virtual void publish(const Data &data);

		// primary interface
		virtual void addPublisher(Node *);
		virtual void dropPublisher(Node *, bool callbacks=true );

		bool enabled() const;

		// used internally - see documentation
		virtual void addSubscriber(Node *);
		virtual void dropSubscriber(Node *);

		virtual void isInterested(Node *node, bool isInterested);

	protected:
		// called by Node when enable state changed.
		virtual void setEnabled(bool newState);

		//! list of nodes we are subscribed to
		std::list<Node *> publishers;

		//! list of nodes we publish to
		std::list<Node *> subscribers;

		//! list of subscribers that are interested in receiving publications..
		std::list<Node *> interestList;

		boost::mutex mutex;
};

}
}

#endif
