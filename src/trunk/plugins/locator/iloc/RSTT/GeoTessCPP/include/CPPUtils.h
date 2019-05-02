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

#ifndef CPPUTILS_OBJECT_H
#define CPPUTILS_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include "CPPGlobals.h"
#include <fstream>

// **** # Defines **************************************************************

// use standard library objects
using namespace std;

//--------------------------

// **** _LOCAL INCLUDES_ *******************************************************

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _GLOBAL_REFERENCES_ ****************************************************

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Basic static utility functions and variables.
 *
 * The CPPUtils class provides basic system level static utility functions for
 * GeoTess.
 */
class GEOTESS_EXP_IMP CPPUtils
{
private:

	/**
	 * Private copy constructor. Not used.
	 */
	CPPUtils(const CPPUtils& bs) {};

	/**
	 * Private assignment operator. Not used.
	 */
	CPPUtils&					operator=(const CPPUtils& bs) { return *this; };

public:

	/**
	 * Default constructor.
	 */
	CPPUtils() {};

	/**
	 * Protected destructor ... made virtual as is usual practice.
	 */
	virtual						~CPPUtils() {};

	/**
	 * Returns the class name.
	 * @return class name
	 */
	static  string		class_name() { return "CPPUtils"; };

	/**
	 * Returns the class size.
	 */
	virtual int				class_size() const
	{ return (int) sizeof(CPPUtils); };

	/**
	 * Returns the operating system type.
	 */
	static const string getOpSys();

	/**
	 * Replaces all occurrences of sf in string s with sr on output and
	 * returns the new string.
	 */
	static string			stringReplaceAll(const string& sf, const string& sr,
			const string& s);

	/**
	 * Removes '/r', '/n', or '/r/n' from the end of the input string if it exists.
	 */
	static void 			removeEOL(string& s);

	/**
	 * If the supplied path does not already end with a path separator,
	 * append it to the end.  On windows, use '\\', otherwise '/'.
	 */
	static void addPathSeparator(string& path);

	/**
	 * combine the two path components together, inserting a 
	 * path separator only if necessary.
	 */
	static string insertPathSeparator(const string& dir, const string& name);

	/**
	 * If the supplied path ends with a path separator,
	 * remove it.  On windows, use '\\', otherwise '/'.
	 */
	static void removePathSeparator(string& path);

	/**
	 * Static functions that returns the input integer, float, double,
	 * or boolean, as a string.
	 */
	static string			itos(int i, const string& frmt = "%d");
	static string			ltos(LONG_INT l, const string& frmt = "%llu");
	static string			ftos(float f, const string& frmt = "%.6f");
	static string			dtos(double d, const string& frmt = "%.14f");
	static string			btos(bool b);

	/**
	 * Static functions that returns the input string as an integer,
	 * float, double, or boolean.
	 */
	static int				stoi(const string& i, const string& frmt = "%d");
	static LONG_INT				stol(const string& i64, const string& frmt = "%llu");
	static float			stof(const string& f, const string& frmt = "%f");
	static double			stod(const string& d, const string& frmt = "%lf");
	static bool				stob(const string& b);

	/**
	 * These functions return the input string (str) without any leading
	 * (trimLeft) or trailing (trimRight) delimiters (delim ... defaults to
	 * a space and a tab).
	 */
	static string			trim(const string& str,
			const string& delim = " \t");
	static string			trimLeft(const string& str,
			const string& delim = " \t");
	static string			trimRight(const string& str,
			const string& delim = " \t");

	/**
	 * Retrieves all properties defined in the input string str and saves
	 * them into the property map props.
	 *
	 * The properties must be of the form "name = value; name = value; ..."
	 * where the name value associations are set into the property map. All
	 * "names" are lower-cased into the property map. "values" are simply
	 * assigned without case change. All name and value pairs are stripped
	 * of leading and trailing blanks before placement into the property map.
	 * All properties must be separated by a semi-colon.
	 *
	 * @param str   Input semicolon separated string of properties.
	 * @param props Input map that will be filled with the properties
	 *              name --> value associations.
	 */
	static void				getProperties(const string& str,
			map<string, string>& props);

