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

#ifndef IFSTREAMBINARY_OBJECT_H
#define IFSTREAMBINARY_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessException.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Opens a file for binary read and write access.
 *
 * Opens a file for binary read and write access. The read portion is configured
 * to parse binary files using standard readType(...) functionality.
 *
 * Functionality is provided to read elemental, strings, booleans, bytes,
 * shorts, ints, longs, floats, or doubles, as-well-as array input forms of
 * each (except strings). The binary file can be read in intrinsic alignment form
 * (somewhat faster) where doubles and longs are assumed to be read (and written)
 * along 8 bytes boundaries, floats and ints are aligned on 4 byte boundaries,
 * and shorts are aligned on a 2 byte boundary. A little/big-endian byte
 * reversal flag can be set to reverse the byte structure if necessary.
 */
class GEOTESS_EXP_IMP IFStreamBinary
{
  protected:

		/**
		 * A string object is used to contain the actual data.
		 */
		string*					bData;

		/**
		 * The current iterator position in the data container (dbData).
		 */
		int							bDataPos;

		/**
		 * The current size of the data container.
		 */
		int							bSize;

		/**
		 * A boolean, that if true, maintains 4 and 8 byte alignment in
		 * support of double alignment compilation.
		 */
		bool						bAlign;

		/**
		 * A boolean, that if true, reverses the byte order of all 2, 4, and
		 * 8 byte intrinsics (shorts, ints, long, floats, doubles, etc.). This
		 * flag is set by client using this IFStreamBinary (default to false ...
		 * no reversal). The flag is used to convert byte order between big- and
		 * little- endian formats.
		 */
		bool						bReverse;

		/**
		 * A boolean, that if true, indicates the storage string for this
		 * IFStreamBinary is owned and therefore deleted when the IFStreamBinary
		 * is deleted.
		 */
		bool						bOwnStr;

		string					bFileName;

		int							bMemIncr;

    /**
		 * Ensure that the buffer position pointer (dbDataPos) is
		 * aligned on a 4 byte boundary.
     */
    void						align2Byte()
		{
			int sm = bDataPos % CPPUtils::SSHT;
			if (sm && bAlign) bDataPos += CPPUtils::SSHT - sm;
		}

    /**
		 * Ensure that the buffer position pointer (dbDataPos) is
		 * aligned on a 4 byte boundary.
     */
    void						align4Byte()
		{
			int sm = bDataPos % CPPUtils::SINT;
			if (sm && bAlign) bDataPos += CPPUtils::SINT - sm;
		}

    /**
		 * Ensure that the buffer position pointer (dbDataPos) is
		 * aligned on a 8 byte boundary.
     */
    void						align8Byte()
		{
			int sm = bDataPos % CPPUtils::SDBL;
			if (sm && bAlign) bDataPos += CPPUtils::SDBL - sm;
		}

		/**
		 * This function checks to make sure that the buffer is large enough to
		 * contain sincr more bytes ... if not it is resized so that it can.
		 */
    void						checkBufferSize(int sincr)
		{
			if (bDataPos + sincr > (int) bData->size())
			{
				if (sincr + bData->size() > bData->capacity())
					bData->reserve(bData->capacity() + bMemIncr);
				bData->append(bDataPos + sincr - bData->size(), ' ');
			}
		}

		/*
		 * Reverses each s-byte element of array a containing n elements
		 * (s*n bytes). The element size s must be 2, 4, or 8.
		 */
		static void			reverseBOArray(int n, char* a, int s);

		/*
		 * Reverses the byte order of each element of a where a is an array of n
		 * 2, 4, or 8 byte elements. Used to convert between big-and
		 * little-endian formats.
		 */
		static void			reverseBO2Array(int n, char* a);
		static void			reverseBO4Array(int n, char* a);
		static void			reverseBO8Array(int n, char* a);

		/*
		 * Reverses the byte order of d, where d is a 2, 4, or 8 byte array. Used
		 * to convert between big-and little-endian formats.
		 */
		static void			reverseBO2(char* d);
		static void			reverseBO4(char* d);
		static void			reverseBO8(char* d);

	public:

		/**
		 * Public Default Constructor.
		 */
										IFStreamBinary();

		/**
		 * Standard constructor that sets the padding (alignment) option to align.
		 */
										IFStreamBinary(bool align);

		/**
		 * Standard constructor creates a new IFStreamBinary and opens reads its
		 * buffer from the input filename
		 */
										IFStreamBinary(const string& filename);

		/**
		 * Standard constructor creates a new IFStreamBinary and opens reads its
		 * buffer from the input filename
		 */
										IFStreamBinary(const string& filename, int num_bytes);

		/**
		 * Standard constructor that uses a provided string ptr as the data
		 * container.
		 */
										IFStreamBinary(string* str);

		/**
		 * Copy Constructor.
		 */
										IFStreamBinary(const IFStreamBinary& db);

		/**
		 * Public destructor
		 */
		virtual					~IFStreamBinary();

		/**
		 * Assignment operator
		 */
		IFStreamBinary&	operator=(const IFStreamBinary& db);

		/**
		 * Debug dump of the data.
		 */
		void						dumpBuffer();

		/**
		 * Static function that returns true if the input filename exists.
		 */
		static bool 		exists(const string& filename);

		/**
		 * Write the buffer to the output file name.
		 */
		void						writeToFile(const string& filename);

