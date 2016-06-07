/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_PLOT_RANGE_H__
#define __SEISCOMP_GUI_PLOT_RANGE_H__


#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


struct SC_GUI_API Range {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	//! Constructs an invalid range
	Range();

	//! Constructs a range with lower and upper bounds which will be normalized
	Range(double lower, double upper);


	// ----------------------------------------------------------------------
	//  Pulic interface
	// ----------------------------------------------------------------------
	double length() const;
	double center() const;
	bool contains(double value) const;
	void normalize();
	void extend(const Range &other);
	void translate(double delta);
	bool isValid() const;


	// ----------------------------------------------------------------------
	//  Attributes
	// ----------------------------------------------------------------------
	double lower;
	double upper;
};




inline Range::Range() : lower(1), upper(-1) {}
inline Range::Range(double lower, double upper) : lower(lower), upper(upper) {
	normalize();
}

inline double Range::length() const { return upper - lower; }

inline double Range::center() const { return (lower-upper)*0.5; }

inline bool Range::contains(double value) const {
	return (value >= lower) && (value <= upper);
}

inline bool Range::isValid() const { return lower <= upper; }

inline void Range::translate(double delta) { lower += delta; upper += delta; }


}
}


#endif
