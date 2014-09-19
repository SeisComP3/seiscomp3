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

#ifndef __SEISCOMP_CORE_STATUS__
#define __SEISCOMP_CORE_STATUS__

#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Core {

/**
 * Seiscomp status codes definitions.
 */
class SC_SYSTEM_CORE_API Status
{

	// -------------------------------------------------------------------
	// Status codes
	// -------------------------------------------------------------------
public:
	enum STATUS_CODES
	{
	    SEISCOMP_SUCCESS,
	    SEISCOMP_FAILURE,
		SEISCOMP_INVALID_GROUP_ERROR,
		SEISCOMP_INVALID_CLIENT_NAME_ERROR,
		SEISCOMP_TIMEOUT_ERROR,
		SEISCOMP_MESSAGE_SIZE_ERROR,
		SEISCOMP_ARCHIVE_REQUEST_ERROR,
		SEISCOMP_CONNECT_ERROR,
		SEISCOMP_NETWORKING_ERROR,
		SEISCOMP_NOT_CONNECTED_ERROR,
		SEISCOMP_CLIENT_NAME_NOT_UNIQUE,
		SEISCOMP_WRONG_SERVER_VERSION,
		SEISCOMP_TOO_MANY_USERS,
	    SC_QUANTITY
	};


	// ---------------------------------------------------------------
	// Public interfcae
	// ---------------------------------------------------------------
public:

	/** Returns the string representation of the passed seiscomp error code
	 * @return error code string representaion
	 */
	static const char* StatusToStr(const int statusCode);


	// ---------------------------------------------------------------
	// X'struction
	// ---------------------------------------------------------------
private:
	Status() {}


	// ---------------------------------------------------------------
	// Private data memebers
	// ---------------------------------------------------------------
private:
	static const char* const STATUS_CODES_NAMES[];
	
};


} // namespace Core
} // namespace Seiscomp

#endif
