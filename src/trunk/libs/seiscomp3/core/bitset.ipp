namespace Seiscomp {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::BitSet() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::BitSet(int n) : _impl(n) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::BitSet(const BitSet &other) : _impl(other._impl) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::BitSet(const ImplType &impl) : _impl(impl) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet &BitSet::operator=(const BitSet &other) {
	_impl = other._impl;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::operator boost::dynamic_bitset<>&() {
	return _impl;
}

inline BitSet::operator const boost::dynamic_bitset<>&() const {
	return _impl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet::ReferenceType BitSet::operator[](size_t pos) {
	return _impl[pos];
}

inline bool BitSet::operator[](size_t pos) const {
	return _impl[pos];
}

inline BitSet &BitSet::operator&=(const BitSet &b) {
	_impl &= b._impl;
	return *this;
}

inline BitSet &BitSet::operator|=(const BitSet &b) {
	_impl |= b._impl;
	return *this;
}

inline BitSet &BitSet::operator^=(const BitSet &b) {
	_impl ^= b._impl;
	return *this;
}

inline BitSet &BitSet::operator-=(const BitSet &b) {
	_impl -= b._impl;
	return *this;
}

inline BitSet &BitSet::operator<<=(size_t n) {
	_impl <<= n;
	return *this;
}

inline BitSet &BitSet::operator>>=(size_t n) {
	_impl >>= n;
	return *this;
}

inline BitSet BitSet::operator<<(size_t n) const {
	return BitSet(_impl << n);
}

inline BitSet BitSet::operator>>(size_t n) const {
	return BitSet(_impl >> n);
}

inline BitSet BitSet::operator~() const {
	return BitSet(~_impl);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void BitSet::resize(size_t num_bits, bool value) {
	_impl.resize(num_bits, value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void BitSet::clear() {
	_impl.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline void BitSet::append(bool bit) {
	_impl.append(bit);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline BitSet &BitSet::set(size_t n, bool val) {
	_impl.set(n, val);
	return *this;
}

inline BitSet &BitSet::set() {
	_impl.set();
	return *this;
}

inline BitSet &BitSet::reset(size_t n) {
	_impl.reset(n);
	return *this;
}

inline BitSet &BitSet::reset() {
	_impl.reset();
	return *this;
}

inline BitSet &BitSet::flip(size_t n) {
	_impl.flip(n);
	return *this;
}

inline BitSet &BitSet::flip() {
	_impl.flip();
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline bool BitSet::test(size_t n) const {
	return _impl.test(n);
}

inline bool BitSet::any() const {
	return _impl.any();
}

inline bool BitSet::none() const {
	return _impl.none();
}

inline size_t BitSet::numberOfBitsSet() const {
	return _impl.count();
}

inline unsigned long BitSet::toUlong() const {
	return _impl.to_ulong();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline size_t BitSet::size() const {
	return _impl.size();
}

inline size_t BitSet::numberOfBlocks() const {
	return _impl.num_blocks();
}

inline size_t BitSet::maximumSize() const {
	return _impl.max_size();
}

inline bool BitSet::empty() const {
	return _impl.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline size_t BitSet::findFirst() const {
	return _impl.find_first();
}

inline size_t BitSet::findNext(size_t pos) const {
	return _impl.find_next(pos);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const BitSet::ImplType &BitSet::impl() const {
	return _impl;
}

inline BitSet::ImplType &BitSet::impl() {
	return _impl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
