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


#ifndef __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_PGAV__
#define __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_PGAV__

#include <seiscomp3/processing/amplitudeprocessor.h>


namespace Seiscomp {
namespace Processing {


DEFINE_SMARTPOINTER(PGAV);
class PGAV : public TimeWindowProcessor {
	public:
		struct Config {
			// Converts a string to a frequency value. "fNyquist" suffix is
			// parsed and returns a negative value.
			// Returns
			// 0: no error
			// 1: invalid value (parser error)
			// 2: negative value
			static int freqFromString(double &val, const std::string &str);

			// Converts a frequency to a string.
			// Returns always true
			static bool freqToString(std::string &str, double val);

			double  preEventWindowLength;
			double  totalTimeWindowLength;

			bool    aftershockRemoval;
			bool    preEventCutOff;
			bool    useDeconvolution;

			int     SCorder;
			double  loSCFreq, hiSCFreq;

			int     filterOrder;
			double  loFilterFreq, hiFilterFreq;

			bool    noncausal;
			double  padLength;

			double  saturationThreshold;
			double  taperLength;

			double  STAlength;
			double  LTAlength;
			double  STALTAratio;
			double  STALTAmargin;

			std::vector<double> dampings;
			int     naturalPeriods;
			bool    naturalPeriodsLog;
			double  Tmin;
			double  Tmax;
			bool    fixedPeriods;
		};


	public:
		struct ResponseSpectrumItem {
			double period;
			double sd;
			double psa;
		};

		typedef std::vector<ResponseSpectrumItem> ResponseSpectrum;
		typedef std::pair<double, ResponseSpectrum> DampingResponseSpectrum;
		typedef std::list<DampingResponseSpectrum> ResponseSpectra;


	public:
		PGAV(const Seiscomp::Core::Time& trigger);
		~PGAV();

		const Config &config() const { return _config; }

		void setEventWindow(double preEventWindow, double totalEventWindow);
		void setSTALTAParameters(double sta, double lta, double ratio, double margin);

		void setResponseSpectrumParameters(double damping);
		void setResponseSpectrumParameters(const std::vector<double> &dampings);

		void setResponseSpectrumParameters(double damping, int nPeriods,
		                                   double Tmin, double Tmax,
		                                   bool logarithmicSpacing);
		void setResponseSpectrumParameters(const std::vector<double> &dampings,
		                                   int nPeriods,
		                                   double Tmin, double Tmax,
		                                   bool logarithmicSpacing);

		void setAftershockRemovalEnabled(bool);
		void setPreEventCutOffEnabled(bool);
		void setDeconvolutionEnabled(bool);

		// Set the sensititive correction filter parameters
		// If either fmin or fmax is negative, its absolute value taken
		// as a scale of the Nyquist frequency (fabs(f)*Nyquist)
		void setSensitivityCorrectionFilterParams(int order, double fmin, double fmax);

		// Set the filter parameters
		// If either fmin or fmax is negative, its absolute value taken
		// as a scale of the Nyquist frequency (fabs(f)*Nyquist)
		void setFilterParams(int order, double fmin, double fmax);

		void setNonCausalFiltering(bool, double taperLength);

		void setSaturationThreshold(double);
		void setPadLength(double);

		bool setup(const Settings &settings);

		// Should be called when waveform acquisition is completed
		// to use available data for processing
		void finish();

		void computeTimeWindow();


	public:
		const Core::Time &trigger() const { return _trigger; }

		bool processed() const { return _processed; }
		double maximumRawValue() const { return _maximumRawValue; }
		double PGA() const { return _pga; }
		double PGV() const { return _pgv; }
		const OPT(double) &duration() const { return _duration; }

		double loSCFilterUsed() const { return _loSCFilter; }
		double hiSCFilterUsed() const { return _hiSCFilter; }

		double loFilterUsed() const { return _loFilter; }
		double hiFilterUsed() const { return _hiFilter; }

		bool   isVelocity() const { return _velocity; }

		const ResponseSpectra &responseSpectra() const;


	protected:
		void process(const Record *record, const DoubleArray &filteredData);


	private:
		void init();


	private:
		Config      _config;

		Core::Time  _trigger;

		double      _maximumRawValue;
		OPT(double) _duration;
		double      _pga, _pgv;
		ResponseSpectra _responseSpectra;

		double      _loSCFilter;
		double      _hiSCFilter;

		double      _loFilter;
		double      _hiFilter;

		bool        _force;
		bool        _velocity;
		bool        _processed;


};



}
}


#endif
