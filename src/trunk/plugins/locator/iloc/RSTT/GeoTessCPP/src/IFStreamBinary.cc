//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
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

#include <sstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Public Default Constructor.
 */
IFStreamBinary::IFStreamBinary() :	bData(new string("")), bDataPos(0),
															bSize(0), bAlign(true), bReverse(false),
															bOwnStr(true), bFileName(""), bMemIncr(1000000)
{
}

/**
 * Standard constructor that sets the padding (alignment) option to align.
 */
IFStreamBinary::IFStreamBinary(bool align) :	bData(new string("")),
															bDataPos(0), bSize(0), bAlign(align),
															bReverse(false), bOwnStr(true), bFileName(""),
															bMemIncr(1000000)
{
}

/**
 * Standard constructor creates a new IFStreamBinary and opens reads its
 * buffer from the input filename
 */
IFStreamBinary::IFStreamBinary(const string& filename) :
															bData(new string("")), bDataPos(0),
															bSize(0), bAlign(true), bReverse(false),
															bOwnStr(true), bFileName(filename),
															bMemIncr(1000000)
{
	readFromFile(filename);
}

/**
 * Standard constructor creates a new IFStreamBinary and opens reads its
 * buffer from the input filename
 */
IFStreamBinary::IFStreamBinary(const string& filename, int num_bytes) :
															bData(new string("")), bDataPos(0),
															bSize(0), bAlign(true), bReverse(false),
															bOwnStr(true), bFileName(filename),
															bMemIncr(1000000)
{
	readFromFile(filename, num_bytes);
}

/**
 * Standard constructor that uses a provided string ptr as the data
 * container.
 */
IFStreamBinary::IFStreamBinary(string* str) :	bData(str),
															bDataPos(0), bSize(0), bAlign(true),
															bReverse(false), bOwnStr(false), bFileName(""),
															bMemIncr(1000000)
{
}

/**
 * Copy Constructor.
 */
IFStreamBinary::IFStreamBinary(const IFStreamBinary& db) :
															bData(new string("")),
															bDataPos(db.bDataPos), bSize(db.bSize),
															bAlign(db.bAlign), bReverse(db.bReverse),
															bOwnStr(true), bFileName(db.bFileName),
															bMemIncr(db.bMemIncr)
{
  (*bData) = (*db.bData);
}

/**
 * Public destructor
 */
IFStreamBinary::~IFStreamBinary()
{
  if (bOwnStr) delete bData;
}

/**
 * Assignment operator
 */
IFStreamBinary&	IFStreamBinary::operator=(const IFStreamBinary& db)
{
  // set parameters

  (*bData)  = (*db.bData);
  bSize     = db.bSize;
  bDataPos  = db.bDataPos;
  bAlign    = db.bAlign;
  bReverse  = db.bReverse;
  bFileName = db.bFileName;
  bMemIncr  = db.bMemIncr;
  return *this;
}

/**
 * Debug dump of the data.
 */
void	IFStreamBinary::dumpBuffer()
{
  int i;

  // write out each character

  for (i = 0; i < (int) bData->size(); i++)
  {
    // precede control characters with "^"

    if ((*bData)[i] < 32)
      cout << "^" << int((*bData)[i]);
    else
      cout << (*bData)[i];
  }

  // end line and exti

  cout << endl;
}

/**
 * Static function that returns true if the input filename exists.
 */
bool IFStreamBinary::exists(const string& filename)
{
	ifstream ifs;
	ifs.open(filename.c_str(), std::ios::in|std::ios::binary);
	bool opn = ifs.is_open();
	ifs.close();

	return opn;
}

/**
 * Write the buffer to the input file name.
 */
void	IFStreamBinary::writeToFile(const string& filename)
{
	ofstream ofs;
	ofs.open(filename.c_str(), std::ios::out|std::ios::binary);
	if (!ofs.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::writeToFile" << endl
			 << "Could not open output file: " << filename << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9101);
	}

	writeToFile(ofs);
	if (ofs.bad())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::writeToFile" << endl
			 << "Error writing " << size()
			 << " bytes to file: " << filename << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9102);
	}

	ofs.close();
}

