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



#define SEISCOMP_COMPONENT scquery

#include <iostream>
#include <memory>
#include <algorithm>

#include <seiscomp3/client/application.h>

#include "dbquery.h"
#include "dbconnection.h"



using namespace Seiscomp;
using namespace boost;


void showQueries(const Config::Config& conf)
{
	std::vector<std::string> sqlQueries;

	try {
		sqlQueries = conf.getStrings("queries");
	}
	catch ( Config::Exception & ) {
		std::cerr << "No query found" << std::endl;
		return;
	}

	std::cout << "[ " << sqlQueries.size() << " queries found ]\n"  << std::endl;
	for ( size_t i = 0; i < sqlQueries.size(); ++i )
	{
		std::string desc, query;

		try { desc = conf.getString("query." + sqlQueries[i] + ".description"); } catch ( ... ) {}
		try { query = conf.getString("query." + sqlQueries[i]); } catch ( ... ) {}

		DBQuery q(sqlQueries[i], desc, query);
		std::cout << "Query name: " << q.name() << std::endl;
		std::cout << "Description: " << q.description() << std::endl;
		if (q.hasParameter())
		{
			std::cout << "number of parameter: " << q.parameter().size() << std::endl;
			std::cout << "Parameter: ";
			for (std::vector<std::string>::const_iterator it = q.parameter().begin();
			        it < q.parameter().end(); ++it)
				std::cout << *it << " ";
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
}




DBQuery* findQuery(const Config::Config& conf, const std::string& name)
{
	DBQuery* q = NULL;

	std::vector<std::string> sqlQueries;
	try
	{
		sqlQueries = conf.getStrings("queries");
	}
	catch (const Config::Exception& e)
	{
		std::cout << e.what() << std::endl;
		return q;
	}

	for (size_t i = 0; i < sqlQueries.size(); ++i)
	{
		std::string desc, query;

		if (name == sqlQueries[i])
		{
			try { desc = conf.getString("query." + sqlQueries[i] + ".description"); } catch ( ... ) {}
			try { query = conf.getString("query." + sqlQueries[i]); } catch ( ... ) {}

			q = new DBQuery(sqlQueries[i], desc, query);
			break;
		}
	}
	return q;
}


class AppQuery : public Client::Application {
	public:
		AppQuery(int argc, char** argv) :
			Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(true, false);
		}

		void createCommandLineDescription() {
			commandline().addGroup("Commands");
			commandline().addOption("Commands", "query,Q", "Execute the given query from the commandline.", &_query);
			commandline().addOption("Commands", "showqueries", "Show the stored queries in queries.cfg");
		}

		virtual void printUsage() {
			std::cout << "Basic usage: scquery <queryname> parameter0 parameter1 ...\n"
			             "The predefined queries are stored in queries.cfg and can be listed via --showqueries\n"
			          << std::endl;

			Client::Application::printUsage();
		}

		bool run() {
			Config::Config queriesConf;
			if ( !queriesConf.readConfig(Environment::Instance()->configDir() + "/queries.cfg") )
				return false;

			if ( commandline().hasOption("showqueries") ) {
				showQueries(queriesConf);
				return true;
			}

			std::vector<std::string> qParameter = commandline().unrecognizedOptions();

			if (!qParameter.empty())
			{
				std::auto_ptr<DBQuery> q(findQuery(queriesConf, qParameter[0]));
				if (q.get())
				{
					std::vector<std::string> params;
					std::vector<std::string>::iterator it = qParameter.begin();
					std::copy(++it, qParameter.end(), std::back_inserter(params));

					if (!q->setParameter(params))
					{
						std::cerr << "The amount of parameter is not corresponding with given query!" << std::endl;
						std::cerr << "Given arguments: ";
						for (size_t i = 0; i < params.size(); ++i)
							std::cout << params[i] << " ";
						std::cout << std::endl;

						std::cerr << "Query parameter: ";
						for (size_t i = 0; i < q->parameter().size(); ++i)
							std::cerr << q->parameter()[i] << " ";
						std::cerr << std::endl;

						std::cerr << "Query: " << q->query() << std::endl;
						return false;
					}

					DBConnection dbConnection(database());
					//std::cerr << *q << std::endl;
					if (!dbConnection.executeQuery(*q))
						std::cerr << "Could not execute query: " << q->query() << std::endl;
					std::cout << dbConnection.table() << std::endl;
				}
				else
				{
					std::cout << "Could not execute query: " << qParameter[0] << std::endl;
				}
			}
			else if ( !_query.empty() ) {
				DBQuery q("default", "default", _query);
				DBConnection dbConnection(database());
				if (!dbConnection.executeQuery(q))
					std::cerr << "Could not execute query: " << _query << std::endl;
				std::cout << dbConnection.table() << std::endl;
			}

			return true;
		}

	private:
		std::string _query;
};



int main(int argc, char* argv[])
{
	AppQuery app(argc, argv);
	return app.exec();
}
