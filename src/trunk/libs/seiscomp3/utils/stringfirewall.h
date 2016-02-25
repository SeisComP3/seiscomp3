/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker <jabe@gempa.de>                                    *
 ***************************************************************************/


#ifndef __SEISCOMP_UTILS_STRINGFIREWALL_H__
#define __SEISCOMP_UTILS_STRINGFIREWALL_H__


#include <seiscomp3/core/baseobject.h>

#include <string>
#include <set>
#include <map>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(StringFirewall);


/**
 * @brief The StringFirewall class implements a "firewall" for strings.
 *
 * It manages a blacklist and a whitelist of strings. Whether a string is
 * accepted is checked with the following algorithm:
 *   - Accept if both lists are empty
 *   - If either list is empty the partial result (accepted1 or accepted2) is true
 *   - If the input string matches any item in the whitelist, accepted1 is true,
 *     false otherwise
 *   - If the input string matches any item in the blacklist, accepted2 is false,
 *     true otherwise
 *   - The result is accepted1 && accepted2
 */
class SC_SYSTEM_CORE_API StringFirewall : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  Public typedefs and variables
	// ----------------------------------------------------------------------
	public:
		typedef std::set<std::string> StringSet;

		StringSet allow; //!< The whitelist
		StringSet deny;  //!< The blacklist


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief isAllowed evaluates if a string may pass the firewall.
		 * @param s The input string
		 * @return true if it may pass, false otherwise
		 */
		virtual bool isAllowed(const std::string &s) const;

		/**
		 * @brief isDenied evaluates if a string is blocked by the firewall.
		 *
		 * It holds that isDenied(s) == !isAllowed(s).
		 *
		 * @param s The input string
		 * @return true if it is blocked, false otherwise
		 */
		bool isDenied(const std::string &s) const;
};



/**
 * @brief The WildcardStringFirewall class extends the StringFirewall by
 *        allowing wildcard ('*' or '?') matches for strings.
 *
 * Each call to either isAllowed or isDenied is cached by default to reduce
 * evaluation time for repeating queries. If queries do not repeat or are
 * more or less random then caching should be disabled.
 */
class SC_SYSTEM_CORE_API WildcardStringFirewall : public StringFirewall {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		WildcardStringFirewall();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * Clears the internal evaluation cache. This should be called when
		 * either #allow or #deny have changed.
		 */
		void clearCache() const;

		/**
		 * @brief Enables query caching. The default is true.
		 * @param enable Whether to cache queries.
		 */
		void setCachingEnabled(bool enable);

		/**
		 * @brief isAllowed evaluates if a string may pass the firewall.
		 * @param s The input string
		 * @return true if it may pass, false otherwise
		 */
		virtual bool isAllowed(const std::string &s) const;


	// ----------------------------------------------------------------------
	//  Private members and typedefs
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, bool> StringPassMap;
		mutable StringPassMap _cache;
		bool                  _enableCaching;
};


}
}


#endif
