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


#define SEISCOMP_COMPONENT SCEventDump

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>

#include <iostream>
#include <fstream>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;


class Zipper : public Client::Application {
	public:
		Zipper(int argc, char** argv) : Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(false, false);
			setDaemonEnabled(false);
			setLoggingToStdErr(true);
		}

		void createCommandLineDescription() {
			commandline().addGroup("Mode");
			commandline().addOption("Mode", "decompress,d", "decompress data");
			commandline().addOption("Mode", "output,o", "output file (default is stdout)", &_outputFile, false);
		}

		bool run() {
			vector<string> files = commandline().unrecognizedOptions();
			if ( files.size() > 1 ) {
				cerr << "Too many input files given, only one file allowed" << endl;
				return false;
			}

			istream *input = NULL;
			ostream *output = NULL;
			bool deleteInputOnExit = false;
			bool deleteOutputOnExit = false;

			if ( files.size() == 1 ) {
				ifstream *file = new ifstream(files[0].c_str(), ios_base::in | ios_base::binary);
				deleteInputOnExit = true;
				if ( !file->is_open() ) {
					cerr << "Unable to open input file '" << files[0] << "'" << endl;
					delete file;
					return false;
				}
				input = file;
			}
			else {
				input = &cin;
				deleteInputOnExit = false;
			}

			if ( !_outputFile.empty() ) {
				ofstream *file = new ofstream(_outputFile.c_str(), ios_base::out | ios_base::binary);
				deleteOutputOnExit = true;
				if ( !file->is_open() ) {
					cerr << "Unable to create output file '" << _outputFile << "'" << endl;
					delete file;
					return false;
				}
				output = file;
			}
			else {
				output = &cout;
				deleteOutputOnExit = false;
			}

			if ( commandline().hasOption("decompress") ) {
				boost::iostreams::filtering_ostreambuf filtered_buf;
				filtered_buf.push(boost::iostreams::zlib_decompressor());
				filtered_buf.push(*output->rdbuf());
				*input >> &filtered_buf;
			}
			else {
				boost::iostreams::filtering_ostreambuf filtered_buf;
				filtered_buf.push(boost::iostreams::zlib_compressor());
				filtered_buf.push(*output->rdbuf());
				*input >> &filtered_buf;
			}

			if ( deleteInputOnExit ) delete input;
			if ( deleteOutputOnExit ) delete output;

			return true;
		}

	private:
		string _outputFile;
};


int main(int argc, char **argv) {
	Zipper app(argc, argv);
	return app.exec();
}

