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


#ifndef __SEISCOMP_UTILS_BASE64_H__
#define __SEISCOMP_UTILS_BASE64_H__

#include <seiscomp3/core.h>
#include <string>

namespace Seiscomp {
namespace Util {

/**
 * Converts a datastream into a base64 encoded string.
 * @param target The target string
 * @param data The source data stream
 * @param data_size The source data streamsize (number of elements, not bytes!)
 */
SC_SYSTEM_CORE_API void encodeBase64(std::string &target, const char *data, size_t data_size);

/**
 * Converts a datastream into a base64 encoded string.
 * @param target The target string
 * @param data The source data string
 */
SC_SYSTEM_CORE_API void encodeBase64(std::string &target, const std::string &data);

/**
 * Decodes a base64 encoded string.
 * @param target The container for the decoded data stream
 * @param data The base64 encoded string
 * @param data_size The base64 encoded string size (number of elements, not bytes!)

 */
SC_SYSTEM_CORE_API void decodeBase64(std::string &target, const char *data, size_t data_size);

/**
 * Decodes a base64 encoded string.
 * @param target The container for the decoded data stream
 * @param data The base64 encoded string
 */
SC_SYSTEM_CORE_API void decodeBase64(std::string &target, const std::string &data);

}
}


#endif
