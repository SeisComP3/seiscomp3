/*===========================================================================================================================
    Name:      	tmanip.C 

    Purpose:   	creation of a time strings that complies certain formatting standards

    Calls: 	os.iword, Date::formatindex 

    Problems: 	can only use default, others is not possible

    Language:   C++, ANSI standard.

    Author:  	Taken from the notebook of AT Computing, De programmeertaal C++   

    Revision:	2007-01-30	0.1	initial

 ==========================================================================================================================*/
#include "tmanip.h"
using namespace std;

int Date::formatindex = ios::xalloc();
int Time::formatindex = ios::xalloc();

ostream& Spaces(ostream& os)
{
	os.iword(Date::formatindex) = Date::SPACES;
	// iword() levert een reference, dus lvalue!
	return os;
}

ostream& Dashes(ostream& os)
{
	os.iword(Date::formatindex) = Date::DASHES;
	// iword() levert een reference, dus lvalue!
	return os;
}

ostream& Slashes(ostream& os)
{
	os.iword(Date::formatindex) = Date::SLASHES;
	// iword() levert een reference, dus lvalue!
	return os;
}

ostream& operator<<(ostream& os, const Date& d)
{
	switch(os.iword(Date::formatindex))
	{
		case Date::SLASHES:
			os << d.year << '/';
			os << setfill('0') << setw(2) << d.month << '/';
			os << setfill('0') << setw(2) << d.day;
			break;
		case Date::DASHES:
			os << d.year << '-';
			os << setfill('0') << setw(2) << d.month << '-';
			os << setfill('0') << setw(2) << d.day;
			break;
		case Date::SPACES:
			os << d.year << ' ';
			os << setfill('0') << setw(2) << d.month << ' ';
			os << setfill('0') << setw(2) << d.day;
			break;
	}
	return os;
}

ostream& Colon(ostream& os)
{
	os.iword(Time::formatindex) = Time::COLON;
	// iword() levert een reference, dus lvalue!
	return os;
}

ostream& operator<<(ostream& os, const Time& d)
{
	switch(os.iword(Time::formatindex))
	{
		case Time::COLON:
			os << setfill('0') << setw(2) << d.hour << ':';
			os << setfill('0') << setw(2) << d.minute << ':';
			os << setfill('0') << setw(2) << d.second;
			break;
	}
	return os;
}
