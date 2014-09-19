/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_UTILS_DAEMON_H__
#define __SEISCOMP_UTILS_DAEMON_H__

#include <boost/program_options/options_description.hpp>
#include <seiscomp3/client.h>

namespace Seiscomp {
namespace Utils {

/**
 * Forks the process to background and yields controlling
 * TTY. Standard input/output/error are redirected to
 * /dev/null. Other file descriptors are not closed. umask
 * is not modified. Errors are reported using Seiscomp log
 * facility.
 * @return 0 on success, -1 on error
 */
SC_SYSTEM_CLIENT_API
int initDaemon();

/**
 * Opens and locks a file, used to ensure that only one
 * instance of an executable is running. The PID of current
 * process is written into the lockfile. If the file is
 * sucessfully locked, then file descriptor is returned.
 * Closing the file descriptor will release the lock.
 * @param File name
 * @return 0 if the file is already locked by another process,
 *         positive integer if the file was successfully locked,
 *         -1 on error.
 */
SC_SYSTEM_CLIENT_API
int acquireLockfile(const char *lockfile);

/**
 * Adds common options (supported by all daemons) to
 * options_description.
 * @param desc Reference to options_description
 * @param logLevel (in/out) Level of verbosity
 * @param logLevelAdd (in/out) Level of verbosity increment
 * @param lockfile (in/out) Name of lock file
 * @return none
 */
SC_SYSTEM_CLIENT_API
void addCommonOptions(boost::program_options::options_description& desc,
	unsigned int& logLevel, unsigned int& logLevelAdd, std::string& lockfile);

/**
 * Initializes logging according to given parameters.
 * @param logLevel Level of verbosity
 * @param useSyslog Send messages to Syslog instead of writing to files
 * @param debug Debug mode (send all log output to stdout)
 * @param copyToStdOut Send requested log output to stdout (ignored when using debug)
 * @return 0 on success, -1 on error
 */
SC_SYSTEM_CLIENT_API
int initLogging(int argc, char** argv, unsigned int logLevel,
                bool useSyslog, bool debug, bool copyToStdOut = false);

}
}

#endif

