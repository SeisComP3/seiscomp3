//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract 
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains 
//- certain rights in this software.
//-
//- BSD Open Source License.
//- All rights reserved.
//- 
//- Redistribution and use in source and binary forms, with or without 
//- modification, are permitted provided that the following conditions are met:
//-
//-    * Redistributions of source code must retain the above copyright notice, 
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright 
//-      notice, this list of conditions and the following disclaimer in the 
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its 
//-      contributors may be used to endorse or promote products derived from  
//-      this software without specific prior written permission.
//-
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
//- POSSIBILITY OF SUCH DAMAGE.
//-

//- ****************************************************************************
//-
//- Program:       SNL Base Utility Library (Util)
//- Module:        $RCSfile: MD50.cc,v $
//- Creator:       Lee Jensen
//- Creation Date: November 1, 2000
//- Revision:      $Revision: 1.7 $
//- Last Modified: $Date: 2011/10/07 13:14:59 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cstdio> 		/* for sprintf() */
#include <cstring>		/* for memcpy() */

// **** _LOCAL INCLUDES_ *******************************************************

#include "MD50.h"

// **** _BEGIN UTIL NAMESPACE_ *************************************************

namespace util {

// **** _STATIC INITIALIZATIONS_************************************************
// **** _FUNCTION IMPLEMENTATIONS_ *********************************************


// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Default constructor.
//
// *****************************************************************************
MD50::MD50() : md5BufRef((uint32 const*) md5Buf)
{
  md5BigEndian = false;
  if (isBigEndian()) md5BigEndian = true;
  init();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Copy constructor.
//
// *****************************************************************************
MD50::MD50(const MD50& md50) : md5BufRef((uint32 const*) md5Buf)
{
  *this = md50;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Destructor.
//
// *****************************************************************************
MD50::~MD50() 
{
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Assignment operator.
//
// *****************************************************************************
MD50& MD50::operator=(const MD50& md50)
{
  int i;

  md5BigEndian = md50.md5BigEndian;
  for (i = 0; i < 4; ++i)  md5BinKey[i] = md50.md5BinKey[i];
  for (i = 0; i < 2; ++i)  md5Bits[i]   = md50.md5Bits[i];
  for (i = 0; i < 64; ++i) md5Buf[i]    = md50.md5Buf[i];

  return *this;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Converts the 16 byte binary hash stored in the attribute
//! \em md5BinKey into a 32 byte hexadecimal hash returned as a string.
//
//******************************************************************************
const string& MD50::getMD5HashHex() const
{
  int i;
  static string s(32, '\0');
  uchar* hb = (uchar*) md5BinKey;

  for (i=0; i<16; i++) sprintf(&s[2*i], "%02x", hb[i]);
  return s;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Converts the 16 byte binary hash stored in the attribute \em md5BinKey
//! into a 32 byte hexadecimal hash stored in \em s.
//
//******************************************************************************
void MD50::getMD5HashHex(string& s) const
{
  int i;
  if (s.length() != 32) s.resize(32);
  uchar* hb = (uchar*) md5BinKey;

  for (i=0; i<16; i++) sprintf(&s[2*i], "%02x", hb[i]);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Static function that returns true if the machine type is big-endian.
//
// *****************************************************************************
bool MD50::isBigEndian()
{
  int ii = 256;
  char* ip = (char*) &ii;

  // if ip[2] = 1 then big-endian ... otherwise ip[1] = 1 which is little-endian

  if ((int) ip[2] == 1) return true;
  return false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Initialize MD5Context structure.
//
//! Set bit count to 0 and buffer to mysterious initialization constants.
//
//******************************************************************************
void MD50::init()
{
    md5BinKey[0] = 0x67452301;
    md5BinKey[1] = 0xefcdab89;
    md5BinKey[2] = 0x98badcfe;
    md5BinKey[3] = 0x10325476;

    md5Bits[0]   = 0;
    md5Bits[1]   = 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Update context to reflect the concatenation of another buffer full of bytes.
//
//******************************************************************************
void MD50::update(uchar const* data, unsigned len)
{
  uint32 t;

  // Update bitcount ... test for carry from low to high

  t = md5Bits[0];
  if ((md5Bits[0] = t + ((uint32) len << 3)) < t) md5Bits[1]++;
  md5Bits[1] += len >> 29;

  t = (t >> 3) & 0x3f; // Bytes already in shsInfo->data

  // Handle any leading odd-sized chunks

  if (t)
  {
    uchar* p = (uchar*) md5Buf + t;

    t = 64 - t;
    if (len < t)
    {
      memcpy(p, data, len);
      return;
    }
    memcpy(p, data, t);
    if (md5BigEndian) byteReverse(md5Buf, 16);
    transform();
    data += t;
    len  -= t;
  }

  // Process data in 64-byte chunks

  if (md5BigEndian)
  {
    while (len >= 64)
    {
      memcpy(md5Buf, data, 64);
      byteReverse(md5Buf, 16);
      transform();
      data += 64;
      len  -= 64;
    }
  }
  else
  {
    while (len >= 64)
    {
      memcpy(md5Buf, data, 64);
      transform();
      data += 64;
      len  -= 64;
    }
  }

  // Handle any remaining bytes of data.

  memcpy(md5Buf, data, len);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Final wrapup - pad to 64-byte boundary with the bit pattern 
//! 1 0 * (64-bit count of bits processed, MSB-first)
//
//******************************************************************************
void MD50::final()
{
  unsigned count;
  uchar *p;

  // Compute number of bytes mod 64

  count = (md5Bits[0] >> 3) & 0x3F;

  // Set the first char of padding to 0x80.  This is safe since there is
  // always at least one byte free

  p = md5Buf + count;
  *p++ = 0x80;

  // Bytes of padding needed to make 64 bytes

  count = 64 - 1 - count;

  // Pad out to 56 mod 64

  if (count < 8)
  {
    // Two lots of padding:  Pad the first block to 64 bytes

    memset(p, 0, count);
    if (md5BigEndian) byteReverse(md5Buf, 16);
    transform();

    // Now fill the next block with 56 bytes

    memset(md5Buf, 0, 56);
  }
  else
  {
    // Pad block to 56 bytes

    memset(p, 0, count - 8);
  }
  if (md5BigEndian) byteReverse(md5Buf, 14);

  // Append length in bits and transform

  ((uint32*) md5Buf)[14] = md5Bits[0];
  ((uint32*) md5Buf)[15] = md5Bits[1];

  transform();
  if (md5BigEndian) byteReverse((uchar*) md5BinKey, 4);
  //memcpy(digest, md5BinKey, 16);
  //memset(ctx, 0, sizeof(ctx)); // In case it's sensitive
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! The core of the MD5 algorithm.
//
//! This function alters an existing MD5 hash to reflect the addition
//! of 16 longwords of new data.  MD5Update blocks the data and converts
//! bytes into longwords for this routine.
//
//******************************************************************************
void MD50::transform()
{
  static md5step<f1> md51;
  static md5step<f2> md52;
  static md5step<f3> md53;
  static md5step<f4> md54;

  register uint32 a, b, c, d;

  a = md5BinKey[0];
  b = md5BinKey[1];
  c = md5BinKey[2];
  d = md5BinKey[3];

  md51(a, b, c, d, md5BufRef[0] + 0xd76aa478, 7);
  md51(d, a, b, c, md5BufRef[1] + 0xe8c7b756, 12);
  md51(c, d, a, b, md5BufRef[2] + 0x242070db, 17);
  md51(b, c, d, a, md5BufRef[3] + 0xc1bdceee, 22);
  md51(a, b, c, d, md5BufRef[4] + 0xf57c0faf, 7);
  md51(d, a, b, c, md5BufRef[5] + 0x4787c62a, 12);
  md51(c, d, a, b, md5BufRef[6] + 0xa8304613, 17);
  md51(b, c, d, a, md5BufRef[7] + 0xfd469501, 22);
  md51(a, b, c, d, md5BufRef[8] + 0x698098d8, 7);
  md51(d, a, b, c, md5BufRef[9] + 0x8b44f7af, 12);
  md51(c, d, a, b, md5BufRef[10] + 0xffff5bb1, 17);
  md51(b, c, d, a, md5BufRef[11] + 0x895cd7be, 22);
  md51(a, b, c, d, md5BufRef[12] + 0x6b901122, 7);
  md51(d, a, b, c, md5BufRef[13] + 0xfd987193, 12);
  md51(c, d, a, b, md5BufRef[14] + 0xa679438e, 17);
  md51(b, c, d, a, md5BufRef[15] + 0x49b40821, 22);

  md52(a, b, c, d, md5BufRef[1] + 0xf61e2562, 5);
  md52(d, a, b, c, md5BufRef[6] + 0xc040b340, 9);
  md52(c, d, a, b, md5BufRef[11] + 0x265e5a51, 14);
  md52(b, c, d, a, md5BufRef[0] + 0xe9b6c7aa, 20);
  md52(a, b, c, d, md5BufRef[5] + 0xd62f105d, 5);
  md52(d, a, b, c, md5BufRef[10] + 0x02441453, 9);
  md52(c, d, a, b, md5BufRef[15] + 0xd8a1e681, 14);
  md52(b, c, d, a, md5BufRef[4] + 0xe7d3fbc8, 20);
  md52(a, b, c, d, md5BufRef[9] + 0x21e1cde6, 5);
  md52(d, a, b, c, md5BufRef[14] + 0xc33707d6, 9);
  md52(c, d, a, b, md5BufRef[3] + 0xf4d50d87, 14);
  md52(b, c, d, a, md5BufRef[8] + 0x455a14ed, 20);
  md52(a, b, c, d, md5BufRef[13] + 0xa9e3e905, 5);
  md52(d, a, b, c, md5BufRef[2] + 0xfcefa3f8, 9);
  md52(c, d, a, b, md5BufRef[7] + 0x676f02d9, 14);
  md52(b, c, d, a, md5BufRef[12] + 0x8d2a4c8a, 20);

  md53(a, b, c, d, md5BufRef[5] + 0xfffa3942, 4);
  md53(d, a, b, c, md5BufRef[8] + 0x8771f681, 11);
  md53(c, d, a, b, md5BufRef[11] + 0x6d9d6122, 16);
  md53(b, c, d, a, md5BufRef[14] + 0xfde5380c, 23);
  md53(a, b, c, d, md5BufRef[1] + 0xa4beea44, 4);
  md53(d, a, b, c, md5BufRef[4] + 0x4bdecfa9, 11);
  md53(c, d, a, b, md5BufRef[7] + 0xf6bb4b60, 16);
  md53(b, c, d, a, md5BufRef[10] + 0xbebfbc70, 23);
  md53(a, b, c, d, md5BufRef[13] + 0x289b7ec6, 4);
  md53(d, a, b, c, md5BufRef[0] + 0xeaa127fa, 11);
  md53(c, d, a, b, md5BufRef[3] + 0xd4ef3085, 16);
  md53(b, c, d, a, md5BufRef[6] + 0x04881d05, 23);
  md53(a, b, c, d, md5BufRef[9] + 0xd9d4d039, 4);
  md53(d, a, b, c, md5BufRef[12] + 0xe6db99e5, 11);
  md53(c, d, a, b, md5BufRef[15] + 0x1fa27cf8, 16);
  md53(b, c, d, a, md5BufRef[2] + 0xc4ac5665, 23);

  md54(a, b, c, d, md5BufRef[0] + 0xf4292244, 6);
  md54(d, a, b, c, md5BufRef[7] + 0x432aff97, 10);
  md54(c, d, a, b, md5BufRef[14] + 0xab9423a7, 15);
  md54(b, c, d, a, md5BufRef[5] + 0xfc93a039, 21);
  md54(a, b, c, d, md5BufRef[12] + 0x655b59c3, 6);
  md54(d, a, b, c, md5BufRef[3] + 0x8f0ccc92, 10);
  md54(c, d, a, b, md5BufRef[10] + 0xffeff47d, 15);
  md54(b, c, d, a, md5BufRef[1] + 0x85845dd1, 21);
  md54(a, b, c, d, md5BufRef[8] + 0x6fa87e4f, 6);
  md54(d, a, b, c, md5BufRef[15] + 0xfe2ce6e0, 10);
  md54(c, d, a, b, md5BufRef[6] + 0xa3014314, 15);
  md54(b, c, d, a, md5BufRef[13] + 0x4e0811a1, 21);
  md54(a, b, c, d, md5BufRef[4] + 0xf7537e82, 6);
  md54(d, a, b, c, md5BufRef[11] + 0xbd3af235, 10);
  md54(c, d, a, b, md5BufRef[2] + 0x2ad7d2bb, 15);
  md54(b, c, d, a, md5BufRef[9] + 0xeb86d391, 21);

  md5BinKey[0] += a;
  md5BinKey[1] += b;
  md5BinKey[2] += c;
  md5BinKey[3] += d;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Reverses big-endian to little endian
//
//! Note: this code is harmless on little-endian machines (Really?).
//
//******************************************************************************
void MD50::byteReverse(uchar* buf, unsigned n)
{
  uint32 t;
  do
  {
    t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
        ((unsigned) buf[1] << 8 | buf[0]);
    *(uint32 *) buf = t;
    buf += 4;
  } while (--n);
}

} // end namespace util
