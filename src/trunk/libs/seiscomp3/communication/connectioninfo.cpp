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


#define SEISCOMP_COMPONENT Communication


#if !defined(_MSC_VER)
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/times.h>
#else
#include <windows.h>
#include <psapi.h>
#endif

#include <fstream>
#include <sstream>
#include <algorithm>


#include <boost/bind.hpp>

#include <seiscomp3/core/platform/platform.h>
#ifdef __APPLE__
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#endif

#ifdef sun
#include <procfs.h>
#include <fcntl.h>
#include <stdio.h>
#define PROCFS "/proc"
#endif

#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/status.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/systemconnection.h>
#include <seiscomp3/communication/connectioninfo.h>


#ifdef sun
namespace {

std::string getProgramName() {
	psinfo_t currproc;
        int fd;
        char buf[30];
        snprintf(buf, sizeof(buf), "%s/%d/psinfo", PROCFS, getpid());
        if ((fd = open (buf, O_RDONLY)) >= 0) {
                if (read(fd, &currproc, sizeof(psinfo_t)) == sizeof(psinfo_t))
                        return currproc.pr_fname;
                close(fd);
        }
	return "";
}


int getPageShift() {
	int i = sysconf(_SC_PAGESIZE);
	int pageShift = 0;
	while ((i >>= 1) > 0)
		++pageShift;
	pageShift -= 10;
	return pageShift;
}


std::string staticProgramName = getProgramName();
int staticPageShift = getPageShift();

}
#endif


