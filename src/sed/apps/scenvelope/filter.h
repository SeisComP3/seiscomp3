/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
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


#ifndef __SEISCOMP_APPLICATIONS_SCENVELOPE_FILTER_H__
#define __SEISCOMP_APPLICATIONS_SCENVELOPE_FILTER_H__


#include <seiscomp3/math/filter.h>
#include "butterworth_c.h"


namespace Seiscomp {


class ButterworthFilter : public Math::Filtering::InPlaceFilter<double> {
	public:
		ButterworthFilter(int order, double cornerFreq);

		virtual void setSamplingFrequency(double fsamp);
		virtual int setParameters(int n, const double *params) { return 0; }
		virtual void apply(int n, double *inout);

		virtual Math::Filtering::InPlaceFilter<double>* clone() const {
			return new ButterworthFilter(_order, _freq);
		}

		void reset();


	private:
		void initHP(double cutoff);
		void applyHP(int n, double *inout);


	private:
		double  _dt;

		// these are used by the  C butterworth filter
		double  _d1[BUTTER_MAX_ORDER]; // recursive values
		double  _d2[BUTTER_MAX_ORDER]; // recursive values

		complex _poles[BUTTER_MAX_ORDER]; // poles for the filter

		double  _gain; // gain correction for the filter

		int     _order;
		double  _freq;
};


}


#endif
