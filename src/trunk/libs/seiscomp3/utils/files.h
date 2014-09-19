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

#ifndef __SEISCOMP_UTILS_FILES__
#define __SEISCOMP_UTILS_FILES__

#include <string>
#include <streambuf>
#include <iostream>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Util {

/**
 * Removes the path portion from a given path:
 * /dir/file.cpp -> file.cpp
 * @return basename */
SC_SYSTEM_CORE_API std::string basename(const std::string& name);

//! Checks if a file is exists
SC_SYSTEM_CORE_API bool fileExists(const std::string& file);

//! Checks if a path (directory) exists
SC_SYSTEM_CORE_API bool pathExists(const std::string& path);

//! Creates a new directory inclusive all unavailable
//! parent directories
SC_SYSTEM_CORE_API bool createPath(const std::string& path);

/** Removes the extension from the filename
 * test.xyz -> test
 * @return name without extension */
SC_SYSTEM_CORE_API std::string removeExtension(const std::string& name);

/**
 * Converts a string or char array to a streambuf object.
 * @return The streambuf object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::streambuf *bytesToStreambuf(char *data, size_t n);
SC_SYSTEM_CORE_API std::streambuf *stringToStreambuf(const std::string &str);

/**
 * Tries to open a file and returns a corresponding output stream.
 * The special name '-' refers to stdout.
 * @return The ostream object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::ostream *file2ostream(const char *fn);

/**
 * Tries to open a file and returns a corresponding input stream.
 * The special name '-' refers to stdin.
 * @return The istream object. The caller is responsible to delete the object
 */
SC_SYSTEM_CORE_API std::istream *file2istream(const char *fn);


} // namespace Util
} // namespace Seiscomp

#endif
