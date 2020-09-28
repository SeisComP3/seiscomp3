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


#ifndef SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H
#define SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H


#include <cstddef>


namespace Seiscomp {
namespace Processing {
namespace Utils {
namespace IDC {


bool runningAverage(const double *data, const size_t *state,
                    const size_t numPoints, const size_t averageWindowLength,
                    const size_t threshold, double (*function)(double x),
                    double *runningAverage,
                    size_t *runningAverageState);


bool recursiveAverage(const double *data, const size_t *state,
                      const size_t numPoints,
                      const size_t recursionLookbackLength,
                      const size_t averageWindowLength,
                      const size_t threshold, double (*function)(double x),
                      double *recursiveAverage,
                      size_t *recursiveAverageState);


double samex(double x);


} // namespace IDC
} // namespace Utils
} // namespace Processing
} // namespace Seiscomp


#endif // SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_IDCUTILS_H
