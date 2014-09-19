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



#ifndef __SC_LOGGING_LOCATION_H__
#define __SC_LOGGING_LOCATION_H__

#include <string>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/logging/node.h>

namespace Seiscomp {
namespace Logging {

// documentation in implementation file
class SC_SYSTEM_CORE_API FileNode : public Node {
	public:
		FileNode(const char *componentName, const char *fileName);
		FileNode(const char *fileName);
		virtual ~FileNode();

		static FileNode *Lookup(const char *componentName,
		                        const char *fileName);
		static FileNode *Lookup(const char *fileName);

		std::string componentName;
		std::string fileName;

	private:
        FileNode(const FileNode&);
		FileNode & operator = (const FileNode&);
};

}
}

#endif

