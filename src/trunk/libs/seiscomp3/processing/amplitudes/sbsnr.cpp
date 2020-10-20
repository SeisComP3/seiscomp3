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


#define AMP_TYPE "SBSNR"
#define SEISCOMP_COMPONENT Amplitudes/SBSNR  // For logging

#include <cmath>  // fabs, sqrt

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/math/windows/cosine.h>

#include "idc_utils.h"

#include "sbsnr_private.h"

namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




using namespace Seiscomp::Processing::Utils::IDC;




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeSBSNR::AmplitudeSBSNR()
: Seiscomp::Processing::AmplitudeProcessor(AMP_TYPE)
, _demean(false)  // Demean? False for SBSNR
, _filbuf(10.0)  // Filter onset length (filter buffer) in seconds
, _taper(0.5)  // Taper fraction of filter buffer
, _ford(3)  // Filter order (number of poles)
, _flo(2.0)  // Filter low band cut-off frequency in cycles/second
, _fhi(4.0)  // Filter high band cut-off frequency in cycles/second
, _zp(false)  // Zero phase filter? False for SBSNR
, _coherent(false)  // False for SBSNR
, _stavLength(1.0)  // Short-term average window length in seconds
, _stavFraction(0.95)  // Fraction of _stavLength for stav threshold
, _stavFunction(fabs)  // Function to apply to running average
, _maxStavWindowLength(4.0)  // Window length in seconds for finding maximum absolute short-term average
, _ltavLength(60.0)  // Long-term average window length in seconds
, _ltavFraction(0.9)  // Fraction of _ltavLength for ltav threshold
, _ltavFunction(samex)  // Function to apply to recursive average
, _preLtavStabilityLength(180.0)  // Pre long-term average stability length in seconds
, _postLtavStabilityLength(0.50)  // Post long-term average stability length in seconds
{
	// Set signal and noise windows to cover entire data window required to
	//   ensure data completeness, actual/real signal and noise windows are
	//   determined in computeAmplitude, see {signal,noise}Start and
	//   {signal,noise}End
	_preTriggerDataBufferLength =
	    _ltavLength + _filbuf + _preLtavStabilityLength;
	SEISCOMP_DEBUG("_preTriggerDataBufferLength = %f",
	               _preTriggerDataBufferLength);
	_postTriggerDataBufferLength =
	    _maxStavWindowLength + _postLtavStabilityLength;
	if ( _zp ) {
		_postTriggerDataBufferLength += _filbuf;
	}
	SEISCOMP_DEBUG("_postTriggerDataBufferLength = %f",
	               _postTriggerDataBufferLength);
	setMargin(Seiscomp::Core::TimeSpan(0.0));
	setSignalStart(0.0 - _preTriggerDataBufferLength);
	setSignalEnd(0.0 + _postTriggerDataBufferLength);
	setNoiseStart(0.0 - _preTriggerDataBufferLength);
	setNoiseEnd(0.0 + _postTriggerDataBufferLength);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeSBSNR::computeAmplitude(const Seiscomp::DoubleArray &data,
                                      size_t i1, size_t i2,
                                      size_t si1, size_t si2,
                                      double offset,
                                      AmplitudeIndex *dt,
                                      AmplitudeValue *amplitude,
                                      double *period, double *snr) {
	SEISCOMP_DEBUG("Entering Amplitude SBSNR computeAmplitude");

	// Determine useful constants
	const size_t dataStart = i1;
	SEISCOMP_DEBUG("dataStart = %zu", dataStart);
	const double samplingRate = samplingFrequency();
	SEISCOMP_DEBUG("samplingRate = %f", samplingRate);
	const size_t signalCount = (size_t)(_maxStavWindowLength * samplingRate);
	SEISCOMP_DEBUG("signalCount = %zu", signalCount);
	const size_t signalStart =
	    dataStart + ((size_t)_preTriggerDataBufferLength * samplingRate);
	SEISCOMP_DEBUG("signalStart = %zu", signalStart);
	const size_t signalEnd = signalStart + signalCount;  // exclusive bound
	SEISCOMP_DEBUG("signalEnd = %zu", signalEnd);
	const size_t noiseCount = (size_t)(_ltavLength * samplingRate);
	SEISCOMP_DEBUG("noiseCount = %zu", noiseCount);
	const size_t noiseStart = signalStart - noiseCount;
	SEISCOMP_DEBUG("noiseStart = %zu", noiseStart);
	const size_t noiseEnd = noiseStart + noiseCount;  // exclusive bound
	SEISCOMP_DEBUG("noiseEnd = %zu", noiseEnd);
	const size_t preLtavStabilityCount =
	    (size_t)(_preLtavStabilityLength * samplingRate);
	SEISCOMP_DEBUG("preLtavStabilityCount = %zu", preLtavStabilityCount);
	const size_t postLtavStabilityCount =
	    (size_t)(_postLtavStabilityLength * samplingRate);
	SEISCOMP_DEBUG("postLtavStabilityCount = %zu", postLtavStabilityCount);
	const size_t filbufCount = (size_t)(_filbuf * samplingRate);
	SEISCOMP_DEBUG("filbufCount = %zu", filbufCount);
	const size_t dataSBSNRCount =
	    (signalEnd - noiseStart)
	    + preLtavStabilityCount + postLtavStabilityCount
	    + ((_zp ? 2 : 1) * filbufCount);
	SEISCOMP_DEBUG("dataSBSNRCount = %zu", dataSBSNRCount);
	const size_t dataSBSNRStart =
	    noiseStart - preLtavStabilityCount - filbufCount;
	SEISCOMP_DEBUG("dataSBSNRStart = %zu", dataSBSNRStart);
	const size_t dataSBSNREnd = dataSBSNRStart + dataSBSNRCount;  // exclusive bound
	SEISCOMP_DEBUG("dataSBSNREnd = %zu", dataSBSNREnd);

	// Check for out-of-bounds indices which indicates missing data
	if ( dataSBSNRStart < 0 || dataSBSNRStart > (size_t)data.size()
	     || dataSBSNREnd > (size_t)data.size() ) {
		SEISCOMP_ERROR("Requested time window not fulfilled");
		return false;
	}

	// Make work copy of data which can be changed
	SEISCOMP_DEBUG("data.size() = %zu", (size_t)data.size());
	Seiscomp::DoubleArray dataSBSNR(data);

	// Demean data
	if ( _demean ) {
		double mean = Seiscomp::Math::Statistics::mean(dataSBSNRCount,
		                                               dataSBSNR.typedData()
		                                               + dataSBSNRStart);
		SEISCOMP_DEBUG("dataSBSNR mean = %f", mean);

		for ( size_t i = dataSBSNRStart; i < dataSBSNREnd; ++i ) {
			dataSBSNR[i] -= mean;
		}
	}

	// Taper data
	const double taperSBSNRWidth = _taper * filbufCount / dataSBSNRCount;
	Seiscomp::Math::CosineWindow<double> cosineTaper;

	if ( _zp ) {
		// Taper both ends
		cosineTaper.apply(dataSBSNRCount,
		                  dataSBSNR.typedData() + dataSBSNRStart,
		                  taperSBSNRWidth, taperSBSNRWidth);
	}
	else {
		// Taper left only
		cosineTaper.apply(dataSBSNRCount,
		                  dataSBSNR.typedData() + dataSBSNRStart,
		                  taperSBSNRWidth, 0.0);
	}

	// Initialize butterworth bandpass filter
	Seiscomp::Math::Filtering::IIR::ButterworthBandpass<double> filter(_ford,
	                                                                   _flo,
	                                                                   _fhi);
	try {
		filter.setSamplingFrequency(samplingRate);
	}
	catch ( std::exception &e ) {
		SEISCOMP_WARNING("%s: init SBSNR filter: %s",
		                 _stream.lastRecord->streamID().c_str(),
		                 e.what());
		return false;
	}

	// Forward filter data
	filter.apply(dataSBSNRCount, dataSBSNR.typedData() + dataSBSNRStart);

	// Zero phase -> backward filter data
	if ( _zp ) {
		filter.reset();

		double *dataPointer = dataSBSNR.typedData() + dataSBSNRStart;

		for ( size_t i = dataSBSNRCount; i > 0; --i ) {  // i is unsigned
			filter.apply(1, dataPointer + i - 1);
		}
	}

	// Rectify if incoherent
	if ( !_coherent) {
		double *dataPointer = dataSBSNR.typedData() + dataSBSNRStart;

		for ( size_t i = 0; i < dataSBSNRCount; ++i ) {
			dataPointer[i] = fabs(dataPointer[i]);
		}
	}

	// Compute stav running average and stav
	const size_t stavCount = dataSBSNRCount;
	SEISCOMP_DEBUG("stavCount = %zu", stavCount);
	const double *stavData = dataSBSNR.typedData() + dataSBSNRStart;
	size_t stavState[stavCount];
	const size_t stavAverageWindowLength = (size_t)(_stavLength * samplingRate);
	SEISCOMP_DEBUG("stavAverageWindowLength = %zu", stavAverageWindowLength);
	const size_t stavThreshold =
	    (size_t)(stavAverageWindowLength * _stavFraction);
	SEISCOMP_DEBUG("stavThreshold = %zu", stavThreshold);

	double stavRunningAverage[stavCount];
	size_t stavRunningAverageState[stavCount];

	for ( size_t i = 0; i < stavCount; ++i ) {
		stavState[i] = 1;
	}

	if ( !runningAverage(stavData, stavState, stavCount,
	                     stavAverageWindowLength, stavThreshold, _stavFunction,
	                     stavRunningAverage, stavRunningAverageState) ) {
		SEISCOMP_ERROR("Error computing stav running average");
		return false;
	}

	// Find maximum absolute short-term running average in the signal window
	const size_t stavSignalStart = signalStart - dataSBSNRStart;
	SEISCOMP_DEBUG("stavSignalStart = %zu", stavSignalStart);
	double maxAbsStav = fabs(stavRunningAverage[stavSignalStart]);
	for ( size_t i = 1; i < signalCount; ++i ) {
		double absStav = fabs(stavRunningAverage[stavSignalStart + i]);
		if ( absStav > maxAbsStav) {
			maxAbsStav = absStav;
			SEISCOMP_DEBUG("New maxAbsStav = %f at i = %zu", maxAbsStav, i);
		}
	}

	const double stav = maxAbsStav;
	SEISCOMP_DEBUG("stav = %f", stav);

	// Compute ltav recursive average and ltav
	const size_t ltavCount = stavCount;
	SEISCOMP_DEBUG("ltavCount = %zu", ltavCount);
	const double *ltavData = stavRunningAverage;
	const size_t *ltavState = stavRunningAverageState;
	const size_t ltavRecursionLookbackLength = stavAverageWindowLength;
	SEISCOMP_DEBUG("ltavRecursionLookbackLength = %zu",
	               ltavRecursionLookbackLength);
	const size_t ltavAverageWindowLength = (size_t)(_ltavLength * samplingRate);
	SEISCOMP_DEBUG("ltavAverageWindowLength = %zu", ltavAverageWindowLength);
	const size_t ltavThreshold =
	    (size_t)(ltavAverageWindowLength * _ltavFraction);
	SEISCOMP_DEBUG("ltavThreshold = %zu", ltavThreshold);

	double ltavRecursiveAverage[ltavCount];
	size_t ltavRecursiveAverageState[ltavCount];

	if ( !recursiveAverage(ltavData, ltavState, ltavCount,
		                   ltavRecursionLookbackLength,
	                       ltavAverageWindowLength, ltavThreshold,
	                       _ltavFunction, ltavRecursiveAverage,
	                       ltavRecursiveAverageState) ) {
		SEISCOMP_ERROR("Error computing ltav recursive average");
		return false;
	}

	const size_t ltavSignalStart = signalStart - dataSBSNRStart;
	SEISCOMP_DEBUG("ltavSignalStart = %zu", ltavSignalStart);
	const double ltav = ltavRecursiveAverage[ltavSignalStart];
	SEISCOMP_DEBUG("ltav = %f", ltav);

	// Compute amplitude
	double sbsnrAmplitude = sqrt((stav * stav) - (ltav * ltav));
	SEISCOMP_DEBUG("sbsnrAmplitude raw = %f", sbsnrAmplitude);

	// Convert amplitude to nano units
	sbsnrAmplitude *= 1E9 / _streamConfig[_usedComponent].gain;
	SEISCOMP_DEBUG("sbsnrAmplitude nano units = %f", sbsnrAmplitude);

	SignalUnit unit;
	if ( !unit.fromString(_streamConfig[_usedComponent].gainUnit.c_str()) ) {
		// Invalid unit string
		setStatus(IncompatibleUnit, 2);
		return false;
	}

	double gainFrequency = _streamConfig[_usedComponent].gainFrequency
		? *_streamConfig[_usedComponent].gainFrequency : 1.0;
	SEISCOMP_DEBUG("gainFrequency = %f", gainFrequency);

	switch ( unit ) {
		case MeterPerSecondSquared:
			// Integrate amplitude to velocity
			sbsnrAmplitude /= 2.0 * M_PI * gainFrequency;
			// NOBUG: The missing break statement is no bug; in case of m/s**2,
			//        the amplitude must be scaled twice
		case MeterPerSecond:
			// Integrate amplitude to displacement
			sbsnrAmplitude /= 2.0 * M_PI * gainFrequency;
			break;
		default:
			break;
	}
	SEISCOMP_DEBUG("sbsnrAmplitude integrated = %f", sbsnrAmplitude);

	// Compute SNR
	const double sbsnrSNR = stav / ltav;
	SEISCOMP_DEBUG("sbsnrSNR = %f", sbsnrSNR);

	if ( sbsnrSNR < 1.0 ) {
		SEISCOMP_ERROR("SBSNR SNR is less than 1.0");
		return false;
	}

	SEISCOMP_NOTICE("Amplitude: %f", sbsnrAmplitude);
	SEISCOMP_NOTICE("SNR: %f", sbsnrSNR);

	// Set return values
	amplitude->value = sbsnrAmplitude;
	*period = -1.0;  // Not computed
	dt->index = signalStart;
	dt->begin = signalStart;
	dt->end = signalEnd;
	*snr = sbsnrSNR;

	SEISCOMP_DEBUG("Returning amplitude = %f, period = %f, snr = %f",
	               amplitude->value, *period, *snr);

	SEISCOMP_DEBUG("Leaving Amplitude SBSNR computeAmplitude");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_AMPLITUDEPROCESSOR(AmplitudeSBSNR, AMP_TYPE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
