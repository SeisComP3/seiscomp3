/*====================================================================
    Name:       exception.h

    Purpose:  	handling of exception that can be risen in the
		program

    Problems: 	not known sofar

    Language:   C++, ANSI standard.

    Author:    	Peter de Boer 

    Revision:	2007-01-30	0.1	initial version

 ====================================================================*/
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>

class BadConversion : public std::runtime_error
{
        public:
                BadConversion(const std::string& s): std::runtime_error(s){}
};

class IllegalConstraint : public std::domain_error
{
	public:
		IllegalConstraint(const std::string& s):std::domain_error(s){}
};

class MalformedRequest : public std::domain_error
{
	public:
		MalformedRequest(const std::string& s):std::domain_error(s){}
};
class MysqlError : public std::runtime_error
{
	public:
		MysqlError(const std::string& s):std::runtime_error(s){}
};
#endif // EXCEPTION_H