namespace Seiscomp {
namespace Communication {




ConnectionInfo *ConnectionInfo::_Instance = NULL;


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionInfo::ConnectionInfo() :
	_pid(0),
	_totalMemory(0),
	_usedMemory(0),
	_delay(3),
	_clockTicks(0),
	_cpuUsage(0),
	_isRunning(false)
{
	programName();
	pid();
	hostname();
	ip();
	totalMemory();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionInfo::~ConnectionInfo()
{
	if ( _Instance == this )
		_Instance = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionInfo* ConnectionInfo::Instance()
{
	if ( _Instance == NULL )
		_Instance = new ConnectionInfo();

	return _Instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConnectionInfo::start()
{
	boost::mutex::scoped_lock lk(_mutex);

#if !defined(_MSC_VER)
	_clockTicks = sysconf(_SC_CLK_TCK);
#else
	_clockTicks = 10000000U;
#endif

	if (_isRunning)
		return true;
	_isRunning = true;

	// Every 4 timeouts (_delay * 4 seconds) the state will be updated
	_sendInterval = 4;
	_currentTimeout = 0;
	_firstMeasurement = true;

	_timer.setTimeout(_delay);
	_timer.setCallback(boost::bind(&ConnectionInfo::calculateCpuUsage, this));
	_timer.start();

	return _isRunning;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConnectionInfo::stop()
{
	// No need to lock, the mutex is already locked by the calling
	// method.

	if ( !_isRunning ) return false;

	_isRunning = false;
	_timer.stop();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string ConnectionInfo::info(const SystemConnection* con)
{
	if (!con)
		return "";

	std::ostringstream os;
	os << info(con->networkInterface());

	os << ConnectionInfoTag(SENT_MESSAGES_TAG).toString() << "=" << con->messageStat().totalSentMessages << "&";

	unsigned int receivedMessages = con->messageStat().totalReceivedMessages;
	os << ConnectionInfoTag(RECEIVED_MESSAGES_TAG).toString() << "=" << receivedMessages << "&";

	os << ConnectionInfoTag(MESSAGE_QUEUE_SIZE_TAG).toString() << "=" << con->queuedMessageCount() << "&";

	unsigned int summedQueueSize = con->messageStat().summedMessageQueueSize;
	os << ConnectionInfoTag(SUMMED_MESSAGE_QUEUE_SIZE_TAG).toString() << "=" << summedQueueSize << "&";

	os << ConnectionInfoTag(SUMMED_MESSAGE_SIZE_TAG).toString() << "=" << con->messageStat().summedMessageSize << "&";

	os << ConnectionInfoTag(OBJECT_COUNT_TAG).toString() << "=" << Core::BaseObject::ObjectCount() << "&";

	if ( _infoCallback )
		_infoCallback(con, _lastLogTime, os);

	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string ConnectionInfo::info(const NetworkInterface* ni)
{
	if (!ni)
		return "";

	std::ostringstream os;
	os << "&";
	os << ConnectionInfoTag(TIME_TAG).toString() << "=" << _lastLogTime.iso() << "&";
	os << ConnectionInfoTag(PRIVATE_GROUP_TAG).toString() << "=" << ni->privateGroup() << "&";
	os << ConnectionInfoTag(HOSTNAME_TAG).toString() << "="      << _hostname << "&";
	os << ConnectionInfoTag(IPS_TAG).toString() << "="           << _ips << "&";
	os << ConnectionInfoTag(PROGRAMNAME_TAG).toString() << "="   << _programName << "&";
	os << ConnectionInfoTag(PID_TAG).toString() << "="           << _pid << "&";

	int currentCpuUsage = (cpuUsage() * 100);
	os << ConnectionInfoTag(CPU_USAGE_TAG).toString() << "=" << currentCpuUsage << "&";

	os << ConnectionInfoTag(TOTAL_MEMORY_TAG).toString() << "=" << _totalMemory << "&";

	int currentMemoryUsage = memoryUsage();
	os << ConnectionInfoTag(CLIENT_MEMORY_USAGE_TAG).toString() << "=" << currentMemoryUsage << "&";

	return os.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionInfo::registerConnection(SystemConnection* con, boost::mutex* mutex)
{
	boost::mutex::scoped_lock lk(_mutex);
	_systemConnections.push_back(con);
	_systemConnectionsMutexes.push_back(mutex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConnectionInfo::unregisterConnection(SystemConnection* con)
{
	boost::mutex::scoped_lock lk(_mutex);

	for ( size_t i = 0; i < _systemConnections.size(); ++i ) {
		if ( _systemConnections[i] == con ) {
			_systemConnections.erase(_systemConnections.begin() + i);
			_systemConnectionsMutexes.erase(_systemConnectionsMutexes.begin() + i);
			return true;
		}
	}

	if ( _systemConnections.empty() && _networkInterfaces.empty() )
		stop();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionInfo::registerConnection(NetworkInterface* ni, boost::mutex* mutex)
{
	boost::mutex::scoped_lock lk(_mutex);
	_networkInterfaces.push_back(ni);
	_networkInterfacesMutexes.push_back(mutex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConnectionInfo::unregisterConnection(NetworkInterface* ni)
{
	boost::mutex::scoped_lock lk(_mutex);

	for ( size_t i = 0; i < _networkInterfaces.size(); ++i ) {
		if ( _networkInterfaces[i] == ni ) {
			_networkInterfaces.erase(_networkInterfaces.begin() + i);
			_networkInterfacesMutexes.erase(_networkInterfacesMutexes.begin() + i);
			return true;
		}
	}

	if ( _systemConnections.empty() && _networkInterfaces.empty() )
		stop();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ConnectionInfo::registerInfoCallback(InfoCallback callback)
{
	if ( _infoCallback && callback ) return false;
	_infoCallback = callback;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
unsigned long ConnectionInfo::pid()
{
	_pid = Core::pid();
	return _pid;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ConnectionInfo::hostname()
{
	/*
	size_t len = 64;
	char hostname[len];
	memset(hostname, 0, len);

	if (!gethostname(hostname, len))
		_hostname.assign(hostname);
	*/
	_hostname = Core::getHostname();
	return _hostname;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ConnectionInfo::programName()
{
	_programName.clear();

#ifdef LINUX
	std::string firstLine = getLineFromFile("/proc/self/status", "Name");

	std::vector<std::string> tokens;
	Core::split(tokens, firstLine.c_str(), ":");
	if (tokens.size() < 2)
		return _programName;

	Core::trim(tokens[1]);

	_programName.assign(tokens[1]);
#endif

#ifdef __APPLE__
	const char* programName  = getprogname();
	if ( programName )
	    _programName = programName;
#endif

#if defined(_MSC_VER)
	char path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	char *prog = strrchr(path, '\\');
	if ( !prog )
		prog = path;
	else
		++prog;
	char *ending = strrchr(prog, '.');
	if ( ending ) *ending = '\0';
	_programName = prog;
#endif

#ifdef sun
	_programName = staticProgramName;
#endif

	return _programName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& ConnectionInfo::ip()
{
	struct hostent* info = gethostbyname(_hostname.c_str());

	if (info)
	{
		for (int i = 0; info->h_addr_list[i] != NULL; ++i)
		{
			_ips += std::string(inet_ntoa(*((struct in_addr*)info->h_addr_list[i]))) + " ";
		}
	}

	return _ips;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ConnectionInfo::totalMemory()
{
    _totalMemory = -1;
#ifdef LINUX
	std::string line = getLineFromFile("/proc/meminfo", "MemTotal");
	if (line.empty())
		return _totalMemory;

	std::vector<std::string> tokens;
	Core::split(tokens, line.c_str(), ":");
	if (tokens.size() < 2)
		return _totalMemory;

	line.assign(tokens[1]);
	tokens.clear();
	Core::split(tokens, line.c_str(), "kB");
	if (tokens.size() < 1)
		return _totalMemory;

	Core::trim(tokens[0]);
	_totalMemory = 0;
	Core::fromString(_totalMemory, tokens[0]);
#endif

#ifdef __APPLE__
	int mib[2];
	size_t size = sizeof(int);

	mib[0] = CTL_HW;
	mib[1] = HW_PHYSMEM;
    sysctl(mib, 2, &_totalMemory, &size, NULL, 0);
    _totalMemory /= 1024;
#endif

#ifdef sun
	_totalMemory = sysconf(_SC_PHYS_PAGES);
	if ( staticPageShift > 0 )
		_totalMemory <<= staticPageShift;
	else if ( staticPageShift < 0 )
		_totalMemory >>= staticPageShift;
#endif

#if _MSC_VER
	MEMORYSTATUS memstat;
	GlobalMemoryStatus(&memstat);
	_totalMemory = memstat.dwTotalPhys / 1024;
#endif

	return _totalMemory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int ConnectionInfo::memoryUsage()
{
	_usedMemory = -1;

#ifdef LINUX
	std::string line = getLineFromFile("/proc/self/status", "VmRSS");
	if (line.empty())
		return -1;

	std::vector<std::string> tokens;
	if (Core::split(tokens, line.c_str(), ":") != 2)
		return 0;
	line = tokens[1];
	tokens.clear();
	if (Core::split(tokens, line.c_str(), "kB") != 2)
		return 0;
	Core::trim(tokens[0]);
	_usedMemory = 0;
	Core::fromString(_usedMemory, tokens[0]);
#endif

#ifdef __APPLE__
	/*
	 struct rusage myRUsage;
	_usedMemory = 0;
	getrusage(RUSAGE_SELF, &myRUsage);
	_usedMemory = myRUsage.ru_maxrss;
	return _usedMemory;
	*/
	FILE *pipe;
	char cmd[256];
	pid_t PID = getpid();
	long rsize = 0;
	sprintf(cmd,"ps -o rss -p %d | grep -v RSS", PID);
	pipe = popen(cmd, "r");
	if ( pipe ) {
	    fscanf( pipe, "%ld", &rsize);
	    pclose(pipe);
	}
	_usedMemory = rsize;
#endif

#ifdef sun
	psinfo_t currproc;
	int fd;
	char buf[30];
	_usedMemory = 0;
	snprintf(buf, sizeof(buf), "%s/%lu/psinfo", PROCFS, _pid);
	if ((fd = open (buf, O_RDONLY)) >= 0) {
		if (read(fd, &currproc, sizeof(psinfo_t)) == sizeof(psinfo_t)) {
			_usedMemory = currproc.pr_rssize;
		}
		close(fd);
	}
#endif

#if _MSC_VER
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
	                       PROCESS_VM_READ,
	                       FALSE, _pid);
    if ( hProcess != NULL ) {
		if ( GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) )
			// map to kbyte
			_usedMemory = pmc.WorkingSetSize / 1024;
	}
#endif

	return _usedMemory;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void ConnectionInfo::calculateCpuUsage()
{
	// Average the cpu usage over one minute
	double sumUsage = 0;
	double currentUsage = calculateCurrentCpuUsage();

	// First call to calculateCurrentCpuUsage?
	if ( currentUsage == -1 ) return;

	if ( _firstMeasurement ) {
		_cpuValues.resize(_sendInterval, currentUsage);
		sumUsage = currentUsage * _cpuValues.size();
		_firstMeasurement = false;
	}
	else {
		sumUsage -= _cpuValues[_currentTimeout];
		sumUsage += currentUsage;
		_cpuValues[_currentTimeout] = currentUsage;
	}

	_cpuUsage = sumUsage / _sendInterval;

	++_currentTimeout;
	if ( _currentTimeout >= (int)_cpuValues.size() )
	{
		_currentTimeout -= _cpuValues.size();
		_lastLogTime = Core::Time::GMT();

		//boost::mutex::scoped_lock lk(_mutex);
		for ( size_t i = 0; i < _systemConnections.size(); ++i )
		{
			ServiceMessage msg(Protocol::STATE_OF_HEALTH_RESPONSE_MSG,
			                   Protocol::TYPE_DEFAULT, Protocol::PRIORITY_HIGH);
			std::string data = info(_systemConnections[i]);
			msg.setData(data);

			int result = 0;
			if ( _systemConnectionsMutexes[i] != NULL ) {
				boost::mutex::scoped_lock lk(*_systemConnectionsMutexes[i]);
				if ( _systemConnections[i]->isConnected() )
					result = _systemConnections[i]->send("STATUS_GROUP", &msg);
			}
			else {
				if ( _systemConnections[i]->isConnected() )
					result = _systemConnections[i]->send("STATUS_GROUP", &msg);
			}

			if ( result != Core::Status::SEISCOMP_SUCCESS )
				SEISCOMP_DEBUG("Sending Statusmessage to STATUS_GROUP failed with status: %s", Core::Status::StatusToStr(result));
			/*
			else
				SEISCOMP_DEBUG("Sending Statusmessage to STATUS_GROUP succeeded");
			*/
		}

		for ( size_t i = 0; i < _networkInterfaces.size(); ++i )
		{
			ServiceMessage msg(Protocol::STATE_OF_HEALTH_RESPONSE_MSG,
			                   Protocol::TYPE_DEFAULT, Protocol::PRIORITY_HIGH);
			msg.setData(info(_networkInterfaces[i]));
			msg.setDestination("STATUS_GROUP");

			msg.setPrivateSenderGroup(_networkInterfaces[i]->privateGroup());
			SEISCOMP_DEBUG("Sending Statusmessage to: %s", _networkInterfaces[i]->privateGroup().c_str());
			if ( _networkInterfacesMutexes[i] != NULL ) {
				boost::mutex::scoped_lock lk(*_networkInterfacesMutexes[i]);
				if ( _networkInterfaces[i]->isConnected() )
					_networkInterfaces[i]->send(_networkInterfaces[i]->privateGroup(), msg.type(), &msg, false);
			}
			else {
				if ( _networkInterfaces[i]->isConnected() )
					_networkInterfaces[i]->send(_networkInterfaces[i]->privateGroup(), msg.type(), &msg, false);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConnectionInfo::calculateCurrentCpuUsage() const
{
#if !defined(_MSC_VER)
	static struct tms last;
	static bool first = true;
	//static Util::StopWatch watch;
	struct tms current;

	times(&current);

	if ( first ) {
		//watch.restart();
		last = current;
		first = false;
		return -1;
	}

	//double elapsed = watch.elapsed();
	//watch.restart();

	return ((current.tms_utime - last.tms_utime) +
	        (current.tms_stime - last.tms_stime) +
	        (current.tms_cutime - last.tms_cutime) +
	        (current.tms_cstime - last.tms_cstime)) / double(_delay*_clockTicks);
#else
	static ULARGE_INTEGER last;
	static bool first = true;
	HANDLE hProcess;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
	                       FALSE, _pid);
    if ( hProcess != NULL ) {
		FILETIME creationtime, exittime, kerneltime, usertime;
		if ( GetProcessTimes(hProcess, &creationtime, &exittime, &kerneltime, &usertime) ) {
			ULARGE_INTEGER current;
			current.LowPart = usertime.dwLowDateTime;
			current.HighPart = usertime.dwHighDateTime;

			if ( first ) {
				last.QuadPart = current.QuadPart;
				first = false;
				return -1;
			}

			return (current.QuadPart - last.QuadPart) / double(_delay*_clockTicks);
		}
	}
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double ConnectionInfo::cpuUsage() const
{
	boost::mutex::scoped_lock lk(_mutex);
	return _cpuUsage;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string ConnectionInfo::getLineFromFile(const std::string& fileName, const std::string& tag)
{
	std::string line;
	std::ifstream file(fileName.c_str());
	if (!file.is_open())
		return line;

	while (getline(file, line))
	{
		std::vector<std::string> tokens;
		Core::split(tokens, line.c_str(), ":");
		if (tokens.size() > 1)
			if (tokens[0] == tag)
				break;
		line.clear();
	}

	return line;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Communication
} // namespace Seiscomp
