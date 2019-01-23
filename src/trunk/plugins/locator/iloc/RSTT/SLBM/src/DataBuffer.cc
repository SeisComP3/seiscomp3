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
//- Module:        $RCSfile: DataBuffer.cc,v $
//- Creator:       Lee Jensen
//- Creation Date: January 20, 2001
//- Revision:      $Revision: 1.8 $
//- Last Modified: $Date: 2012/10/25 00:25:13 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _LOCAL INCLUDES_ *******************************************************

#include "DataBuffer.h"
#include "MD50.h" 

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace util {

// **** _STATIC INITIALIZATIONS_************************************************

const int  DataBuffer::ALLOCATION_REQ_SIZE = 2 * (sizeof(uByte) + sizeof(int));

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Standard Constructor sets the padding boolean to \em doPad.
//
// *****************************************************************************
DataBuffer::DataBuffer(bool doPad) : dbData(new string("")), dbDataPos(0),
                                     dbSize(0), dbPad(doPad), dbReverse(false),
                                     dbOwnStr(true)
{
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Default Constructor.
//
// *****************************************************************************
DataBuffer::DataBuffer() : dbData(new string("")), dbDataPos(0), dbSize(0), 
			                     dbPad(true), dbReverse(false), dbOwnStr(true)
{
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Constructor. Given a string.
//
// *****************************************************************************
DataBuffer::DataBuffer(string* str) : dbData(str), dbDataPos(0), dbSize(0),
                         				      dbPad(true), dbReverse(false), 
				                              dbOwnStr(false)
{
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Copy Constructor.
//
// *****************************************************************************
DataBuffer::DataBuffer(const DataBuffer& db) : dbData(new string("")),
                       dbDataPos(db.dbDataPos), dbSize(db.dbSize),
                       dbPad(db.dbPad), dbReverse(db.dbReverse), dbOwnStr(true)
{
  (*dbData) = (*db.dbData);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Destructor.
//
// *****************************************************************************
DataBuffer::~DataBuffer()
{
  if (dbOwnStr) delete dbData;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Assignment Operator.
//
// *****************************************************************************
DataBuffer& DataBuffer::operator=(const DataBuffer& db)
{
  // set parameters

  (*dbData)    = (*db.dbData);
  dbSize    = db.dbSize;
  dbDataPos = db.dbDataPos;
  dbPad     = db.dbPad;
  dbReverse = db.dbReverse;

  return *this;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Dumps the data to the standard output device (cout) as a a
//! stream of characters.
//
// *****************************************************************************
void DataBuffer::dumpBuffer()
{
  int i;

  // write out each character

  for (i = 0; i < (int)dbData->size(); i++)
  {
    // precede control characters with "^"

    if ((*dbData)[i] < 32)
      cout << "^" << int((*dbData)[i]);
    else
      cout << (*dbData)[i];
  }

  // end line and exti

  cout << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write 'this' DataBuffer to the output stream \em ofs.
//
// *****************************************************************************
void DataBuffer::writeToFile(ofstream& ofs)
{
  if (dbDataPos > dbSize) dbSize = dbDataPos;
  ofs.write(dbData->data(), dbSize);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Write 'this' DataBuffer to the file stream \em ofs.
//
// *****************************************************************************
void DataBuffer::writeToFile(fstream& ofs)
{
  if (dbDataPos > dbSize) dbSize = dbDataPos;
  ofs.write(dbData->data(), dbSize);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read \em num_bytes from the file stream \em ifs into 'this'
//! DataBuffer beginning at file position \em file_pos.
//
//!
//! \param ifs - Input stream from which data will be read.
//! \param file_pos - Start location in \em ifs at which data will be read.
//! \param num_bytes - Number of bytes to be read from \em ifs.
//
// *****************************************************************************
void DataBuffer::readFromFile(ifstream& ifs, int64 file_pos, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // read file data

  ifs.seekg(file_pos);
  ifs.read(&(*dbData)[dbDataPos], num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Read \em num_bytes from the file stream (\em ifs) beginning at file
//! position \em file_pos into 'this' DataBuffer.
//
//!
//! \param ifs - Input stream from which data will be read.
//! \param file_pos - Start location in \em ifs at which data will be read.
//! \param num_bytes - Number of bytes to be read from \em ifs.
//
// *****************************************************************************
void DataBuffer::readFromFile(fstream& ifs, int64 file_pos, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // read file data

  ifs.seekg(file_pos);
  ifs.read(&(*dbData)[dbDataPos], num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_bytes from the file stream (\em ifs) into 'this' DataBuffer.
//
//!
//! \param ifs - Input stream from which data will be read.
//! \param num_bytes - Number of bytes to be read from \em ifs.
//
// *****************************************************************************
void DataBuffer::readFromFile(ifstream& ifs, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // read file data

  ifs.read(&(*dbData)[dbDataPos], num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Read \em num_bytes from the file stream (\em ifs) into 'this' DataBuffer
//
//!
//! \param ifs - Input stream from which data will be read.
//! \param num_bytes - Number of bytes to be read from \em ifs.
//
// *****************************************************************************
void DataBuffer::readFromFile(fstream& ifs, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // read file data

  ifs.read(&(*dbData)[dbDataPos], num_bytes);
  dbDataPos += num_bytes;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the md5 hash key assumed to be stored at the beginning of
//! this DataBuffer (32 bytes). Don't call this function if a hash key was
//! not stored at the beginning of the buffer. The buffer pointer (dbDataPos)
//! is left unchanged on exit.
//
// *****************************************************************************
const string& DataBuffer::readMD5HashKey()
{
  int db, len;
  static string s;

  // save the current buffer pointer and set it to zero

  db = dbDataPos;
  dbDataPos = 0;

  // read the data and reset the buffer pointer

  len = readInt32();
  s = dbData->substr(dbDataPos, len);
  dbDataPos = db;

  // return the string

  return s;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes an md50 hash key generated from this DataBuffer into the
//! first 32 locations of the DataBuffer. This function assumes that the
//! first 32 positions of the DataBuffer were reserved for the hexadecimal
//! string.
//
// *****************************************************************************
string DataBuffer::generateMD5HashKey()
{
  string hexstring;
  MD50   md5;

  // save size and data position and reset to zero ... read hexstring at
  // beginning of buffer to set data position to be ready for hashing

  int   sze = size();
  int savdb = dbDataPos;
  dbDataPos = 0;
  hexstring = readString();

  // get the md5 hash key into hexstring ... start at dbDataPos and hash
  // sze - dbDataPos bytes

  md5.getMD5HashHex((uchar const*) &dbData->data()[dbDataPos],
                    sze - dbDataPos, hexstring);
  //hexstring = getMD5HashHex((unsigned char const*) &dbData->data()[dbDataPos],
  //                          sze - dbDataPos);

  // write the binary string into the buffer at position zero and reset buffer
  // position to savdb

  dbDataPos = 0;
  writeString(hexstring);
  dbDataPos = savdb;

  // return hexstring

  return hexstring;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief This function simply returns a key string representing the
//! binary data of the data buffer.
//
// *****************************************************************************
string DataBuffer::generateDataBufMD5HashKey()
{
  string hexstring;
  MD50   md5;

  // save size and data position and reset to zero ... read hexstring at
  // beginning of buffer to set data position to be ready for hashing

  int   sze = size();
  dbDataPos = 0;

  // get the md5 hash key into hexstring ... start at dbDataPos and hash
  // sze bytes

  md5.getMD5HashHex((unsigned char const*) &dbData->data()[dbDataPos],
                    sze, hexstring);
  //hexstring = getMD5HashHex((unsigned char const*) &dbData->data()[dbDataPos],
  //                          sze);

  // return hexstring

  return hexstring;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Reverses each \em s-byte element of array \em a containing \em n
//! elements (s*n bytes). The element size \em s must be 2, 4, or 8.
//
// *****************************************************************************
void DataBuffer::reverseBOArray(int n, char* a, int s)
{
  if (s == 8)
    reverseBO8Array(n, a);
  else if (s == 4)
    reverseBO4Array(n, a);
  else if (s == 2)
    reverseBO2Array(n, a);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Reverses each two-byte element of array \em a containing \em n
//! elements (2*n bytes).
//
// *****************************************************************************
void DataBuffer::reverseBO2Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO2((char*) &a[2 * i]);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Reverses each four-byte element of array \em a containing \em n
//! elements (4*n bytes).
//
// *****************************************************************************
void DataBuffer::reverseBO4Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO4((char*) &a[4 * i]);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Reverses each eight-byte element of array \em a containing \em n
//! elements (8*n bytes).
//
// *****************************************************************************
void DataBuffer::reverseBO8Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO8((char*) &a[8 * i]);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Returns the default DataBuffer allocation size.
//
// *****************************************************************************
int DataBuffer::getAllocationReqSize()
{
  return ALLOCATION_REQ_SIZE;
}

} // end util namespace
