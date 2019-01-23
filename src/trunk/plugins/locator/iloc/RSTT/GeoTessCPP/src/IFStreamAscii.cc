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
#include "IFStreamAscii.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Opens a stream for ascii read.
 */
void	IFStreamAscii::openForRead(const string& fn)
{
	resetReader();
	strFileName = fn;
	ifs.open(fn.c_str(), std::ios::in);
	if (!ifs.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::readFromFile" << endl
			 << "Could not open input file: " << fn << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9207);
	}
}

/**
 * Opens a stream for ascii write.
 */
void	IFStreamAscii::openForWrite(const string& fn)
{
	resetReader();
	strFileName = fn;
	ofs.open(fn.c_str(), std::ios::out);
	if (!ofs.is_open())
	{
		ostringstream os;
		os << endl << "ERROR in IFStreamBinary::writeToFile" << endl
			 << "Could not open output file: " << fn << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 9208);
	}
	ofs.precision(numeric_limits<double>::digits10 + 2);
}

/**
 * Resets all parameters back to their initial state (excluding delimiters).
 */
void IFStreamAscii::resetReader()
{
  // if file is open close it

  close();

  // reset lines read and file name

  strTotlLinesRead = strDataLinesRead = strBytesRead = 0;
  strBlankLinesRead = strCommentLinesRead = strBlkCommentLinesRead = 0;
	strBlkCommntSet  = false;
  strFileName = "";

  // clear tokens and reset token pointer

  strTokenPtr = 0;
  strTokens.clear();
}

/**
 * Sets the default delimiter settings.
 */
void IFStreamAscii::setDefaultDelimiters()
{
  // set default delimiters

  strDelim[0] = " ,";
  strDelim[0] += '	'; // add a tab

  strDelim[1] = "";
  strDelim[1].push_back('"');

  strDelim[2] = "//";

  strDelim[3] = "/*";
  strDelim[4] = "*/";
}

/**
 * Read a new line of text from the input stream.
 *
 * This function reads a line of text from the input stream regardless of how
 * long the line is. The internal buffer is resized until the entire line can
 * be read into the internal buffer. If eof is reached an empty buffer is
 * returned.
 */
void IFStreamAscii::getLine(string& buf)
{
  // set internal buffer to 100 bytes and save current read position

  if (buf.size() < 100) buf.resize(100);
  streampos curr_fp = ifs.tellg();

  // get line ... if line doesn't fit (fail is true) and eof was not reached,
  // then resize buffer and try again

  ifs.getline(&buf[0], buf.size());
  while (ifs.fail() && !ifs.eof())
  {
    // buffer was to small ... resize buffer to twice as large as last time

    buf.resize(2 * buf.size());

    // clear stream flags ... seek back to original read position ... read line
    // again

    ifs.clear();
    ifs.seekg((streamoff) curr_fp, ios::beg);
    ifs.getline(&buf[0], buf.size());
  }
}

/**
 * Parses a new line of text into ln.
 *
 * This function reads the next valid data line from the input stream into the
 * string ln. If eof is reached false is returned. If a valid line is found
 * it is set into ln and true is returned. This function removes empty and
 * comment lines and removes leading and trailing white space and trailing
 * comments from the input line before returning. Whitespace delimiters are
 * contained in the string strDelim[0] and comment delimiters in the string
 * strDelim[2].
 */
