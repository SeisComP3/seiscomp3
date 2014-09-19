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



#ifndef __SC_LOGGING_PUBLISHER_H__
#define __SC_LOGGING_PUBLISHER_H__

#include <seiscomp3/logging/common.h>
#include <seiscomp3/logging/node.h>

#include <stdarg.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

// documentation in implementation file
class SC_SYSTEM_CORE_API Publisher : public Node {
	public:
		Publisher(PublishLoc *src);
		Publisher();
		virtual ~Publisher();

		// metadata about the publisher and its publication
		PublishLoc *src;

		static void Publish(PublishLoc *, Channel *,
		                    const char *format, ...);
		static void PublishVA(PublishLoc *, Channel *,
		                      const char *format, va_list args);

	protected:
		virtual void setEnabled(bool newState);

		Publisher(const Publisher &);
		Publisher & operator=(const Publisher &);
};


}
}


#endif
