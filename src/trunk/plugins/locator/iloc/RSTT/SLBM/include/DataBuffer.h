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
//- Module:        $RCSfile: DataBuffer.h,v $
//- Creator:       Lee Jensen
//- Creation Date: January 20, 2001
//- Revision:      $Revision: 1.7 $
//- Last Modified: $Date: 2012/12/20 15:26:24 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

#ifndef DATABUFFER_H
#define DATABUFFER_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "UtilGlobals.h"
#include "CPPUtils.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace util {

// **** _CLASS CONSTANTS_ ******************************************************
// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************
//
//! \brief A byte array container used to hold binary data in the same
//! manner as disk based file system.
//
//! The object contains a padding facility to maintain 1, 4, and 8 byte
//! aligned objects so that the objects that utilize this class can be
//! compiled as doubly aligned. This object is used extensively in
//! support of database object serialization. The class contains a long
//! list of methods and properties supporting read/write capability for
//! string, char, byte, bool, int, long, float, and double types.  Most
//! types also contain an array read/write capability. Finally, the
//! ability to read and write compressed data is provided using the
//! zlib library facility.
//
// *****************************************************************************
//class PGL_EXP_IMP DataBuffer : public DataObject
class UTIL_EXP DataBuffer
{
  public:

    // **** _PUBLIC LIFECYCLES_ ************************************************

    DataBuffer();
    //- Public Default Constructor.

    DataBuffer(bool doPad);
    //- Standard constructor that sets the padding option to doPad.

    DataBuffer(string * str);
    //- Standard constructor that uses a string ptr provided.

    DataBuffer(const DataBuffer& db);
    //- Copy Constructor.

    virtual ~DataBuffer();
    //- Public destructor

    // **** _PUBLIC OPERATORS_ *************************************************

    DataBuffer&      operator=(const DataBuffer& db);
    //- Assignment operator

    // **** _PUBLIC METHODS_ ***************************************************

    void             dumpBuffer();
    //- Debug dump of the data.

    void             writeToFile(ofstream& ofs);
    void             writeToFile(fstream& ofs);
    //- Write the buffer to the output stream ofs

    void             readFromFile(fstream& ifs, int num_bytes);
    void             readFromFile(ifstream& ifs, int num_bytes);
    void             readFromFile(fstream& ifs, int64 filePos,
                                  int num_bytes);
    void             readFromFile(ifstream& ifs, int64 filePos,
                                  int num_bytes);
    //- read the num_bytes from the input file stream (ifs) into the buffer.
    //- The second form sets the streams input file pointer to file_pos as the
    //- point to begin reading.

    const string&    readMD5HashKey();
    //- Returns the md5 hash key assumed to be stored at the beginning of this
    //- DataBuffer (32 bytes). Don't call this function if a hash key was not
    //- stored at the beginning of the buffer. The buffer pointer (dbDataPos)
    //- is left unchanged on exit.

    string           generateMD5HashKey();
    //- Writes an md5 hash key generated from this DataBuffer into the first 32
    //- locations of the DataBuffer. This function assumes that the first 32
    //- positions of the DataBuffer were reserved for the hexadecimal string.

    string           generateDataBufMD5HashKey();
    //- Returns an MD5 hash key string that represents all the data
    //- stored in this data buffer.

    void             align4Byte();
    void             align8Byte();
    //- These functions ensure that the buffer position pointer (dbDataPos) is
    //- aligned on a 4 or 8 byte boundary, respectivly.

    void             checkBufferSize(int sincr);
    //- This function checks to make sure that the buffer is large enough to
    //- contain sincr more bytes ... if not it is resized so that it can.

    void             clear();
    //- Clears the buffer.

    const string&    readString();
    void             readString(string& str, int num_chars);
    void             readCharArray(char* array, int num_chars);
    //- Read string data. readString assumes the string length immediately
    //- precedes the string data. The other two functions read num_chars of
    //- characters at a time into a string or character array.

    uByte            readByte();
    uByte            readByte(int pos);
    void             readByteArray(uByte* array, int num_bytes);
    //- Read uByte data. readByte reads one uByte from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The last form reads num_bytes into the
    //- input array.

