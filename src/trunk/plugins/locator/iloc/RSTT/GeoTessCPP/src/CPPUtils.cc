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

#include "CPPUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

#if defined WIN32 || defined _WIN32
char const 		CPPUtils::FILE_SEP        = '\\';
#else
char const		CPPUtils::FILE_SEP        = '/';
#endif

const int		CPPUtils::SBOL		= sizeof(bool);
const int		CPPUtils::SBYT		= sizeof(byte);
const int		CPPUtils::SSHT		= sizeof(short);
const int		CPPUtils::SINT		= sizeof(int);
const int		CPPUtils::SLNG		= sizeof(LONG_INT);
const int		CPPUtils::SFLT		= sizeof(float);
const int		CPPUtils::SDBL		= sizeof(double);

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

// Definition of OpSys = one of Windows, Linux, SunOS, MacOs, Undefined
#if defined WIN32 || defined _WIN32
	#define OpSys "Windows"
#else
	#if defined Linux
		#define OpSys "Linux"
	#else
		#if defined SunOS
			#define OpSys "SunOS"
		#else
			#if defined Darwin
				#define OpSys "MacOSX"
			#else
				#define OpSys "Undefined"
			#endif
		#endif
	#endif
#endif

// Definition of  NEWLINE
#if defined WIN32 || defined _WIN32
	string const CPPUtils::NEWLINE = "\r\n";
#else
//	#if defined Darwin
//		string const CPPUtils::NEWLINE = "\r";
//	#else
		string const CPPUtils::NEWLINE = "\n";
//	#endif
#endif

/**
 * Returns the operating system type:
 * [ Windows | Linux | SunOS | MacOSX | Undefined ]
 */
const string CPPUtils::getOpSys() { return OpSys; }

/**
 * Static function that returns true if the machine type is big-endian.
 */
bool CPPUtils::isBigEndian()
{
  int ii = 256;
  char* ip = (char*) &ii;

  // if ip[2] = 1 then big-endian ... otherwise ip[1] = 1 which is little-endian

  if ((int) ip[2] == 1) return true;
  return false;
}

/**
 * Converts the input int to a string. The string frmt can be used to control
 * the format of the output string.
 *
 * @param i    The input int to be returned as a string.
 * @param frmt The input format string to control the format of the output string.
 * @return The converted parameter as a string.
 */
string CPPUtils::itos(int i, const string& frmt)
{
  char s[300];

  sprintf(s, frmt.c_str(), i);
  return s;
}

/**
 * Converts the input string \em s to a integer.
 *
 * @param s    The input string to be returned as a integer.
 * @param frmt The frmt defining how the string should be converted.
 */
int CPPUtils::stoi(const string& s, const string& frmt)
{
  int i = -999999;
  sscanf(s.c_str(), frmt.c_str(), &i);
  return i;
}

/**
 * Returns True if the input string \em s can be represented as an integer.
 *
 * @param s The input string to be tested for integer conversion.
 * @return True if the input string \em s can be represented as an integer.
 */
bool CPPUtils::isint(const string& s)
{
  return (s.find_first_not_of("-0123456789") == string::npos);
}

/**
 * Converts the input long to a string. The string frmt can be used to control
 * the format of the output string.
 *
 * @param l    The input long to be returned as a string.
 * @param frmt The input format string to control the format of the output string.
 * @return The converted parameter as a string.
 */
string CPPUtils::ltos(LONG_INT l, const string& frmt)
{
  char s[300];

  sprintf(s, frmt.c_str(), l);
  return s;
}

/**
 * Converts the input string \em s to a long.
 *
 * @param s    The input string to be returned as a long.
 * @param frmt The frmt defining how the string should be converted.
 */
LONG_INT CPPUtils::stol(const string& s, const string& frmt)
{
  LONG_INT l = 0;
  sscanf(s.c_str(), frmt.c_str(), &l);
  return l;
}

/**
 * Converts the input float to a string. The string frmt can be used to control
 * the format of the output string.
 *
 * @param f    The input float to be returned as a string.
 * @param frmt The input format string to control the format of the output string.
 * @return The converted parameter as a string.
 */
