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


#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <map>


using namespace Seiscomp;
using namespace std;


struct Logger : Config::Logger {
	void log(Config::LogLevel level, const char *filename, int line, const char *msg) {
		switch ( level ) {
			case Config::ERROR:
				cerr << filename << ":" << line << ": error: ";
				break;
			case Config::WARNING:
				cerr << filename << ":" << line << ": warning: ";
				break;
			default:
				return;
		}

		cerr << msg << endl;
	}
};


string toupper(const string &s) {
	string tmp;
	string::const_iterator it;
	for ( it = s.begin(); it != s.end(); ++it )
		tmp += ::toupper(*it);
	return tmp;
}


int main(int argc, char **argv) {
	bool standalone = false;

	if ( argc < 2 ) {
		cerr << "scchkcfg modulename [standalone]" << endl;
		return EXIT_FAILURE;
	}

	if ( argc > 2 ) {
		if ( strcmp(argv[2], "standalone") == 0 )
			standalone = true;
		else {
			cerr << "Unknown specifier: " << argv[2] << ": expected 'standalone'" << endl;
			return EXIT_FAILURE;
		}
	}

	Config::Config cfg;
	Logger logger;
	cfg.setLogger(&logger);
	if ( !Environment::Instance()->initConfig(
	          &cfg, argv[1], Environment::CS_FIRST,
	          Environment::CS_LAST, standalone) ) {
		cerr << "Failed to read configuration" << endl;
		return EXIT_FAILURE;
	}

	cout << "Read configuration files OK" << endl;

	typedef vector<Config::Symbol*> SymbolList;
	typedef map<string, SymbolList> SymbolConflicts;
	SymbolConflicts conflicts;

	Config::SymbolTable *symtab = cfg.symbolTable();
	Config::SymbolTable::iterator it;

	// First pass, prepare uppercase symbols
	for ( it = symtab->begin(); it != symtab->end(); ++it )
		conflicts[toupper((*it)->name)].push_back(*it);

	SymbolConflicts::iterator cit;
	int count = 0;

	for ( cit = conflicts.begin(); cit != conflicts.end(); ++cit ) {
		if ( cit->second.size() <= 1 ) continue;

		++count;
		cout << "Conflict #" << count << endl;

		SymbolList::iterator sit;
		for ( sit = cit->second.begin(); sit != cit->second.end(); ++sit ) {
			Config::Symbol *sym = *sit;
			cout << " " << sym->name << "    ";
			cout << sym->uri << ":" << sym->line << endl;
		}
	}

	if ( !count ) {
		cout << "No possible conflict detected" << endl;
		return EXIT_SUCCESS;
	}
	else {
		cout << count << " conflict" << (count > 1?"s":"") << " detected" << endl;
		return EXIT_FAILURE + 1;
	}
}
