/*===========================================================================================================================
    Name:       string.C

    Purpose: 	this file contains all kinds of functions for handling strings 

    Language:   C++, ANSI standard.

    Author:     the community

    Revision:	0.1	initial		2007-01-31

===========================================================================================================================*/
#include <cstdlib>
#include "mystring.h"

#ifdef WIN32
#include <time.h>
#endif

using namespace std;

/****************************************************************************************************************************
* Function:     ToCString												    *
* Parameters:   string s	- string to convert into a char*							    *
* Returns:      pointer to character representation of s								    *
* Description:  create a character pointer of length of s + 1, copy s into pointer and add terminating zero		    *
****************************************************************************************************************************/
char *ToCString(string &s) {
	char *cString = new char[s.length() + 1];
	s.copy(cString, string::npos); // copy s into cString
	cString[s.length()] = 0; // copy doesn't do null byte

	return cString;
}

/****************************************************************************************************************************
* Function:     SplitString                                                                                                 *
* Parameters:   buffer  - string to be split                                                                                *
*               c       - character that is used as separator in buffer                                                     *
*               p1      - starting point of search in buffer                                                                *
*               p2      - first point after p1 that is c located at                                                         *
* Returns:      substring of buffer                                                                                         *
* Description:  this function finds the first occurance of a separator in a given string from a given startpoint            *
*               and returns an substring of the string                                                                      *
****************************************************************************************************************************/
string SplitString(string &buffer, char c, int &p1, int &p2) {
	size_t p = buffer.find(c, p1);
	if ( p == string::npos )
		p2 = (int)buffer.size();
	else
		p2 = (int)p;

	string value = buffer.substr(p1, p2-p1);

	// the output gives a strange value at the end of each line, check for its presence and delete it.
	if ( value[value.size()-1]==static_cast<char>(13) )
		return value.substr(0, value.size()-1);
	else
		return value;
}

/****************************************************************************************************************************
* Function:     SplitString                                                                                                *
* Parameters:   buffer  - string to be split                                                                                *
*               c       - character that is used as separator in buffer                                                     *
* Returns:      substring of buffer                                                                                         *
* Description:  this function finds the first occurance of a separator in a given string from a given startpoint            *
*               and returns an substring of the string                                                                      *
****************************************************************************************************************************/
string SplitString(string &buffer, char c) {
	size_t p1=0, p2;
	p2 = buffer.find(c, p1);
	if ( p2 == string::npos )
		p2 = buffer.size();
	string value = buffer.substr(p1, p2-p1);

	// the output gives a strange value at the end of each line, check for its presence and delete it.
	if ( value[value.size()-1] == static_cast<char>(13) )
		return value.substr(0, value.size()-1);
	else
		return value;
}

/****************************************************************************************************************************
* Function:     SplitStrings                                                                                                *
* Parameters:   buffer  - string to be split                                                                                *
*               c       - character that is used as separator in buffer                                                     *
* Returns:      substring of buffer                                                                                         *
* Description:  this function finds the first occurance of a separator in a given string from a given startpoint            *
*               and returns an substring of the string                                                                      *
****************************************************************************************************************************/
vector<string> SplitStrings(string &buffer, char c) {
	vector<string> info;
	size_t p1=0, p2=0;
	while ( p2 < buffer.size() ) {
		p2 = buffer.find(c, p1);
		if ( p2 == string::npos )
			p2 = buffer.size();
		string value = buffer.substr(p1, p2-p1);
		if(value[value.size()-1]==static_cast<char>(13))
			info.push_back(value.substr(0, value.size()-1));
		else
			info.push_back(value);
		p1 = ++p2;
	}
	return info;
}

