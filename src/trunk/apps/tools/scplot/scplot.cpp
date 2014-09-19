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



#define SEISCOMP_COMPONENT scplot

#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>

#include "plotter.h"



int main(int argc, char* argv[])
{
	Plotter p;
	Table table;
	bool x11 = true;

	std::string xLabel;
	std::string yLabel;
	std::string title;
	std::string outFile;
	std::string inFile;
	std::string columns;
	int c0 = -1;
	int c1 = -1;

	boost::program_options::options_description description("Options");
	description.add_options()
	("help,h", "Help: Supported output format: ps, png, x11. File types are recognized automatically.")
	("columns,c", boost::program_options::value<std::string>(&columns), "Columns to be printed. Syntax: colX:colY")
	("title,t", boost::program_options::value<std::string>(&title), "Title of the diagram")
	("xlabel,x", boost::program_options::value<std::string>(&xLabel), "X axis description")
	("ylabel,y", boost::program_options::value<std::string>(&yLabel), "Y axis description")
	("outfile,o", boost::program_options::value<std::string>(&outFile), "Output file")
	("infile,i", boost::program_options::value<std::string>(&inFile), "Input file")
	("verbose,v", "Verbose program output");
	// ("verbose,v", boost::program_options::value<std::string>(&serverAddress)
	//	->default_value("localhost"), "spread server address");

	boost::program_options::variables_map variableMap;
	boost::program_options::store(
	    boost::program_options::parse_command_line(
	        argc, argv, description), variableMap);
	boost::program_options::notify(variableMap);

	if (variableMap.count("help"))
	{
		std::cout << description << std::endl;
		return EXIT_SUCCESS;
	}

	if (variableMap.count("verbose"))
	{
		Seiscomp::Logging::init(argc, argv);
		Seiscomp::Logging::enableConsoleLogging(
		    Seiscomp::Logging::getComponentAll("scplot"));
	}

	if (!variableMap.count("columns"))
	{
		SEISCOMP_ERROR("Specifying column0 (and additionally column1) is mandatory");
		std::cout << description << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		std::vector<std::string> tokens;
		int ret = Seiscomp::Core::split(tokens, columns.c_str(), ":");
		if (ret < 1 || ret > 2)
		{
			SEISCOMP_ERROR("Malformed columns string: %s", columns.c_str());
			std::cout << description << std::endl;
			return EXIT_FAILURE;
		}
		else if (ret == 1)
		{
			std::istringstream is(tokens[0]);
			is >> c0;
		}
		else if ( ret == 2 )
		{
			std::istringstream is(tokens[0]);
			is >> c0;
			is.clear();
			is.str(tokens[1]);
			is >> c1;
		}
	}

	if (variableMap.count("outfile"))
	{
		x11 = false;
		size_t idx = outFile.find_last_of(".");
		if (idx == std::string::npos || idx+1 == std::string::npos)
		{
			SEISCOMP_ERROR("Cannot determine file type from file name %s", outFile.c_str());
			std::cout << description << std::endl;
			return EXIT_FAILURE;
		}
		std::string type = outFile.substr(idx+1);
		if (type == "png")
			p.setOutputDevice(Plotter::PNG, outFile);
		else if (type == "ps")
			p.setOutputDevice(Plotter::POSTSCRIPT, outFile);
		else
		{
			SEISCOMP_ERROR("File type %s not supported!", type.c_str());
			std::cout << description << std::endl;
			return EXIT_FAILURE;
		}
	}

	if (variableMap.count("infile"))
	{
		std::ifstream ifile(inFile.c_str());
		if (!ifile)
		{
			SEISCOMP_ERROR("Could not open file %s", inFile.c_str());
			return EXIT_FAILURE;
		}
		table.readFrom(ifile);
	}
	else
		table.readFrom(std::cin);
	std::cout << table << std::endl;

	p.setXLabel(xLabel);
	p.setYLabel(yLabel);
	p.setPlottingStyle(Plotter::LINES);

	if (c0 > -1 && c1 > -1)
		p.plot(table, c0, c1, title);
	else
		p.plot(table, c0, title);
	if (x11)
		Seiscomp::Core::sleep(5);

	// 	Plotter plotter;
	// 	std::vector<double> d;
	// 	for (int i = 0; i < 8000; ++i)
	// 		d.push_back((double)(i));
	// 	plotter.plot(d, "testplot");
	// 	sleep(5);

	return EXIT_SUCCESS;
}