		/**
		 * Write the buffer to the output stream.
		 */
		void						writeToFile(ofstream& ofs);

		/**
     * Read and fills the buffer from the input file name.
		 */
		void						readFromFile(const string& filename);

		/**
		 * Read num_bytes from input file (Partial read).
		 */
		void						readFromFile(const string& filename, int num_bytes);

		/**
		 * Reads num_bytes data from the input ifstream into this buffer.
		 * the read begins at position file_pos in the input stream.
		 */
//		void						readFromFile(ifstream& ifs, streampos file_pos,
//																	int num_bytes);

		/**
		 * Reads num_bytes data from the input ifstream into this buffer.
		 */
		void						readFromFile(ifstream& ifs, int num_bytes);

		/**
		 * Return the file name with which this binary reader was opened
		 */
		const string&		getFileName() const { return bFileName; }

		/**
		 * Clears the buffer.
		 */
		void						clear()
		{
			(*bData) = "";
			bDataPos = 0;
			bSize = 0;
		}

		/**
		 * Set the amount of memory increase (bytes) for bData everytime it's
		 * current capacity is exceeded (defaults to 1MB = 1000000).
		 */
		void						setMemoryCapacityIncrement(int mci)
		{
			if (mci > 0) bMemIncr = mci;
		}

		/**
		 * Read string data. readString() assumes the string length immediately
		 * precedes the string data. readString(string& s) is the same as the
		 * first but assigns the string to the input reference. The next two
		 * functions read in num_chars and assign them into the input string
		 * reference s, or presized character array, respectively. The functions
		 * readType* are for templatized access.
		 */
		string					readString();
		void						readString(string& s);
		void						readCharArray(string& s, int num_chars);
		void						readCharArray(char* array, int num_chars);
		void						readType(string& s);
		void						readTypeArray(string& array, int num_chars);

		/**
		 * Read bool data. readBool() reads one bool from the IFStreamBinary and
		 * updates buffer iterator. The second form specifies where to read but
		 * does not update the iterator. The last form reads num_bool into the
		 * input array. readType is for templatization and behaves like readBool().
		 * Similarly, readTypeArray behaves as readBoolArray.
		 */
		bool						readBool();
		bool						readBool(int pos);
		void						readBoolArray(bool* array, int num_bools);
		void						readType(bool& b);
		void						readTypeArray(bool* array, int num_bools);

		/**
		 * Read byte data. readByte() reads one byte from the IFStreamBinary and
		 * updates buffer iterator. The second form specifies where to read but
		 * does not update the iterator. The last form reads num_bytes into the
		 * input array. readType is for templatization and behaves like readByte().
		 * Similarly, readTypeArray behaves as readByteArray.
		 */
		byte						readByte();
		byte						readByte(int pos);
		void						readByteArray(byte* array, int num_bs);
		void						readType(byte& b);
		void						readTypeArray(byte* array, int num_bs);

		/**
		 * Read short data. readShort() reads one short from the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment is assumed to be ok (not checked). The third
		 * form specifies where to read but does not update the iterator or
		 * check alignment. The last form reads num_shorts into the input array.
		 * readType is for templatization and behaves like readShort(). Similarly,
		 * readTypeArray behaves as readShortArray.
		 */
		short						readShort();
		short						readShortNC();
		short						readShort(int pos);
		void						readShortArray(short* array, int num_shorts);
		void						readType(short& s);
		void						readTypeArray(short* array, int num_shorts);

		/**
		 * Read int data. readInt() reads one int from the IFStreamBinary and
		 * updates buffer iterator. The second form is like the first except
		 * that byte alignment is assumed to be ok (not checked). The third
		 * form specifies where to read but does not update the iterator or
		 * check alignment. The last form reads num_ints into the input array.
		 * readType is for templatization and behaves like readInt(). Similarly,
		 * readTypeArray behaves as readIntArray.
		 */
		int							readInt();
		int							readIntNC();
		int							readInt(int pos);
		void						readIntArray(int* array, int num_ints);
		void						readType(int& i);
		void						readTypeArray(int* array, int num_ints);

		/**
		 * Read long data. readLong() reads one long from the IFStreamBinary and
		 * updates buffer iterator. The second form is like the first except
		 * that byte alignment is assumed to be ok (not checked). The third
		 * form specifies where to read but does not update the iterator or
		 * check alignment. The last form reads num_longs into the input array.
		 * readType is for templatization and behaves like readLong(). Similarly,
		 * readTypeArray behaves as readLongArray.
		 */
		LONG_INT						readLong();
		LONG_INT						readLongNC();
		LONG_INT						readLong(int pos);
		void						readLongArray(LONG_INT* array, int num_longs);
		void						readType(LONG_INT& l);
		void						readTypeArray(LONG_INT* array, int num_longs);

		/**
		 * Read float data. readFloat() reads one float from the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment is assumed to be ok (not checked). The third
		 * form specifies where to read but does not update the iterator or
		 * check alignment. The last form reads num_floats into the input array.
		 * readType is for templatization and behaves like readFloat(). Similarly,
		 * readTypeArray behaves as readFloatArray.
		 */
		float						readFloat();
		float						readFloatNC();
		float						readFloat(int pos);
		void						readFloatArray(float* array, int num_floats);
		void						readType(float& f);
		void						readTypeArray(float* array, int num_floats);