	/**
	 * Retrieves the "value" associated with "tag" from the property
	 * map "props". If the "tag" is found in props then true is
	 * returned and "value" is set. Otherwise, false is returned.
	 *
	 * @param props The input properties map (name = value).
	 * @param tag   The input key for which an associated value will be
	 *              sought.
	 * @param value The returned value associated with the input tag ...
	 *              if one was found.
	 * @return True if the value was set.
	 */
	static bool				getProperty(const map<string, string>& props,
			const string& tag, string& value);

	/**
	 * This function tokenizes the input string, str, into a set of tokens
	 * given a set of delimiters specified in delim. For example, the string
	 * "May 15, 2002 5:56:20 pm" tokenized with the set of delimiters " ,:"
	 * would yield the set of 7 tokens given as "May", "15", "2002", "5",
	 * "56", "20", and "pm".
	 */
	static void				tokenizeString(const string& str, const string& delim,
			vector<string>& tokens);

	/**
	 * These functions convert the input string to all lower/upper case
	 * characters.
	 */
	static string			lowercase_string(const string& str);
	static string			uppercase_string(const string& str);

	/**
	 * Returns the minimum (mn) and maximum (mx) of the input vector v.
	 */
	template <typename T>
	static void				minmax(const vector<T>& v, T& mn, T& mx);

	/**
	 * Returns input radian measure in degrees.
	 */
	static double			toDegrees(double a);

	/**
	 * Returns input degrees measure in radians.
	 */
	static double			toRadians(double a);

	/**
	 * Create a 2D array of arrays (Java style).
	 */
	template <typename T>
	static T**				new2DArrayOfArrays(int ni, int nj)
	{
		T** a = new T* [ni];
		for (int i = 0; i < ni; ++i)
			a[i] = new T [nj];
		return a;
	}

	/**
	 * Returns a new intrinsic 2D array of size [ni][nj].
	 *
	 * @param ni First array dimension size.
	 * @param nj Second array dimension size.
	 * @return The new 2D array.
	 */
	template <typename T>
	static T**				new2DArray(int ni, int nj)
	{
		T** a = new T* [ni];
		a[0] = new T [ni*nj];
		for (int i=1; i<ni; ++i) a[i] = &a[0][i*nj];
		return a;
	}

	/**
	 * Returns a new intrinsic 3D array of size [ni][nj].
	 *
	 * @param ni First array dimension size.
	 * @param nj Second array dimension size.
	 * @param nk Third array dimension size.
	 * @return The new 2D array.
	 */
	template <typename T>
	static T***				new3DArray(int ni, int nj, int nk)
	{
		T*** a  = new T** [ni];
		a[0]    = new T*  [ni*nj];
		a[0][0] = new T   [ni*nj*nk];
		for (int i = 0; i < ni; ++i)
		{
			a[i] = &a[0][i*nj];
			for (int j = 0; j < nj; ++j)
				a[i][j] = &a[0][0][(i*nj + j)*nk];
		}

		return a;
	}

	/**
	 * Deletes the input 2D array reference and sets it to null.
	 */
	template <typename T>
	static void				delete2DArray(T**& a);

	/**
	 * Deletes the input 2D array of arrays reference and sets it to null.
	 */
	template <typename T>
	static void				delete2DArrayOfArrays(T**& a, int ni)
	{
		if (a)
		{
			for (int i = 0; i < ni; ++i) delete [] a[i];
			delete [] a;
			a = NULL;
		}
	}

	/**
	 * Deletes the input 3D array reference and sets it to null.
	 */
	template <typename T>
	static void				delete3DArray(T***& a);

	/**
	 * Resets all n entries in array to val.
	 */
	template <typename T>
	static void				resetArray(int n, T* array, T val)
	{
		for (int i = 0; i < n; ++i) array[i] = val;
	}

	/**
	 * Return a deep copy of the specified array.
	 */
	template <typename T>
	static T* copyArray(T* a, int n)
	{
		T* copy = new T[n];
		for (int i=0; i<n; ++i)
			copy[i] = a[i];
		return copy;
	}

