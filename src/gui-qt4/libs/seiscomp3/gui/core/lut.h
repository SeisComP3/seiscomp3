/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_LUT_H__
#define __SEISCOMP_GUI_LUT_H__


#include <seiscomp3/gui/core/gradient.h>
#include <QRgb>


namespace Seiscomp {
namespace Gui {


template<typename Key, typename Value>
class LUT {
	public:
		typedef Key   key_type;
		typedef Value value_type;


	protected:
		LUT(int n, Value *values) : _size(n), _lut(values) {}


	public:
		int size() const { return _size; }

		Key lowerBound() const { return _lowerBound; }
		Key upperBound() const { return _upperBound; }

		void setRange(const Key &lower, const Key &upper) {
			_lowerBound = lower;
			_upperBound = upper;
			_scale = (_size-1)/(upper-lower);
		}

		void setValue(int idx, const Value &v) {
			if ( idx < 0 || idx >= _size ) return;
			_lut[idx] = v;
		}

		const Value &valueAt(Key v) const {
			int idx = (int)((v-_lowerBound)*_scale);
			if ( idx < 0 ) idx = 0;
			else if ( idx >= _size ) idx = _size-1;

			return _lut[idx];
		}

		const Value &valueAtNormalizedIndex(Key v) const {
			int idx = (int)(v*(_size-1));
			if ( idx < 0 ) idx = 0;
			else if ( idx >= _size ) idx = _size-1;

			return _lut[idx];
		}

		Key indexToKey(int idx) const {
			return idx / _scale + _lowerBound;
		}

		Value &operator[](int idx) {
			return _lut[idx];
		}

		const Value &operator[](int idx) const {
			return _lut[idx];
		}


	protected:
		Key    _lowerBound;
		Key    _upperBound;
		Key    _scale;
		int    _size;
		Value *_lut;
};


template <typename Key, typename Value, int N>
class StaticLUT : public LUT<Key,Value> {
	public:
		StaticLUT() : LUT<Key,Value>(N, _staticLUT) {}

	protected:
		Value _staticLUT[N];
};


template <int N>
class StaticColorLUT : public StaticLUT<double, QRgb, N> {
	public:
		StaticColorLUT() {}
		StaticColorLUT(const Gradient &gradient) {
			generateFrom(gradient);
		}

	public:
		bool setRangeFrom(const Gradient &gradient) {
			if ( gradient.empty() ) return false;

			double lower =  gradient.begin().key();
			double upper = (--gradient.end()).key();
			StaticLUT<double, QRgb, N>::setRange(lower, upper);

			return true;
		}

		void generateFrom(const Gradient &gradient) {
			if ( !setRangeFrom(gradient) ) return;
			for ( int i = 0; i < N; ++i )
				StaticLUT<double, QRgb, N>::_staticLUT[i] =
					gradient.colorAt(StaticLUT<double, QRgb, N>::indexToKey(i)).rgba();
		}
};


}
}


#endif
