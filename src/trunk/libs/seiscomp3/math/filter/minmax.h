/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker, jabe@gempa.de                                     *
 ***************************************************************************/


#ifndef _SEISCOMP_MATH_FILTER_MINMAX_H_
#define _SEISCOMP_MATH_FILTER_MINMAX_H_


#include<vector>
#include<seiscomp3/math/filter.h>


namespace Seiscomp {
namespace Math {
namespace Filtering {


/**
 * @brief The MinMax class is the base class for either the Min or Max
 *        filter. Each output sample holds the minimum/maximum of all prior
 *        samples within the configured time window.
 */
template<typename TYPE>
class MinMax : public InPlaceFilter<TYPE> {
	public:
		MinMax(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);


	public:
		/**
		 * @brief Sets the length of the minmax time window.
		 * @param timeSpan Length in seconds
		 */
		void setLength(double timeSpan);

		virtual void setSamplingFrequency(double fsamp);
		virtual int setParameters(int n, const double *params);

		// Resets the filter values
		void reset();


	protected:
		double            _timeSpan;
		double            _fsamp;
		int               _sampleCount;
		int               _index;
		bool              _firstSample;
		std::vector<TYPE> _buffer;
};


template<typename TYPE>
class Min : public MinMax<TYPE> {
	public:
		Min(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		// apply filter to data vector **in*place**
		virtual void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;

	private:
		TYPE _minimum;
};


template<typename TYPE>
class Max : public MinMax<TYPE> {
	public:
		Max(double timeSpan /*sec*/ = 1.0, double fsamp = 0.0);

	public:
		// apply filter to data vector **in*place**
		virtual void apply(int n, TYPE *inout);
		virtual InPlaceFilter<TYPE>* clone() const;

	private:
		TYPE _maximum;
};



} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif

