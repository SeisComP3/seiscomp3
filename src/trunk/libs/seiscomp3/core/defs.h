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


#ifndef __SEISCOMP_CORE_DEFS_H__
#define __SEISCOMP_CORE_DEFS_H__

#include <boost/intrusive_ptr.hpp>

#ifndef __SUNPRO_CC

namespace std {

// unary_compose (extension, not part of the standard).

template <class _Operation1, class _Operation2>
class unary_compose : public std::unary_function<typename _Operation2::argument_type,
                                            typename _Operation1::result_type> {
	protected:
		_Operation1 _M_fn1;
		_Operation2 _M_fn2;
	public:
		unary_compose(const _Operation1& __x, const _Operation2& __y)
			: _M_fn1(__x), _M_fn2(__y) {}
		typename _Operation1::result_type
		operator()(const typename _Operation2::argument_type& __x) const {
			return _M_fn1(_M_fn2(__x));
		}
};

template <class _Operation1, class _Operation2>
inline unary_compose<_Operation1,_Operation2>
compose1(const _Operation1& __fn1, const _Operation2& __fn2) {
	return unary_compose<_Operation1,_Operation2>(__fn1, __fn2);
}

}

#endif

namespace Seiscomp {
namespace Core {

template <typename T>
struct SmartPointer {
	typedef ::boost::intrusive_ptr<T> Impl;
};


template <typename B, typename D>
struct isTypeOf {
	bool operator()(B*& base) {
		return base->typeInfo().isTypeOf(D::TypeInfo());
	}

	bool operator()(boost::intrusive_ptr<B>& base) {
		return base->typeInfo().isTypeOf(D::TypeInfo());
	}
};


}
}


#define TYPEDEF_SMARTPOINTER(classname) \
    typedef Seiscomp::Core::SmartPointer<classname>::Impl classname##Ptr

#define TYPEDEF_CONST_SMARTPOINTER(classname) \
    typedef Seiscomp::Core::SmartPointer<const classname>::Impl classname##CPtr

#define DEFINE_SMARTPOINTER(classname) \
    class classname; \
    TYPEDEF_SMARTPOINTER(classname); \
    TYPEDEF_CONST_SMARTPOINTER(classname)


#endif
