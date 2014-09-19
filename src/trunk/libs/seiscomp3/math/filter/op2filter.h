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


#ifndef __SEISCOMP_MATH_OP2FILTER__
#define __SEISCOMP_MATH_OP2FILTER__


#include <seiscomp3/math/filter.h>


namespace Seiscomp
{
namespace Math
{
namespace Filtering
{

template<typename TYPE, template <class T> class OPERATION>
class Op2Filter : public InPlaceFilter<TYPE>
{
	public:
		Op2Filter(InPlaceFilter<TYPE> *op1, InPlaceFilter<TYPE> *op2, double fsamp=0.0);
		~Op2Filter();

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
		virtual int setParameters(int n, double const *params);

		virtual InPlaceFilter<TYPE>* clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		InPlaceFilter<TYPE> *_op1;
		InPlaceFilter<TYPE> *_op2;
};


#include <seiscomp3/math/filter/op2filter.ipp>


} // namespace Seiscomp::Math::Filter

} // namespace Seiscomp::Math

} // namespace Seiscomp


#endif