    bool             readBool();
    bool             readBool(int pos);
    void             readBoolArray(bool* array, int num_bools);
    //- Read bool data. readBool reads one bool from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The last form reads num_bool into the
    //- input array.

    int              readInt32();
    int              readInt32(int pos);
    int              readRawInt32();
    void             readIntArray(int* array, int num_ints);
    //- Read int data. readInt32 reads one int from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment is assumed to be ok. This is faster but not as safe.
    //- The last form reads num_ints into the input array.

    int64            readInt64();
    int64            readInt64(int pos);
    int64            readRawInt64();
    void             readLongArray(int64* array, int num_longs);
    //- Read long data. readInt64 reads one long from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment is assumed to be ok. This is faster but not as safe.
    //- The last form reads num_longs into the input array.

    float            readFloat();
    float            readFloat(int pos);
    float            readRawFloat();
    void             readFloatArray(float* array, int num_floats);
    //- Read float data. readFloat reads one float from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment is assumed to be ok. This is faster but not as safe.
    //- The last form reads num_floats into the input array.

    double           readDouble();
    double           readDouble(int pos);
    double           readRawDouble();
    void             readDoubleArray(double* array, int num_doubles);
    //- Read double data. readDouble reads one double from the DataBuffer and
    //- updates buffer iterator. The second form specifies where to read but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment is assumed to be ok. This is faster but not as safe.
    //- The last form reads num_doubles into the input array.

    void             writeString(const string& in_string);
    void             writeString(char* char_string);
    void             writeCharArray(const char* array, int num_chars);
    //- Write string data to 'this' DataBuffer. The two writeString functions
    //- first writes the the input string length immediately followed by the
    //- string data. The last function writes num_chars from the input character
    //- array into 'this' DataBuffer.

    void             writeByte(uByte b);
    void             writeByte(uByte b, int pos);
    void             writeRawByte(uByte b);
    void             writeByteArray(const uByte* array, int num_bytes);
    //- Write uByte data. writeByte writes one uByte to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that buffer size checking is not performed and is assumed to be ok. The
    //- last form writes num_bytes from the input array into the 'this'
    //- DataBuffer.

    void             writeBool(bool b);
    void             writeBool(bool b, int pos);
    void             writeRawBool(bool b);
    void             writeBoolArray(const bool* array, int num_bools);
    //- Write bool data. writeBool writes one bool to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that buffer size checking is not performed and is assumed to be ok. The
    //- last form writes num_bools from the input array into the 'this'
    //- DataBuffer.

    void             writeInt32(int i);
    void             writeInt32(int i, int pos);
    void             writeRawInt32(int i);
    void             writeIntArray(const int* array, int num_ints);
    //- Write int data. writeInt32 writes one int to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment and buffer size checking is not performed and is
    //- assumed to be ok. This is faster but not as safe. The last form writes
    //- num_ints from the input array into 'this' DataBuffer.

    void             writeInt64(int64 i);
    void             writeInt64(int64 i, int pos);
    void             writeRawInt64(int64 i);
    void             writeLongArray(const int64* array, int num_longs);
    //- Write long data. writeInt64 writes one long to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment and buffer size checking is not performed and is
    //- assumed to be ok. This is faster but not as safe. The last form writes
    //- num_longs from the input array into 'this' DataBuffer.

    void             writeFloat(float f);
    void             writeFloat(float f, int pos);
    void             writeRawFloat(float f);
    void             writeFloatArray(const float* array, int num_floats);
    //- Write float data. writeFloat writes one float to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment and buffer size checking is not performed and is
    //- assumed to be ok. This is faster but not as safe. The last form writes
    //- num_floats from the input array into 'this' DataBuffer.

    void             writeDouble(double d);
    void             writeDouble(double d, int pos);
    void             writeRawDouble(double d);
    void             writeDoubleArray(const double* array, int num_doubles);
    //- Write double data. writeDouble writes one double to the DataBuffer and
    //- updates buffer iterator. The second form specifies where to write but
    //- does not update the iterator. The third form is like the first except
    //- that byte alignment and buffer size checking is not performed and is
    //- assumed to be ok. This is faster but not as safe. The last form writes
    //- num_doubles from the input array into 'this' DataBuffer.