		/**
		 * Read double data. readDouble() reads one double from the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment is assumed to be ok (not checked). The third
		 * form specifies where to read but does not update the iterator or
		 * check alignment. The last form reads num_doubles into the input array.
		 * readType is for templatization and behaves like readDouble(). Similarly,
		 * readTypeArray behaves as readDoubleArray.
		 */
		double					readDouble();
		double					readDoubleNC();
		double					readDouble(int pos);
		void						readDoubleArray(double* array, int num_doubles);
		void						readType(double& d);
		void						readTypeArray(double* array, int num_doubles);

		/**
		 * Write string data to 'this' IFStreamBinary. The two writeString functions
		 * first writes the the input string length immediately followed by the
		 * string data. The next function writes num_chars from the input character
		 * array into 'this' IFStreamBinary. readType is for templatization and
		 * behaves like readString(). Similarly, readTypeArray behaves like
		 * readCharArray.
		 */
		void						writeString(const string& str);
		void						writeString(const char* char_string);
		void						writeCharArray(const char* array, int num_chars);
		void            writeType(const string& str);
		void						writeType(const char* char_string);

		/**
		 * Write bool data. writeBool writes one byte to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that sufficient buffer size is not checked. The third form simply
		 * writes the data at the input position (pos) but does not check for
		 * buffer size, nor does it update the internal buffer iterator. The
		 * last method writes num_bools from the input array into 'this'
		 * IFStreamBinary. writeType is for templatization and behaves like
		 * writeBool. Similarly, writeTypeArray behaves like writeBoolArray.
		 */
		void						writeBool(bool b);
		void						writeBoolNC(bool b);
		void						writeBool(bool b, int pos);
		void						writeBoolArray(const bool* array, int num_bools);
		void						writeType(bool b);
		void						writeTypeArray(const bool* array, int num_bools);

		/**
		 * Write byte data. writeByte writes one byte to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that sufficient buffer size is not checked. The third form simply
		 * writes the data at the input position (pos) but does not check for
		 * buffer size, nor does it update the internal buffer iterator. The
		 * last method writes num_bytes from the input array into 'this'
		 * IFStreamBinary. writeType is for templatization and behaves like
		 * writeByte. Similarly, writeTypeArray behaves like writeByteArray.
		 */
		void						writeByte(byte b);
		void						writeByteNC(byte b);
		void						writeByte(byte b, int pos);
		void						writeByteArray(const byte* array, int num_bytes);
		void						writeType(byte b);
		void						writeTypeArray(const byte* array, int num_bytes);

		/**
		 * Write short data. writeShort writes one short to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment and sufficient buffer size are not checked. The
		 * third form simply writes the data at the input position (pos) but does
		 * not check for alignment, buffer size, nor does it update the internal
		 * buffer iterator. The last method writes num_shorts from the input
		 * array into 'this' IFStreamBinary. writeType is for templatization and
		 * behaves like writeShort. Similarly, writeTypeArray behaves like
		 * writeShortArray.
		 */
		void						writeShort(short i);
		void						writeShortNC(short i);
		void						writeShort(short i, int pos);
		void						writeShortArray(const short* array, int num_ints);
		void						writeType(short i);
		void						writeTypeArray(const short* array, int num_ints);

		/**
		 * Write int data. writeInt writes one int to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment and sufficient buffer size are not checked. The
		 * third form simply writes the data at the input position (pos) but does
		 * not check for alignment, buffer size, nor does it update the internal
		 * buffer iterator. The last method writes num_ints from the input
		 * array into 'this' IFStreamBinary. writeType is for templatization and
		 * behaves like writeInt. Similarly, writeTypeArray behaves like
		 * writeIntArray.
		 */
		void						writeInt(int i);
		void						writeIntNC(int i);
		void						writeInt(int i, int pos);
		void						writeIntArray(const int* array, int num_ints);
		void						writeType(int i);
		void						writeTypeArray(const int* array, int num_ints);

		/**
		 * Write long data. writeLong writes one long to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment and sufficient buffer size are not checked. The
		 * third form simply writes the data at the input position (pos) but does
		 * not check for alignment, buffer size, nor does it update the internal
		 * buffer iterator. The last method writes num_longs from the input
		 * array into 'this' IFStreamBinary. writeType is for templatization and
		 * behaves like writeLong. Similarly, writeTypeArray behaves like
		 * writeLongArray.
		 */
		void						writeLong(LONG_INT l);
		void						writeLongNC(LONG_INT l);
		void						writeLong(LONG_INT l, int pos);
		void						writeLongArray(const LONG_INT* array, int num_longs);
		void						writeType(LONG_INT l);
		void						writeTypeArray(const LONG_INT* array, int num_longs);

		/**
		 * Write float data. writeFloat writes one float to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment and sufficient buffer size are not checked. The
		 * third form simply writes the data at the input position (pos) but does
		 * not check for alignment, buffer size, nor does it update the internal
		 * buffer iterator. The last method writes num_floats from the input
		 * array into 'this' IFStreamBinary. writeType is for templatization and
		 * behaves like writeFloat. Similarly, writeTypeArray behaves like
		 * writeFloatArray.
		 */
		void						writeFloat(float f);
		void						writeFloatNC(float f);
		void						writeFloat(float f, int pos);
		void						writeFloatArray(const float* array, int num_floats);
		void						writeType(float f);
		void						writeTypeArray(const float* array, int num_floats);

