/***************************************************************************
 *   Copyright (C) Preparatory Commission for the Comprehensive            *
 *   Nuclear-Test-Ban Treaty Organization (CTBTO).                         *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_A52_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_A52_H


#include <seiscomp3/processing/amplitudeprocessor.h>


namespace {


class AmplitudeA5_2 : public Seiscomp::Processing::AmplitudeProcessor {
	public:
		AmplitudeA5_2();

	protected:
		bool computeAmplitude(const Seiscomp::DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr);

	protected:
		const double _tiLead;
		const double _tiLag;
		double (*_stavFunction)(double x);
		const double _stavLength;
		double (*_ltavFunction)(double x);
		const double _ltavLength;
		const double _noiseSignalGap;
		const bool _demean;
		const double _taper;
		const double _filbuf;
		const int _ford;
		const double _flo;
		const double _fhi;
		const bool _zp;
		const bool _coherent;
		const bool _removeFiltResp;
		const bool _removeInstResp;
		const double _filtRolloff;
		const bool _interpolation;
		const double _interpolationPeriodSidePeakThreshold;
		const size_t _interpolationPeriodMinHalfPeriods;
		const double _interpolationPeriodMaxHalfPeriodRatio;
		const double _interpolationPeriodMaxNyquistPercentage;
		const double _interpolationPeriodWindowLengthPercentage;
		const double _interpolationPeriodMaxHiCutPercentage;
		const double _interpolationPeriodMinLoCutPercentage;
		const double _interpolationPeriodMaxFilterCorrection;
		const int _interpolationMaxFilterOrder;

	private:
		enum DataPointType {
			NEITHER,
			TROUGH,
			PEAK
		};

	private:
		bool interpolate(const double *data,
		                 const DataPointType *dataPointTypes,
		                 const size_t numPoints, const size_t maxLeft,
		                 const size_t maxRight,
		                 double *interpolatedAmplitude,
		                 double *interpolatedPeriod);

	private:
		double _preTriggerDataBufferLength;
		double _postTriggerDataBufferLength;
};


}


#endif