/****************************************************************************************************************************
* Function:     StringToTimeStamp                                                                                           *
* Parameters:   attr    - string containing the date                                                                        *
* Returns:      time structure tm                                                                                           *
* Description:  this function converts string into the time structure, the string must to have any sort of character        *
*               separator                                                                                                   *
*               Warning!! this function only works on the following format: yyyymmddhhMMss, with separators                 *
*		If the time is in ISO-format, than the template is as follows: yyyy-mm-ddTHH:MM:ss.wwwZ			    *
*		If the format is non ISO than the length of attr should be 19 else 24					    *
****************************************************************************************************************************/
tm *StringToTimeStamp(string &attr) {
	int pos1=0, pos2;
	string part;
	tm *tijd = new tm();
	vector<char> seps;
	char d_sep, d_term, t_sep, t_term;

	// find separators and terminators
	seps = FindSeparators(attr);

	// first check if the separators are identical or not
	if ( seps.size() > 1 ) {
		d_sep = seps[0];
		d_term = seps[1];
		t_sep = seps[2];
		t_term = seps[seps.size()-1];
		part = SplitString(attr, d_term, pos1, pos2);
	}
	else {
		d_sep = d_term = t_sep = t_term = seps[0];
		part = attr;
	}

	StringToDate(part, tijd, d_sep, pos1);

	if ( d_sep != d_term )
		pos1 = pos2+1;

	StringToTime(attr, tijd, t_sep, pos1);

	return tijd;
}

/****************************************************************************************************************************
* Function:     FindSeparators	                                                                                            *
* Parameters:   attr    - string containing the date                                                                        *
* Returns:      map container	                                                                                            *
* Description:  this functions searches the given string for none numeric characters and adds them to a map that will be    *
*		returned												    *
****************************************************************************************************************************/
vector<char> FindSeparators(string& attr) {
	vector<char> seps;

	for ( size_t i = 0; i < attr.size(); i++ ) {
		if ( ispunct(attr[i]) || isalpha(attr[i]) ) {
			if ( !Contains(seps, attr[i]) )
				seps.push_back(attr[i]);
		}
	}

	return seps;
}

/****************************************************************************************************************************
* Function:     Contains	                                                                                            *
* Parameters:   seps    - pointer to vector that contains all existing separators                                           *
*		c	- separator to check if in seps									    *
* Returns:      bool		                                                                                            *
* Description:  check if a character is already registered								    *
****************************************************************************************************************************/
bool Contains(vector<char>& seps, char c) {
	bool cont = false;

	for ( size_t j = 0; j < seps.size(); j++ ) {
		if ( seps[j] == c )
			cont = true;
	}

	return cont;
}

/****************************************************************************************************************************
* Function:     StringToDate     	                                                                                    *
* Parameters:   attr    - string containing the date                                                                        *
*		tijd	- time structure										    *
*		d_sep	- separator for date, can be dash, slash, space or nothing					    *
*		begin	- starting point in attr for searching d_sep							    *
* Returns:      None		                                                                                            *
* Description:  this function converts string into the time structure, the string must to have any sort of character        *
*               separator                                                                                                   *
****************************************************************************************************************************/
void StringToDate(string &attr, tm *tijd, char d_sep, int& begin) {
	int end;
	tijd->tm_year = FromString<int>(SplitString(attr, d_sep, begin, end));
	begin = end+1;
	tijd->tm_mon = FromString<int>(SplitString(attr, d_sep, begin, end));
	begin = end+1;
	tijd->tm_mday = FromString<int>(SplitString(attr, d_sep, begin, end));
	begin = end+1;
}

/****************************************************************************************************************************
* Function:     StringToTime	                                                                                            *
* Parameters:   attr    - string containing the date                                                                        *
*		tijd	- time structure										    *
*		t_sep	- separator for time, can be colon or nothing					    		    *
*		begin	- starting point in attr for searching t_sep							    *
* Returns:      time structure tm                                                                                           *
* Description:  this function converts string into the time structure, the string must to have any sort of character        *
*               separator                                                                                                   *
****************************************************************************************************************************/
void StringToTime(string& attr, tm *tijd, char t_sep, int& begin) {
	int end;
	tijd->tm_hour = FromString<int>(SplitString(attr, t_sep, begin, end));
	begin = end+1;
	tijd->tm_min = FromString<int>(SplitString(attr, t_sep, begin, end));
	begin = end+1;
	tijd->tm_sec = FromString<int>(SplitString(attr, t_sep, begin, end));
	begin = end+1;
}

