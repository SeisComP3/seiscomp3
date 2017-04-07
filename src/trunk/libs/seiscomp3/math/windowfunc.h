/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker, gempa GmbH                                        *
 ***************************************************************************/


#ifndef __SEISCOMP_MATH_WINDOWFUNC_H__
#define __SEISCOMP_MATH_WINDOWFUNC_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core.h>

#include <vector>


namespace Seiscomp {
namespace Math {


template <typename TYPE>
class SC_SYSTEM_CORE_API WindowFunc : public Core::BaseObject {
	public:
		virtual ~WindowFunc();


	public:
		/**
		 * @brief Applies the window function to the given data
		 * @param n Number of samples
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(int n, TYPE *inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(std::vector<TYPE> &inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data array where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(TypedArray<TYPE> &inout, double width = 0.5) const;

		/**
		 * @brief Applies the window function to the given data
		 * @param inout The data array where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		void apply(TypedArray<TYPE> *inout, double width = 0.5) const;


	protected:
		/**
		 * @brief Applies the window function to the given data. This method has
		 *        to be implemented by derived classes. It is called by all
		 *        apply variants.
		 * @param n Number of samples
		 * @param inout The data vector where each sample is multiplied with
		 *              the respective sample of the window function.
		 * @param width The width of the window function. The default is 0.5
		 *              which means 50% at either side. 0.1 would mean that the
		 *              left half of the window function is applied on 10% of
		 *              the left portion of the data vector and the right half
		 *              of the window function is applied on the right 10% of
		 *              the data vector. The value is clipped into range [0,0.5].
		 */
		virtual void process(int n, TYPE *inout, double width = 0.5) const = 0;
};


}
}


#endif
