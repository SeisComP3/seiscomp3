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
//- Module:        $RCSfile: MD50.h,v $
//- Creator:       Lee Jensen
//- Creaton Date:  January 20, 2001
//- Revision:      $Revision: 1.6 $
//- Last Modified: $Date: 2012/10/25 00:25:13 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

#ifndef MD50_H
#define MD50_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <string>

// **** _LOCAL INCLUDES_ *******************************************************

#include "UtilGlobals.h"

using namespace std;
// use standard library objects

// **** _BEGIN UTIL NAMESPACE_ *************************************************

namespace util {

typedef unsigned int  uint32;
typedef unsigned char uchar;
typedef const unsigned char cuchar;

// **** _CLASS DESCRIPTION_ ****************************************************
//
//! \brief This code implements the MD5 message-digest algorithm.
//! The algorithm is due to Ron Rivest.  This code was
//! written by Colin Plumb in 1993, no copyright is claimed.
//! This code is in the public domain; do with it what you wish.
//
//! The MD5 code was converted to an MD5 C++ object (MD50) by Jim Hipp,
//! Sandia National Laboratories, in May of 2004. The code was modified
//! Specifically for the PGL library and optimized to replace all "define"
//! definitions with equivalent inlined function objects. The machine
//! dependent byte reversal flag is called automatically at object
//! construction to turn byte-reversal ON (big-endian) or OFF (little-endian).
//! The flag can be manually set to either-endian format with the functions
//! setBigEndian() or setLittleEndian().
//!
//! The 32 byte hexadecimal string message digest is computed with any of the
//! functions getMD5HashHex(...). The equivalent 16 byte binary message digest
//! is returned with the functions getMD5BinHex(...).
//
//******************************************************************************
class UTIL_EXP_IMP MD50
{
  public:

    // **** _PUBLIC LIFECYCLES_ ************************************************

    MD50();
    //- Default constructor.

    MD50(const MD50& md50);
    //- Copy constructor.

    virtual ~MD50();
    //- Virtual destructor.

    // **** _PUBLIC OPERATORS_ *************************************************

    MD50&            operator=(const MD50& md50);
    //- Assignment operator.

    // **** _PUBLIC METHODS_ ***************************************************

    const string&    getMD5HashHex(uchar const* data, int sze);
    void             getMD5HashHex(uchar const* data, int sze, string& hhstr)
{
  // create the md5 binary hash string in hashbin and convert to hexadecimal
  // in hashkey

  getMD5HashBin(data, sze);
  getMD5HashHex(hhstr);
}
    void             getMD5HashHex(const string& sin, string& hhstr);
    //- Performs the md5 conversion from the input data into a 32 byte
    //- hexadecimal string and returns the string in hhstr (the last two
    //- functions).

    const string&    getMD5HashHex() const;
    void             getMD5HashHex(string& s) const;
    //- Converts the input MD5 16 byte binary hash key into a 32 byte
    //- hexadecimal hash key. These functions assume the MD5 conversion has
    //- already occurred.

    cuchar const*    getMD5HashBin(uchar const* data, int sze);
    //- Performs the md5 conversion from the input data into the internal 16
    //- byte binary hash key and returns the key.

    cuchar const*    getMD5HashBin();
    //- Returns the 16 byte binary hash key. This function assumes the MD5
    //- conversion has already occurred.

    static int       hexStringSize();
    static int       hexSize();
    static int       binSize();
    //- functions to return the hex string (32 + 4), hex (32), and bin (16)
    //- size of the MD5 hash key.

    void             setDefaultEndian();
    void             setBigEndian();
    void             setLittleEndian();
    bool             getByteReverse() const;
    //- Sets/gets the big-endian flag. This flag is generally set to true for
    //- big-endian machines.

    static bool      isBigEndian();
    //- Static function that returns true if the machine type is big-endian.

  private:

    // **** _PRIVATE METHODS_ **************************************************

    void             init();
    //- Reinitializes this MD50 object

    void             update(uchar const *data, unsigned len);
    //- Loops through all bytes in the input buffer (md5BinKey) 64 at a time
    //- upto but short of len. The remaining bytes are processed by final.

    void             final();
    //- Performs the final required padding of the md5 algorithm.

    void             transform();
    //- The core of the MD5 algorithm. This function alters an existing MD5
    //- hash to reflect the addition of 16 longwords of new data. update()
    //- blocks the data and converts bytes into longwords for this routine.

    void             byteReverse(uchar* buf, unsigned n);
    //- Reverses big-endian to little endian. Note: this code is harmless on
    //- little-endian machines (Really?).

    // **** _PRIVATE DATA_ *****************************************************

    //! If true byte reversal occurs (Big-Endian).
    bool             md5BigEndian;

    //! The 16 byte buffer that contains hash key after calling getMD5HashBin.
    uint32           md5BinKey[4];

    //! Used by md5 for padding operations
    uint32           md5Bits[2];

    //! \brief The temporary buffer that contains 64 bytes of the string for
    //! which the key is generated. These 64 bytes are processed to modify
    //! the values in md5BinKey. The entire input string is processed 64 bytes
    //! at a time through this buffer.
    uchar            md5Buf[64];

    //! \brief A recast reference to md5Buf that is used in the function
    //! transform() to perform the core MD5 algorithmic transformation.
    uint32 const*    md5BufRef;

};

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Returns the hexadecimal size of an MD5 hash key.
//
//******************************************************************************
inline int MD50::hexStringSize()
{
  return hexSize() + sizeof(int);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Returns the hexadecimal size of an MD5 hash key.
//
//******************************************************************************
inline int MD50::hexSize()
{
  return 32;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Returns the binary size of an MD5 hash key.
//
//******************************************************************************
inline int MD50::binSize()
{
  return 16;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Sets the big endian flag to \em br.
//
//******************************************************************************
inline void MD50::setDefaultEndian()
{
  if (isBigEndian())
    md5BigEndian = true;
  else
    md5BigEndian = false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Sets the big endian flag to true.
//
//******************************************************************************
inline void MD50::setBigEndian()
{
  md5BigEndian = true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Sets the big endian flag to false.
//
//******************************************************************************
inline void MD50::setLittleEndian()
{
  md5BigEndian = false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Returns the big endian flag setting.
//
//******************************************************************************
inline bool MD50::getByteReverse() const
{
  return md5BigEndian;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Given an input \em data array and size \em sze, this function
//! creates a 32 byte hexadecimal MD5 hash key from the data and returns the
//! result as a string. No checking is performed on the size of the input array
//! \em data.
//
//******************************************************************************
inline const string& MD50::getMD5HashHex(uchar const* data, int sze)
{
  // create the md5 binary hash string in hashbin and convert to hexadecimal
  // in hashkey

  getMD5HashBin(data, sze);
  return getMD5HashHex();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Given an input \em data array and size \em sze, this function
//! creates a 32 byte hexadecimal MD5 hash key from the data and returns the
//! result in \em hhstr. No checking is performed on the size of the input
//! array \em data.
//
//******************************************************************************
//inline void MD50::getMD5HashHex(uchar const* data, int sze, string& hhstr)
//{
//  // create the md5 binary hash string in hashbin and convert to hexadecimal
//  // in hashkey
//
//  getMD5HashBin(data, sze);
//  getMD5HashHex(hhstr);
//}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Given an input string \em sin, this function creates a 32 byte
//! hexadecimal MD5 hash key from the strings data and returns the
//! result in \em hhstr.
//
//******************************************************************************
inline void MD50::getMD5HashHex(const string& sin, string& hhstr)
{
  // create the md5 binary hash string in hashbin and converts to hexadecimal
  // in hashkey

  uchar const* data = (uchar const*) &sin[0];
  getMD5HashBin(data, sin.size());
  getMD5HashHex(hhstr);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Given an input \em data array and size \em sze, this function
//! creates an MD5 hash key from the data and stores the 16 byte binary
//! key in the attribute array \em hashkey. No checking is performed on the
//! size of the array \em data.
//
//******************************************************************************
inline cuchar const* MD50::getMD5HashBin(uchar const* data, int sze)
{
  init();
  update(data, sze);
  final();
  return getMD5HashBin();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Returns the 16 byte binary hash key. This function assumes that the
//! MD5 conversion has already been performed.
//
//******************************************************************************
inline cuchar const* MD50::getMD5HashBin()
{
  return (cuchar* const) md5BinKey;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Primary MD5 algorithmic step function object. The function object is
//! templated (class f) off of the four MD5 functions f1, f2, f3, and f4
//! defined below.
//
//******************************************************************************
template<class f>
class md5step
{
  public:

    //! \brief overloaded function operator
    void             operator() (uint32& w, uint32 x, uint32 y, uint32 z,
                                 uint32 data, uint32 s)
    {
      static f fnc;
      w += fnc(x, y, z) + data;
      w = w << s | w >> (32 - s);
      w += x;
    };
};

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief First MD5 algorithmic operation function object. Optimized from
//! x & y | ~x & z
//
//******************************************************************************
class f1
{
  public:

    //! \brief overloaded function operator
    uint32           operator() (uint32 x, uint32 y, uint32 z)
    {
      return z ^ (x & (y ^ z));
    };
};

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Second MD5 algorithmic operation function object.
//
//******************************************************************************
class f2
{
  public:

    //! \brief overloaded function operator
    uint32           operator() (uint32 x, uint32 y, uint32 z)
    {
      //5return f1(z, x, y);
      return y ^ (z & (x ^ y));
    };
};

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Third MD5 algorithmic operation function object.
//
//******************************************************************************
class f3
{
  public:

    //! \brief overloaded function operator
    uint32           operator() (uint32 x, uint32 y, uint32 z)
    {
      return x ^ y ^ z;
    };
};

// **** _FUNCTION DESCRIPTION_ *************************************************
// 
//! \brief Fourth MD5 algorithmic operation function object.
//
//******************************************************************************
class f4
{
  public:

    //! \brief overloaded function operator
    uint32           operator() (uint32 x, uint32 y, uint32 z)
    {
      return y ^ (x | ~z);
    };
};

} // end namespace util

#endif // !MD50_H
