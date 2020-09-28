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


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_SBSNR_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_SBSNR_H


#include <seiscomp3/processing/amplitudeprocessor.h>


namespace {


class AmplitudeSBSNR : public Seiscomp::Processing::AmplitudeProcessor {
	public:
		AmplitudeSBSNR();

	protected:
		bool computeAmplitude(const Seiscomp::DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt,
		                      AmplitudeValue *amplitude,
		                      double *period, double *snr);

	protected:
		const bool _demean;
		const double _filbuf;
		const double _taper;
		const int _ford;
		const double _flo;
		const double _fhi;
		const bool _zp;
		const bool _coherent;
		const double _stavLength;
		const double _stavFraction;
		double (*_stavFunction)(double x);
		const double _maxStavWindowLength;
		const double _ltavLength;
		const double _ltavFraction;
		double (*_ltavFunction)(double x);
		const double _ltavStabilityLength;

	private:
		double _preTriggerDataBufferLength;
		double _postTriggerDataBufferLength;
};


}


#endif
