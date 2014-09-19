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

#ifndef __SC_LOGGING_DEFS_H__
#define __SC_LOGGING_DEFS_H__

#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Logging {


class SC_SYSTEM_CORE_API Channel;

SC_SYSTEM_CORE_API extern Channel *_SCDebugChannel;
SC_SYSTEM_CORE_API extern Channel *_SCInfoChannel;
SC_SYSTEM_CORE_API extern Channel *_SCWarningChannel;
SC_SYSTEM_CORE_API extern Channel *_SCErrorChannel;
SC_SYSTEM_CORE_API extern Channel *_SCNoticeChannel;


}
}


#endif
