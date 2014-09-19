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


#ifndef __SEISCOMP_CORE_OPTIONAL_H__
#define __SEISCOMP_CORE_OPTIONAL_H__

#include <seiscomp3/core.h>

#include <exception>
#include <boost/optional.hpp>
#include <boost/none.hpp>

namespace Seiscomp {
namespace Core {

/** \brief Redefines boost::optional<T>
  * Optional values can be set or unset.
  * \code
  *   void print(const Optional<int>::Impl& v) {
  *     if ( !v )
  *       cout << "value of v is not set" << endl;
  *     else
  *       cout << *v << endl;
  *   }
  *
  *   Optional<int>::Impl a = 5;
  *   print(a);  // output: "5"
  *   a = None;
  *   print(a);  // output: "value of v is not set"
  * \endcode
  */
template <typename T>
struct Optional {
	typedef ::boost::optional<T> Impl;
};

/** Defines None */
SC_SYSTEM_CORE_API extern ::boost::none_t const None;

template <typename T>
T value(const boost::optional<T>&);

class SC_SYSTEM_CORE_API ValueError : public std::exception {
	public:
		ValueError() throw();
		~ValueError() throw();

	public:
		const char* what() const throw();
};

/** Macro to use optional values easily */
#define OPT(T) Seiscomp::Core::Optional<T>::Impl
/** Macro to use optional values as const reference */
#define OPT_CR(T) const Seiscomp::Core::Optional<T>::Impl&

#include <seiscomp3/core/optional.inl>

}
}

#endif
