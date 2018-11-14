namespace Seiscomp {
namespace Util {


template <class T, class A>
T join(const A &begin, const A &end, const T &glue) {
	T result;
	bool first = true;
	for ( A it = begin; it != end; ++it ) {
		if ( first )
			first = false;
		else
			result += glue;
		result += *it;
	}
	return result;
}


template <typename T>
void toHex(std::string &out, T v) {
	const unsigned char *bytes = reinterpret_cast<const unsigned char*>(&v);
	for ( int i = sizeof(v)-1; i >= 0; --i )
		toHex(out, bytes[i]);
}


template <>
inline void toHex<unsigned char>(std::string &out, unsigned char v) {
	out += HEXCHARS[v >> 4];
	out += HEXCHARS[v & 0x0F];
}


template <>
inline void toHex<char>(std::string &out, char v) {
	toHex(out, (unsigned char)v);
}


}
}
