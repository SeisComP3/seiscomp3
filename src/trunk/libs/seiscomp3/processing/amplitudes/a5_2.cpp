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


#define AMP_TYPE "A5/2"
#define SEISCOMP_COMPONENT Amplitudes/A5_2  // For logging

#include <cmath>  // fabs, tan, M_PI, sqrt, pow

#include <seiscomp3/core/datetime.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/math/windows/cosine.h>

#include "a5_2_private.h"

namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeA5_2::AmplitudeA5_2()
: Seiscomp::Processing::AmplitudeProcessor(AMP_TYPE)
, _tiLead(0.5)  // Signal window lead in seconds from IDC tirec lag5.5
, _tiLag(5.5)  // Signal window lag in seconds from IDC tirec lag5.5
, _stavFunction(fabs)  // Function to apply to running average
, _stavLength(1.0)  // Short-term average signal window length in seconds
, _ltavFunction(fabs)  // Function to apply to running average
, _ltavLength(60.0)  // Long-term average noise window length in seconds
, _noiseSignalGap(3.0)  // Gap between noise/signal windows in seconds
, _ford(3)  // Filter order (number of poles)
, _flo(0.8)  // Filter low band cut-off frequency in cycles/second
, _fhi(4.5)  // Filter high band cut-off frequency in cycles/second
, _zp(true)  // Use zero phase filter? True for non-causal
, _taper(0.5)  // Taper fraction of filter buffer
, _filbuf(10.0)  // Filter onset length (filter buffer) in seconds
, _demean(true)  // Use demean?
, _considerLastPeakTrough(true)  // Set to false for classic IDC behavior
, _removeFiltResp(true)  // Correct amplitude for filter response?
, _removeInstResp(true)  // Correct amplitude for instrument response?
, _filtRolloff(20.0)  // Filter roll-off beyond which a filter correction
                      // cannot be made (decibels)