    static void      reverseBOArray(int n, char* a, int s);
    //- Reverses each s-byte element of array a containing n elements
    //- (s*n bytes). The element size s must be 2, 4, or 8.

    static void      reverseBO2Array(int n, char* a);
    static void      reverseBO4Array(int n, char* a);
    static void      reverseBO8Array(int n, char* a);
    //- Reverses the byte order of each element of a where a is an array of n
    //- 2, 4, or 8 byte elements. Used to convert between big-and
    //- little-endian formats.

    static void      reverseBO2(char* d);
    static void      reverseBO4(char* d);
    static void      reverseBO8(char* d);
    //- Reverses the byte order of d, where d is a 2, 4, or 8 byte array. Used
    //- to convert between big-and little-endian formats.

    // **** _PUBLIC PROPERTIES_ ************************************************

    static  string   class_name();
    virtual string   get_class_name() const;
    virtual int      class_size() const;
    //- Return class name and size.

    void             setSize(int num_bytes);
    //- This function reserves num_bytes in the buffer to minimize reallocation.

    int              size();
    //- Returns the DataBuffer size.

    void             resetPos();
    void             setPosToEnd();
    int              getPos() const;
    //- Reset, set, and get the current iterator position.

    void             incrementPos(int increment);
    void             decrementPos(int decrement);
    //- Increment / decrement the iterator position.

    string           getData() const;
    //- Return a const reference to 'this' DataBuffers data.

    int              getCapacity() const;
    //- Return the allocated capacity of this DataBuffer.

    char*            getPosPointer();
    //- Return a pointer at the current iterator location in the DataBuffers
    //- data.

    char*            getPosPointer(int pos);
    //- Return a pointer to position pos in 'this' DataBuffers data.

    void             setByteOrderReverse(bool bor);
    void             byteOrderReverseOn();
    void             byteOrderReverseOff();
    //- Sets the byte order reverse flag to bor (1st function), true (on
    //- function) or false (off function).

    static int       getAllocationReqSize();
    //- Returns the default DataBuffer allocation size.

    // **** _PUBLIC INQUIRIES_ *************************************************

    bool             isByteOrderReversed() const;
    //- Returns true if the byte order in reads and writes is reversed for all
    //- 2, 4, or 8 byte intrinsincs (short, int, long, float, double, etc.).

    // **** _PUBLIC DATA_ ******************************************************

    //! Default allocation requirement size for a data buffer
    static const int ALLOCATION_REQ_SIZE;

    //! brief Reserves space in the DataBuffer
    void reserve(int sze);

  protected:

    // **** _PRIVATE LIFECYCLES_ ***********************************************
    // **** _PROTECTED OPERATORS_ ***********************************************
    // **** _PROTECTED METHODS_ *************************************************

    // **** _PROTECTED DATA_ ****************************************************

    //! A string object is used to contain the actual data.
    string*          dbData;

    //! The current iterator position in the data container (dbData).
    int              dbDataPos;

    //! The current size of the data container.
    int	             dbSize;

    //! \brief A boolean, that if true, maintains 4 and 8 byte alignment in
    //! support of double alignment compilation.
    bool             dbPad;

    //! \brief A boolean, that if true, reverses the byte order of all 2, 4, and
    //! 8 byte intrinsics (shorts, ints, long, floats, doubles, etc.). This
    //! flag is set by the FileDatabase object only and otherwise defaults to
    //! false. The flag is used to convert byte order between big- and little-
    //! endian formats.
    bool             dbReverse;

    //! \brief A boolean, that if true, indicates the storage string for this
    //! DataBuffer is owned and therefore deleted when the DataBuffer is
    //! deleted.
    bool             dbOwnStr;

}; // DataBuffer End Definition

// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Static function that returns the class name.
//
// *****************************************************************************
inline string DataBuffer::class_name()
{
  return "DataBuffer";
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the class name.
//
// *****************************************************************************
inline string DataBuffer::get_class_name() const
{
  return class_name();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Returns the class size (in bytes).
//
// *****************************************************************************
inline int DataBuffer::class_size() const
{
  return (int) sizeof(DataBuffer);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Aligns the DataBuffer position pointer on a 4 byte
//! boundary if it is not currently aligned as such.
//
// *****************************************************************************
inline void DataBuffer::align4Byte()
{
  // get position from an 4 byte alignment location

  int sm = dbDataPos % sizeof(int);
  
  // If not zero and dbPad is not set then add pad

  if (sm && dbPad) dbDataPos += sizeof(int) - sm;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Aligns the DataBuffer position pointer on an 8 byte
//! boundary if it is not currently aligned as such.
//
// *****************************************************************************
inline void DataBuffer::align8Byte()
{
  // get position from an 8 byte alignment location

  int sm = dbDataPos % sizeof(double);
  
  // If not zero and dbPad is not set then add pad

  if (sm && dbPad) dbDataPos += sizeof(double) - sm;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Checks the current buffer storage size and increments to see if it
//! can contain \em sincr more bytes. If not it increments the size so that
//! \em sincr bytes can be added safely.
//
// *****************************************************************************
inline void DataBuffer::checkBufferSize(int sincr)
{
  // append spaces to dbData if its current size is less than the position
  // pointer + sincr.

  if (dbDataPos + sincr > (int) dbData->size())
    dbData->append(dbDataPos + sincr - dbData->size(), ' ');
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Clear the DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::clear()
{
  (*dbData) = "";
  dbDataPos = 0;
  dbSize = 0;
}

//----------
//----------------------------  READ FUNCTIONS ------------------------//
//----------

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a string and return a reference to be assigned to another
//! string in the client. This function assumes the length of the
//! string occurs imediately before the actual string data.
//
// *****************************************************************************
inline const string& DataBuffer::readString()
{
  int len;
  static string s;

  // read the data

  len = readInt32();
  s = dbData->substr(dbDataPos, len);
  dbDataPos += len;

  return s;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read num_chars from 'this' databuffer into the input string, str.
//
// *****************************************************************************
inline void DataBuffer::readString(string& str, int num_chars)
{
  // read the data

  str = dbData->substr(dbDataPos, num_chars);
  dbDataPos += num_chars;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_chars characters from 'this' DataBuffer into /em array.
//
// *****************************************************************************
inline void DataBuffer::readCharArray(char* array, int num_chars)
{
  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_chars);
  dbDataPos += num_chars;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read a byte from the buffer and update the iterator.
//
// *****************************************************************************
inline uByte DataBuffer::readByte()
{
  // read the data

  uByte b = *((uByte*) &(*dbData)[dbDataPos]);
  dbDataPos += sizeof(uByte);
  return b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//!  \brief Read a byte from the positions specified by \em pos
//!  ... doesn't update dbDataPos.
//
// *****************************************************************************
inline uByte DataBuffer::readByte(int pos)
{
  // read the data

  uByte b = *((uByte*) &(*dbData)[pos]);
  return b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_bytes characters from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readByteArray(uByte* array, int num_bytes)
{
  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read a bool from the buffer and update the iterator.
//
// *****************************************************************************
inline bool DataBuffer::readBool()
{
  // read the data

  bool b = *((bool*) &(*dbData)[dbDataPos]);
  dbDataPos += sizeof(bool);
  return b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a bool from the positions specified by \em pos
//!  ... doesn't update dbDataPos.
//
// *****************************************************************************
inline bool DataBuffer::readBool(int pos)
{
  // read the data

  bool b = *((bool*) &(*dbData)[pos]);
  return b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_bools booleans from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readBoolArray(bool* array, int num_bools)
{
  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_bools);
  dbDataPos += num_bools;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read an int from the buffer. Make sure the buffer is 4byte
//! alligned before reading.
//
// *****************************************************************************
inline int DataBuffer::readInt32()
{
  // check alignment

  align4Byte();

  // read the data

  return readRawInt32();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read the int from the positions specified by \em pos
//! ... doesn't update dbDataPos.
//
// *****************************************************************************
inline int DataBuffer::readInt32(int pos)
{
  // read the data

  int i = *((int*) &(*dbData)[pos]);
  if (dbReverse) reverseBO4((char*) &i);
  return i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read an Int32 ... don't check for alignment ... fast but client is
//! responsible for alignment.
//
// *****************************************************************************
inline int DataBuffer::readRawInt32()
{
  // read the data

  int i = *((int*) &(*dbData)[dbDataPos]);
  if (dbReverse) reverseBO4((char*) &i);
  dbDataPos += sizeof(int);
  return i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_ints integers from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readIntArray(int* array, int num_ints)
{
  int num_bytes = num_ints * sizeof(int);

  // check alignment

  align4Byte();

  // read the data
  memcpy(array, &(*dbData)[dbDataPos], num_bytes);
  if (dbReverse) reverseBO4Array(num_ints, (char*) array);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a long from the buffer. Make sure the buffer is 8byte
//! alligned before reading.
//
// *****************************************************************************
inline int64 DataBuffer::readInt64()
{
  // check alignment

  align8Byte();

  // read the data

  return readRawInt64();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read the long from the positions specified by \em pos
//! ... doesn't update dbDataPos.
//
// *****************************************************************************
inline int64 DataBuffer::readInt64(int pos)
{
  // read the data

  int64 i = *((int64*) &(*dbData)[pos]);
  if (dbReverse) reverseBO8((char*) &i);
  return i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a long ... don't check for alignment ... fast but client is
//! responsible for alignment.
//
// *****************************************************************************
inline int64 DataBuffer::readRawInt64()
{
  // read the data

  int64 i = *((int64*) &(*dbData)[dbDataPos]);
  if (dbReverse) reverseBO8((char*) &i);
  dbDataPos += sizeof(int64);
  return i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_longs int64s from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readLongArray(int64* array, int num_longs)
{
  int num_bytes = num_longs * sizeof(int64);

  // check alignment

  align8Byte();

  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_bytes);
  if (dbReverse) reverseBO8Array(num_longs, (char*) array);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a float from the buffer. Make sure the buffer is 4byte
//! alligned before reading.
//
// *****************************************************************************
inline float DataBuffer::readFloat()
{
  // check alignment

  align4Byte();

  // read the data

  return readRawFloat();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read the float from the positions specified by \em pos
//! ... doesn't update dbDataPos.
//
// *****************************************************************************
inline float DataBuffer::readFloat(int pos)
{
  // read the data

  float f = *((float*) &(*dbData)[pos]);
  if (dbReverse) reverseBO4((char*) &f);
  return f;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a float ... don't check for alignment ... fast but client is
//! responsible for alignment.
//
// *****************************************************************************
inline float DataBuffer::readRawFloat()
{
  // read the data

  float f = *((float*) &(*dbData)[dbDataPos]);
  if (dbReverse) reverseBO4((char*) &f);
  dbDataPos += sizeof(float);
  return f;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_floats floats from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readFloatArray(float* array, int num_floats)
{
  int num_bytes = num_floats * sizeof(float);

  // check alignment

  align4Byte();

  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_bytes);
  if (dbReverse) reverseBO4Array(num_floats, (char*) array);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a double from the buffer. Make sure the buffer is 8byte
//! alligned before reading.
//
// *****************************************************************************
inline double DataBuffer::readDouble()
{
  // check alignment

  align8Byte();

  // read the data

  return readRawDouble();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Just read the double from the positions specified by \em pos
//! ... doesn't update dbDataPos.
//
// *****************************************************************************
inline double DataBuffer::readDouble(int pos)
{
  // read the data

  double d = *((double*) &(*dbData)[pos]);
  if (dbReverse) reverseBO8((char*) &d);
  return d;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read a double ... don't check for alignment ... fast but client is
//! responsible for alignment.
//
// *****************************************************************************
inline double DataBuffer::readRawDouble()
{
  // read the data

  double d = *((double*) &(*dbData)[dbDataPos]);
  if (dbReverse) reverseBO8((char*) &d);
  dbDataPos += sizeof(double);
  return d;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_doubles doubles from 'this' DataBuffer into \em array.
//
// *****************************************************************************
inline void DataBuffer::readDoubleArray(double* array, int num_doubles)
{
  int num_bytes = num_doubles * sizeof(double);

  // check alignment

  align8Byte();

  // read the data

  memcpy(array, &(*dbData)[dbDataPos], num_bytes);
  if (dbReverse) reverseBO8Array(num_doubles, (char*) array);
  dbDataPos += num_bytes;
}

//----------
//----------------------------  WRITE FUNCTIONS ------------------------//
//----------

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write the string \em in_string into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeString(const string& in_string)
{
  // get total size

  int sz = (int) in_string.size() + sizeof(int);

  // check alignment

  align4Byte();

  // check buffer size

  checkBufferSize(sz);

  // write the data

  writeRawInt32((int) in_string.size());

  // In Sun's V5.2 CC compiler, string replace seems to have a bug or
  // odd behavior that is not understood.  Whenever a replace occurs
  // using the following line, the strings size() is reduced by 
  // dbDataPos bytes???  So we will use memcpy instead!
  // dbData.replace(dbDataPos, dbDataPos + in_string.size(), in_string);
  memcpy(&(*dbData)[dbDataPos], &in_string[0], in_string.size());

  dbDataPos += (int) in_string.size();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write the string \em char_string into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeString(char* char_string)
{
  string tmp_string = char_string;
  writeString(tmp_string);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_chars of the character \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeCharArray(const char* array, int num_chars)
{
  // check buffer size

  checkBufferSize(num_chars);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_chars);
  dbDataPos += num_chars;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a byte (\em b) to the DataBuffer. Make sure the buffer
//! is sized appropriately before writing.
//
// *****************************************************************************
inline void DataBuffer::writeByte(uByte b)
{
  // check buffer size

  checkBufferSize(sizeof(uByte));

  // write the data

  writeRawByte(b);  
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a uByte (\em b) at databuffer position \em pos. Does
//! not update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeByte(uByte b, int pos)
{
  // write the data

  *((uByte*) &(*dbData)[pos]) = b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a uByte (\em b) ... don't check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawByte(uByte b)
{
  // write the data

  *((uByte*) &(*dbData)[dbDataPos]) = b;
  dbDataPos += sizeof(uByte);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_bytes uBytes from the input \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeByteArray(const uByte* array, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a bool (\em b) to the DataBuffer. Make sure the buffer
//! is sized appropriately before writing.
//
// *****************************************************************************
inline void DataBuffer::writeBool(bool b)
{
  // check buffer size

  checkBufferSize(sizeof(bool));

  // write the data

  writeRawBool(b);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a bool (\em b) at databuffer position \em pos. Does
//! not update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeBool(bool b, int pos)
{
  // write the data

  *((bool*) &(*dbData)[pos]) = b;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a bool (\em b) ... do not check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawBool(bool b)
{
  // write the data

  *((bool*) &(*dbData)[dbDataPos]) = b;
  dbDataPos += sizeof(bool);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_bools bools from the input \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeBoolArray(const bool* array, int num_bools)
{
  // check buffer size

  checkBufferSize(num_bools);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bools);
  dbDataPos += num_bools;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write an Int32 (\em i) to the DataBuffer. Make sure the
//! buffer is 4byte alligned and sized appropriatly before reading.
//
// *****************************************************************************
inline void DataBuffer::writeInt32(int i)
{
  // check alignment

  align4Byte();

  // check buffer size

  checkBufferSize(sizeof(int));

  // write the data

  writeRawInt32(i);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write an int (\em i) at databuffer position \em pos. Do not
//! update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeInt32(int i, int pos)
{
  // write the data

  if (dbReverse) reverseBO4((char*) &i);
  *((int*) &(*dbData)[pos]) = i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write an Int32 (\em i) ... do not check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawInt32(int i)
{
  // write the data

  if (dbReverse) reverseBO4((char*) &i);
  *((int*) &(*dbData)[dbDataPos]) = i;
  dbDataPos += sizeof(int);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_ints ints from the input \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeIntArray(const int* array, int num_ints)
{
  int num_bytes = num_ints * sizeof(int);

  // check alignment

  align4Byte();

  // check buffer size

  checkBufferSize(num_bytes);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bytes);
  if (dbReverse) reverseBO4Array(num_ints, &(*dbData)[dbDataPos]);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write an Int64 (\em i) to the DataBuffer. Make sure the
//! buffer is 8byte alligned and sized appropriatly before reading.
//
// *****************************************************************************
inline void DataBuffer::writeInt64(int64 i)
{
  // check alignment

  align8Byte();

  // check buffer size

  checkBufferSize(sizeof(int64));

  // write the data

  writeRawInt64(i);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a long (\em i) at databuffer position \em pos. Does
//! not update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeInt64(int64 i, int pos)
{
  // write the data

  if (dbReverse) reverseBO8((char*) &i);
  *((int64*) &(*dbData)[pos]) = i;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write an Int64 (\em i) ... don't check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawInt64 (int64 i)
{
  // write the data

  if (dbReverse) reverseBO8((char*) &i);
  *((int64*) &(*dbData)[dbDataPos]) = i;
  dbDataPos += sizeof(int64);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_longs longs from the input \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeLongArray(const int64* array, int num_longs)
{
  int num_bytes = num_longs * sizeof(int64);

  // check alignment

  align8Byte();

  // check buffer size

  checkBufferSize(num_bytes);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bytes);
  if (dbReverse) reverseBO8Array(num_longs, &(*dbData)[dbDataPos]);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a float (\em f) to the DataBuffer. Make sure the
//! buffer is 4byte alligned and sized appropriatly before reading.
//
// *****************************************************************************
inline void DataBuffer::writeFloat(float f)
{
  // check alignment

  align4Byte();

  // check buffer size

  checkBufferSize(sizeof(float));

  // write the data

  writeRawFloat(f);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a float (\em f) at databuffer position \em pos. Does
//! not update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeFloat(float f, int pos)
{
  // write the data

  if (dbReverse) reverseBO4((char*) &f);
  *((float*) &(*dbData)[pos]) = f;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a float (\em f) ... do not check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawFloat(float f)
{
  // write the data

  if (dbReverse) reverseBO4((char*) &f);
  *((float*) &(*dbData)[dbDataPos]) = f;
  dbDataPos += sizeof(float);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write \em num_floats floats from the input \em array into 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeFloatArray(const float* array, int num_floats)
{
  int num_bytes = num_floats * sizeof(float);

  // check alignment

  align4Byte();

  // check buffer size

  checkBufferSize(num_bytes);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bytes);
  if (dbReverse) reverseBO4Array(num_floats, &(*dbData)[dbDataPos]);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a double (\em d) to the DataBuffer. Make sure the
//! buffer is 8byte alligned and sized appropriatly before reading.
//
// *****************************************************************************
inline void DataBuffer::writeDouble(double d)
{
  // check alignment

  align8Byte();

  // check buffer size

  checkBufferSize(sizeof(double));

  // write data

  writeRawDouble(d);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a double (\em d) at databuffer position \em pos. Does
//! not update the iterator.
//
// *****************************************************************************
inline void DataBuffer::writeDouble(double d, int pos)
{
  // write the data

  if (dbReverse) reverseBO8((char*) &d);
  *((double*) &(*dbData)[pos]) = d;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write a double (\em d) ... do not check for alignment or size
//! ... fast but the client is responsible for handling alignment and
//! sizing correctly.
//
// *****************************************************************************
inline void DataBuffer::writeRawDouble(double d)
{
  // write data

  if (dbReverse) reverseBO8((char*) &d);
  *((double*) &(*dbData)[dbDataPos]) = d;
  dbDataPos += sizeof(double);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Write \em num_doubles doubles from the input \em array into
//! 'this' DataBuffer.
//
// *****************************************************************************
inline void DataBuffer::writeDoubleArray(const double* array, int num_doubles)
{
  int num_bytes = num_doubles * sizeof(double);

  // check alignment

  align8Byte();

  // check buffer size

  checkBufferSize(num_bytes);

  // write data

  memcpy(&(*dbData)[dbDataPos], array, num_bytes);
  if (dbReverse) reverseBO8Array(num_doubles, &(*dbData)[dbDataPos]);
  dbDataPos += num_bytes;
}

//----------
//----------------------------  PROPERTY FUNCTIONS ------------------------//
//----------

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Set the size of the data buffer to \em num_bytes.
//
// *****************************************************************************
inline void DataBuffer::setSize(int num_bytes)
{
  dbData->resize(num_bytes);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return the current DataBuffer size.
//
// *****************************************************************************
inline int DataBuffer::size()
{
  // return dbData.size();

  if (dbDataPos > dbSize)
    dbSize = dbDataPos;

  return dbSize;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Reset the buffers iterator location to zero.
//
// *****************************************************************************
inline void DataBuffer::resetPos()
{
  if (dbDataPos > dbSize)
    dbSize = dbDataPos;

  dbDataPos = 0;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Set the buffers iterator position to dbSize. If current
//! iterator exceeds dbsize then dbsize is set and dbDataPos is
//! unchanged.
//
// *****************************************************************************
inline void DataBuffer::setPosToEnd()
{
  if (dbDataPos > dbSize)
    dbSize = dbDataPos;

  dbDataPos = dbSize;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return the buffers iterator location.
//
// *****************************************************************************
inline int DataBuffer::getPos() const
{
  return dbDataPos;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Increment the buffers iterator by \em increment bytes. This
//! is an unsafe operation. If dbDataPos + \em increment >
//! dbSize. dbSize is NOT checked.
//
// *****************************************************************************
inline void DataBuffer::incrementPos(int increment)
{
  dbDataPos += increment;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Decrement the buffers iterator by \em decrement bytes. This
//! is an unsafe operation. If dbDataPos - \em decrement < 0.
//
// *****************************************************************************
inline void DataBuffer::decrementPos(int decrement)
{
  dbDataPos -= decrement;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return a reference to the DataBuffer
//
// *****************************************************************************
inline string DataBuffer::getData() const
{
  return *dbData;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return allocated capacity of 'this' DataBuffer
//
// *****************************************************************************
inline int DataBuffer::getCapacity() const
{
  return (int) dbData->capacity();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return a pointer to the current iterator location in the buffer.
//
// *****************************************************************************
inline char* DataBuffer::getPosPointer()
{
  char* p = &(*dbData)[dbDataPos];
  return p;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Return a Pointer to position \em pos in the buffer.
//
// *****************************************************************************
inline char* DataBuffer::getPosPointer(int pos)
{
  char* p = &(*dbData)[pos];
  return p;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Reverses byte order of d. d is assumed to point to an 2 byte element.
//
// *****************************************************************************
inline void DataBuffer::reverseBO2(char* d)
{
  char tmp = d[0];
  d[0] = d[1];
  d[1] = tmp;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Reverses byte order of d. d is assumed to point to an 4 byte element.
//
// *****************************************************************************
inline void DataBuffer::reverseBO4(char* d)
{
  char tmp = d[0];
  d[0] = d[3];
  d[3] = tmp;

  tmp = d[1];
  d[1] = d[2];
  d[2] = tmp;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Reverses byte order of d. d is assumed to point to an 8 byte element.
//
// *****************************************************************************
inline void DataBuffer::reverseBO8(char* d)
{
  char tmp = d[0];
  d[0] = d[7];
  d[7] = tmp;

  tmp = d[1];
  d[1] = d[6];
  d[6] = tmp;

  tmp = d[2];
  d[2] = d[5];
  d[5] = tmp;

  tmp = d[3];
  d[3] = d[4];
  d[4] = tmp;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Sets the byte order reverse flag to \em bor.
//
// *****************************************************************************
inline void DataBuffer::setByteOrderReverse(bool bor)
{
  dbReverse = bor;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Sets the byte order reverse flag to true.
//
// *****************************************************************************
inline void DataBuffer::byteOrderReverseOn()
{
  dbReverse = true;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Sets the byte order reverse flag to false.
//
// *****************************************************************************
inline void DataBuffer::byteOrderReverseOff()
{
  dbReverse = false;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns true if the byte order in reads and writes is reversed for
//! all 2, 4, or 8 byte intrinsincs (short, int, long, float, double, etc.).
//
// *****************************************************************************
inline bool DataBuffer::isByteOrderReversed() const
{
  return dbReverse;
}

//} // end namespace pgl

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Reserves space in the DataBuffer
//
// *****************************************************************************
inline void DataBuffer::reserve(int sze)
{
  dbData->reserve(sze);
}

} // end util namespace

#endif // DATABUFFER_H
