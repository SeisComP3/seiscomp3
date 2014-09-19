/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#ifndef __SEISCOMP_APPLICATIONS_SCENVELOPE_APP_H__
#define __SEISCOMP_APPLICATIONS_SCENVELOPE_APP_H__


#include <seiscomp3/client/streamapplication.h>

#include <iostream>
#include <vector>
#include <list>


class Application : public Seiscomp::Client::StreamApplication {
	public:
		DEFINE_SMARTPOINTER(Option);
		struct Option : public Seiscomp::Core::BaseObject {
			Option(const char *cfgname,
				   const char *cligroup = NULL,
				   const char *cliparam = NULL,
				   const char *clidesc = NULL,
				   bool clidefault = false,
				   bool cliswitch = false)
			: cfgName(cfgname), cliGroup(cligroup),
			  cliParam(cliparam), cliDesc(clidesc),
			  cliDefault(clidefault), cliSwitch(cliswitch) {}

			virtual void bind(Seiscomp::Client::CommandLine *cli) = 0;
			virtual bool get(Seiscomp::Client::CommandLine *cli) = 0;
			virtual bool get(const Seiscomp::Client::Application *app) = 0;
			virtual void printStorage(std::ostream &os) = 0;

			const char *cfgName;
			const char *cliGroup;
			const char *cliParam;
			const char *cliDesc;
			bool cliDefault;
			bool cliSwitch;
		};

		typedef std::list<OptionPtr> Options;


	public:
		Application(int argc, char **argv);


	protected:
		void createCommandLineDescription();
		bool validateParameters();
		bool initConfiguration();

		void addOption(OptionPtr);

		void addOption(int *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(double *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(bool *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(std::string *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(std::vector<int> *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(std::vector<double> *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(std::vector<bool> *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		void addOption(std::vector<std::string> *var, const char *cfgname,
		               const char *cligroup = NULL, const char *cliparam = NULL,
		               const char *clidesc = NULL, bool clidefault = false,
		               bool cliswitch = false);

		const Options &options() const;


	private:
		Options _options;
};


#endif
