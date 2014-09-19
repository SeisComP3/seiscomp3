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


#ifndef __SEISCOMP_MATH_CHAINFILTER__
#define __SEISCOMP_MATH_CHAINFILTER__


#include <seiscomp3/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename TYPE>
class ChainFilter : public InPlaceFilter<TYPE>
{
	public:
		ChainFilter();
		~ChainFilter();

	// ------------------------------------------------------------------
	//  Interface
	// ------------------------------------------------------------------
	public:
		//! Adds a filter to the chain. The ownership of the filter to
		//! be added goes the ChainFilter instance which will delete
		//! the filter.
		bool add(InPlaceFilter<TYPE> *filter);

		//! Removes and deletes the filter at a certain position in
		//! the chain.
		bool remove(size_t pos);

		//! Removes the filter at position pos and returns it. The
		//! filter instance will not be deleted and the ownership goes
		//! to the caller.
		InPlaceFilter<TYPE>* take(size_t pos);

		//! Returns the index of a filter in the chain or -1 if not
		//! found
		size_t indexOf(InPlaceFilter<TYPE> *filter) const;

		//! Returns the number of filters in the chain
		size_t filterCount() const;


	// ------------------------------------------------------------------
	//  Derived filter interface
	// ------------------------------------------------------------------
	public:
		virtual void apply(int n, TYPE *inout);

		virtual void setStartTime(const Core::Time &time);

		virtual void setStreamID(const std::string &net,
		                         const std::string &sta,
		                         const std::string &loc,
		                         const std::string &cha);

		virtual void setSamplingFrequency(double fsamp);
		virtual int setParameters(int n, const double *params);

		virtual InPlaceFilter<TYPE>* clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		typedef std::vector<InPlaceFilter<TYPE>*> FilterChain;
		FilterChain _filters;
};


} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp


#endif
