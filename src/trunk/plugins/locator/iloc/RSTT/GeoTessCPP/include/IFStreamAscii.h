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

#ifndef IFSTREAMASCII_OBJECT_H
#define IFSTREAMASCII_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
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
 * \brief Opens ascii file for read and write access.
 *
 * Opens ascii file for read and write access. The read portion is configured
 * to parse generically formatted ascii files from an input stream.
 *
 * Functionality is provided to read elemental, strings, booleans, bytes,
 * shorts, ints, longs, floats, or doubles, as-well-as array input forms of
 * each (except strings). Alternatively, indivdual lines can be read and
 * parsed by the caller if desired. To perform elemental reads a tokenized
 * scanner is provided that must be configured by the user with a White-Space
 * (WS) definition. The WS delimiters default to a space, tab, and a comma.
 * A single comment line delimiter and block comment start and end delimiters
 * can also be set by the caller. These default to "//", "slash-*", and '*-slash"
 * respectively.
 */
class GEOTESS_EXP_IMP IFStreamAscii
{
  private:

		/**
		 * Input stream.
		 */
		ifstream			ifs;

		/**
		 * Output stream.
		 */
		ofstream			ofs;

		/**
     * Parser delimiter strings.
     *
     * Delimiters include:
     * <ul>
     * <li>[0] = whitespace,</li>
     * <li>[1] = string,</li>
     * <li>[2] = comment,</li>
     * <li>[3] = begin-block comment,</li>
     * <li>[4] = end-block comment.</li>
     * </ul>
     * These strings default to space, comma and tab for
     * whitespace, double-quote for string, double-slash for comment,
     * and slash-* and *-slash for begin and end block comments.
		 */
    string						strDelim[5];

		/**
		 * Opened file name.
		 */
		string						strFileName;

    // The total number of lines read from this stream
    int								strTotlLinesRead;
    // The total number of "data" lines read from this stream
    int								strDataLinesRead;
    // The total number of "blank" lines read from this stream
    int								strBlankLinesRead;
    // The total number of "comment" lines read from this stream
    int								strCommentLinesRead;
    // The total number of "block comment" lines read from this stream
    int								strBlkCommentLinesRead;
    // The total number of "bytes" read from this stream
    int								strBytesRead;

		/**
     * Set to true while the file line reader reads lines searching
     * for the end block comment delimiter.
		 */
    bool							strBlkCommntSet;

		/**
     * Index of the next token to be read from the stream token list.
     *
     * This parameter is only modified when a new stream
     * is set (constructors and the function set) and in the private function
     * read(string& s).
		 */
    int								strTokenPtr;

		/**
     * The stream token list.
     *
     *  This parameter is only modified when a new stream
     *  is set (constructors and the function set) and in the private function
     *  read(string& s). The list and pointer point to the next token availble
     *  in the current stream. When strTokenPtr == strTokens.size() it is
     *  zeroed and a new line is read (read_line) from the stream. The new
     *  line is then tokenized into the vector strTokens.
		 */
    vector<string>		strTokens;

	public:

	/**
	 * Returns the next line in the input stream in the string buf.
	 */
	void							getLine(string& buf);

	/**
	 * Read a single line from the underlying istream.
	 * Added by sballar 2013-03-22.
	 */
	void getline(string& s) { std::getline(ifs, s); };

		/**
		 * Default constructor.
		 */
											IFStreamAscii() :	strFileName(""), strTotlLinesRead(0),
																				strDataLinesRead(0), strBlankLinesRead(0),
																				strCommentLinesRead(0), strBlkCommentLinesRead(0),
																				strBytesRead(0), strBlkCommntSet(false),
																				strTokenPtr(0)
											{setDefaultDelimiters(); setCommentDelimiter("#"); };

		/**
		 * Destructor.
		 */
		virtual						~IFStreamAscii() { close(); };

		/**
		 * Open stream for read or write access.
		 */
		void							openForRead(const string& fn);
		void							openForWrite(const string& fn);

		/**
		 * Returns true if the stream is open.
		 */
		bool							isOpen() { return (ifs.is_open() || ofs.is_open()) ? true : false; }

		/**
		 * Close the input stream if it is open.
		 */
		void							close()
											{
												if (ifs.is_open())
													ifs.close();
												else if (ofs.is_open())
													ofs.close();
											}

		/**
		 * Close the input stream if it is open.
		 */
		void							flush()
											{
												if (ofs.is_open())
													ofs.flush();
											}

