/*====================================================================
    Name:      tmanip.h

    Purpose:   defining of the classes/functions used in tmanip.C

    Problems: 

    Language:   C++, ANSI standard.

    Author:     Taken from AT Computing, De programmeertaal C++

    Revision:   2007-01-30  0.1  initial

 ====================================================================*/
#ifndef TMANIP_H
#define TMANIP_H

#include <sstream>
#include <iomanip>

class Date
{
	public:
		Date(int y, int m, int d) : year(y), month(m), day(d){}
		// operator<< is a friend
		friend std::ostream& operator<<(std::ostream& os, const Date& d);
		// de manipulator-functions are also friends
		friend std::ostream& Spaces(std::ostream&);
		friend std::ostream& Slashes(std::ostream&);
		friend std::ostream& Dashes(std::ostream&);
	protected:
	private:
		int year, month, day;
		static int formatindex;
		enum formatstatus { DASHES, SLASHES, SPACES };
};

class Time
{
	public:
		Time(int h, int m, int s) : hour(h), minute(m), second(s){}
		// operator<< is a friend
		friend std::ostream& operator<<(std::ostream& os, const Time& t);
		// de manipulator-functions are also friends
		friend std::ostream& Colon(std::ostream&);
	protected:
	private:
		int hour, minute, second;
		static int formatindex;
		enum formatstatus { COLON };
};
#endif // TMANIP_H