		/**
		 * Write double data. writeDouble writes one double to the IFStreamBinary
		 * and updates buffer iterator. The second form is like the first except
		 * that byte alignment and sufficient buffer size are not checked. The
		 * third form simply writes the data at the input position (pos) but does
		 * not check for alignment, buffer size, nor does it update the internal
		 * buffer iterator. The last method writes num_doubles from the input
		 * array into 'this' IFStreamBinary. writeType is for templatization and
		 * behaves like writeDouble. Similarly, writeTypeArray behaves like
		 * writeDoubleArray.
		 */
		void						writeDouble(double d);
		void						writeDoubleNC(double d);
		void						writeDouble(double d, int pos);
		void						writeDoubleArray(const double* array, int num_doubles);
		void						writeType(double d);
		void						writeTypeArray(const double* array, int num_doubles);

		/*
		 * Return class name.
		 */
		static string		className()
		{ return "IFStreamBinary"; }

		/*
		 * Return class size.
		 */
		static int			classSize()
		{ return (int) sizeof(IFStreamBinary); }

		/**
		 * This function allocates spc bytes in the buffer. Used to
		 * minimize reallocation.
		 */
		void						reSize(int spc)
		{ bData->resize(spc); }

		/**
		 * Sets the storage capacity. Attained on next required resize.
		 */
		void						reserve(int sze)
		{ bData->reserve(sze); }

		/**
		 * Returns the IFStreamBinary size.
		 */
		int							size()
		{
			if (bDataPos > bSize) bSize = bDataPos;
			return bSize;
		}

		/**
		 * Reset the current iterator position to the beginning of the buffer.
		 */
		void						resetPos()
		{
			if (bDataPos > bSize) bSize = bDataPos;
			bDataPos = 0;
		}

		/**
		 * Set the current iterator position to the end of the buffer.
		 */
		void						setPosToEnd()
		{
			if (bDataPos > bSize) bSize = bDataPos;
			bDataPos = bSize;
		}

		/**
		 * Get the current iterator position.
		 */
		int							getPos() const
		{ return bDataPos; }

		/**
		 * Increment the iterator position.
		 */
		void						incrementPos(int increment = 1)
		{ bDataPos += increment; }

		/**
		 * Decrement the iterator position.
		 */
		void						decrementPos(int decrement = 1)
		{ bDataPos -= decrement; }

		/**
		 * Return a const reference to 'this' IFStreamBinarys data.
		 */
		const string&		getData() const
		{	return *bData; }

		/**
		 * Return the allocated capacity of this IFStreamBinary.
		 */
		int							getCapacity() const
		{ return (int) bData->capacity(); }

		/**
		 * Return a pointer at the current iterator location in the IFStreamBinary
		 * data.
		 */
		char*						getPosPointer()
		{	return getPosPointer(bDataPos); }

		/**
		 * Return a pointer to position pos in 'this' IFStreamBinarys data.
		 */
		char*						getPosPointer(int pos)
		{
			char* p = &(*bData)[pos];
			return p;
		}

		/**
		 * Sets the byte order reverse flag to bor.
		 */
		void						setByteOrderReverse(bool bor)
		{ bReverse = bor; }

		/**
		 * Turns on byte order reversal.
		 */
		void						byteOrderReverseOn()
		{ bReverse = true; }

		/**
		 * Turns off byte order reversal.
		 */
		void						byteOrderReverseOff()
		{ bReverse = false; }

		/**
		 * Returns true if the byte order in reads and writes is reversed for all
		 * 2, 4, or 8 byte intrinsincs (short, int, long, float, double, etc.).
		 */
		bool						isByteOrderReversalOn() const
		{ return bReverse; }

		/**
		 * Sets the intrinsic boundary alignment flag to align.
		 */
		void						setBoundaryAlignment(bool align)
		{ bAlign = align; }

		/**
		 * Turns on intrinsic boundary alignment.
		 */
		void						boundaryAlignmentOn()
		{ bAlign = true; }

		/**
		 * Turns off intrinsic boundary alignment.
		 */
		void						boundaryAlignmentOff()
		{ bAlign = false; }

		/**
		 * Returns true if the byte order in reads and writes is reversed for all
		 * 2, 4, or 8 byte intrinsincs (short, int, long, float, double, etc.).
		 */
		bool						isBoundaryAlignmentOn() const
		{ return bAlign; }

}; // end class IFStreamBinary

//************* Read Strings **************************************************

/**
 * Read in a single string and return it.
 */
inline string IFStreamBinary::readString()
{
  string s;
  readString(s);
  return s;
}

/**
 * Read in a string and assign it to s.
 */
inline void IFStreamBinary::readString(string& s)
{
  int len;
  // check for zero added by sballar 2013-01-04
  len = readInt();
  if (len == 0)
	  s="";
  else
  {
	  // read the data
	  s = bData->substr(bDataPos, len);
	  bDataPos += len;
  }
}

/**
 * Read in num_chars to the string s
 */
inline void IFStreamBinary::readCharArray(string& s, int num_chars)
{
	s.resize(num_chars);
	readCharArray(&s[0], num_chars);
}

/**
 * Read in num_chars to the presized character array
 */
inline void IFStreamBinary::readCharArray(char* array, int num_chars)
{
  // read the data

  memcpy(array, &(*bData)[bDataPos], num_chars);
  bDataPos += num_chars;
}

/**
 * Read in a string and assign it to s.
 */
inline void IFStreamBinary::readType(string& s)
{
	readString(s);
}

/**
 * Read in num_chars to the string s
 */
inline void IFStreamBinary::readTypeArray(string& s, int num_chars)
{
	readCharArray(s, num_chars);
}

//************* Read Bools ****************************************************

/**
 * Read in a single bool and return it.
 */
inline bool IFStreamBinary::readBool()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SBOL;
	return readBool(pos);
}

/**
 * Read in a single bool starting at the input position and return it.
 * Don't check for alignment.
 */
inline bool IFStreamBinary::readBool(int pos)
{
  return *((bool*) &(*bData)[pos]);
}

/**
 * Read in num_bools into the input array.
 */
inline void IFStreamBinary::readBoolArray(bool* array, int num_bools)
{
	// get bytes and perform alignment

  int num_bytes = num_bools * CPPUtils::SBOL;

	// copy from buffer into array ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  bDataPos += num_bytes;
}

/**
 * Read in a single bool and assign to s.
 */
inline void IFStreamBinary::readType(bool& b)
{
	b = readBool();
}

/**
 * Read in num_bools into the input array.
 */
inline void IFStreamBinary::readTypeArray(bool* array, int num_bools)
{
	readBoolArray(array, num_bools);
}

//************* Read Bytes ****************************************************

/**
 * Read in a single byte and return it.
 */
inline byte IFStreamBinary::readByte()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SBYT;
	return readByte(pos);
}

/**
 * Read in a single byte starting at the input position and return it.
 * Don't check for alignment.
 */
inline byte IFStreamBinary::readByte(int pos)
{
  return *((byte*) &(*bData)[pos]);
}

/**
 * Read in num_bytes into the input array.
 */
inline void IFStreamBinary::readByteArray(byte* array, int num_bytes)
{
	// get bytes and perform alignment

  num_bytes *= CPPUtils::SBYT;

	// copy from buffer into array ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  bDataPos += num_bytes;
}

/**
 * Read in a single byte and assign to s.
 */
inline void IFStreamBinary::readType(byte& b)
{
	b = readByte();
}

/**
 * Read in num_bytes into the input array.
 */
inline void IFStreamBinary::readTypeArray(byte* array, int num_bytes)
{
	readByteArray(array, num_bytes);
}

//************* Read Shorts ***************************************************

/**
 * Read in a single short and return it.
 */
inline short IFStreamBinary::readShort()
{
  align2Byte();
  return readShortNC();
}

/**
 * Read in a single short and return it. Don't check for alignment.
 */
inline short IFStreamBinary::readShortNC()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SSHT;
	return readShort(pos);
}

/**
 * Read in a single short starting at the input position and return it.
 * Don't check for alignment.
 */
inline short IFStreamBinary::readShort(int pos)
{
  // read the data ... use memcpy if alignment is off

	short s;
	if (bAlign)
    s = *((short*) &(*bData)[pos]);
	else
	  memcpy(&s, &(*bData)[pos], 2);

  if (bReverse) reverseBO2((char*) &s);
	return s;
}

/**
 * Read in num_shorts into the input array.
 */
inline void IFStreamBinary::readShortArray(short* array, int num_shorts)
{
	// get bytes and perform alignment

  int num_bytes = num_shorts * CPPUtils::SSHT;
  align2Byte();

	// copy from buffer into array ... reverse data if necessary ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  if (bReverse) reverseBO2Array(num_shorts, (char*) array);
  bDataPos += num_bytes;
}

/**
 * Read in a single short and assign to s.
 */
inline void IFStreamBinary::readType(short& s)
{
	s = readShort();
}

/**
 * Read in num_shorts into the input array.
 */
inline void IFStreamBinary::readTypeArray(short* array, int num_shorts)
{
	readShortArray(array, num_shorts);
}

//************* Read Ints *****************************************************

/**
 * Read in a single int and return it.
 */
inline int IFStreamBinary::readInt()
{
  align4Byte();
  return readIntNC();
}

/**
 * Read in a single int and return it. Don't check for alignment.
 */
inline int IFStreamBinary::readIntNC()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SINT;
	return readInt(pos);
}

/**
 * Read in a single int starting at the input position and return it.
 * Don't check for alignment.
 */
inline int IFStreamBinary::readInt(int pos)
{
  // read the data ... use memcpy if alignment is off

	int i;
	if (bAlign)
		i = *((int*) &(*bData)[pos]);
	else
		memcpy(&i, &(*bData)[pos], 4);

  if (bReverse) reverseBO4((char*) &i);
	return i;
}

/**
 * Read in num_ints into the input array.
 */
inline void IFStreamBinary::readIntArray(int* array, int num_ints)
{
	// get bytes and perform alignment

  int num_bytes = num_ints * CPPUtils::SINT;
  align4Byte();

	// copy from buffer into array ... reverse data if necessary ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  if (bReverse) reverseBO4Array(num_ints, (char*) array);
  bDataPos += num_bytes;
}

/**
 * Read in a single int and assign to i.
 */
inline void IFStreamBinary::readType(int& i)
{
	i = readInt();
}

/**
 * Read in num_ints into the input array.
 */
inline void IFStreamBinary::readTypeArray(int* array, int num_ints)
{
	readIntArray(array, num_ints);
}