		/**
		 * Reads the next token (string) from the input file stream. If eof()
		 * occurs false is returned. Otherwise true is returned.
		 */
		bool							read(string& token);

		/**
		 * Read a new line into ln from the stream. If eof() then return false.
		 * Otherwise return true.
		 */
		bool							readLine(string& ln);

		/**
		 * Tokenize the input string (str) and place the tokens into the input
		 * array list.
		 */
		void							tokenize(const string& str, vector<string>& tokens);

		/**
		 * Return the opened file name.
		 */
		const string&			getFileName() const { return strFileName; };

		/**
		 * Sets the default delimiter settings.
		 */
		void							setDefaultDelimiters();

		/**
		 * Resets all parameters back to their initial state (excluding delimiters).
		 */
		void							resetReader();

		/**
		 * Sets the whitespace, string, comment, and block comment
		 * delimiters for this IFStreamAscii object.
		 */
		void							setDelimiters(const string& wspcDelims,
																		const string& strgDelim,
																		const string& cmntDelim,
																		const string& begBlk,
																		const string& endBlk)
		{
			strDelim[0] = wspcDelims;
			strDelim[1] = strgDelim;
			strDelim[2] = cmntDelim;
			strDelim[3] = begBlk;
			strDelim[4] = endBlk;
		}

		/**
		 * Set the whitespace delimiter string.
		 */
		void						setWhitespaceDelimiters(const string& wsDelims)
										{ strDelim[0] = wsDelims; };

		/**
		 * Return the whitespace delimiter string.
		 */
		const string&		getWhitespaceDelimiters() const
										{ return strDelim[0]; };

		/**
		 * Set the string delimiter string.
		 */
		void						setStringDelimiter(const string& strgDelim)
										{ strDelim[1] = strgDelim; };

		/**
		 * Return the string delimiter string.
		 */
		const string&		getStringDelimiter() const
										{ return strDelim[1]; };

		/**
		 * Set the comment delimiter string.
		 */
		void						setCommentDelimiter(const string& cmntDelim)
										{ strDelim[2] = cmntDelim; };

		/**
		 * Return the comment delimiter string.
		 */
		const string&		getCommentDelimiter() const
										{ return strDelim[2]; };

		/**
		 * Sets the begin and end block comment strings for this
		 * IFStreamAscii object.
		 */
		void							setBlockCommentDelimiters(const string& begBlk,
																								const string& endBlk)
		{
			strDelim[3] = begBlk;
			strDelim[4] = endBlk;
		}

		/**
		 * Set the begin block comment delimiter string.
		 */
		void							setBeginBlockCommentDelimiter(const string& begBlkCmntDelim)
											{ strDelim[3] = begBlkCmntDelim; };

		/**
		 * Return the begin block comment delimiter string.
		 */
		const string&			getBeginBlockCommentDelimiter() const
											{	return strDelim[3]; };

		/**
		 * Set the end block comment delimiter string.
		 */
		void							setEndBlockCommentDelimiter(const string& endBlkCmntDelim)
											{ strDelim[4] = endBlkCmntDelim; };

		/**
		 * Return the end block comment delimiter string.
		 */
		const string&			getEndBlockCommentDelimiter() const
											{	return strDelim[4]; };

		/**
		 * Return the current number of total lines read from the input stream.
		 */
		int								getTotalLinesRead() const
											{	return strTotlLinesRead; };

		/**
		 * Return the current number of data lines read from the input stream.
		 */
		int								getDataLinesRead() const
											{ return strDataLinesRead; };

		/**
		 * Return the current number of blank lines read from the input stream.
		 */
		int								getBlankLinesRead() const
											{ return strBlankLinesRead; };

		/**
		 * Return the current number of comment lines read from the input stream.
		 */
		int								getCommentLinesRead() const
											{ return strCommentLinesRead; };

		/**
		 * Return the current number of block comment lines read from the input stream.
		 */
		int								getBlockCommentLinesRead() const
											{ return strBlkCommentLinesRead; };

		/**
		 * Return the current number of bytes read from the input stream.
		 */
		int								getBytesRead() const
											{ return strBytesRead; };

		/**
		 * Returns true if EOF is reached.
		*/
		bool							isEOF() const;

		/**
		 * Skips to the next token.
		 */
		bool							next();

		/**
		 * Read the next string. Return true if SUCCESSFUL
		 */
		string						readString();

		/**
		 * Read the next string. Return true if SUCCESSFUL
		 */
		bool							readString(string& s);

		/**
		 * Read and return the next byte.
		 */
		byte							readByte();

