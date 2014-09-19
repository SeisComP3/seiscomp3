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


#ifndef __SEISCOMP_UTILS_REPLACE_H__
#define __SEISCOMP_UTILS_REPLACE_H__

#include <string>
#include <seiscomp3/core.h>

namespace Seiscomp {
namespace Util {


/**
 * Functor structur to resolve a variable and return
 * the content of it.
 */
struct SC_SYSTEM_CORE_API VariableResolver {
	/**
	 * Resolves a variable. The default implementation replaces:
     *  - hostname: The name of the host (uname -n)
     *  - user: The name of the user (echo $USER)
     *  - more to come...
     * When inheriting this class, call this method in the
     * new implementation to enable the build-in variables if this
     * behaviour in intended.
	 * @param variable The variable name that has to be resolved. The result
	 *                 will be written into it as well (inout).
	 * @return Whether the variable could be resolved or not
	 */
	virtual bool resolve(std::string& variable) const;
	virtual ~VariableResolver() {};
};


/**
 * Replaces variables of a string by their content.
 * Variables have to be enclosed by '@'. Two '@' will be
 * replaced by one '@'.
 * Replacing the variable "host" results when feeding the
 * input string "hostname: @host@".
 * This function used the VariableResolver class and replaces
 * beside the default variables:
 *  - [empty] => '@'
 * @param input The string containing variables to be replaced
 * @return The replaced input string
 */
SC_SYSTEM_CORE_API
std::string replace(const std::string& input);


/**
 * See: string replace(string)
 * @param input The string containing variables to be replaced
 * @param resolver A resolver functor that resolves the variable name
 *                 to its content.
 * @return The replaced input string
 */
SC_SYSTEM_CORE_API
std::string replace(const std::string& input,
                    const VariableResolver& resolver);

SC_SYSTEM_CORE_API
std::string replace(const char* input,
                    const VariableResolver& resolver);

SC_SYSTEM_CORE_API
std::string replace(const std::string &input,
                    const VariableResolver &resolver,
                    const std::string &prefix, const std::string &postfix,
                    const std::string &emptyValue);

}
}

#endif
