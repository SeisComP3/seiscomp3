
#include "converter.h"

#define SEISCOMP_COMPONENT sync_dlsv
#include <seiscomp3/logging/log.h>

int main(int argc, char **argv) {
	int retCode = EXIT_SUCCESS;

	// Create an own block to make sure the application object
	// is destroyed when printing the overall objectcount
	{
		Seiscomp::Applications::Converter app(argc, argv);
		retCode = app.exec();
	}

	SEISCOMP_DEBUG("EXIT(%d), remaining objects: %d",
	               retCode, Seiscomp::Core::BaseObject::ObjectCount());

	return retCode;
}