//************* Read Longs ****************************************************

/**
 * Read in a single long and return it.
 */
inline LONG_INT IFStreamBinary::readLong()
{
  align8Byte();
  return readLongNC();
}

/**
 * Read in a single long and return it. Don't check for alignment.
 */
inline LONG_INT IFStreamBinary::readLongNC()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SLNG;
	return readLong(pos);
}

/**
 * Read in a single long starting at the input position and return it.
 * Don't check for alignment.
 */
inline LONG_INT IFStreamBinary::readLong(int pos)
{
  // read the data ... use memcpy if alignment is off

	LONG_INT l;
	if (bAlign)
		l = *((LONG_INT*) &(*bData)[pos]);
	else
	  memcpy(&l, &(*bData)[pos], 8);

  if (bReverse) reverseBO8((char*) &l);
	return l;
}

/**
 * Read in num_longs into the input array.
 */
inline void IFStreamBinary::readLongArray(LONG_INT* array, int num_longs)
{
	// get bytes and perform alignment

  int num_bytes = num_longs * CPPUtils::SLNG;
  align8Byte();

	// copy from buffer into array ... reverse data if necessary ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  if (bReverse) reverseBO8Array(num_longs, (char*) array);
  bDataPos += num_bytes;
}

/**
 * Read in a single long and assign to l.
 */
inline void IFStreamBinary::readType(LONG_INT& l)
{
	l = readLong();
}

/**
 * Read in num_longs into the input array.
 */
inline void IFStreamBinary::readTypeArray(LONG_INT* array, int num_longs)
{
	readLongArray(array, num_longs);
}

//************* Read Floats ***************************************************

/**
 * Read in a single float and return it.
 */
inline float IFStreamBinary::readFloat()
{
  align4Byte();
  return readFloatNC();
}

/**
 * Read in a single float and return it. Don't check for alignment.
 */
inline float IFStreamBinary::readFloatNC()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SFLT;
	return readFloat(pos);
}

/**
 * Read in a single float starting at the input position and return it.
 * Don't check for alignment.
 */
inline float IFStreamBinary::readFloat(int pos)
{
  // read the data ... use memcpy if alignment is off

	float f;
	if (bAlign)
		f = *((float*) &(*bData)[pos]);
	else
	  memcpy(&f, &(*bData)[pos], 4);

  if (bReverse) reverseBO4((char*) &f);
	return f;
}

/**
 * Read in num_floats into the input array.
 */
inline void IFStreamBinary::readFloatArray(float* array, int num_floats)
{
	// get bytes and perform alignment

  int num_bytes = num_floats * CPPUtils::SFLT;
  align4Byte();

	// copy from buffer into array ... reverse data if necessary ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  if (bReverse) reverseBO4Array(num_floats, (char*) array);
  bDataPos += num_bytes;
}

/**
 * Read in a single float and assign to f.
 */
inline void IFStreamBinary::readType(float& f)
{
	f = readFloat();
}

/**
 * Read in num_floats into the input array.
 */
inline void IFStreamBinary::readTypeArray(float* array, int num_floats)
{
	readFloatArray(array, num_floats);
}

//************* Read Doubles **************************************************

/**
 * Read in a single double and return it.
 */
inline double IFStreamBinary::readDouble()
{
  align8Byte();
  return readDoubleNC();
}

/**
 * Read in a single double and return it. Don't check for alignment.
 */
inline double IFStreamBinary::readDoubleNC()
{
  // read the data

	int pos = bDataPos;
	bDataPos += CPPUtils::SDBL;
	return readDouble(pos);
}

/**
 * Read in a single double starting at the input position and return it.
 * Don't check for alignment.
 */
inline double IFStreamBinary::readDouble(int pos)
{
  // read the data ... use memcpy if alignment is off

	double d;
	if (bAlign)
		d = *((double*) &(*bData)[pos]);
	else
	  memcpy(&d, &(*bData)[pos], 8);

  if (bReverse) reverseBO8((char*) &d);
	return d;
}

/**
 * Read in num_doubles into the input array.
 */
inline void IFStreamBinary::readDoubleArray(double* array, int num_doubles)
{
	// get bytes and perform alignment

  int num_bytes = num_doubles * CPPUtils::SDBL;
  align8Byte();

	// copy from buffer into array ... reverse data if necessary ...
	// update buffer position and exit

  memcpy(array, &(*bData)[bDataPos], num_bytes);
  if (bReverse) reverseBO8Array(num_doubles, (char*) array);
  bDataPos += num_bytes;
}

/**
 * Read in a single double and assign to d.
 */
inline void IFStreamBinary::readType(double& d)
{
	d = readDouble();
}

/**
 * Read in num_doubles into the input array.
 */
inline void IFStreamBinary::readTypeArray(double* array, int num_doubles)
{
	readDoubleArray(array, num_doubles);
}

//************* Write Strings *************************************************

/**
 * Write the input string at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit. Both the string size (int) and
 * the strings data are written.
 */
inline void IFStreamBinary::writeString(const string& str)
{
  // get total size, align, and check capacity

	int sz = (int) str.size() + CPPUtils::SINT;
  align4Byte();
  checkBufferSize(sz);

  // write the data

	writeIntNC((int) str.size());
  memcpy(&(*bData)[bDataPos], &str[0], str.size());
  bDataPos += (int) str.size();
}