, _interpolation(true) // Use interpolation?
, _interpolationPeriodSidePeakThreshold(0.25)  // Default
, _interpolationPeriodMinHalfPeriods(1)  // Default
, _interpolationPeriodMaxHalfPeriodRatio(2.0)  // Default
, _interpolationPeriodMaxNyquistPercentage(0.8)  // Default
, _interpolationPeriodWindowLengthPercentage(0.25)  // Default
, _interpolationPeriodMaxHiCutPercentage(2.0)  // Default
, _interpolationPeriodMinLoCutPercentage(0.833)  // Default
, _interpolationPeriodMaxFilterCorrection(100.0)  // Default
, _interpolationMaxFilterOrder(10)  // Butterworth max filter order
{
	setMargin(Seiscomp::Core::TimeSpan(0.0));
	setSignalStart(0.0 - _tiLead);
	setSignalEnd(0.0 + _tiLag);
	setNoiseStart(config().signalBegin - _noiseSignalGap - _ltavLength);
	setNoiseEnd(config().noiseBegin + _ltavLength);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeA5_2::computeTimeWindow() {
	// Call default implementation
	Seiscomp::Processing::AmplitudeProcessor::computeTimeWindow();

	// Retrieve the time window with filter onset length (filter buffer)
	//   - on both sides for zero-phase filtering
	Seiscomp::Core::TimeWindow tw = timeWindow();
	tw.setStartTime(tw.startTime() - Seiscomp::Core::TimeSpan(_filbuf));

	if ( _zp ) {
		tw.setEndTime(tw.endTime() + Seiscomp::Core::TimeSpan(_filbuf));
	}
	else {
		tw.setEndTime(tw.endTime());
	}
	setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeA5_2::computeAmplitude(const Seiscomp::DoubleArray &data,
                                     size_t i1, size_t i2,
                                     size_t si1, size_t si2,
                                     double offset,
                                     AmplitudeIndex *dt,
                                     AmplitudeValue *amplitude,
                                     double *period, double *snr) {
	SEISCOMP_DEBUG("Entering Amplitude A5/2 computeAmplitude");

	// Make work copies of data which can be changed
	SEISCOMP_DEBUG("data.size() = %d", data.size());
	Seiscomp::DoubleArray dataAmpPer(data);
	Seiscomp::DoubleArray dataSNR(data);

	// Determine useful constants
	const double samplingRate = samplingFrequency();
	SEISCOMP_DEBUG("samplingRate = %f", samplingRate);
	const size_t filbufCount = (size_t)(_filbuf * samplingRate);
	SEISCOMP_DEBUG("filbufCount = %zu", filbufCount);
	const size_t noiseCount = (size_t)(_ltavLength * samplingRate);
	SEISCOMP_DEBUG("noiseCount = %zu", noiseCount);
	const size_t noiseSignalGapCount = (size_t)(_noiseSignalGap * samplingRate);
	SEISCOMP_DEBUG("noiseSignalGapCount = %zu", noiseSignalGapCount);
	const size_t signalStart = si1;
	SEISCOMP_DEBUG("signalStart = %zu", signalStart);
	const size_t signalEnd = si2;  // exclusive bound
	SEISCOMP_DEBUG("signalEnd = %zu", signalEnd);
	const size_t signalCount = signalEnd - signalStart;
	SEISCOMP_DEBUG("signalCount = %zu", signalCount);
	const size_t noiseStart = signalStart - noiseSignalGapCount - noiseCount;
	SEISCOMP_DEBUG("noiseStart = %zu", noiseStart);
	const size_t noiseEnd = noiseStart + noiseCount;  // exclusive bound
	SEISCOMP_DEBUG("noiseEnd = %zu", noiseEnd);
	const size_t dataAmpPerCount = signalCount + (2 * filbufCount);
	SEISCOMP_DEBUG("dataAmpPerCount = %zu", dataAmpPerCount);
	const size_t dataAmpPerStart = signalStart - filbufCount;
	SEISCOMP_DEBUG("dataAmpPerStart = %zu", dataAmpPerStart);
	const size_t dataAmpPerEnd = dataAmpPerStart + dataAmpPerCount;
	SEISCOMP_DEBUG("dataAmpPerEnd = %zu", dataAmpPerEnd);  // exclusive bound
	const size_t dataSNRCount = (signalEnd - noiseStart) + ( 2 * filbufCount);
	SEISCOMP_DEBUG("dataSNRCount = %zu", dataSNRCount);
	const size_t dataSNRStart = noiseStart - filbufCount;
	SEISCOMP_DEBUG("dataSNRStart = %zu", dataSNRStart);
	const size_t dataSNREnd = dataSNRStart + dataSNRCount;  // exclusive bound
	SEISCOMP_DEBUG("dataSNREnd = %zu", dataSNREnd);

	// Demean data
	if ( _demean ) {
		double mean;
		mean = Seiscomp::Math::Statistics::mean(dataAmpPerCount,
		                                        dataAmpPer.typedData()
		                                        + dataAmpPerStart);
		SEISCOMP_DEBUG("dataAmpPer mean = %f", mean);

		for ( size_t i = dataAmpPerStart; i < dataAmpPerEnd; ++i ) {
			dataAmpPer[i] -= mean;
		}

		mean = Seiscomp::Math::Statistics::mean(dataSNRCount,
		                                        dataSNR.typedData()
		                                        + dataSNRStart);
		SEISCOMP_DEBUG("dataSNR mean = %f", mean);

		for ( size_t i = dataSNRStart; i < dataSNREnd; ++i ) {
			dataSNR[i] -= mean;
		}
	}

	// Taper data
	const double taperAmpPerWidth = _taper * filbufCount / dataAmpPerCount;
	const double taperSNRWidth = _taper * filbufCount / dataSNRCount;
	Seiscomp::Math::CosineWindow<double> cosineTaper;

	if ( _zp ) {
		// Tapers both ends
		cosineTaper.apply(dataAmpPerCount,
		                  dataAmpPer.typedData() + dataAmpPerStart,
		                  taperAmpPerWidth);
		cosineTaper.apply(dataSNRCount, dataSNR.typedData() + dataSNRStart,
		                  taperSNRWidth);
	}
	else {
		SEISCOMP_ERROR("No support for only tapering of the beginning");
		return false;
	}

	// Initialize butterworth bandpass filter
	Seiscomp::Math::Filtering::IIR::ButterworthBandpass<double> filter(_ford,
	                                                                   _flo,
	                                                                   _fhi);
	filter.setSamplingFrequency(samplingRate);

	// Forward filter data
	filter.apply(dataAmpPerCount, dataAmpPer.typedData() + dataAmpPerStart);
	filter.reset();
	filter.apply(dataSNRCount, dataSNR.typedData() + dataSNRStart);

	// Zero phase -> backward filter data
	if ( _zp ) {
		double *dataPointer;

		filter.reset();
		dataPointer = dataAmpPer.typedData() + dataAmpPerStart;

		for ( size_t i = dataAmpPerCount; i > 0; --i ) {  // i is unsigned
			filter.apply(1, dataPointer + i - 1);
		}

		filter.reset();
		dataPointer = dataSNR.typedData() + dataSNRStart;

		for ( size_t i = dataSNRCount; i > 0; --i ) {  // i is unsigned
			filter.apply(1, dataPointer + i - 1);
		}
	}

	if ( signalCount <= 0 ) {
		SEISCOMP_ERROR("No signal, no fun");
		return false;
	}

	// Assign types to all signal data points
	// By definition, first and last points cannot be peaks/troughs
	DataPointType dataPointTypes[signalCount];
	dataPointTypes[0] = NEITHER;  // Set first (needed in loop below)

	for ( size_t ti = 1, si = signalStart + 1; // Skip first (initialized above)
	      ti < signalCount;  // Do not skip last (needed but re-set below)
	      ++ti, ++si ) {
		const size_t tim1 = ti - 1;
		const size_t sim1 = si - 1;
		if ( dataAmpPer[si] > dataAmpPer[sim1] ) {
			dataPointTypes[ti] = PEAK;

			if ( dataPointTypes[tim1] == PEAK ) {
				dataPointTypes[tim1] = NEITHER;
			}
			else if ( dataPointTypes[tim1] == NEITHER ) {
				dataPointTypes[tim1] = TROUGH;
			}
		}
		else if ( dataAmpPer[si] < dataAmpPer[sim1] ) {
			dataPointTypes[ti] = TROUGH;

			if ( dataPointTypes[tim1] == TROUGH ) {
				dataPointTypes[tim1] = NEITHER;
			}
			else if ( dataPointTypes[tim1] == NEITHER ) {
				dataPointTypes[tim1] = PEAK;
			}
		}
		else {
			dataPointTypes[ti] = NEITHER;
		}
	}

	dataPointTypes[0] = NEITHER;  // Re-set first (by definition)
	dataPointTypes[signalCount - 1] = NEITHER;  // Re-set last (by definition)

	for ( size_t i = 0; i < signalCount; ++i ) {
		if ( dataPointTypes[i] == PEAK ) {
			SEISCOMP_DEBUG("Found peak at %zu (data index %zu)",
			               i, signalStart + i);
		}
		else if ( dataPointTypes[i] == TROUGH ) {
			SEISCOMP_DEBUG("Found trough at %zu (data index %zu)",
			               i, signalStart + i);
		}
	}

	// Save min, max signal data points for threshold determination
	size_t maxDataPoint = signalStart;  // index of highest data point
	size_t minDataPoint = signalStart;  // index of lowest data point
	// NOBUG: IDC algorithm skips first data point (index 0) which could be
	//        okay as it is is neither peak nor trough by definition, however,
	//        the algorithm does check the last data point which also by
	//        definition is neither peak nor trough
	// NOBUG: IDC algorithm skips second data point (index 1) for unknown
	//        reason
	for ( size_t i = signalStart + 2; i < signalEnd; ++i ) {
		if ( dataAmpPer[i] > dataAmpPer[maxDataPoint] ) {
			maxDataPoint = i;
		}

		if ( dataAmpPer[i] < dataAmpPer[minDataPoint] ) {
			minDataPoint = i;
		}
	}

	SEISCOMP_DEBUG("maxDataPoint = %zu, minDataPoint = %zu",
	               maxDataPoint, minDataPoint);

	// Determine largest amplitude excursion
	double maxDataDiff = fabs(dataAmpPer[maxDataPoint]
	                          - dataAmpPer[minDataPoint]);
	SEISCOMP_DEBUG("maxDataDiff = %f", maxDataDiff);

	if ( maxDataDiff <= 0.0 ) {
		SEISCOMP_ERROR("Largest amplitude excursion too small: %f",
		               maxDataDiff);
		return false;
	}

	// Smooth the peaks and troughs according to minimum amplitude threshold
	// NOBUG: Because IDC parameter amp-decimation-fraction is unset and hence
	//        defaults to 0.0 this part of the IDC algorithm is intentionally
	//        omitted (not implemented) as it does nothing (all peaks and
	//        troughs are preserved) when this threshold is 0.0 
	//        (minimumAmplitudeThreshold = maxDataDiff * _ampDecimationFraction)

	// Compute maximum peak-to-trough excursion
	double maxAmplitude = 0.0;  // Largest peak-to-trough amplitude */
	size_t maxLeft = 0, maxRight = 0;  // Max peak-trough pair
	// Find first and last peak/trough points
	size_t firstPeakTrough;

	for ( firstPeakTrough = 0;
	      dataPointTypes[firstPeakTrough] == NEITHER
	          && firstPeakTrough < signalCount;
	      ++firstPeakTrough );

	// NOBUG: IDC algorithm uses signalCount - 1 which is wrong as after a full
	//        uninterupted loop above then firstPeakTrough is signalCount
	if ( firstPeakTrough == signalCount ) {
		SEISCOMP_ERROR("No peak/trough points found in signal");
		return false;
	}

	SEISCOMP_DEBUG("First peak-trough point in signal: %zu",
	               firstPeakTrough);

	size_t lastPeakTrough;
	for ( lastPeakTrough = signalCount - 1;
	      dataPointTypes[lastPeakTrough] == NEITHER
	          && lastPeakTrough > firstPeakTrough;
	      --lastPeakTrough );

	if ( lastPeakTrough == firstPeakTrough ) {
		SEISCOMP_ERROR("No peak/trough points found in signal");
		return false;
	}

	SEISCOMP_DEBUG("Last peak-trough point in signal: %zu",
	               lastPeakTrough);

	// Initialize left end point
	size_t leftDataEndPoint = firstPeakTrough + signalStart;
	SEISCOMP_DEBUG("Left end point in data: %zu (%f)",
	               leftDataEndPoint, dataAmpPer[leftDataEndPoint]);

	// Loop through remaining points and find max amplitude and indices
	// NOBUG: The IDC algorithm does not include lastPeakTrough as it uses
	//        less than (<) which is incorrect instead of less than and equal
	//        (<=)
	if ( !_considerLastPeakTrough ) {
		SEISCOMP_DEBUG("Skipping last peak/trough in maximum amplitude search as per classic IDC behavior");
		--lastPeakTrough;
	}
	else {
		SEISCOMP_DEBUG("Including last peak/trough in maximum amplitude search");
	}
	for ( size_t ti = firstPeakTrough + 1, si = leftDataEndPoint + 1;
          ti <= lastPeakTrough;  // We want to include lastPeakTrough in search
          ++ti, ++si ) {
		// Find next peak or trough, skip if neither
		if ( dataPointTypes[ti] != NEITHER ) {
			// Assign right end point
			size_t rightDataEndPoint = si;
			SEISCOMP_DEBUG("New right end point in data: %zu (%f)",
			               rightDataEndPoint, dataAmpPer[rightDataEndPoint]);

			// Compute amplitude
			double amplitude = fabs(dataAmpPer[leftDataEndPoint]
			                        - dataAmpPer[rightDataEndPoint]);
			SEISCOMP_DEBUG("Amplitude for data end points %zu, %zu: %f",
						   leftDataEndPoint, rightDataEndPoint, amplitude);

			// Find maximum amplitude and indices
			if ( amplitude > maxAmplitude ) {
				SEISCOMP_DEBUG("New maximum amplitude %f at %zu, %zu",
				               amplitude, leftDataEndPoint, rightDataEndPoint);
				maxAmplitude = amplitude;
				maxLeft = leftDataEndPoint;
				maxRight = rightDataEndPoint;
			}

			// Re-assign left end point
			leftDataEndPoint = rightDataEndPoint;
			SEISCOMP_DEBUG("New left end point in data: %zu (%f)",
			               leftDataEndPoint, dataAmpPer[leftDataEndPoint]);
		}
	}

	SEISCOMP_DEBUG("maxAmplitude = %f, maxLeft = %zu, maxRight = %zu",
	               maxAmplitude, maxLeft, maxRight);

	// Initially, use zero-to-peak amplitude
	//   (half maximum peak-to-trough/trough-to-peak amplitude)
	double finalAmplitude = maxAmplitude * 0.5;
	SEISCOMP_NOTICE("Initial zero-to-peak amplitude: %f", finalAmplitude);

	// Initially, use twice the time between peak/trough (in samples)
	double finalPeriod = 2 * (maxRight - maxLeft);
	SEISCOMP_NOTICE("Initial period (in samples): %f", finalPeriod);

	// Use max left point as amplitude time (as index in input data)
	double finalAmplitudeTime = maxLeft;
	SEISCOMP_NOTICE("Amplitude time (as index in data): %f",
	                finalAmplitudeTime);

	if ( _interpolation ) {
		double interpolatedAmplitude = 0.0, interpolatedPeriod = 0.0;
		if ( interpolate(dataAmpPer.typedData() + signalStart, dataPointTypes,
		                 signalCount, maxLeft - signalStart,
		                 maxRight - signalStart, &interpolatedAmplitude,
		                 &interpolatedPeriod) ) {
			// Make interpolated amplitude into zero-to-peak amplitude
			//   (half maximum peak-to-trough/trough-to-peak amplitude)
			interpolatedAmplitude *= 0.5;

			SEISCOMP_NOTICE("Interpolated zero-to-peak amplitude: %f",
			                interpolatedAmplitude);
			SEISCOMP_NOTICE("Interpolated period (in samples): %f",
			                interpolatedPeriod);

			// Only use interpolated amplitude and period if interpolated
			//   period is larger than 0.0
			if ( interpolatedPeriod > 0.0 ) {
				SEISCOMP_DEBUG("Will use interpolated period");
				finalPeriod = interpolatedPeriod;

				// Only use interpolated amplitude if less than 1.5x initial
				//   amplitude
				if ( interpolatedAmplitude < 1.5 * finalAmplitude ) {
					SEISCOMP_DEBUG("Will use interpolated amplitude");
					finalAmplitude = interpolatedAmplitude;
				}
				else {
					SEISCOMP_DEBUG("Will not use interpolated amplitude");
				}
			}
			else {
				SEISCOMP_DEBUG("Will use neither interpolated amplitude nor interpolated period");
			}
		}
		else {
			SEISCOMP_DEBUG("Will use neither interpolated amplitude nor interpolated period");
		}
	}

	SEISCOMP_NOTICE("Final period: %f", finalPeriod);

	SEISCOMP_NOTICE("Amplitude before filter response correction: %f",
	                finalAmplitude);

	const double periodInSeconds = finalPeriod / samplingRate;
	SEISCOMP_DEBUG("Period in seconds: %f", periodInSeconds);
	const double frequencyInHz = 1.0 / periodInSeconds;
	SEISCOMP_DEBUG("Frequency in hertz: %f", frequencyInHz);

	// Correct amplitude for filter response if requested
	if ( _removeFiltResp ) {
		const double sampleInterval = 1.0 / samplingRate;
		SEISCOMP_DEBUG("Sample interval: %f", sampleInterval);

		// Convert the "digital" frequencies to "analog" ones
		const double floAnalog =
		    tan(M_PI * _flo * sampleInterval) / (M_PI * sampleInterval);
		const double fhiAnalog =
		    tan(M_PI * _fhi * sampleInterval) / (M_PI * sampleInterval);
		const double frequencyAnalog =
		    tan(M_PI * frequencyInHz * sampleInterval) / (M_PI * sampleInterval);

		// Convert the signal "analog" frequency to that of standard
		//   (normalized) LP filter for bandpass filter
		const double standardNormalizedLPFilterFrequency =
		    (frequencyAnalog * frequencyAnalog - fhiAnalog * floAnalog)
		    / frequencyAnalog / (fhiAnalog - floAnalog);

		// Compute the amplitude adjustment factor
		double filterCorrection =
		    sqrt(fabs(1.0 / (1.0 + pow(standardNormalizedLPFilterFrequency,
		                               2.0 * _ford))));

		// For zero phase filter use the square of the correction coefficient
		if ( _zp ) {
			filterCorrection = filterCorrection * filterCorrection;
		}

		SEISCOMP_DEBUG("Filter correction: %f", filterCorrection);

		// Determine if frequency is close enough to filter band to allow
		//   filter correction; if it is too far away from the filter band, the
		//   amplitude will be magnified too much upon correction
		const double minResponse = pow(10.0, -1.0 * _filtRolloff / 20.0);
		SEISCOMP_DEBUG("Minimum response: %f", minResponse);

		if ( filterCorrection < minResponse ) {
			SEISCOMP_ERROR("Measured period %f too far outside filter band (> -%f dB) to correct for filter response",
			               periodInSeconds, _filtRolloff);
			return false;
		}

		// Correct amplitude
		finalAmplitude /= filterCorrection;
	}

	SEISCOMP_NOTICE("Amplitude after filter response correction: %f",
	                finalAmplitude);

	// Correct amplitude for instrument if desired
	SEISCOMP_NOTICE("Amplitude before instrument correction: %f",
	                finalAmplitude);

	if ( _removeInstResp ) {
		// Convert to nano units
		finalAmplitude *= 1E9 / _streamConfig[_usedComponent].gain;
		// Integrate to displacement
		finalAmplitude /= 2.0 * M_PI / periodInSeconds;
	}

	SEISCOMP_NOTICE("Amplitude after instrument correction: %f",
	                finalAmplitude);

	SEISCOMP_NOTICE("Final amplitude: %f", finalAmplitude);

	// Compute SNR
	double finalSNR = 0.0;

	// Compute running average of noise (noiseRunningAverage)
	double *noiseData = dataSNR.typedData() + noiseStart;

	size_t noiseState[noiseCount];
	double noiseRunningAverage[noiseCount];
	size_t noiseRunningAverageState[noiseCount];

	for ( size_t i = 0; i < noiseCount; ++i ) {
		noiseState[i] = 1;
	}

	if ( !runningAverage(noiseData, noiseState, noiseCount, noiseCount,
	                     noiseCount, _ltavFunction, noiseRunningAverage,
	                     noiseRunningAverageState) ) {
		SEISCOMP_ERROR("Error computing average over noise window");
		return false;
	}

	SEISCOMP_DEBUG("Running average of noise: %f", noiseRunningAverage[0]);

	// Compute running average of signal (signalRunningAverage)
	const size_t stavCount = (size_t)(_stavLength * samplingRate);
	SEISCOMP_DEBUG("stavCount = %zu", stavCount);
	const size_t stavWindowMoveInterval = (size_t)(stavCount / 4);  // ??? 4
	SEISCOMP_DEBUG("stavWindowMoveInterval = %zu", stavWindowMoveInterval);
	const size_t stavNumWindows =
	    (size_t)((signalCount - stavCount) / stavWindowMoveInterval) + 1;
	SEISCOMP_DEBUG("stavNumWindows = %zu", stavNumWindows);

	double *signalData = dataSNR.typedData() + signalStart;
	size_t signalState[stavCount];
	double signalRunningAverage[stavCount];
	size_t signalRunningAverageState[stavCount];

	for ( size_t i = 0; i < signalCount; ++i ) {
		signalState[i] = 1;
	}

	double maxSTAV = 0.0;
	for ( size_t i = 0, signalOffset = 0;
	      i < stavNumWindows;
	      ++i, signalOffset += stavWindowMoveInterval ) {
		// Compute running average of signal (signalRunningAverage)
		SEISCOMP_DEBUG("signalOffset = %zu", signalOffset);
		if ( !runningAverage(signalData + signalOffset, signalState,
		                     stavCount, stavCount, stavCount, _stavFunction,
		                     signalRunningAverage,
		                     signalRunningAverageState) ) {
			SEISCOMP_ERROR("Error computing short-term average in signal window");
			return false;
		}

		SEISCOMP_DEBUG("Running average of signal: %f",
		               signalRunningAverage[0]);

		// Find the maximum short-term average value
		if ( maxSTAV < signalRunningAverage[0] ) {
			maxSTAV = signalRunningAverage[0];
		}
	}

	SEISCOMP_DEBUG("Maximum short-term average of signal: %f", maxSTAV);
	finalSNR = maxSTAV / noiseRunningAverage[0];
	SEISCOMP_NOTICE("SNR: %f", finalSNR);

	// Set return values
	amplitude->value = finalAmplitude;
	*period = finalPeriod;
	dt->index = finalAmplitudeTime;
	dt->begin = signalStart;
	dt->end = signalEnd;
	*snr = finalSNR;

	SEISCOMP_DEBUG("Returning amplitude = %f, period = %f, snr = %f",
	               amplitude->value, *period, *snr);

	SEISCOMP_DEBUG("Leaving Amplitude A5/2 computeAmplitude");

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeA5_2::interpolate(const double *data,
                                const DataPointType *dataPointTypes,
                                const size_t numPoints, const size_t maxLeft,
                                const size_t maxRight,
                                double *interpolatedAmplitude,
                                double *interpolatedPeriod) {
	SEISCOMP_DEBUG("Entering Amplitude A5/2 interpolate");

	// Peak-trough amplitude from indices maxLeft, maxRight
	const double maxAmplitude = fabs(data[maxLeft] - data[maxRight]);

	// Find previous, next extrema
	const double sideExtremaThreshold =
	    maxAmplitude * _interpolationPeriodSidePeakThreshold;
	const size_t nullDataExtremumIndex =
	    (size_t)-1;  // -1 (signed) or MAX_SIZE (unsigned)

	size_t previousDataExtremum = nullDataExtremumIndex;
	size_t nextDataExtremum = nullDataExtremumIndex;

	for ( size_t i = maxLeft - 1; i > 0; --i ) {
		if ( dataPointTypes[i] != NEITHER
		     && fabs(data[i] - data[maxLeft]) > sideExtremaThreshold ) {
			previousDataExtremum = i;
			break;
		}
	}

	for ( size_t i = maxRight + 1; i < numPoints - 1; ++i ) {
		if ( dataPointTypes[i] != NEITHER
		     && fabs(data[i] - data[maxRight]) > sideExtremaThreshold ) {
			nextDataExtremum = i;
			break;
		}
	}

	std::vector<size_t> dataExtremumIndexes;
	dataExtremumIndexes.reserve(4);
	dataExtremumIndexes.push_back(previousDataExtremum);
	dataExtremumIndexes.push_back(maxLeft);
	dataExtremumIndexes.push_back(maxRight);
	dataExtremumIndexes.push_back(nextDataExtremum);

	for ( size_t i = 0; i < dataExtremumIndexes.size(); ++i ) {
		SEISCOMP_DEBUG("Initial extremum %zu index: %zu",
		               i, dataExtremumIndexes[i]);
	}

	// Add one more extremum if nothing found before maxLeft or after maxRight
	if ( previousDataExtremum == nullDataExtremumIndex ) {
		// NOBUG: IDC algorithm skips last data point which is okay as it is
		//        always NEITHER by definition
		// nextDataExtremum + 1 becomes 0 if nextDataExtremum ==
		//   nullDataExtremumIndex as nullDataExtremumIndex is either -1
		//   (signed) or MAX_SIZE (unsigned)
		for ( size_t i = nextDataExtremum + 1; i < numPoints - 1; ++i ) {
			if ( dataPointTypes[i] != NEITHER
			     && fabs(data[i]
			            - data[nextDataExtremum]) > sideExtremaThreshold ) {
				previousDataExtremum = i;
				SEISCOMP_DEBUG("One more extremum index is appended: %zu",
				               previousDataExtremum);
				break;
			}
		}

		dataExtremumIndexes.clear();
		dataExtremumIndexes.push_back(maxLeft);
		dataExtremumIndexes.push_back(maxRight);
		dataExtremumIndexes.push_back(nextDataExtremum);
		dataExtremumIndexes.push_back(previousDataExtremum);
	}
	else if ( nextDataExtremum == nullDataExtremumIndex ) {
		// NOBUG: previousDataExtremum must be larger than zero (if not
		//        nullDataExtremumIndex, condition above) as first data point
		//        is always NEITHER by definition, so using
		//        previousDataExtremum - 1 is safe "by definition"
		for ( size_t i = previousDataExtremum - 1; i > 0; --i ) {
			if ( dataPointTypes[i] != NEITHER
			     && fabs(data[i]
			            - data[previousDataExtremum]) > sideExtremaThreshold ) {
				nextDataExtremum = i;
				SEISCOMP_DEBUG("One more extremum index is prepended: %zu",
				               nextDataExtremum);
				break;
			}
		}

		dataExtremumIndexes.clear();
		dataExtremumIndexes.push_back(nextDataExtremum);
		dataExtremumIndexes.push_back(previousDataExtremum);
		dataExtremumIndexes.push_back(maxLeft);
		dataExtremumIndexes.push_back(maxRight);
	}

	for ( size_t i = 0; i < dataExtremumIndexes.size(); ++i ) {
		SEISCOMP_DEBUG("Final extremum %zu index: %zu",
		               i, dataExtremumIndexes[i]);
	}

	// Make Lagrangian interpolation for each extrema
	std::vector<double> interpolatedExtremumIndexes;
	interpolatedExtremumIndexes.reserve(dataExtremumIndexes.size());
	std::vector<double> interpolatedExtremumValues;
	interpolatedExtremumValues.reserve(dataExtremumIndexes.size());

	for ( size_t i = 0; i < dataExtremumIndexes.size(); ++i ) {
		// Don't process null extrema
		if ( dataExtremumIndexes[i] == nullDataExtremumIndex ) {
			SEISCOMP_DEBUG("Extremum data point index %zu is null, skipping",
		                   i);
			continue;
		}

		const size_t dataExtremumIndex = dataExtremumIndexes[i];
		double interpolatedExtremumIndex = (double)dataExtremumIndex;
		double interpolatedExtremumValue = data[dataExtremumIndex];

		// Make 3-point Lagrangian interpolation (error if center point not
		//   extrema)
		const double ym1 = data[dataExtremumIndex - 1];
		const double y0 = data[dataExtremumIndex];
		const double yp1 = data[dataExtremumIndex + 1];
		double shift = 0.0;  // shift of maximum in sample intervals

		// 	With current PEAK and TROUGH definition this should not ever happen
		if ( (ym1 - 2 * y0 + yp1) == 0.0 ) {
			SEISCOMP_WARNING("PEAK or TROUGH? Collinear samples detected");
			return false;
		}

		// Determine PEAK/TROUGH position
		shift = (ym1 - yp1) / (ym1 - 2 * y0 + yp1) / 2.0;
		// Test again: if the "shift" is in bounds - i.e. less than 1 sample
		//   interval?

		if ( fabs(shift) >= 1.0 ) {
			SEISCOMP_WARNING("%f %f %f do not define PEAK or TROUGH",
			                 ym1, y0, yp1);
			return false;
		}

		// Calculate maximum/minimum position
		interpolatedExtremumIndex = interpolatedExtremumIndex + shift;
		// Compute the approximation of the PEAK/TROUGH amplitude
		interpolatedExtremumValue = (shift * (shift - 1.0) ) * ym1 / 2.0
		    - ((shift + 1.0) * (shift - 1.0)) * y0
		    + (shift * (shift + 1.0)) * yp1 / 2.0;

		interpolatedExtremumIndexes.push_back(interpolatedExtremumIndex);
		interpolatedExtremumValues.push_back(interpolatedExtremumValue);
	}

	for (size_t i = 0; i < interpolatedExtremumIndexes.size(); ++i) {
		SEISCOMP_DEBUG("Interpolated extremum %zu index: %f",
		               i, interpolatedExtremumIndexes[i]);
	}

	for (size_t i = 0; i < interpolatedExtremumValues.size(); ++i) {
		SEISCOMP_DEBUG("Interpolated extremum %zu value: %f",
		               i, interpolatedExtremumValues[i]);
	}

	// Find the maximum from the (non-null) interpolated amplitudes
	double maxInterpolatedAmplitude = -1.0;
	for ( size_t i = 1; i < interpolatedExtremumValues.size(); ++i ) {
		// NOBUG: As we skipped processing null extremas above, there are no
		//        null interpolated ones - this differs from IDC algorithm
		//        which uses -1.0 for those null amplitudes and hence possibly
		//        computes the difference with that faux value at this point
		double amplitude = fabs(interpolatedExtremumValues[i]
		                        - interpolatedExtremumValues[i-1]);
		if ( maxInterpolatedAmplitude < amplitude ) {
			maxInterpolatedAmplitude = amplitude;
		}
	}

	SEISCOMP_DEBUG("Maximum interpolated amplitude: %f",
	               maxInterpolatedAmplitude);

	// Calculate half periods from the (non-null) extremas
	std::vector<double> halfPeriods;
	halfPeriods.reserve(interpolatedExtremumIndexes.size() - 1);

	for ( size_t i = 0; i < interpolatedExtremumIndexes.size() - 1; ++i ) {
		// As we skipped processing null extremas above, there are no null
		//   interpolated ones so no need to keep track of null half periods
		double halfPeriod = (interpolatedExtremumIndexes[i + 1]
		                     - interpolatedExtremumIndexes[i]);
		halfPeriods.push_back(halfPeriod);
		SEISCOMP_DEBUG("Half period %zu before sorting: %f", i, halfPeriod);
	}

	// Screen half periods using various criteria
	std::vector<double> screenedHalfPeriods;
	screenedHalfPeriods.reserve(halfPeriods.size());
	const double samplingRate = samplingFrequency();
	SEISCOMP_DEBUG("Sampling rate: %f", samplingRate);
	const double sampleInterval = 1.0 / samplingRate;
	SEISCOMP_DEBUG("Sample interval: %f", sampleInterval);
	const double nyquist = samplingRate / 2.0;
	SEISCOMP_DEBUG("Nyquist: %f", nyquist);
	const double nyquistThreshold =
	    nyquist * _interpolationPeriodMaxNyquistPercentage;
	SEISCOMP_DEBUG("Nyquist threshold: %f", nyquistThreshold);
	const double windowLength = numPoints / samplingRate;
	SEISCOMP_DEBUG("Window length: %f", windowLength);
	const double windowLengthThreshold =
	    windowLength * _interpolationPeriodWindowLengthPercentage;
	SEISCOMP_DEBUG("Window length threshold: %f", windowLengthThreshold);
	const double maxHiCutFrequency =
	    _fhi * _interpolationPeriodMaxHiCutPercentage;
	SEISCOMP_DEBUG("Max high cut frequency: %f", maxHiCutFrequency);
	const double minLoCutFrequency =
	    _flo * _interpolationPeriodMinLoCutPercentage;
	SEISCOMP_DEBUG("Min low cut frequency: %f", minLoCutFrequency);

	// Test on sampling rate
	if ( samplingRate <= 0.0 ) {
		SEISCOMP_WARNING("Wrong signal sampling rate: %f", samplingRate);
		return false;
	}

	// Test on filter order
	if ( _ford > _interpolationMaxFilterOrder
		 || (_ford < 1 && (_flo > 0.0 || _fhi > 0.0))
		 || (_ford < 0 && (_flo > 0.0 && _fhi > 0.0)) ) {
		SEISCOMP_WARNING("Filter order: %d out of limits, max order allowed is: %d",
		                 _ford, _interpolationMaxFilterOrder);
		return false;
	}

	for ( size_t i = 0; i < halfPeriods.size(); ++i ) {
		const double period = 2.0 * halfPeriods[i] / samplingRate;
		SEISCOMP_DEBUG("Half period %zu period: %f", i, period);
		const double frequency = 1.0 / ((period == 0.0) ? .0000001 : period);
		SEISCOMP_DEBUG("Half period %zu frequency: %f", i, frequency);

		// Nyquist test
		if ( frequency >= nyquistThreshold ) {
			SEISCOMP_INFO("Half period %zu frequency %f exceeds the %f nyquist threshold",
			              i, frequency, nyquistThreshold);
			continue;
		}

		// Window length check
		if ( period >= windowLengthThreshold ) {
			SEISCOMP_INFO("Half period %zu period %f exceeds the %f seconds window length threshold",
			              i, period, windowLengthThreshold);
			continue;
		}

		// Corner frequency check
		if ( frequency >= maxHiCutFrequency || frequency <= minLoCutFrequency ) {
			SEISCOMP_INFO("Half period %zu frequency %f out of filter band range %f - %f",
			              i, frequency, minLoCutFrequency, maxHiCutFrequency);
			continue;
		}

		// Filter response correction check
		double expectedCorrection = 1.0;
		// Test on input frequency
		if ( frequency <= 0.0 || frequency >= nyquist ) {
			SEISCOMP_WARNING("Half period %zu wrong signal frequency: %f",
			                  i, frequency);
			return false;
		}

		// Convert the "digital" frequencies to "analog" ones
		const double floAnalog =
		    tan(M_PI * _flo * sampleInterval) / (M_PI * sampleInterval);
		const double fhiAnalog =
		    tan(M_PI * _fhi * sampleInterval) / (M_PI * sampleInterval);
		const double frequencyAnalog =
		    tan(M_PI * frequency * sampleInterval) / (M_PI * sampleInterval);
		// Convert the signal "analog" frequency to that of standard
		//   (normalized) LP filter for bandpass filter
		const double standardNormalizedLPFilterFrequency =
		    (frequencyAnalog * frequencyAnalog - fhiAnalog * floAnalog)
		    / frequencyAnalog / (fhiAnalog - floAnalog);
		// Compute the amplitude adjustment factor
		double filterCorrection =
		    sqrt(fabs(1.0 / (1.0 + pow(standardNormalizedLPFilterFrequency,
		                               2.0 * _ford))));
		// For zero phase filter use the square of the correction coefficient
		if ( _zp ) {
			filterCorrection = filterCorrection * filterCorrection;
		}

		SEISCOMP_DEBUG("Filter correction: %f", filterCorrection);
		const double inverseFilterCorrection = 1.0 / filterCorrection;
		if ( inverseFilterCorrection > _interpolationPeriodMaxFilterCorrection ) {
			SEISCOMP_WARNING("Inverse of filter correction %f exceed maximum value of %f",
			                 inverseFilterCorrection,
			                 _interpolationPeriodMaxFilterCorrection);
			return false;
		}

		// Apply amplitude adjustment
		expectedCorrection /= filterCorrection;
		SEISCOMP_DEBUG("Half period %zu expected correction: %f",
		               i, expectedCorrection);

		// Filter response correction check
		if ( expectedCorrection >= _interpolationPeriodMaxFilterCorrection ) {
			SEISCOMP_INFO("Half period %zu frequency %f causes filter correction to exceed maximum value of %f",
			              i, frequency,
			              _interpolationPeriodMaxFilterCorrection);
			continue;
		}

		// Half period is okay
		screenedHalfPeriods.push_back(halfPeriods[i]);
		SEISCOMP_DEBUG("Half period %zu is okay", i);
	}

	// Calculate best half period from screened half periods
	// No null half periods here which greatly simplifies this compared to IDC
	//   algorithm
	std::sort(screenedHalfPeriods.begin(), screenedHalfPeriods.end());

	// Everything below assumes half periods are sorted
	const size_t numScreenedHalfPeriods = screenedHalfPeriods.size();
	for ( size_t i = 0; i < numScreenedHalfPeriods; ++i ) {
		SEISCOMP_DEBUG("Half period %zu after sorting: %f",
		               i, screenedHalfPeriods[i]);
	}

	if ( numScreenedHalfPeriods == 0 ) {
		SEISCOMP_WARNING("No valid half periods found");
		return false;
	}
	else if ( numScreenedHalfPeriods < _interpolationPeriodMinHalfPeriods ) {
		SEISCOMP_WARNING("Only %zu valid periods, below threshold of %zu",
		                 numScreenedHalfPeriods,
		                 _interpolationPeriodMinHalfPeriods);
		return false;
	}

	double fullPeriod = -1.0;

	// We don't use switch statement here so we can compute and use constants
	// which greatly improves readability and reduces re-computation
	if ( numScreenedHalfPeriods == 1 ) {
		fullPeriod = 2.0 * screenedHalfPeriods[0];
		SEISCOMP_DEBUG("fullPeriod = 2.0 * %f", screenedHalfPeriods[0]);
	}
	else if ( numScreenedHalfPeriods == 2 ) {
		const double minHalfPeriod = screenedHalfPeriods[0];
		const double maxHalfPeriod = screenedHalfPeriods[1];
		const double ratioMaxMin = maxHalfPeriod / minHalfPeriod;

		// Check threshold
		if ( ratioMaxMin > _interpolationPeriodMaxHalfPeriodRatio ) {
			// Use min value for period
			SEISCOMP_INFO("Ratio of max and min half periods %f exceeds threshold of %f",
			              ratioMaxMin,
			              _interpolationPeriodMaxHalfPeriodRatio);
			fullPeriod = 2.0 * minHalfPeriod;
			SEISCOMP_DEBUG("fullPeriod = 2.0 * minHalfPeriod[%f]",
			               minHalfPeriod);

		}
		else {
			// Use average of min and max
			fullPeriod = minHalfPeriod + maxHalfPeriod;
			SEISCOMP_DEBUG("fullPeriod = minHalfPeriod[%f] + maxHalfPeriod[%f]",
			               minHalfPeriod, maxHalfPeriod);

		}
	}
	else if ( numScreenedHalfPeriods == 3 ) {
		const double minHalfPeriod = screenedHalfPeriods[0];
		const double midHalfPeriod = screenedHalfPeriods[1];
		const double maxHalfPeriod = screenedHalfPeriods[2];
		const double ratioMidMin = midHalfPeriod / minHalfPeriod;
		const double ratioMaxMid = maxHalfPeriod / midHalfPeriod;

		// Check min and mid, if min too small move on with mid and max;
		//   otherwise as min and mid are okay, check min and max, if max too
		//   big, use min and mid;
		//   otherwise as three values are okay, average them
		if ( ratioMidMin > _interpolationPeriodMaxHalfPeriodRatio ) {
			SEISCOMP_INFO("Ratio of mid and min half periods %f exceeds threshold of %f, discarding min period",
			              ratioMidMin,
			              _interpolationPeriodMaxHalfPeriodRatio);
			// Check mid and max
			if ( ratioMaxMid > _interpolationPeriodMaxHalfPeriodRatio ) {
				// Use mid value only as period
				SEISCOMP_INFO("Ratio of max and mid half periods %f exceeds threshold of %f, using mid period as final period",
				              ratioMaxMid,
				              _interpolationPeriodMaxHalfPeriodRatio);
				fullPeriod = 2.0 * midHalfPeriod;
				SEISCOMP_DEBUG("fullPeriod = 2.0 * midHalfPeriod[%f]",
				               midHalfPeriod);
			}
			else {
				// Mid and max are okay, use average
				fullPeriod = midHalfPeriod + maxHalfPeriod;
				SEISCOMP_DEBUG("fullPeriod = midHalfPeriod[%f] + maxHalfPeriod[%f]",
				               midHalfPeriod, maxHalfPeriod);
			}
		}
		else if ( ratioMaxMid > _interpolationPeriodMaxHalfPeriodRatio ) {
			SEISCOMP_INFO("Ratio of max and mid half periods %f exceeds threshold of %f, discarding max period",
			              ratioMaxMid,
			              _interpolationPeriodMaxHalfPeriodRatio);
			fullPeriod = minHalfPeriod + midHalfPeriod;
			SEISCOMP_DEBUG("fullPeriod = minHalfPeriod[%f] + midHalfPeriod[%f]",
			               minHalfPeriod, midHalfPeriod);
		}
		else {
			fullPeriod = 2.0 * (minHalfPeriod + midHalfPeriod
			                    + maxHalfPeriod) / 3.0;
			SEISCOMP_DEBUG("fullPeriod = 2.0 * (minHalfPeriod[%f] + midHalfPeriod[%f] + maxHalfPeriod[%f]) / 3.0",
			               minHalfPeriod, midHalfPeriod, maxHalfPeriod);
		}
	}
	else {
		SEISCOMP_WARNING("Strange number of half_periods: %zu",
		                 numScreenedHalfPeriods);
		return false;
	}

	SEISCOMP_DEBUG("Full period: %f", fullPeriod);

	// Set return values
	*interpolatedAmplitude = maxInterpolatedAmplitude;
	*interpolatedPeriod = fullPeriod;
	SEISCOMP_DEBUG("Returning interpolated amplitude = %f, period = %f",
	               *interpolatedAmplitude, *interpolatedPeriod);

	SEISCOMP_DEBUG("Leaving Amplitude A5/2 interpolate");
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeA5_2::runningAverage(const double *data, const size_t *state,
                                   const size_t numPoints,
                                   const size_t averageWindowLength,
                                   const size_t threshold,
                                   double (*function)(double x),
                                   double *runningAverage,
                                   size_t *runningAverageState) {
	if ( averageWindowLength < 1 ) {
		return false;
	}

	// Determine odd number of samples for centered averages
	const size_t startPoint = (size_t)(averageWindowLength / 2);
	size_t endPoint = startPoint;

	if ( averageWindowLength % 2 == 0 ) {
		endPoint -= 1;
	}

	// Compute index of first centered average point and number of points which
	//   will have centered averages
	const size_t firstCenteredAveragePoint = startPoint + 1;
	const size_t numPointsCenteredAverages = numPoints - endPoint;

	// Initialize the first half window of data, apply the function and
	//   multiply by the data state as each point is summed, sum the state
	//   values as well, store sums in averageSum, stateSum
	size_t initLength = averageWindowLength;

	if ( numPoints < averageWindowLength ) {
		initLength = numPoints;
	}

	double averageSum = 0.0;
	int stateSum = 0;

	for ( size_t i = 0; i < initLength; ++i ) {
		averageSum += (*function)(data[i]) * (double)state[i];
		stateSum += state[i];
	}

	// Set the first half window data values to the sum averageSum and the
	//   first half window data state values to the sum stateSum
	initLength = firstCenteredAveragePoint;

	if ( numPoints < averageWindowLength ) {
		initLength = numPoints;
	}

	for ( size_t i = 0; i < initLength; ++i )	{
		runningAverage[i] = averageSum;
		runningAverageState[i] = stateSum;
	}

	if ( initLength != numPoints ) {
		// Compute centered running sums for each data point and each state
		//   point, remember to apply function and state to data points
		for ( size_t i = firstCenteredAveragePoint;
		      i < numPointsCenteredAverages;
		      ++i ) {
			const size_t ipe = i + endPoint;
			const size_t imf = i - firstCenteredAveragePoint;
			const size_t im1 = i - 1;

			stateSum += state[ipe] - state[imf];
			runningAverageState[i] = stateSum;

			const double term = (*function)(data[ipe]) * (double)state[ipe]
			    - (*function)(data[imf]) * (double)state[imf];

			runningAverage[i] = runningAverage[im1] + term;
		}

		// Set last half window values to last sum value
		averageSum = runningAverage[numPointsCenteredAverages - 1];
		stateSum = runningAverageState[numPointsCenteredAverages - 1];

		for ( size_t i = numPointsCenteredAverages; i < numPoints; ++i ) {
			runningAverage[i] = averageSum;
			runningAverageState[i] = stateSum;
		}
	}

	if ( initLength < 0 ) {
		return false;
	}

	// Last valid centered average
	const size_t lastValidCenteredAverage = numPoints - initLength + 1;

	// Compute the running average of the first half window of data; if the
	//   state of the running sum is greater than or equal to threshold, the
	//   running average is the running sum divided by the running state sum,
	//   and the running state is set to one; otherwise, the running average
	//   and state are set to zero
	const double term = (double)runningAverageState[initLength - 1];

	if ( term >= (double)threshold ) {
		for ( size_t i = 0; i < initLength; ++i ) {
			runningAverage[i] /= term;
			runningAverageState[i] = 1;
		}
	}
	else {
		for ( size_t i = 0; i < initLength; ++i ) {
			runningAverage[i] = 0.0;
			runningAverageState[i] = 0;
		}
	}

	// Compute the centered running average of the data
	for ( size_t i = initLength; i < lastValidCenteredAverage; ++i ) {
		if ( runningAverageState[i] >= (double)threshold ) {
			runningAverage[i] /= (double)runningAverageState[i];
			runningAverageState[i] = 1;
		}
		else {
			runningAverage[i] = 0.0;
			runningAverageState[i] = 0;
		}
	}

	// Set the final half window to the final running average value
	const size_t lm1 = lastValidCenteredAverage - 1;

	for ( size_t i = lastValidCenteredAverage; i < numPoints; ++i ) {
		runningAverage[i] = runningAverage[lm1];
		runningAverageState[i] = runningAverageState[lm1];
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_AMPLITUDEPROCESSOR(AmplitudeA5_2, AMP_TYPE);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
