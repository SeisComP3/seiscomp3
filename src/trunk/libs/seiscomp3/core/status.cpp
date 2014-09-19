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


#include <seiscomp3/core/status.h>

namespace Seiscomp
{
namespace Core
{


const char* const Status::STATUS_CODES_NAMES[ SC_QUANTITY ] =
    {
		"SEISCOMP_SUCCESS",
		"SEISCOMP_ERROR",
		"SEISCOMP_INVALID_GROUP_ERROR",
		"SEISCOMP_INVALID_CLIENT_NAME_ERROR",
		"SEISCOMP_TIMEOUT_ERROR",
		"SEISCOMP_MESSAGE_SIZE_ERROR",
		"SEISCOMP_ARCHIVE_REQUEST_ERROR",
		"SEISCOMP_CONNECT_ERROR",
		"SEISCOMP_NETWORKING_ERROR",
		"SEISCOMP_NOT_CONNECTED_ERROR",
		"SEISCOMP_CLIENT_NAME_NOT_UNIQUE",
		"SEISCOMP_WRONG_SERVER_VERSION",
		"SEISCOMP_TOO_MANY_USERS"
    };



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* Status::StatusToStr(const int statusCode)
{
	if (statusCode >= 0 && statusCode < SC_QUANTITY)
		return STATUS_CODES_NAMES[ statusCode ];
	return "Unknown status code";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Core
} // namespace seiscomp3

