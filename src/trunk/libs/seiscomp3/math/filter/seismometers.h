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


#ifndef _SEISCOMP_SEISMOMETERS_H_
#define _SEISCOMP_SEISMOMETERS_H_

#include <complex>

#include<seiscomp3/math/filter/biquad.h>


namespace Seiscomp {
namespace Math {

enum GroundMotion { Displacement, Velocity, Acceleration };

namespace SeismometerResponse {

typedef std::complex<double> Pole;
typedef std::complex<double> Zero;

typedef std::vector<Pole> Poles;
typedef std::vector<Zero> Zeros;

class PolesAndZeros {
	public:
		PolesAndZeros();

		PolesAndZeros(const Poles &poles, const Zeros &zeros,
		              double norm);

		PolesAndZeros(const PolesAndZeros &other);

	public:
		Poles  poles;
		Zeros  zeros;
		double norm;
};


class WoodAnderson : public PolesAndZeros {
	public:
		// Gutenberg(1935)              -> gain=2800, T0=0.8s, h=0.8
		// Uhrhammer and Collins (1990) -> gain=2080, T0=0.8s, h=0.7
		//
		// Note that use of the Uhrhammer and Collins (1990) version
		// is recommended by the IASPEI Magnitude Working Group
		// recommendations of 2011 September 9.
		struct Config {
			Config(double gain=2800, double T0=0.8, double h=0.8) :
				gain(gain), T0(T0), h(h) {}

			double gain, T0, h;
		};
		WoodAnderson(GroundMotion input, Config config=Config());
};


class Seismometer5sec : public PolesAndZeros {
public:
	Seismometer5sec(GroundMotion input);
};


}


namespace Filtering {


// Responses that soon need to be implemented are (at least)
// * STS2
// * STS1
// * Kirnos
// * corresponding restitution filters
// * generic filters that read PAZ from file/database/etc.

namespace IIR {

template <typename T>
class Filter : public SeismometerResponse::PolesAndZeros , public Math::Filtering::InPlaceFilter<T> {
	public:
		Filter();

		Filter(const SeismometerResponse::Poles &poles,
		       const SeismometerResponse::Zeros &zeros, double norm);

//		Filter(const Poles &poles, const Zeros &zeros,
//		       double norm, double fsamp);

		Filter(const Filter &other);

		void setSamplingFrequency(double fsamp);
		int setParameters(int n, const double *params);

		void apply(int n, T *inout);
	
		Math::Filtering::InPlaceFilter<T>* clone() const;

	private:
		Math::Filtering::IIR::BiquadCascade<T> _cascade;
};


template <typename T>
class WWSSN_SP_Filter : public Filter<T> {
	public:
		WWSSN_SP_Filter(GroundMotion input=Velocity);
		WWSSN_SP_Filter(const WWSSN_SP_Filter &other);

	public:
		int setParameters(int n, const double *params);
		Math::Filtering::InPlaceFilter<T>* clone() const;

		void setInput(GroundMotion input);
};


template <typename T>
class WWSSN_LP_Filter : public Filter<T> {
	public:
		WWSSN_LP_Filter(GroundMotion input=Velocity);
		WWSSN_LP_Filter(const WWSSN_LP_Filter &other);

	public:
		int setParameters(int n, const double *params);
		Math::Filtering::InPlaceFilter<T>* clone() const;

		void setInput(GroundMotion input);
};


template <typename T>
class WoodAndersonFilter : public Filter<T> {
	public:
		WoodAndersonFilter(GroundMotion input=Velocity);
		WoodAndersonFilter(const WoodAndersonFilter &other);

	public:
		int setParameters(int n, const double *params);
		Math::Filtering::InPlaceFilter<T>* clone() const;

		void setInput(GroundMotion input);
};


template <typename T>
class GenericSeismometer : public Filter<T> {
	public:
		GenericSeismometer(double cornerPeriod=1.,
		                   GroundMotion input=Velocity);
		GenericSeismometer(const GenericSeismometer &other);

	public:
		int setParameters(int n, const double *params);
		Math::Filtering::InPlaceFilter<T>* clone() const;

		void setInput(GroundMotion input);

	private:
		double _cornerPeriod;
};

template <typename T>
class Seismometer5secFilter : public Filter<T> {
	public:
		Seismometer5secFilter(GroundMotion input=Velocity);
		Seismometer5secFilter(const Seismometer5secFilter &other);

	public:
		int setParameters(int n, const double *params);
		Math::Filtering::InPlaceFilter<T>* clone() const;

		void setInput(GroundMotion input);
};


}

namespace FFT {
	// We will in future also have corresponding FFT-based filters
}



} // namespace Seiscomp::Math::Filtering
} // namespace Seiscomp::Math
} // namespace Seiscomp

#endif