/****************************************************************************************************************************
* Function:     TimeStampToString                                                                                           *
* Parameters:   dt   	- structure containing the time to be converted to string      		                            *
*		d_sep	- separator for date, can be dash, slash, space or nothing					    *
*		t_sep	- separator for time, can be colon or nothing							    *
*		dt_sep	- separator between date and time								    *
* Returns:      string with date/time converted                                                                             *
* Description:  this function converts time structure into a string						            *
****************************************************************************************************************************/
string TimeStampToString(tm *dt, string d_sep, string t_sep, string dt_sep) {
	ostringstream part;

	DateToString(part, dt, d_sep);
	part << dt_sep;
	TimeToString(part, dt, t_sep);
	
	return part.str();
}

/****************************************************************************************************************************
* Function:     DateToString	                                                                                            *
* Parameters:   dt   	- structure containing the time to be converted to string      		                            *
*		d_sep	- separator for date, can be dash, slash, space or nothing					    *
* Returns:      string with date/time converted                                                                             *
* Description:  this function converts the day part of the time structure into a string				            *
****************************************************************************************************************************/
void DateToString(ostringstream& part, tm *dt, string d_sep) {
	// because tm_year is the number of years since 1900, this number has to be added to get the correct calendar year	
	part << dt->tm_year+1900 << d_sep;
	
	// in the time structure tm the months range from 0 to 11, add 1 to get the calendar month
	part << setfill('0') << setw(2) << dt->tm_mon+1 << d_sep;
	part << setfill('0') << setw(2) << dt->tm_mday;
}

/****************************************************************************************************************************
* Function:     TimeToString	                                                                                            *
* Parameters:   dt   	- structure containing the time to be converted to string      		                            *
*		t_sep	- separator for time, can be colon or nothing							    *
* Returns:      string with date/time converted                                                                             *
* Description:  this function converts the hour part of the time structure into a string			            *
****************************************************************************************************************************/
void TimeToString(ostringstream& part, tm *dt, string t_sep) {
	part << setfill('0') << setw(2) << dt->tm_hour << t_sep;
	part << setfill('0') << setw(2) << dt->tm_min << t_sep;
	part << setfill('0') << setw(2) << dt->tm_sec;
}

/****************************************************************************************************************************
* Function:     Convert2Ascii	                                                                                            *
* Parameters:   text   	- structure containing the non ascii characters		      		                            *
* Returns:   	the adjusted string		                                                                            *
* Description:  takes every non ascii character and changes it into alphabethical character			            *
****************************************************************************************************************************/
string Convert2Ascii(string text) {
	for ( size_t i = 0; i < text.size(); i++ ) {
		switch((int)text[i]) {
			case(-23):
			case(-24):
				text[i]='e';
				break;
			case(-31):
				text[i]='a';
				break;
			case(-25):
				text[i]='c';
				break;
			case(-13):
			case(-12):
				text[i]='o';
				break;
			case(-15):
				text[i]='n';
				break;
		}
	}
	return text;
}

/****************************************************************************************************************************
* Function:     GetCurrentDay                                                                                       *
* Parameters:   none                                                                                                        *
****************************************************************************************************************************/
string GetCurrentDay() {
	time_t now = time((time_t *)0);
	struct tm *today = (struct tm *)malloc(sizeof(struct tm));

	today = localtime(&now);
	string year(ToString<int>(today->tm_year+1900));
	string month(ToString<int>(today->tm_mon+1));
	string day(ToString<int>(today->tm_mday));
	return year + "-" + month + "-" + day;
}