		/**
		 * Read the next byte. Return true if SUCCESSFUL
		 */
		bool							readType(byte& b) { return readByte(b); };

		/**
		 * Read the next byte. Return true if SUCCESSFUL
		 */
		bool							readByte(byte& b);

		/**
		 * Read and return the next short.
		 */
		short							readShort();

		/**
		 * Read the next short. Return true if SUCCESSFUL
		 */
		bool							readType(short& s) { return readShort(s); };

		/**
		 * Read the next short. Return true if SUCCESSFUL
		 */
		bool							readShort(short& s);

		/**
		 * Read and return the next int.
		 */
		int								readInteger();

		/**
		 * Read the next int. Return true if SUCCESSFUL
		 */
		bool							readType(int& i) { return readInteger(i); };

		/**
		 * Read the next int. Return true if SUCCESSFUL
		 */
		bool							readInteger(int& i);

		/**
		 * Read and return the next long.
		 */
		LONG_INT							readLong();

		/**
		 * Read the next long. Return true if SUCCESSFUL
		 */
		bool							readType(LONG_INT& l) { return readLong(l); };

		/**
		 * Read the next long. Return true if SUCCESSFUL
		 */
		bool							readLong(LONG_INT& l);

		/**
		 * Read and return the next float.
		 */
		float							readFloat();

		/**
		 * Read the next float. Return true if SUCCESSFUL
		 */
		bool							readType(float& f) { return readFloat(f); };

		/**
		 * Read the next float. Return true if SUCCESSFUL
		 */
		bool							readFloat(float& f);

		/**
		 * Read and return the next double.
		 */
		double						readDouble();

		/**
		 * Read the next double. Return true if SUCCESSFUL
		 */
		bool							readType(double& d) { return readDouble(d); };

		/**
		 * Read the next double. Return true if SUCCESSFUL
		 */
		bool							readDouble(double& d);

		// ***** Write functions **************************************************

		void							writeString(const string& s) {ofs << s;}
		void							writeStringNL(const string& s) {ofs << s << endl;}
		void							writeType(const string& s) {ofs << s;}
		void							writeTypeNL(const string& s) {ofs << s << endl;}
		void							writeBool(bool b) {ofs << b;}
		void							writeBoolNL(bool b) {ofs << b << endl;}
		void							writeType(bool b) {ofs << b;}
		void							writeTypeNL(bool b) {ofs << b << endl;}
		void							writeByte(byte b) {ofs << (int)b;}
		void							writeByteNL(byte b) {ofs << (int)b << endl;}
		void							writeType(byte b) {ofs << (int)b;}
		void							writeTypeNL(byte b) {ofs << (int)b << endl;}
		void							writeShort(short s) {ofs << s;}
		void							writeShortNL(short s) {ofs << s << endl;}
		void							writeType(short s) {ofs << s;};
		void							writeTypeNL(short s) {ofs << s << endl;};
		void							writeInt(int i) {ofs << i;}
		void							writeIntNL(int i) {ofs << i << endl;}
		void							writeType(int i) {ofs << i;}
		void							writeTypeNL(int i) {ofs << i << endl;}
		void							writeLong(LONG_INT l) {ofs << l;}
		void							writeLongNL(LONG_INT l) {ofs << l << endl;}
		void							writeType(LONG_INT l) {ofs << l;}
		void							writeTypeNL(LONG_INT l) {ofs << l << endl;}
		void							writeFloat(float f) {ofs << f;}
		void							writeFloatNL(float f) {ofs << f << endl;}
		void							writeType(float f) {ofs << f;}
		void							writeTypeNL(float f) {ofs << f << endl;}
		void							writeDouble(double d) {ofs << d;}
		void							writeDoubleNL(double d) {ofs << d << endl;}
		void							writeType(double d) {ofs << d;}
		void							writeTypeNL(double d) {ofs << d << endl;}
		void							writeNL() {ofs << endl;}

}; // end class IFStreamAscii

/**
 * Reads and returns a single token from the stream.
 *
 * The new token is contained in the input string reference token. The
 * function returns true if successful. If eof() is reached false is returned.
 */
inline bool IFStreamAscii::read(string& token)
{
  string ln;

  // if the strTokenPtr is >= than strTokens.size() then get more tokens

  if (strTokenPtr >= (int) strTokens.size())
  {
    // get more tokens ... reset token pointer and list

    strTokenPtr = 0;
    strTokens.clear();

    // read another data line from the stream ... if eof() is reached return
    // true

    if (!readLine(ln)) return false;

    // tokennize the line into the tokens list

    tokenize(ln, strTokens);
  }

  // get the token ... increment the pointer ... and return false for success

  token = strTokens[strTokenPtr++];
  return true;
}

