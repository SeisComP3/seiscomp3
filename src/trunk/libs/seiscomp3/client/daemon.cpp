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


#define SEISCOMP_COMPONENT UTILS

#include <cstring>
#include <cerrno>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

#include <seiscomp3/logging/filerotator.h>
#ifndef WIN32
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <unistd.h>
   #include <seiscomp3/logging/syslog.h>
#endif

#include <seiscomp3/client/daemon.h>
#include <seiscomp3/system/environment.h>

#include <boost/program_options/options_description.hpp>

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Utils;
using namespace Seiscomp::Logging;

namespace Seiscomp {
namespace Utils {

int initDaemon() {
#ifndef WIN32
	pid_t pid;

	// Become a session leader to lose controlling TTY.
	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return -1;
	}
	else if ( pid != 0 ) // parent
		exit(0);

	if ( setsid() < 0 ) {
		SEISCOMP_ERROR("setsid: %s", strerror(errno));
		return -1;
	}

	// Ensure future opens won't allocate controlling TTYs.
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if ( sigaction(SIGHUP, &sa, NULL) < 0 ) {
		SEISCOMP_ERROR("can't ignore SIGHUP: %s", strerror(errno));
		return -1;
	}

	if ( (pid = fork()) < 0 ) {
		SEISCOMP_ERROR("can't fork: %s", strerror(errno));
		return -1;
	}
	else if ( pid != 0 ) // parent
		exit(0);

	// Attach file descriptors 0, 1, and 2 to /dev/null.
	close(0);
	close(1);
	close(2);
	int fd0 = open("/dev/null", O_RDWR);
	int fd1 = dup(0);
	int fd2 = dup(0);

	if ( fd0 != 0 || fd1 != 1 || fd2 != 2 ) {
		SEISCOMP_ERROR("initDaemon: unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		return -1;
	}
#endif
	return 0;
}

int acquireLockfile(const char *lockfile) {
#ifndef WIN32
	int fd = open(lockfile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if( fd < 0 ) {
		SEISCOMP_ERROR("could not open %s: %s", lockfile, strerror(errno));
		return -1;
	}
	else if ( fd <= 2 ) {
		SEISCOMP_ERROR("acquireLockfile: unexpected file descriptor %d", fd);
		return -1;
	}

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = 0;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;
  
	if ( fcntl(fd, F_SETLK, &lock ) < 0 ) {
		close(fd);
		if(errno == EACCES || errno == EAGAIN) return 0; // already locked
		
		SEISCOMP_ERROR("could not lock %s: %s\n", lockfile, strerror(errno));
		return -1;
	}
  
	if ( ftruncate(fd, 0) < 0 ) {
		SEISCOMP_ERROR("ftruncate: %s", strerror(errno));
		return -1;
	}

	char buf[10];
	snprintf(buf, 10, "%d", getpid());
	
	if ( write(fd, buf, strlen(buf)) != int(strlen(buf)) ) {
		SEISCOMP_ERROR("could not write %s: %s\n", lockfile, strerror(errno));
		return -1;
	}
		
	int val;
	if ( (val = fcntl(fd, F_GETFD,0)) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	val |= FD_CLOEXEC;
	if ( fcntl(fd, F_SETFD, val) < 0 ) {
		SEISCOMP_ERROR("fcntl: %s", strerror(errno));
		return -1;
	}

	// locking successful
	return fd;
#else
	return -1;
#endif
}

namespace {

class FlagCounter: public boost::program_options::untyped_value {
	public:
		FlagCounter(unsigned int* count);
		void xparse(boost::any&, const vector<string>&) const;

	private:
		unsigned int* _count;
};

FlagCounter::FlagCounter(unsigned int* count)
	: boost::program_options::untyped_value(true), _count(count) {
}

void FlagCounter::xparse(boost::any&, const vector<string>&) const {
	++(*_count);
}

} // unnamed namespace

void addCommonOptions(boost::program_options::options_description& desc,
	unsigned int& logLevel, unsigned int& logLevelAdd, string& lockfile) {
	desc.add_options()
		("help,h", "Show this help/usage message")
		("verbosity", boost::program_options::value<unsigned int>(&logLevel)->default_value(logLevel), "Set verbosity level")
		// Haven't figured out how to define short option (-v) only,
		// so add a long one as well.
		("add-verbosity,v", new FlagCounter(&logLevelAdd), "Increase verbosity level (may be repeated, eg., -vvv)")
		("debug", "Debug mode (send all log output to stdout)")
		("daemon,D", "Daemon mode")
#ifndef WIN32
		("syslog,s", "Use syslog")
#endif
		("lockfile,l", boost::program_options::value<string>(&lockfile), "Path to lock file");
}

int initLogging(int argc, char** argv, unsigned int logLevel, bool useSyslog, bool debug, bool copyToStdOut) {
	Logging::init(argc, argv);

	const Environment* env = Environment::Instance();  // singleton
	Output* logger = NULL;  // never deleted

	if ( debug ) {
		Logging::enableConsoleLogging(Seiscomp::Logging::getAll());
		return 0;
	}

	if ( logLevel > 0 ) {
		const char* appName = strrchr(argv[0], '/');
		if ( appName == NULL ) appName = argv[0]; else ++appName;
	
		bool logIsOpen = false;

#ifndef WIN32
		if ( useSyslog ) {
			SyslogOutput* syslogOutput = new SyslogOutput();
			logger = syslogOutput;
			logIsOpen = syslogOutput->open(appName);
		}
		else {
#endif
			string logFile = env->logFile(appName);
			FileOutput* fileOutput = new FileRotatorOutput;
			logger = fileOutput;
			logIsOpen = fileOutput->open(logFile.c_str());
			if ( logIsOpen ) {
				cout << "Using logfile: " << logFile << endl;
			}
#ifndef WIN32
		}
#endif
			
		if ( logIsOpen ) {
			if (logLevel >= 4) {
				logger->subscribe(Logging::getGlobalChannel("debug"));
				if (copyToStdOut)
					Logging::enableConsoleLogging(Logging::getGlobalChannel("debug"));
			}

			if (logLevel >= 3) {
				logger->subscribe(Logging::getGlobalChannel("info"));
				if (copyToStdOut)
					Logging::enableConsoleLogging(Logging::getGlobalChannel("info"));
			}

			if (logLevel >= 2) {
				logger->subscribe(Logging::getGlobalChannel("warning"));
				if (copyToStdOut)
					Logging::enableConsoleLogging(Logging::getGlobalChannel("warning"));
			}

			if (logLevel >= 1) {
				logger->subscribe(Logging::getGlobalChannel("error"));
				if (copyToStdOut)
					Logging::enableConsoleLogging(Logging::getGlobalChannel("error"));
			}
		}
		else {
			cout << "Error: could not open logger" << endl;
			return -1;
		}
	}

	return 0;
}

} // namespace Utils
} // namespace Seiscomp