bool IFStreamAscii::readLine(string& ln)
{
  string buf;
  string::size_type i, j, k;

  // exit with true if stream is not assigned or is at eof

  if (!ifs.is_open()) return false;
  if (ifs.eof()) return false;

  // read lines until a valid line is found

  do
  {
    // get next line into buffer and set into string (ln) ... increment total
    // lines read

    getLine(buf);
		ln = buf.c_str();
		CPPUtils::removeEOL(ln);
    ++strTotlLinesRead;
    strBytesRead += ln.length();
    if (ln.length() == 0) ++strBlankLinesRead;

    // if block comment flag is set see if closing block comment delimiter
    // is present

    if (strBlkCommntSet)
    {
      // looking for end block comment ... see if ones in current line

      ++strBlkCommentLinesRead;
      i = ln.find(strDelim[4]);
      if (i != string::npos)
      {
        // found end block comment ... remove line contents upto and including
        // block comment

        j  = strDelim[4].length();
        ln = ln.substr(i + j, ln.length() - i - j);

        // reset block comment flag

        strBlkCommntSet = false;
      }

      // else end block comment NOT found ... set ln to empty

      else
        ln = "";
    }

    // if line is not empty see if it contains real data

    if (ln.length())
    {
      // first see if it contains a comment if a line comment is defined

      if (strDelim[2].length())
      {
        i = ln.find(strDelim[2]);
        if (i != string::npos)
        {
          // found comment ... strip it off

          ++strCommentLinesRead;
          ln = ln.substr(0, i);
        }
      }

      // continue parsing line if not empty

      if (ln.length())
      {
        // search for embedded block comments or start of a block comment if
        // block comment strings are defined

        if (strDelim[3].length() && strDelim[3].length())
        {
          i = ln.find(strDelim[3]);
          j = ln.rfind(strDelim[4]);
          if ((i !=  string::npos) && (j !=  string::npos))
          {
            // found embedded block comment ... remove block comment and continue

            k = strDelim[4].length();
            ln = ln.substr(0, i) + " " + ln.substr(j + k, ln.length() - j - k);
          }

          // else if only i was found (ignore if only j was found) then strip off
          // beginning of line and set block comment flag

          else if (i !=  string::npos)
          {
            ln = ln.substr(0, i);
            ++strBlkCommentLinesRead;
            strBlkCommntSet = true;
          }
        }

        // continue parsing line if not empty

        if (ln.length())
        {
          // remove any whitespace from the front of line

          i = ln.find_first_not_of(strDelim[0]);
          if (i != string::npos)
            ln = ln.substr(i, ln.length() - i);
          else
            ln = "";

          // continue parsing line if not empty

          if (ln.length())
          {
            // remove any whitespace from the back of line

            i = ln.find_last_not_of(strDelim[0]);
            if (i != string::npos)
              ln = ln.substr(0, i + 1);
            else
              ln = "";

            // increment data line count and return

            ++strDataLinesRead;
            return true;
          }
        }
      }
    }

    // no data was found ... try again if not EOF

  } while (!ifs.eof());

  // eof was reached return false

  return false;
}

/**
 * Tokenize the input string str into the string vector tokens.
 *
 * This function tokenizes the input string, str, into a series of tokens using
 * the whitespace (strDelim[0]) and string (strDelim[1]) delimiter definitions.
 * This function assumes that str is a valid, non-commented, non-empty line
 * read from the input stream.
 */
void IFStreamAscii::tokenize(const string& str, vector<string>& tokens)
{
  string::size_type beg_pos, end_pos, strt_strng, strt_equal;
  string& wspc = strDelim[0]; // whitespace delimiter
  string& strg = strDelim[1]; // string delimiter

  // loop finding all tokens until done

  end_pos = 0;
  while (end_pos != string::npos)
  {
    // Skip delimiters after end_pos and find beginning of next token

    beg_pos = str.find_first_not_of(wspc, end_pos);

    // Find first "delimiter" after beg_pos

    if (beg_pos != string::npos)
    {
      // see if an "=" character is immediately followed by a string
      // delimiter following beg_pos

      strt_strng = str.find_first_of(strg, beg_pos);
      strt_equal = str.find_first_of("=", beg_pos);
      if (strt_equal + 1 == strt_strng)
      {
        // found an "=" followed immediately by a string delimiter ...
        // see if any whitespace occurs before the "=" but after beg_pos

        if (str.find_first_of(wspc, beg_pos) > strt_strng)
        {
          // no whitespace before "=" ...
          // found an "=" immediately before a string delimiter ... save the
          // sequence before the "=" (including "=") as a separate token
          // increment beg_pos to strt_strng

          tokens.push_back(str.substr(beg_pos, strt_strng - beg_pos));
          beg_pos = strt_strng;
        }
      }

      // see if a string delimiter is the first character of the next token

      strt_strng = strg.find_first_of(str.substr(beg_pos, 1), 0);
      if (strt_strng != string::npos)
      {
        // increment to start of string delimited token ... find matching token
        // at other end ... add string token to the vector

        beg_pos++;
        end_pos = str.find_first_of(strg.substr(strt_strng, 1), beg_pos);
        tokens.push_back(str.substr(beg_pos, end_pos - beg_pos));

        // see if matching string delimiter was found

        if (end_pos == string::npos)
        {
          // error: no match was found for string delimiter print warning and
          // assign remainder of string as token

					ostringstream os;
					os << endl << "ERROR in IFStreamAscii::tokenize" << endl
						 << "  Could not find a closing string delimiter match: " << endl
						 << "    Current Line: " << str << endl
						 << "    String Start: " << strg.substr(strt_strng, 1) << endl;
					throw GeoTessException(os, __FILE__, __LINE__, 9209);
        }
        else
          // else increment to next position after ending string delimiter

          end_pos++;
      }
      else
      {
        // get next wspc delimiter after end of the current token and add
        // token to the vector

        end_pos = str.find_first_of(wspc, beg_pos);
        tokens.push_back(str.substr(beg_pos, end_pos - beg_pos));
      }
    }

    // else if beg_pos is at end of string then done ... exit

    else
      break;
  }
}

} // end namespace geotess