/**
 * Write the null terminated character string at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the internal buffer
 * position on exit. Both the string size (int) and its data are written.
 */
inline void IFStreamBinary::writeString(const char* char_string)
{
  string tmp_string = char_string;
  writeString(tmp_string);
}

/**
 * Write num_chars from the input character array to the output buffer.
 */
inline void IFStreamBinary::writeCharArray(const char* array, int num_chars)
{
  checkBufferSize(num_chars);

  // write data

  memcpy(&(*bData)[bDataPos], array, num_chars);
  bDataPos += num_chars;
}

/**
 * Write the input string at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit. Both the string size (int) and
 * the strings data are written.
 */
inline void IFStreamBinary::writeType(const string& str)
{
	writeString(str);
}

/**
 * Write the null terminated character string at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the internal buffer
 * position on exit. Both the string size (int) and its data are written.
 */
inline void IFStreamBinary::writeType(const char* char_string)
{
	writeString(char_string);
}

//************* Write Bools ***************************************************

/**
 * Write the input bool at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeBool(bool b)
{
	checkBufferSize(CPPUtils::SBOL);

  writeBoolNC(b);
}

/**
 * Write the input bool at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeBoolNC(bool b)
{
	writeBool(b, bDataPos);
  bDataPos += CPPUtils::SBOL;
}

/**
 * Write the input bool at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeBool(bool b, int pos)
{
  *((bool*) &(*bData)[pos]) = b;
}

/**
 * Write num_bools from the input bool array to the output buffer.
 */
inline void IFStreamBinary::writeBoolArray(const bool* array, int num_bools)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  num_bools *= CPPUtils::SBOL;
  checkBufferSize(num_bools);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bools);
  bDataPos += num_bools;
}

/**
 * Write the input bool at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(bool b)
{
	writeBool(b);
}

/**
 * Write num_bools from the input bool array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const bool* array, int num_bools)
{
	writeBoolArray(array, num_bools);
}

//************* Write Bytes ***************************************************

/**
 * Write the input byte at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeByte(byte b)
{
  checkBufferSize(CPPUtils::SBYT);

  writeByteNC(b);
}

/**
 * Write the input byte at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeByteNC(byte b)
{
	writeByte(b, bDataPos);
  bDataPos += CPPUtils::SBYT;
}

/**
 * Write the input byte at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeByte(byte b, int pos)
{
  *((byte*) &(*bData)[pos]) = b;
}

/**
 * Write num_bytes from the input byte array to the output buffer.
 */
inline void IFStreamBinary::writeByteArray(const byte* array, int num_bytes)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  num_bytes *= CPPUtils::SBYT;
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  bDataPos += num_bytes;
}

/**
 * Write the input byte at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(byte b)
{
	writeByte(b);
}

/**
 * Write num_bytes from the input byte array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const byte* array, int num_bytes)
{
	writeByteArray(array, num_bytes);
}

//************* Write Shorts **************************************************

/**
 * Write the input short at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeShort(short s)
{
  align2Byte();
  checkBufferSize(CPPUtils::SSHT);

  writeShortNC(s);
}

/**
 * Write the input short at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeShortNC(short s)
{
	writeShort(s, bDataPos);
  bDataPos += CPPUtils::SSHT;
}

/**
 * Write the input short at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeShort(short s, int pos)
{
  if (bReverse) reverseBO2((char*) &s);

  if (bAlign)
  	*((short*) &(*bData)[pos]) = s;
  else
    memcpy(&(*bData)[pos], &s, CPPUtils::SSHT);
}

/**
 * Write num_shorts from the input short array to the output buffer.
 */
inline void IFStreamBinary::writeShortArray(const short* array, int num_shorts)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  int num_bytes = num_shorts * CPPUtils::SSHT;
  align2Byte();
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  if (bReverse) reverseBO2Array(num_shorts, &(*bData)[bDataPos]);
  bDataPos += num_bytes;
}

/**
 * Write the input short at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(short s)
{
	writeShort(s);
}

/**
 * Write num_shorts from the input short array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const short* array, int num_shorts)
{
	writeShortArray(array, num_shorts);
}

//************* Write Ints ****************************************************

/**
 * Write the input int at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeInt(int i)
{
  align4Byte();
  checkBufferSize(CPPUtils::SINT);

  writeIntNC(i);
}

/**
 * Write the input int at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeIntNC(int i)
{
	writeInt(i, bDataPos);
  bDataPos += CPPUtils::SINT;
}

/**
 * Write the input int at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeInt(int i, int pos)
{
  if (bReverse) reverseBO4((char*) &i);

  if (bAlign)
  	*((int*) &(*bData)[pos]) = i;
  else
    memcpy(&(*bData)[pos], &i, CPPUtils::SINT);
}

/**
 * Write num_ints from the input int array to the output buffer.
 */
inline void IFStreamBinary::writeIntArray(const int* array, int num_ints)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  int num_bytes = num_ints * CPPUtils::SINT;
  align4Byte();
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  if (bReverse) reverseBO4Array(num_ints, &(*bData)[bDataPos]);
  bDataPos += num_bytes;
}