string CPPUtils::ftos(float f, const string& frmt)
{
  char s[300];

  sprintf(s, frmt.c_str(), f);
  return s;
}

/**
 * Converts the input string \em s to a float.
 *
 * @param s    The input string to be returned as a float.
 * @param frmt The frmt defining how the string should be converted.
 */
float CPPUtils::stof(const string& s, const string& frmt)
{
  float f = NaN_FLOAT;
  sscanf(s.c_str(), frmt.c_str(), &f);
  return f;
}

/**
 * Converts the input double to a string. The string frmt can be used to control
 * the format of the output string.
 *
 * @param d    The input double to be returned as a string.
 * @param frmt The input format string to control the format of the output string.
 * @return The converted parameter as a string.
 */
string CPPUtils::dtos(double d, const string& frmt)
{
  char s[300];

  sprintf(s, frmt.c_str(), d);
  return s;
}

/**
 * Converts the input string \em s to a double.
 *
 * @param s    The input string to be returned as a double.
 * @param frmt The frmt defining how the string should be converted.
 */
double CPPUtils::stod(const string& s, const string& frmt)
{
  double d = NaN_DOUBLE;
  sscanf(s.c_str(), frmt.c_str(), &d);
  return d;
}

/**
 * Converts the input boolean to a string.
 *
 * @param b The input boolean to be returned as a string.
 */
string CPPUtils::btos(bool b)
{
  if (b)
    return "true";
  else
    return "false";
}

/**
 * Converts the input string \em s to a boolean.
 *
 * @param s The input string to be returned as a boolean.
 */
bool CPPUtils::stob(const string& s)
{
  if ((s.substr(0, 1) == "T") || (s.substr(0, 1) == "t"))
    return true;
  else
    return false;
}

/**
 * Converts the input string to all lowercase characters.
 *
 * @param str The input string to be lower cassed.
 * @return The returned lower-case string.
 */
string CPPUtils::lowercase_string(const string& str)
{
  int i;
  string slower;

  slower.reserve(str.size());
  for (i = 0; i < (int) str.size(); i++) slower.push_back(tolower(str[i]));
  return slower;
}

/**
 * Converts the input string to all uppercase characters.
 *
 * @param str The input string to be upper cassed.
 * @return The returned upper-case string.
 */
string CPPUtils::uppercase_string(const string& str)
{
  int i;
  string supper;

  supper.reserve(str.size());
  for (i = 0; i < (int) str.size(); i++) supper.push_back(toupper(str[i]));
  return supper;
}

/**
 * Returns the input string (\em str) minus any leading and trailing
 * delimiters (\em delim ... defaults to space and tab).
 *
 * @param str   The input string to be trimmed.
 * @param delim The input set of delimiters defining whitespace.
 * @return The trimmed string.
 */
string CPPUtils::trim(const string& str, const string& delim)
{
	return trimRight(trimLeft(str, delim), delim);
}

/**
 * Returns the input string (\em str) minus any leading delimiters
 * (\em delim ... defaults to space and tab).
 *
 * @param str   The input string to be left trimmed.
 * @param delim The input set of delimiters defining whitespace.
 * @return The trimmed string.
 */
string CPPUtils::trimLeft(const string& str, const string& delim)
{
  // get first non-delimited position into i

  string::size_type i = str.find_first_not_of(delim);

  // if all characters are delimiters return an empty string ... if no
  // delimiters precede the string return the string

  if (i == string::npos) return "";
  if (i == 0) return str;

  // return the string minus its leading delimiters

  return str.substr(i);
}

/**
 * Returns the input string (\em str) minus any trailing delimiters
 * (\em delim ... defaults to space and tab).
 *
 * @param str   The input string to be right trimmed.
 * @param delim The input set of delimiters defining whitespace.
 * @return The trimmed string.
 */
string CPPUtils::trimRight(const string& str, const string& delim)
{
  // get last non-delimited position into i

  string::size_type i = str.find_last_not_of(delim);

  // if all characters are delimiters return an empty string ... if no
  // delimiters follow the string return the string

  if (i == string::npos) return "";
  if (i == str.length() - 1) return str;

  // return the string up to its trailing delimiters

  return str.substr(0, i + 1);
}

