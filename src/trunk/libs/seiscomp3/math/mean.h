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


#ifndef __SC_MATH_MEAN_H__
#define __SC_MATH_MEAN_H__

#include<vector>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core.h>

namespace Seiscomp
{
namespace Math
{
namespace Statistics
{

SC_SYSTEM_CORE_API double mean(const DoubleArray &);
SC_SYSTEM_CORE_API double mean(const std::vector<double> &);
SC_SYSTEM_CORE_API double mean(int n, const double *);

SC_SYSTEM_CORE_API double median(const DoubleArray &);
SC_SYSTEM_CORE_API double median(const std::vector<double> &);
SC_SYSTEM_CORE_API double median(int n, const double *);

SC_SYSTEM_CORE_API double fractile(const DoubleArray &, double x);
SC_SYSTEM_CORE_API double fractile(const std::vector<double> &, double x);
SC_SYSTEM_CORE_API double fractile(int n, const double *, double x);

SC_SYSTEM_CORE_API bool computeTrimmedMean(int n, const double *f, double percent, double &value, double &stdev, double *weights = NULL);
SC_SYSTEM_CORE_API bool computeTrimmedMean(int n, const double *f, double &value, double &stdev, double *weights = NULL);
SC_SYSTEM_CORE_API bool computeTrimmedMean(const std::vector<double> &v, double percent, double &value, double &stdev, std::vector<double> *weights = NULL);

SC_SYSTEM_CORE_API bool computeMean(const std::vector<double> &v, double &value, double &stdev);

SC_SYSTEM_CORE_API bool average(int n, const double *values, const double *weights, double &value, double &stdev);
SC_SYSTEM_CORE_API bool average(const std::vector<double> &values, const std::vector<double> &weights, double &value, double &stdev);

SC_SYSTEM_CORE_API void computeLinearTrend(const std::vector<float> &data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(const std::vector<double> &data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(int cnt, const float *data, double &m, double &n);
SC_SYSTEM_CORE_API void computeLinearTrend(int cnt, const double *data, double &m, double &n);

SC_SYSTEM_CORE_API void detrend(std::vector<float> &data, double m, double n);
SC_SYSTEM_CORE_API void detrend(std::vector<double> &data, double m, double n);
SC_SYSTEM_CORE_API void detrend(int cnt, float *data, double m, double n);
SC_SYSTEM_CORE_API void detrend(int cnt, double *data, double m, double n);

// From Rosenberger, J.L., and Gasko, M. (1983). "Comparing Location
// Estimators: Trimmed Means, Medians, and Trimean", in Understanding
// Robust and Exploratory Data Analysis, ed. Hoaglin, D.C., Mosteller,
// F., and Tukey, J.W., p. 297-338, John Wiley, NY

// XXX deprecated XXX
SC_SYSTEM_CORE_API double trimmedMean(const DoubleArray &, double percent=25);
SC_SYSTEM_CORE_API double trimmedMean(const std::vector<double> &f, double percent=25);
SC_SYSTEM_CORE_API double trimmedMean(int n, const double *f, double percent=25);

} // namespace Statistics
} // namespace Math
} // namespace Seiscomp

#endif