	/**
	 * Return true if file read / write system is big endian.
	 */
	static bool				isBigEndian();

	/**
	 * Returns True if string i can be represented as an integer.
	 */
	static  bool			isint(const string& i);

	/**
	 * Standard sizes of basic intrinsics.
	 */
	static const int	SBOL;
	static const int	SBYT;
	static const int	SSHT;
	static const int	SINT;
	static const int	SLNG;
	static const int	SFLT;
	static const int	SDBL;

	/**
	 * Path separator.  '\' on Windows, '/' on unix-type systems.
	 */
	static  char const FILE_SEP;

	/**
	 * End-of-line string.  "\r\n" on Windows, "\r" on MacOSX, "\n" on unix-type systems.
	 */
	static  string const NEWLINE;

	static bool fileExists(const string& fileName)
	{
		fstream f;

		f.open(fileName.c_str(), ios::in);
		if (f.is_open())
		{
			f.close();
			return true;
		}
		return false;
	}

}; // End class CPPUtils

// **** _INLINE FUNCTION IMPLEMENTATIONS_ **************************************

/**
 * Returns input radian measure in degrees.
 *
 * @return Input radian measure in degrees.
 */
inline double CPPUtils::toDegrees(double a)
{
	return RAD_TO_DEG * a;
}

/**
 * Returns input degrees measure in radians.
 *
 * @return Input degrees measure in radians.
 */
inline double CPPUtils::toRadians(double a)
{
	return DEG_TO_RAD * a;
}

/**
 * Delete and set to NULL a 2D array that was created with new2DArray()
 */
template <typename T>
inline void CPPUtils::delete2DArray(T**& a)
{
	if (a)
	{
		delete [] a[0];
		delete [] a;
		a = NULL;
	}
}

/**
 * Delete and set to NULL a 3D array that was created with new2DArray()
 */
template <typename T>
inline void CPPUtils::delete3DArray(T***& a)
{
	if (a)
	{
		delete [] a[0][0];
		delete [] a[0];
		delete [] a;
		a = NULL;
	}
}

/**
 * Removes '/r', '/n', or '/r/n' from the end of the input string if it exists.
 */
inline void CPPUtils::removeEOL(string& s)
{
	if (s.size() && (s[s.length() - 1] == '\n'))	s.erase(s.length() - 1);
	if (s.size() && (s[s.length() - 1] == '\r'))	s.erase(s.length() - 1);
}

inline void CPPUtils::removePathSeparator(string& s)
{
	if (s.size() && (s[s.length() - 1] == CPPUtils::FILE_SEP))	s.erase(s.length() - 1);
}

inline void CPPUtils::addPathSeparator(string& s)
{
	if ( s.find_last_of(CPPUtils::FILE_SEP) != s.length() - 1 )
		s += CPPUtils::FILE_SEP;
}

inline string CPPUtils::insertPathSeparator(const string& dir, const string& name)
{

	string path = dir;

	while (path.size() && (path[path.length() - 1] == CPPUtils::FILE_SEP))
		path.erase(path.length() - 1, 1);
	
	if (path.length() > 0)
		path = path+CPPUtils::FILE_SEP;

	string nm = name;
	while (nm.size() && (nm[0] == CPPUtils::FILE_SEP))
		nm.erase((unsigned)0, 1);

	return path+nm;
}

/**
 * Returns the minimum (\em mn) and maximum (\em mx) of the input v.
 */
template <typename T>
void CPPUtils::minmax(const vector<T>& v, T& mn, T& mx)
{
	// exit without modification if v is empty

	if (v.size())
	{
		// assign mx and mn to first entry

		mx = mn = v[0];

		// adjust mx and mn based on remaining entries

		for (int i = 1; i < v.size(); i++)
		{
			if (mn > v[i]) mn = v[i];
			if (mx < v[i]) mx = v[i];
		}
	}
}

} // end namespace geotess

#endif  // CPPUTILS_OBJECT_H
