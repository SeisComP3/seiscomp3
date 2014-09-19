/*===========================================================================================================================
    Name:       string.h

    Language:   C++, ANSI standard.

    Author:     P de Boer

    Revision:	2007-02-02	0.1	initial
===========================================================================================================================*/
#ifndef MYSTRING_H
#define MYSTRING_H

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include "exception.h"
#include "log.h"

char* ToCString(std::string&);
std::string SplitString(std::string&, char, int&, int&);
std::string SplitString(std::string&, char);
std::vector<std::string> SplitStrings(std::string&, char);
std::vector<char> FindSeparators(std::string&);
bool Contains(std::vector<char>&, char);
tm* StringToTimeStamp(std::string&);
void StringToDate(std::string&, tm*, char, int&);
void StringToTime(std::string&, tm*, char, int&);
std::string TimeStampToString(tm*, std::string, std::string, std::string);
void DateToString(std::ostringstream&, tm*, std::string);
void TimeToString(std::ostringstream&, tm*, std::string);
std::string Convert2Ascii(std::string);
std::string GetCurrentDay();

// declaring and defining template functions
/****************************************************************************************************************************
* Function:     FromString                                                                                                  *
* Parameters:   string& s       - string that has to been converted into the type given with template T                     *
* Returns:      T               - can any type of int, double, float, etc.                                                  *
* Remarks:      if conversion is not possible an error of type BadConversion is risen                                       *
****************************************************************************************************************************/
template<typename T>
inline T FromString( const std::string& s)
{
        std::istringstream is(s);
        T t;
        if(!(is>>t))
		{
			// throw BadConversion("FromString(\""+ s + "\")"); }
			// Logging log;
			// log.write("bad conversion: FromString(\""+ s + "\")");
			return T();
		}
        return t;
}

/****************************************************************************************************************************
* Function:     ToString                                                                                                    *
* Parameters:   T& t    - template type T that has to become string                                                         *
* Returns:      string  - the created string                                                                                *
* Remarks:      if conversion is not possible an error of type BadConversion is risen                                       *
****************************************************************************************************************************/
template<typename T>
inline std::string ToString(const T& t)
{
        std::ostringstream os;
        if(!(os<<t))
                throw BadConversion("ToString()");
        return os.str();
}
#endif // MYSTRING_H
