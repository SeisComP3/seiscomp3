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


#ifndef __SEISCOMP_FILTERING_FILTER_H__
#define __SEISCOMP_FILTERING_FILTER_H__

#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{


class SC_SYSTEM_CORE_API AlignmentError : public std::exception
{
  public:
        AlignmentError(const char *txt)  { _txt = txt; }
        const char* what() const throw() { return _txt; }
  private:
	const char *_txt;
};

// virtual base class that all filter classes should be derived from

template<typename TYPE>
class SC_SYSTEM_CORE_API InPlaceFilter : public Core::BaseObject {
	public:
		virtual ~InPlaceFilter() { }

	public:
		/**
		 * Sets the start time of the data to be filtered.
		 * This is not handled in the base class but can be used
		 * in derived classes to apply filters depending on
		 * this information. This function should be called before
		 * apply is called the first time.
		 */
		virtual void setStartTime(const Core::Time &time) {}

		/**
		 * Sets the streamID of the data to be filtered.
		 * This is not handled in the base class but can be used
		 * in derived classes to apply filters depending on
		 * this information. This function should be called before
		 * apply is called the first time.
		 */
		virtual void setStreamID(const std::string &net,
		                         const std::string &sta,
		                         const std::string &loc,
		                         const std::string &cha) {}

		virtual void setSamplingFrequency(double fsamp) = 0;

		/**
		 * Sets the filter parameters
		 * @param n The number of given parameters
		 * @param params The parameter list.
		 * @return Positive value: the number of required parameters or,
		 *         negative value: a value error at parameter position abs(return)-1
		 */
		virtual int setParameters(int n, const double *params) = 0;

		virtual void apply(int n, TYPE *inout) = 0;

		void apply(std::vector<TYPE> &f) {
			apply(f.size(), &f[0]);
		}

		void apply(TypedArray<TYPE> &arr) {
			apply(arr.size(), arr.typedData());
		}

		// EXPERIMENTAL
		virtual void handleGap(int n=0) {}

		//! Creates a new filter instance having the same
		//! type as the called instance. The configuration
		//! parameters will be copied, too but not the internal
		//! filter state depending on former input data
		virtual InPlaceFilter<TYPE>* clone() const = 0;

		//! Creates a new filter by name. The user is responsible to
		//! release the memory
		//! pointed to by the return value.
		//! \return The corresponding filter or NULL
		static InPlaceFilter<TYPE> *Create(const std::string &strFilter,
		                                   std::string *strError = NULL);
};


DEFINE_TEMPLATE_INTERFACE_FACTORY(InPlaceFilter);

#define REGISTER_INPLACE_FILTER(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Math::Filtering::InPlaceFilter<float>, Class<float> > __##Class##FloatInterfaceFactory__(Service); \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Math::Filtering::InPlaceFilter<double>, Class<double> > __##Class##DoubleInterfaceFactory__(Service)

#define INSTANTIATE_INPLACE_FILTER(Class, APIDef) \
template class APIDef Class<float>; \
template class APIDef Class<double>


//! Dummy filter that does nothing but can be used in more complex
//! filters like Op2Filter.
template<typename TYPE>
class SC_SYSTEM_CORE_API SelfFilter : public InPlaceFilter<TYPE> {
	public:
		SelfFilter() {}

	public:
		virtual void setSamplingFrequency(double fsamp) {}
		virtual int setParameters(int n, const double *params) { return 0; }

		virtual void apply(int n, TYPE *inout) {}

		virtual InPlaceFilter<TYPE>* clone() const { return new SelfFilter(); }
};


template <class TYPE>
int minmax(std::vector<TYPE> const &f, int i1, int i2, int *imax, TYPE *fmax);

template <class TYPE>
int find_max(std::vector<TYPE> const &f, int i1, int i2, int *imax, TYPE *fmax);

template <typename TYPE>
int rotate(std::vector<TYPE> &f1, std::vector<TYPE> &f2, double phi);

template<typename TYPE>
int decompose(std::vector<TYPE> &f1, std::vector<TYPE> &f2,
              double p, double vs, double sigma);

template<typename TYPE>
void hilbert_transform(std::vector<TYPE> &trace, int direction=1);
template<typename TYPE>
void envelope(std::vector<TYPE> &trace);

SC_SYSTEM_CORE_API long next_power_of_2(long);

template<typename TYPE>
void cosRamp(std::vector<TYPE> &ramp, TYPE f1, TYPE f2);

} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp


#include<seiscomp3/math/hilbert.ipp>
#include<seiscomp3/math/minmax.ipp>
#include<seiscomp3/math/decomp.ipp>

#endif