/**
 * Write the input int at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(int i)
{
	writeInt(i);
}

/**
 * Write num_ints from the input int array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const int* array, int num_ints)
{
	writeIntArray(array, num_ints);
}

//************* Write Longs ***************************************************

/**
 * Write the input long at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeLong(LONG_INT l)
{
  align8Byte();
  checkBufferSize(CPPUtils::SLNG);

  writeLongNC(l);
}

/**
 * Write the input long at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeLongNC(LONG_INT l)
{
	writeLong(l, bDataPos);
  bDataPos += CPPUtils::SLNG;
}

/**
 * Write the input long at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeLong(LONG_INT l, int pos)
{
  if (bReverse) reverseBO8((char*) &l);

  if (bAlign)
  	*((LONG_INT*) &(*bData)[pos]) = l;
  else
    memcpy(&(*bData)[pos], &l, CPPUtils::SLNG);
}

/**
 * Write num_longs from the input long array to the output buffer.
 */
inline void IFStreamBinary::writeLongArray(const LONG_INT* array, int num_longs)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  int num_bytes = num_longs * CPPUtils::SLNG;
  align8Byte();
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  if (bReverse) reverseBO8Array(num_longs, &(*bData)[bDataPos]);
  bDataPos += num_bytes;
}

/**
 * Write the input long at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(LONG_INT l)
{
	writeLong(l);
}

/**
 * Write num_longs from the input long array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const LONG_INT* array, int num_longs)
{
	writeLongArray(array, num_longs);
}

//************* Write Floats **************************************************

/**
 * Write the input float at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeFloat(float f)
{
  align4Byte();
  checkBufferSize(CPPUtils::SFLT);

  writeFloatNC(f);
}

/**
 * Write the input float at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeFloatNC(float f)
{
	writeFloat(f, bDataPos);
  bDataPos += CPPUtils::SFLT;
}

/**
 * Write the input float at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeFloat(float f, int pos)
{
  if (bReverse) reverseBO4((char*) &f);

  if (bAlign)
  	*((float*) &(*bData)[pos]) = f;
  else
    memcpy(&(*bData)[pos], &f, CPPUtils::SFLT);
}

/**
 * Write num_floats from the input float array to the output buffer.
 */
inline void IFStreamBinary::writeFloatArray(const float* array, int num_floats)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  int num_bytes = num_floats * CPPUtils::SFLT;
  align4Byte();
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  if (bReverse) reverseBO4Array(num_floats, &(*bData)[bDataPos]);
  bDataPos += num_bytes;
}

/**
 * Write the input float at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(float f)
{
	writeFloat(f);
}

/**
 * Write num_floats from the input float array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const float* array, int num_floats)
{
	writeFloatArray(array, num_floats);
}

//************* Write Doubles *************************************************

/**
 * Write the input double at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeDouble(double d)
{
  align8Byte();
	checkBufferSize(CPPUtils::SDBL);

  writeDoubleNC(d);
}

/**
 * Write the input double at the current buffer position.
 * Don't check for alignment or sufficient buffer size. Increment
 * the internal buffer position on exit.
 */
inline void IFStreamBinary::writeDoubleNC(double d)
{
	writeDouble(d, bDataPos);
  bDataPos += CPPUtils::SDBL;
}

/**
 * Write the input double at the input buffer position. Don't
 * check for alignment or sufficient buffer size. Don't update
 * the internal buffer position.
 */
inline void IFStreamBinary::writeDouble(double d, int pos)
{
  if (bReverse) reverseBO8((char*) &d);

  if (bAlign)
  	*((double*) &(*bData)[pos]) = d;
  else
    memcpy(&(*bData)[pos], &d, CPPUtils::SDBL);
}

/**
 * Write num_doubles from the input double array to the output buffer.
 */
inline void IFStreamBinary::writeDoubleArray(const double* array, int num_doubles)
{
	// get number of bytes to be written and check for alignment and buffer
	// size

  int num_bytes = num_doubles * CPPUtils::SDBL;
  align8Byte();
  checkBufferSize(num_bytes);

	// copy array into data buffer. Reverse data buffer entries if necessary
	// and increment the buffer output pointer.

  memcpy(&(*bData)[bDataPos], array, num_bytes);
  if (bReverse) reverseBO8Array(num_doubles, &(*bData)[bDataPos]);
  bDataPos += num_bytes;
}

/**
 * Write the input double at the current buffer position.
 * Ensure alignment and sufficient buffer size. Increment the
 * internal buffer position on exit.
 */
inline void IFStreamBinary::writeType(double d)
{
	writeDouble(d);
}

/**
 * Write num_doubles from the input double array to the output buffer.
 */
inline void IFStreamBinary::writeTypeArray(const double* array, int num_doubles)
{
	writeDoubleArray(array, num_doubles);
}

//************* Byte reversal functions ***************************************

/**
 * Reverses byte order of d. d is assumed to point to an 2 byte element.
 */
inline void IFStreamBinary::reverseBO2(char* d)
{
  char tmp = d[0];
  d[0] = d[1];
  d[1] = tmp;
}

/**
 * Reverses byte order of d. d is assumed to point to an 4 byte element.
 */
inline void IFStreamBinary::reverseBO4(char* d)
{
  char tmp = d[0];
  d[0] = d[3];
  d[3] = tmp;

  tmp = d[1];
  d[1] = d[2];
  d[2] = tmp;
}

/**
 * Reverses byte order of d. d is assumed to point to an 8 byte element.
 */
inline void IFStreamBinary::reverseBO8(char* d)
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

} // end namespace geotess

#endif  // IFSTREAMBINARY_OBJECT_H