/**
 * Returns true if EOF is reached.
 */
inline bool IFStreamAscii::isEOF() const
{
  return (ifs.eof() && (strTokenPtr >= (int) strTokens.size()));
}

/**
 * Read the next string. Return true if SUCCESSFUL
 */
inline string IFStreamAscii::readString()
{
	string s;
  read(s);
	return s;
}

/**
 * Read the next string. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readString(string& s)
{
  return read(s);
}

/**
 * Read the next string. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::next()
{
	string s;
  return read(s);
}

/**
 * Read and return the next byte.
 */
inline byte IFStreamAscii::readByte()
{
	byte b = 0;
	readByte(b);
	return b;
}

/**
 * Read the next byte. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readByte(byte& b)
{
  string sb;

  // read token into sb ... if eof or stream not open return false

  if (!read(sb)) return false;

  // attempt to scan token into b ... if unable issue error and return false

  if (sscanf(sb.c_str(), "%hhd", &b) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readByte" << endl
			 << "  Could Not Scan Byte From Token = " << sb << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9201);
  }

  // successful ... return true

  return true;
}

/**
 * Read and return the next short.
 */
inline short IFStreamAscii::readShort()
{
	short s = 0;
	readShort(s);
	return s;
}

/**
 * Read the next short. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readShort(short& s)
{
  string ss;

  // read token into ss ... if eof or stream not open return false

  if (!read(ss)) return false;

  // attempt to scan token into s ... if unable issue error and return false

  if (sscanf(ss.c_str(), "%hd", &s) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readShort" << endl
			 << "  Could Not Scan Short From Token = " << ss << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9202);
  }

  // successful ... return true

  return true;
}

/**
 * Read and return the next int.
 */
inline int IFStreamAscii::readInteger()
{
	int i = 0;
	readInteger(i);
	return i;
}

/**
 * Read the next int. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readInteger(int& i)
{
  string si;

  // read token into si ... if eof or stream not open return false

  if (!read(si)) return false;

  // attempt to scan token into i ... if unable issue error and return false

  if (sscanf(si.c_str(), "%d", &i) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readInteger" << endl
			 << "  Could Not Scan Integer From Token = " << si << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9203);
  }

  // successful ... return true

  return true;
}

/**
 * Read and return the next long.
 */
inline LONG_INT IFStreamAscii::readLong()
{
	LONG_INT l = 0;
	readLong(l);
	return l;
}

/**
 * Read the next long. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readLong(LONG_INT& l)
{
  string sl;

  // read token into sl ... if eof or stream not open return false

  if (!read(sl)) return false;

  // attempt to scan token into l ... if unable issue error and return false

  if (sscanf(sl.c_str(), LONG_INT_F, &l) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readLong" << endl
			 << "  Could Not Scan Long From Token = " << sl << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9204);
  }

  // successful ... return true

  return true;
}

/**
 * Read and return the next float.
 */
inline float IFStreamAscii::readFloat()
{
	float f = 0;
	readFloat(f);
	return f;
}

/**
 * Read the next float. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readFloat(float& f)
{
  string sf;

  // read token into sf ... if eof or stream not open return false

  if (!read(sf)) return false;

  // attempt to scan token into f ... if unable issue error and return false

  if (sscanf(sf.c_str(), "%f", &f) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readFloat" << endl
			 << "  Could Not Scan Float From Token = " << sf << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9205);
  }

  // successful ... return true

  return true;
}

/**
 * Read and return the next double.
 */
inline double IFStreamAscii::readDouble()
{
	double d = 0;
	readDouble(d);
	return d;
}

/**
 * Read the next double. Return true if SUCCESSFUL
 */
inline bool IFStreamAscii::readDouble(double& d)
{
  string sd;

  // read token into sd ... if eof or stream not open return false

  if (!read(sd)) return false;

  // attempt to scan token into d ... if unable issue error and return false

  if (sscanf(sd.c_str(), "%lf", &d) != 1)
  {
    ostringstream os;
		os << endl << "ERROR in IFStreamAscii::readDouble" << endl
			 << "  Could Not Scan Double From Token = " << sd << endl
       << "  On File Line: " << strTotlLinesRead << " ..." << endl;
   	throw GeoTessException(os, __FILE__, __LINE__, 9206);
  }

  // successful ... return true

  return true;
}

} // end namespace geotess

#endif  // INTERPOLATOR_OBJECT_H
