/*===========================================================================================================================
    Name:       log.h

    Purpose:  	written logging messages to the default logfile

    Language:   C++, ANSI standard.

    Author:     Peter de Boer

    Revision:	2007-05-07	0.1	initial	version

===========================================================================================================================*/
#ifndef LOG_H
#define LOG_H

class Logging
{
	public:
		void write(std::string);
	protected:
	private:
		bool open();
		void close();
};
#endif /* LOG_H */

