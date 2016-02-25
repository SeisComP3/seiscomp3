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
 ***************************************************************************/


#ifndef __SEISCOMP_CORE_BITSET_H__
#define __SEISCOMP_CORE_BITSET_H__


#include <seiscomp3/core/baseobject.h>
#include <boost/dynamic_bitset.hpp>


namespace Seiscomp {


DEFINE_SMARTPOINTER(BitSet);


/**
 * @brief The BitSet class represents a set of bits. It provides accesses to
 *        the value of individual bits via an operator[] and provides all of
 *        the bitwise operators that one can apply to builtin integers, such as
 *        operator& and operator<<. The number of bits in the set is specified
 *        at runtime via a parameter to the constructor of the BitSet.
 *
 * The implementation actually wraps the boost::dynamic_bitset [1]
 * implementation. That has been done to avoid multiple inheritance for the
 * Python wrappers and to have a consistent naming scheme of the class.
 *
 * [1] http://www.boost.org/doc/libs/1_33_1/libs/dynamic_bitset/dynamic_bitset.html
 */
class SC_SYSTEM_CORE_API BitSet : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(BitSet);

	// ----------------------------------------------------------------------
	//  Type traits
	// ----------------------------------------------------------------------
	public:
		typedef boost::dynamic_bitset<> ImplType;
		typedef ImplType::reference ReferenceType;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		BitSet();

		//! Constructs a bitset with n bits
		BitSet(int n);

		//! Copy constructor
		BitSet(const BitSet &other);

		//! Construct from implementation type
		BitSet(const ImplType &impl);


	// ----------------------------------------------------------------------
	//  Operators
	// ----------------------------------------------------------------------
	public:
		BitSet &operator=(const BitSet &other);

		operator ImplType&();
		operator const ImplType&() const;

		ReferenceType operator[](size_t pos);
		bool operator[](size_t pos) const;

		//! Bitwise-AND all the bits in rhs with the bits in this bitset.
		//! Requires this->size() == rhs.size().
		BitSet &operator&=(const BitSet &b);

		//! Bitwise-OR's all the bits in rhs with the bits in this bitset.
		//! Requires this->size() == rhs.size().
		BitSet &operator|=(const BitSet &b);

		//! Bitwise-XOR's all the bits in rhs with the bits in this bitset.
		//! Requires this->size() == rhs.size().
		BitSet &operator^=(const BitSet &b);

		//! Computes the set difference of this bitset and the rhs bitset.
		//! Requires this->size() == rhs.size().
		BitSet &operator-=(const BitSet &b);

		//! Shifts the bits in this bitset to the left by n bits. For each bit
		//! in the bitset, the bit at position pos takes on the previous value
		//! of the bit at position pos - n, or zero if no such bit exists.
		BitSet &operator<<=(size_t n);

		//! Shifts the bits in this bitset to the right by n bits. For each bit
		//! in the bitset, the bit at position pos takes on the previous value
		//! of bit pos + n, or zero if no such bit exists.
		BitSet &operator>>=(size_t n);

		//! Returns s copy of *this shifted to the left by n bits. For each bit
		//! in the returned bitset, the bit at position pos takes on the value
		//! of the bit at position pos - n of this bitset, or zero if no such
		//! bit exists.
		BitSet operator<<(size_t n) const;

		//! Returns a copy of *this shifted to the right by n bits. For each bit
		//! in the returned bitset, the bit at position pos takes on the value
		//! of the bit at position pos + n of this bitset, or zero if no such
		//! bit exists.
		BitSet operator>>(size_t n) const;

		//! Returns a copy of *this with all of its bits flipped.
		BitSet operator~() const;


	// ----------------------------------------------------------------------
	//  Methods
	// ----------------------------------------------------------------------
	public:
		//! Changes the number of bits of the bitset to num_bits. If
		//! num_bits > size() then the bits in the range [0,size()) remain the
		//! same, and the bits in [size(),num_bits) are all set to value. If
		//! num_bits < size() then the bits in the range [0,num_bits) stay the
		//! same (and the remaining bits are discarded).
		void resize(size_t num_bits, bool value = false);

		//! The size of the bitset becomes zero.
		void clear();
		void append(bool bit);

		//! Sets bit n if val is true, and clears bit n if val is false.
		BitSet &set(size_t n, bool val = true);

		//! Sets every bit in this bitset to 1.
		BitSet &set();

		//! Clears bit n.
		BitSet &reset(size_t n);

		//! Clears every bit in this bitset.
		BitSet &reset();

		//! Flips bit n.
		BitSet &flip(size_t n);

		//! Flips the value of every bit in this bitset.
		BitSet &flip();

		//! Returns true if bit n is set and false is bit n is 0.
		bool test(size_t n) const;

		//! Returns true if any bits in this bitset are set, and otherwise
		//! returns false.
		bool any() const;

		//! Returns true if no bits are set, and otherwise returns false.
		bool none() const;

		//! Returns the number of bits in this bitset that are set.
		size_t numberOfBitsSet() const;

		//! Returns the numeric value corresponding to the bits in *this.
		//! Throws std::overflow_error if that value is too large to be
		//! represented in an unsigned long, i.e. if *this has any non-zero bit
		//! at a position >= std::numeric_limits<unsigned long>::digits.
		unsigned long toUlong() const;

		//! Returns the number of bits in this bitset.
		size_t size() const;

		//! Returns the number of blocks in this bitset.
		size_t numberOfBlocks() const;

		//! Returns the maximum size of a BitSet object having the same type
		//! as *this. Note that if any BitSet operation causes size() to exceed
		//! maxSize() then the behavior is undefined.
		size_t maximumSize() const;

		//! Returns true if this->size() == 0, false otherwise. Note: not to be
		//! confused with none(), that has different semantics.
		bool empty() const;

		//! Returns the lowest index i such as bit i is set, or npos if *this
		//! has no on bits.
		size_t findFirst() const;

		//! Returns the lowest index i greater than pos such as bit i is set,
		//! or npos if no such index exists.
		size_t findNext(size_t pos) const;

		//! Returns the boost::dynamic_bitset implementation instance
		const ImplType &impl() const;

		//! Returns the boost::dynamic_bitset implementation instance
		ImplType &impl();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		ImplType _impl;
};


}


#include <seiscomp3/core/bitset.ipp>


#endif
