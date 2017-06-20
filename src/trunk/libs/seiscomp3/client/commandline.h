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


#ifndef __SEISCOMP_CLIENT_COMMANDLINE_H__
#define __SEISCOMP_CLIENT_COMMANDLINE_H__

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/shared_ptr.hpp>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/client.h>

#include <map>

namespace Seiscomp {
namespace Client {


class SC_SYSTEM_CLIENT_API CommandLine {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		CommandLine();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void addGroup(const char*);

		void addOption(const char* group, const char* option,
		               const char* description);

		template <typename T>
		void addOption(const char* group, const char* option,
		               const char* description, T* storage,
		               bool storageAsDefault = true);

		template <typename T>
		void addOption(const char* group, const char* option,
		               const char* description, std::vector<T>* storage);

		template <typename T, typename DT>
		void addOption(const char* group, const char* option,
		               const char* description, T* storage,
		               const DT& defaultValue);

		template <typename T>
		void addCustomOption(const char* group, const char* option,
		                     const char* description, T* customValidator);

		bool parse(int argc, char** argv);

		void printOptions() const;

		/**
		 * Returns whether a command line option is set or not.
		 * This does not apply to mapped parameters from the
		 * configuration file.
		 */
		bool hasOption(const std::string& option) const;

		template <typename T>
		T option(const std::string& option) const;

		template <typename T, int LEN>
		T option(const char (&option)[LEN]) const;

		std::vector<std::string> unrecognizedOptions() const;


	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		typedef boost::program_options::options_description program_options;
		typedef boost::program_options::options_description_easy_init options_description_easy_init;
		typedef boost::program_options::options_description options_description;
		typedef boost::program_options::variables_map variables_map;

		options_description* findGroup(const char* group,
		                               const char* option = NULL) const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		typedef boost::shared_ptr<options_description> program_options_ptr;
		typedef std::vector<program_options_ptr> program_options_list;
		typedef std::map<std::string, program_options_ptr> program_options_map;

		program_options_ptr _options;
		program_options_list _groups;
		program_options_map _groupsMap;
		variables_map _variableMap;

		std::vector<std::string> _unrecognizedOptions;
};


#include <seiscomp3/client/commandline.ipp>

}
}

#endif