/**
 * Retrieves all properties defined in the input string \em str and
 * saves them into the property map \em props.
 *
 * The properties must be of the form "name = value; name = value; ..." where
 * the name value associations are set into the property map. All "names" are
 * lower-cased into the property map. "values" are simply assigned without
 * case change. All name and value pairs are stripped of leading and trailing
 * blanks before placement into the property map. All properties must be
 * separated by a semi-colon.
 *
 * @param str   The input ";" separated string of (property = value)
 *              associations that will be added to the input properties map.
 * @param props The map into which the discovered property --> value
 *              associations will be added.
 */
void CPPUtils::getProperties(const string& str, map<string, string>& props)
{
  int i;
  vector<string> properties;
  vector<string> pair;
  string s1, s2;

	// clear the properties map and tokenize the string on ";"

  props.clear();
  tokenizeString(str, ";", properties);

	// loop over all discovered tokens

  for (i = 0; i < (int) properties.size(); i++)
  {
    // separate the tag from the value of each property

    string::size_type indx = properties[i].find('=');
    if (indx != string::npos)
    {
			// get tag and trim ... get property and trim ... add to map

      s1 = properties[i].substr(0, indx);
      s1 = trimRight(trimLeft(lowercase_string(s1)));
      s2 = properties[i].substr(indx+1);
      s2 = trimRight(trimLeft(s2));
      props[s1] = s2;
    }
  }
}

/**
 * Retrieves the "value" associated with "tag" from the
 * property map "props". If the "tag" is found in props then true is
 * returned and "value" is set. Otherwise, false is returned.
 *
 * @param props The input properties map.
 * @param tag   The input key from which the associated value will be sought in
 *              the input properties map.
 * @param value The returned value associated with the input tag (if it was
 *              found).
 * @return True if the tag --> value association was found.
 */
bool CPPUtils::getProperty(const map<string, string>& props,
													 const string& tag, string& value)
{
  map<string, string>::const_iterator it;

  // see if the lowercased tag exists in props ... if not return false

  it = props.find(lowercase_string(tag));
  if (it == props.end()) return false;

  // otherwise set value and return true

  value = it->second;
  return true;
}

/**
 * Replaces all occurrences of sf in string s with sr on output and
 * returns the new string.
 *
 * @param sf The string to be replaced.
 * @param sr The string with which sf will be replaced with.
 * @param s  The string within which the replacement will occur.
 * @return   The new string with sf replaced by sr everywhere.
 */
string CPPUtils::stringReplaceAll(const string& sf, const string& sr,
																	 const string& s)
{
	string sout = s;
	size_t i = sout.find(sf);
	while (i != string::npos)
	{
		sout = sout.substr(0, i) + sr + sout.substr(i+sf.length());
		i = sout.find(sf);
	}
	return sout;
}

/**
 * Tokenizes the input string, str, into a set of tokens given a set
 * of delimiters specified in delim.
 *
 * For example, the string "May 15, 2002 5:56:20 pm" tokenized with
 * the set of delimiters " ,:" would yield the set of 7 tokens given
 * as "May", "15", "2002", "5", "56", "20", "pm".
 *
 * @param str    The input string to be tokenized.
 * @param delim  A string of delimiters treated as whitespace during
 *               tokenization.
 * @param tokens The returned vector of string tokens.
 */
void CPPUtils::tokenizeString(const string& str, const string& delim,
														 vector<string>& tokens)
{
  string::size_type beg_pos, end_pos;
  end_pos = 0;
  tokens.clear();

  // loop finding all tokens until done

  while (end_pos != string::npos)
  {
    // Skip delimiters after end_pos and find beginning of next token

    beg_pos = str.find_first_not_of(delim, end_pos);

    // Find first "delimiter" after beg_pos

    if (beg_pos != string::npos)
    {
      // get next delimiter after end of the current token

      end_pos = str.find_first_of(delim, beg_pos);

      // add token to the vector.

      tokens.push_back(str.substr(beg_pos, end_pos - beg_pos));
    }

    // else if beg_pos is at end of string set end_pos there to exit

    else
      end_pos = string::npos;
  }
}

} // end namespace geotess
