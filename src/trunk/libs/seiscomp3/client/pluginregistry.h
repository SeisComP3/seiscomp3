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


#ifndef __SEISCOMP_CLIENT_PLUGINREGISTRY_H__
#define __SEISCOMP_CLIENT_PLUGINREGISTRY_H__

#include <list>
#include <string>

#include <seiscomp3/core/plugin.h>
#include <seiscomp3/client.h>


namespace Seiscomp {

namespace Config {
	class Config;
}

namespace Client {

class SC_SYSTEM_CLIENT_API PluginRegistry {	
	// ----------------------------------------------------------------------
	// Public types
	// ----------------------------------------------------------------------
	public:
		struct PluginEntry {
			PluginEntry(void *h, Core::PluginPtr p, const std::string &fn = "")
			 : handle(h), plugin(p), filename(fn) {}
			void *handle;
			Core::PluginPtr plugin;
			std::string filename;
		};

		class SC_SYSTEM_CLIENT_API iterator : public std::list<PluginEntry>::const_iterator {
			private:
				typedef std::list<PluginEntry>::const_iterator base;

			public:
				iterator();
				iterator(const base&);

			public:
				const Core::Plugin* operator*() const;
				Core::Plugin* value_type(const iterator&);
		};


	// ----------------------------------------------------------------------
	// X'truction
	// ----------------------------------------------------------------------
	private:
		PluginRegistry();

	public:
		~PluginRegistry();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the global instance
		static PluginRegistry *Instance();

		//! Adds a plugin to be loaded
		void addPluginName(const std::string &name);

		//! Adds a plugin search path
		void addPluginPath(const std::string &path);

		//! Adds a plugin package search path pointing to
		//! shareDir + "/plugins/" + package
		void addPackagePath(const std::string &package);

		/**
		 * Loads all plugins in the defined search paths
		 * added with addPluginName
		 * @return The number of loaded plugins
		 */
		int loadPlugins();

		/**
		 * Loads all plugins in the defined search paths
		 * configured by the config object. All names added
		 * with addPluginName will be replaced by the configured
		 * plugin names in "core.plugins" if there are any.
		 * Otherwise the default plugin list will be extended by
		 * "plugins".
		 * @return The number of loaded plugins
		 */
		int loadConfiguredPlugins(const Config::Config *config);


		//! Unloads all plugins
		void freePlugins();

		//! Returns the number of registered plugins
		int pluginCount() const;

		//! Returns the start iterator over all registered plugins
		iterator begin() const;
		//! Returns the end iterator over all registered plugins
		iterator end() const;


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		std::string find(const std::string &name) const;
		PluginEntry open(const std::string &file) const;
		bool findLibrary(void *handle) const;


	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
	private:
		typedef std::list<PluginEntry> PluginList;
		typedef std::vector<std::string> PathList;
		typedef std::vector<std::string> NameList;

		PluginList _plugins;
		PathList   _paths;
		NameList   _pluginNames;

		static PluginRegistry *_instance;
};


} // namespace Communication
} // namespace Seiscomp

#endif
