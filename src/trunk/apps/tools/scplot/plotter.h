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



#ifndef __PLOTTER_H__
#define __PLOTTER_H__


#include <iostream>
#include <vector>
#include <string>


extern "C"
{
#include "gnuplot_i.h"
}




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Table
{
	friend std::ostream& operator<<(std::ostream& os, const Table& table);

public:
	Table();
	~Table();


public:
	void readFrom(std::istream& is);
	const std::string& element(int row, int col) const;
	std::vector<double> numberColumn(int c) const;
	std::vector<std::string> stringColumn(int c) const;

	size_t columns() const;
	size_t rows() const;

private:
	size_t                   _columns;
	size_t                   _rows;
	std::vector<int>         _widths;
	std::vector<std::string> _table;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream& operator<<(std::ostream& os, const Table& table);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class Plotter
{

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
public:
	enum PlottingStyle { LINES, POINTS, LINESPOINTS, IMPULSES, DOTS, STEPS };
	enum OutputDevice { PNG, POSTSCRIPT };


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
public:
	Plotter();
	~Plotter();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
public:
	void setXLabel(const std::string& xLabel);
	void setYLabel(const std::string& yLabel);

	void setPlottingStyle(PlottingStyle style);

	void setLowLevelCommand(const std::string& cmd);

	void setOutputDevice(OutputDevice device, const std::string& file);
	void setDefaultOutputDevice();

	void plot(const Table& table, int c0, int c1, const std::string& title = "");
	void plot(const Table& table, int c, const std::string& title = "");
	
	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
private:
	void plot(std::vector<double> x , std::vector<double> y, const std::string& title = "");
	void plot(std::vector<double> x, const std::string& title = "");

	
	// ----------------------------------------------------------------------
	// Private members
	// ----------------------------------------------------------------------
private:
	gnuplot_ctrl* _plotter;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



#endif
