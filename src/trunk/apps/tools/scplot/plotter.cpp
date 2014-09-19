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
#include "plotter.h"

#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <locale>

#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>

#ifdef WIN32
#undef min
#undef max
#endif


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Table::Table() : _columns(0), _rows(0)
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Table::~Table()
{}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream& operator<<(std::ostream& os, const Table& table)
{
	for (size_t i = 0; i < table._table.size(); ++i)
	{
		os << std::setw(table._widths[i % table.columns()]) << std::left << table._table[i];
		if ((i+1) % table._columns == 0)
			os << std::endl;
		else
			os << " | ";
	}
	os.unsetf(os.flags());

	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Table::readFrom(std::istream& is)
{
	std::string line;
	while (getline(is, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		std::vector<std::string> tokens;
		int ret = Seiscomp::Core::split(tokens, line.c_str(), "|");
		std::vector<int> widths(ret, 0);
		
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			// Remove whitespace from the beginning
			size_t idx = tokens[i].find_first_not_of(" ");
			if (idx != std::string::npos)
				tokens[i] = tokens[i].erase(0, idx);

			// Remove whitespace from the beginning
			idx = tokens[i].find_first_of(" ");
			if (idx != std::string::npos)
				tokens[i] = tokens[i].erase(idx, tokens[i].size());

			_table.push_back(tokens[i]);
			widths[i] = tokens[i].size();
			++_columns;
		}
		++_rows;

		if (_widths.size() == 0)
			_widths = widths;
		else
			for (size_t i = 0; i < _widths.size(); ++i)
				_widths[i] = std::max(_widths[i], widths[i]);

	}
	_columns /= _rows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Table::element(int row, int col) const
{
	return _table[col + row * _columns];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<double> Table::numberColumn(int c) const
{
	// Test if elements are numbers
	//bool isNumber = false;
	std::vector<double> column;
	const std::num_get<char, std::istreambuf_iterator<char, std::char_traits<char> > > &ng =
		std::use_facet<std::num_get<char, std::istreambuf_iterator<char, std::char_traits<char> > > >(std::locale ());

	for (size_t r = 0; r < rows(); ++r)
	{
		std::istringstream is(element(r, c));
		if (is.str() == "NULL")
			continue;
		
		double number = 0;
		std::istreambuf_iterator<char, std::char_traits<char> > begin (is);
		std::istreambuf_iterator<char, std::char_traits<char> > end;
		std::ios::iostate state = std::ios::goodbit;
		std::istreambuf_iterator<char, std::char_traits<char> > pos = ng.get(begin, end, is, state, number);
		if ((state & std::istringstream::failbit) != 0 || pos != end)
			return std::vector<double>();
		else
			column.push_back(number);
	}

	return column;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::vector<std::string> Table::stringColumn(int c) const
{
	std::vector<std::string> column;
	for (size_t i = 0; i < rows(); ++i)
		column.push_back(element(i, c));

	return column;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Table::columns() const
{
	return _columns;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t Table::rows() const
{
	return _rows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Plotter::Plotter() : _plotter(NULL)
{
	_plotter = gnuplot_init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Plotter::~Plotter()
{
	if (_plotter)
		gnuplot_close(_plotter);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setXLabel(const std::string& xLabel)
{
	if (_plotter)
		gnuplot_set_xlabel(_plotter, const_cast<char*>(xLabel.c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setYLabel(const std::string& yLabel)
{
	if (_plotter)
		gnuplot_set_ylabel(_plotter, const_cast<char*>(yLabel.c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setPlottingStyle(PlottingStyle style)
{
	const char* styleString = NULL;
	switch (style)
	{
	case LINES:
		styleString = "lines";
		break;

	case POINTS:
		styleString = "points";
		break;

	case LINESPOINTS:
		styleString = "linespoints";
		break;

	case IMPULSES:
		styleString = "impulses";
		break;

	case DOTS:
		styleString = "dots";
		break;

	case STEPS:
		styleString = "steps";
		break;

	default:
		break;
	}

	if (_plotter && styleString)
		gnuplot_setstyle(_plotter, styleString);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setLowLevelCommand(const std::string& cmd)
{
	if (_plotter)
	{
		std::istringstream is(cmd);
		std::string word;
		while (is >> word)
			my_gnuplot_cmd(_plotter, "%s ", word.c_str());
		gnuplot_cmd(_plotter, "");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setOutputDevice(OutputDevice device, const std::string& file)
{
	if (!_plotter)
		return;

	bool validDevice = true;
	switch (device)
	{
	case PNG:
		gnuplot_cmd(_plotter, "set terminal png");
		break;
	case POSTSCRIPT:
		gnuplot_cmd(_plotter, "set terminal postscript");
		break;
	default:
		validDevice = false;
		break;
	}

	if (validDevice)
		gnuplot_cmd(_plotter, const_cast<char*>(std::string("set output "
		                                        + std::string("\"")
		                                        + file
		                                        + std::string("\"")).c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::setDefaultOutputDevice()
{
	gnuplot_cmd(_plotter, "set terminal x11") ;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::plot(std::vector<double> x,
                   std::vector<double> y,
                   const std::string& title)
{
	int size = x.size();
	double* xArray = new double[size];
	double* yArray = new double[size];
	std::copy(x.begin(), x.end(), xArray);
	std::copy(y.begin(), y.end(), yArray);

	if (_plotter)
		gnuplot_plot_xy(_plotter, xArray, yArray, size, const_cast<char*>(title.c_str()));

	delete [] xArray;
	delete [] yArray;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::plot(std::vector<double> fx, const std::string& title)
{
	int size = fx.size();
	double* fxArray = new double[size];
	std::copy(fx.begin(), fx.end(), fxArray);

	if (_plotter)
		gnuplot_plot_x(_plotter, fxArray, size, const_cast<char*>(title.c_str()));

	delete [] fxArray;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::plot(const Table& table, int cx, int cy, const std::string& title)
{
	std::vector<double> cxDoubleVec = table.numberColumn(cx);
	if (!cxDoubleVec.empty())
	{
		plot(cxDoubleVec, table.numberColumn(cy), title);
	}
	else
	{
		setLowLevelCommand("set xtics rotate 90");
		std::vector<std::string> cxStrVec = table.stringColumn(cx);
		std::ostringstream os;
		os << "set xtics ( ";
		for (size_t i = 0; i < cxStrVec.size(); ++i)
			os << "\"" << cxStrVec[i] << "\" " << i << ((i < cxStrVec.size() - 1) ? ", " : " )") ;

		setLowLevelCommand(os.str());
		plot(table.numberColumn(cy), title);
		setLowLevelCommand("unset xtics");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Plotter::plot(const Table& table, int c, const std::string& title)
{
	std::vector<double> doubleVec = table.numberColumn(c);
	if (!doubleVec.empty())
	{
		plot(doubleVec, title);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
