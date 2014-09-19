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


#include <seiscomp3/utils/files.h>
#include <seiscomp3/utils/replace.h>
#include <seiscomp3/system/environment.h>

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <memory.h>


namespace Seiscomp {
namespace Util {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string basename(const std::string& name)
{
	std::string::size_type pos = name.rfind("/");
	if (pos != std::string::npos)
		return name.substr(pos+1);
	return name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool fileExists(const std::string &file) {
	struct stat buf;
	int s = stat(file.c_str(), &buf);
	if ( s != 0 )
		return false;

	if ( !S_ISREG(buf.st_mode) ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool pathExists(const std::string& path) {
	struct stat buf;
	int s = stat(path.c_str(), &buf);
	if ( s != 0 )
		return false;

	if ( !S_ISDIR(buf.st_mode) ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool createPath(const std::string &pathname) {
	if (mkdir(pathname.c_str(), 0755) < 0) {
		if ( errno == ENOENT ) {
			size_t slash = pathname.rfind('/');
			if ( slash != std::string::npos ) {
				std::string prefix = pathname.substr(0, slash);
				if ( !createPath(prefix) ) return false;
				if ( slash == pathname.size() -1 ) return true;
				return mkdir(pathname.c_str(), 0755) == 0;
			}
			else
				return false;
		}
		else
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string removeExtension(const std::string& name)
{
	std::string::size_type pos1 = name.rfind("/");
	std::string::size_type pos2 = name.rfind(".");

	if ( pos1 < pos2 || (pos1 == std::string::npos && pos2 != std::string::npos) )
		return name.substr(0, pos2);

	return name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

class StreamBuffer : public std::streambuf {
	public:
		StreamBuffer() : std::streambuf() {}

		std::streambuf *setbuf(char *s, std::streamsize n) {
			setp(NULL, NULL);
			setg(s, s, s + n);
			return this;
		}
};

}

std::streambuf *bytesToStreambuf(char *data, size_t n) {
	std::streambuf *buf = new StreamBuffer;
	buf->pubsetbuf(data, n);
	buf->pubseekpos(0);
	return buf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::streambuf *stringToStreambuf(const std::string &str) {
	std::streambuf *buf = new std::stringbuf(str);
	return buf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream *file2ostream(const char *fn) {
	std::ofstream *os = new std::ofstream;
	if ( strlen(fn) == 1 && !strcmp(fn, "-") ) {
		os->copyfmt(std::cout);
		os->clear(std::cout.rdstate());
		((std::basic_ios<char>*)os)->rdbuf(std::cout.rdbuf());
	}
	else
		os->open(fn);

	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::istream *file2istream(const char *fn) {
	std::ifstream *is = new std::ifstream;
	if ( strlen(fn) == 1 && !strcmp(fn, "-") ) {
		is->copyfmt(std::cin);
		is->clear(std::cin.rdstate());
		((std::basic_ios<char>*)is)->rdbuf(std::cin.rdbuf());
	}
	else
		is->open(fn);

	return is;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Util
} // namespace Seiscomp
