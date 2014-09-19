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


#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <vector>
#include <string>
#include <iostream>


namespace Seiscomp {
namespace Util {


template<typename CharType>
void encodeBase64(std::string& target, const CharType *data, size_t data_size) {
	typedef
		boost::archive::iterators::insert_linebreaks<
			boost::archive::iterators::base64_from_binary<
				boost::archive::iterators::transform_width<
					CharType *
					,6
					,sizeof(CharType) * 8
				>
			>
			,72
		>
		translate_out;
	
	std::copy(
		translate_out(BOOST_MAKE_PFTO_WRAPPER(static_cast<const CharType*>(data))),
		translate_out(BOOST_MAKE_PFTO_WRAPPER(data + data_size)),
		std::back_inserter(target)
	);
}

template<typename Container>
void encodeBase64(std::string& target, const Container& data) {
	typedef
		boost::archive::iterators::insert_linebreaks<
			boost::archive::iterators::base64_from_binary<
				boost::archive::iterators::transform_width<
					typename Container::value_type *
					,6
					,sizeof(typename Container::value_type) * 8
				>
			>
			,72
		>
		translate_out;
	
	std::copy(
		translate_out(BOOST_MAKE_PFTO_WRAPPER(static_cast<const typename Container::value_type*>(&data[0]))),
		translate_out(BOOST_MAKE_PFTO_WRAPPER(static_cast<const typename Container::value_type*>(&data[0] + data.size()))),
		std::back_inserter(target)
	);
}


template<typename Container>
void decodeBase64(Container& target, std::string& data) {
	// convert from base64 to binary and compare with the original 
    typedef
        boost::archive::iterators::transform_width<
            boost::archive::iterators::binary_from_base64<
                boost::archive::iterators::remove_whitespace<
                    BOOST_DEDUCED_TYPENAME std::string::iterator
                >
            >,
            sizeof(typename Container::value_type) * 8,
            6
        > translate_in;
	translate_in it(data.begin());
	try {
		while ( true )
			target.push_back(*it++);
	}
	catch (...) {}
}


}
}
