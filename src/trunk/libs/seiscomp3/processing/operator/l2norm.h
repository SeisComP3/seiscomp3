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


#ifndef __SEISCOMP_PROCESSING_OPERATOR_L2NORM_H__
#define __SEISCOMP_PROCESSING_OPERATOR_L2NORM_H__


namespace Seiscomp {
namespace Processing {
namespace Operator {


template <typename T, int N>
class L2Norm {
	L2Norm();

	// Process N traces in place of length n
	void operator()(const Record *, T *data[N], int n, double sfreq) const;

	// publishs a processed component
	bool publish(int c) const;
};


template <typename T>
struct L2Norm<T,2> {
	bool publish(int c) const { return c == 0; }

	void operator()(const Record *, T *data[2], int n, const Core::Time &stime, double sfreq) const {
		for ( int i = 0; i < n; ++i )
			data[0][i] = sqrt(data[0][i] * data[0][i] +
			                  data[1][i] * data[1][i]);
	}
};


template <typename T>
struct L2Norm<T,3> {
	bool publish(int c) const { return c == 0; }

	void operator()(const Record *, T *data[3], int n, const Core::Time &stime, double sfreq) const {
		for ( int i = 0; i < n; ++i )
			data[0][i] = sqrt(data[0][i] * data[0][i] +
			                  data[1][i] * data[1][i]);
	}
};


}
}
}


#endif
