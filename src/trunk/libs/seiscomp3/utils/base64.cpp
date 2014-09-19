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



/* Base64 conversion routines license:

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

#include <seiscomp3/utils/base64.h>
//#include <seiscomp3/utils/base64.ipp>
#include <vector>


namespace {

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

}


namespace Seiscomp {
namespace Util {


void encodeBase64(std::string &target, const char *bytes_to_encode, size_t in_len) {
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
		
			for(i = 0; (i <4) ; i++)
				target += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
	
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
	
		for (j = 0; (j < i + 1); j++)
			target += base64_chars[char_array_4[j]];
	
		while((i++ < 3))
			target += '=';
	
	}
}


void encodeBase64(std::string &target, const std::string &base64) {
	encodeBase64(target, base64.c_str(), base64.size());
}


void decodeBase64(std::string &target, const char *bytes_to_decode, size_t in_len) {
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];

	while (in_len-- && ( bytes_to_decode[in_] != '=') && is_base64(bytes_to_decode[in_])) {
		char_array_4[i++] = bytes_to_decode[in_]; in_++;
		if (i ==4) {
			for ( i = 0; i <4; ++i )
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for ( i = 0; (i < 3); ++i )
				target += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for ( j = i; j <4; ++j )
			char_array_4[j] = 0;
	
		for ( j = 0; j <4; ++j )
			char_array_4[j] = base64_chars.find(char_array_4[j]);
	
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
	
		for ( j = 0; (j < i - 1); ++j ) target += char_array_3[j];
	}
}


void decodeBase64(std::string &target, const std::string &encoded_string) {
	return decodeBase64(target, encoded_string.c_str(), encoded_string.size());
}


/*

template void encodeBase64<char>(std::string& target, const char *data, size_t data_size);
template void encodeBase64<unsigned char>(std::string& target, const unsigned char *data, size_t data_size);


template void encodeBase64<std::vector<char> >(std::string& target, const std::vector<char>&);
template void encodeBase64<std::vector<unsigned char> >(std::string& target, const std::vector<unsigned char>&);
template void encodeBase64<std::vector<int> >(std::string& target, const std::vector<int>&);


template void decodeBase64<std::basic_string<char> >(std::basic_string<char>& target, std::string& data);
template void decodeBase64<std::basic_string<unsigned char> >(std::basic_string<unsigned char>& target, std::string& data);
*/

}
}
