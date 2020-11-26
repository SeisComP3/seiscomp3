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


#include <cstddef>

#include "idc_utils.h"


namespace Seiscomp {
namespace Processing {
namespace Utils {
namespace IDC {


static size_t initAverage(const double *data, const size_t *state,
                          const size_t numPoints,
                          const size_t averageWindowLength,
                          double (*function)(double x), double *initAverage,
                          size_t *initAverageState);


bool runningAverage(const double *data, const size_t *state,
                    const size_t numPoints, const size_t averageWindowLength,
                    const size_t threshold, double (*function)(double x),
                    double *runningAverage, size_t *runningAverageState) {
	if ( averageWindowLength < 1 ) {
		return false;
	}

	size_t initLength = initAverage(data, state, numPoints,
	                                averageWindowLength, function,
	                                runningAverage, runningAverageState);

	if ( initLength < 0 ) {
		return false;
	}

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
		if ( runningAverageState[i] >= threshold ) {
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


bool recursiveAverage(const double *data, const size_t *state,
                      const size_t numPoints,
                      const size_t recursionLookbackLength,
                      const size_t averageWindowLength,
                      const size_t threshold, double (*function)(double x),
                      double *recursiveAverage,
                      size_t *recursiveAverageState)
{
	if ( averageWindowLength < 1 ) {
		return false;
	}

	size_t initLength = initAverage(data, state, numPoints,
	                                averageWindowLength, function,
	                                recursiveAverage, recursiveAverageState);

	if ( initLength < 0 ) {
		return false;
	}

	const double inverseAverageLength = 1.0 / (double)averageWindowLength;

	// Compute the running average of the first half window of data; if the
	//   state of the running sum is greater than or equal to threshold, the
	//   running average is the running sum divided by the running state sum,
	//   and the running state is set to one; otherwise, the running average
	//   and state are set to zero; if we were able to initialize, set
	//   initialize flag to false
	const double term = (double)recursiveAverageState[initLength - 1];
	bool initialize;

	if ( term >= (double)threshold ) {
		initialize = false;

		for ( size_t i = 0; i < initLength; ++i ) {
			recursiveAverage[i] /= term;
			recursiveAverageState[i] = 1;
		}
	}
	else {
		initialize = true;

		for ( size_t i = 0; i < initLength; ++i ) {
			recursiveAverage[i] = 0.0;
			recursiveAverageState[i] = 0;
		}
	}

	// Compute the recursive average of the data; for each centered window, if
	//   the average needs to be initialized, and the running sum state is
	//   above the threshold, re-initialize the recursive average as the
	//   centered running average and set the average state to one; if the sum
	//   state is not above the threshold, set the recursive average and state
	//   to zero; if the average does not need to be initialized, then if the
	//   state of the data at the recursion lookback (recursionLookbackLength)
	//   is one, compute the recursive average; f the state at
	//   recursionLookbackLength not one, then if the running sum state is
	//   above the threshold, carry the previous recursive average over;
	//   otherwise, if the running sum state is below the threshold, the
	//   recursive average needs to be re-initialized
	for ( size_t i = initLength; i < numPoints; ++i ) {
		const size_t im1 = i - 1;
		const size_t imr = i - recursionLookbackLength;

		if ( initialize ) {
			if ( recursiveAverageState[i] >= threshold ) {  // Re-init
				recursiveAverage[i] /= (double)recursiveAverageState[i];
				recursiveAverageState[i] = 1;
				initialize = false;
			}
			else {
				recursiveAverage[i] = 0.0;
				recursiveAverageState[i] = 0;
			}
		}
		else {
			if ( state[imr] > 0 ) {
				const double term =
				    (*function)(data[imr]) - recursiveAverage[im1];

				recursiveAverage[i] =
				    recursiveAverage[im1] + (term * inverseAverageLength);
				recursiveAverageState[i] = 1;
			}
			else if ( recursiveAverageState[i] >= threshold ) {
				recursiveAverage[i] = recursiveAverage[im1];  // Carry
				recursiveAverageState[i] = 1;
			}
			else {
				recursiveAverage[i] = 0.0;
				recursiveAverageState[i] = 0;
				initialize = true;
			}
		}
	}

	return true;
}


double samex(double x)
{
	return x;
}


static size_t initAverage(const double *data, const size_t *state,
                          const size_t numPoints,
                          const size_t averageWindowLength,
                          double (*function)(double x), double *initAverage,
                          size_t *initAverageState)
{
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
		initAverage[i] = averageSum;
		initAverageState[i] = stateSum;
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
			initAverageState[i] = stateSum;

			const double term = (*function)(data[ipe]) * (double)state[ipe]
			    - (*function)(data[imf]) * (double)state[imf];

			initAverage[i] = initAverage[im1] + term;
		}

		// Set last half window values to last sum value
		averageSum = initAverage[numPointsCenteredAverages - 1];
		stateSum = initAverageState[numPointsCenteredAverages - 1];

		for ( size_t i = numPointsCenteredAverages; i < numPoints; ++i ) {
			initAverage[i] = averageSum;
			initAverageState[i] = stateSum;
		}
	}

	return initLength;
}


} // namespace IDC
} // namespace Utils
} // namespace Processing
} // namespace Seiscomp