/**
 * Write the buffer to input output stream.
 */
void	IFStreamBinary::writeToFile(ofstream& ofs)
{
  if (bDataPos > bSize) bSize = bDataPos;
  ofs.write(bData->data(), bSize);
}

/**
 * Read the num_bytes from the input file stream (ifs) into the buffer.
 * The second form sets the streams input file pointer to file_pos as the
 * point to begin reading.
 */
void	IFStreamBinary::readFromFile(const string& filename)
{
	ifstream ifs;
	ifs.open(filename.c_str(), std::ios::in|std::ios::binary);
	if (!ifs.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::readFromFile" << endl
			 << "Could not open input file: " << filename << endl;

		cout << os.str() << endl;

		throw GeoTessException(os, __FILE__, __LINE__, 9103);
	}

	// Get size of file.

	ifs.seekg(0, ios::end);
	int fileSize = ifs.tellg();
	ifs.seekg(0, ios::beg);

	readFromFile(ifs, fileSize);
	if (ifs.fail() || ifs.eof())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::readFromFile" << endl
			 << "Error reading " << fileSize
			 << " bytes from input file: " << filename << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9104);
	}

	ifs.close();
}

/**
 * Read the num_bytes from the input file stream (ifs) into the buffer.
 * The second form sets the streams input file pointer to file_pos as the
 * point to begin reading.
 */
void	IFStreamBinary::readFromFile(const string& filename, int num_bytes)
{
	ifstream ifs;
	ifs.open(filename.c_str(), std::ios::in|std::ios::binary);
	if (!ifs.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::readFromFile" << endl
			 << "Could not open input file: " << filename << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9105);
	}

	readFromFile(ifs, num_bytes);
	if (ifs.fail() || ifs.eof())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::readFromFile" << endl
			 << "Error reading " << num_bytes
			 << " bytes from input file: " << filename << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9106);
	}

	ifs.close();
}

/**
 * Reads num_bytes data from the input ifstream into this buffer.
 * the read begins at position file_pos in the input stream.
 */
//void	IFStreamBinary::readFromFile(ifstream& ifs, streampos file_pos,
//																		int num_bytes)
//{
//  // check buffer size
//
//  checkBufferSize(num_bytes);
//
//  // read file data
//
//  ifs.seekg(file_pos);
//  ifs.read(&(*dbData)[dbDataPos], num_bytes);
//  dbDataPos += num_bytes;
//}

/**
 * Reads num_bytes data from the input ifstream into this buffer.
 */
void	IFStreamBinary::readFromFile(ifstream& ifs, int num_bytes)
{
  // check buffer size

  checkBufferSize(num_bytes);

  // read file data and append to buffer

  ifs.read(&(*bData)[bDataPos], num_bytes);
  bDataPos += num_bytes;
}

/**
 * Reverses each s-byte element of array a containing n elements (s*n bytes).
 * The element size s must be 2, 4, or 8.
 */
void	IFStreamBinary::reverseBOArray(int n, char* a, int s)
{
  if (s == 8)
    reverseBO8Array(n, a);
  else if (s == 4)
    reverseBO4Array(n, a);
  else if (s == 2)
    reverseBO2Array(n, a);
}

/**
 * Reverses each two-byte element of array a containing n elements
 * (2*n bytes).
 */
void	IFStreamBinary::reverseBO2Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO2((char*) &a[2 * i]);
}

/**
 * Reverses each four-byte element of array a containing n elements
 * (4*n bytes).
 */
void	IFStreamBinary::reverseBO4Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO4((char*) &a[4 * i]);
}

/**
 * Reverses each eight-byte element of array a containing n elements
 * (8*n bytes).
 */
void	IFStreamBinary::reverseBO8Array(int n, char* a)
{
  int i;

  for (i = 0; i < n; i++) reverseBO8((char*) &a[8 * i]);
}

} // end namespace geotess
