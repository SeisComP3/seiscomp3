/*===========================================================================================================================
    Name:       log.C

    Purpose:  	written logging messages

    Language:   C++

    Author:     Peter de Boer

    Revision:	2007-05-07	0.1	initial version

===========================================================================================================================*/
#include <string>
#include "log.h"

#define SEISCOMP_COMPONENT sync_dlsv
#include <seiscomp3/logging/log.h>

using namespace std;

bool Logging::open()
{
	return true;
}

void Logging::write(std::string message)
{
	SEISCOMP_INFO("%s", message.c_str());
}

void Logging::close()
{
	return;
}

