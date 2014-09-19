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


#ifndef __SEISCOMP_CORE_SYSTEM_H__
#define __SEISCOMP_CORE_SYSTEM_H__

#include <cstdlib>
#include <string>

#include <seiscomp3/core.h>
#include <boost/version.hpp>
#include <boost/filesystem/convenience.hpp>


#if BOOST_VERSION <= 103301
	#define SC_FS_IT_LEAF(it) it->leaf()
	#define SC_FS_IT_STR(it) it->string()
	#define SC_FS_IT_PATH(it) *it
#else
	#if BOOST_FILESYSTEM_VERSION >= 3
		#define SC_FS_IT_LEAF(it) it->path().leaf().string()
	#else
		#define SC_FS_IT_LEAF(it) it->path().leaf()
	#endif
	#define SC_FS_IT_PATH(it) it->path()
	#define SC_FS_IT_STR(it) it->path().string()
#endif

//see, http://www.boost.org/doc/libs/1_51_0/libs/filesystem/doc/deprecated.html
#if BOOST_FILESYSTEM_VERSION >= 3
	#define BOOST_FILESYSTEM_NO_DEPRECATED

	// path
	#define SC_FS_PATH(PATH_STR) boost::filesystem::path(PATH_STR)
	#define SC_FS_DECLARE_PATH(NAME, PATH_STR) \
	        boost::filesystem::path NAME(PATH_STR);

	#define SC_FS_FILE_PATH(PATH) PATH.filename()
	#define SC_FS_FILE_NAME(PATH) SC_FS_FILE_PATH(PATH).string()

	#define SC_FS_HAS_PARENT_PATH(PATH) PATH.has_parent_path()
	#define SC_FS_PARENT_PATH(PATH) PATH.parent_path()

	#define SC_FS_EXT_PATH(PATH) PATH.extension()
	#define SC_FS_EXT_NAME(PATH) SC_FS_EXT_PATH(PATH).string()
	#define SC_FS_STEM_PATH(PATH) PATH.stem()
	#define SC_FS_STEM_NAME(PATH) SC_FS_STEM_PATH(PATH).string()

	// operations
	#define SC_FS_IS_REGULAR_FILE(PATH) boost::filesystem::is_regular_file(PATH)

	// directory entry
	#define SC_FS_DE_PATH(DIRENT) DIRENT->path()
#else
	// path
	#define SC_FS_PATH(PATH_STR) boost::filesystem::path(PATH_STR,\
	                                                  boost::filesystem::native)
	#define SC_FS_DECLARE_PATH(NAME, PATH_STR) \
	        boost::filesystem::path NAME(PATH_STR, boost::filesystem::native);

	#if BOOST_VERSION < 103600
		#define SC_FS_FILE_NAME(PATH) PATH.leaf()
		#define SC_FS_EXT_NAME(PATH) boost::filesystem::extension(PATH)
		#define SC_FS_STEM_NAME(PATH) boost::filesystem::basename(PATH)
		#define SC_FS_HAS_PARENT_PATH(PATH) PATH.has_branch_path()
		#define SC_FS_PARENT_PATH(PATH) PATH.branch_path()
	#else
		#define SC_FS_FILE_NAME(PATH) PATH.filename()
		#define SC_FS_EXT_NAME(PATH) PATH.extension()
		#define SC_FS_STEM_NAME(PATH) PATH.stem()
		#define SC_FS_HAS_PARENT_PATH(PATH) PATH.has_parent_path()
		#define SC_FS_PARENT_PATH(PATH) PATH.parent_path()
	#endif
	#define SC_FS_FILE_PATH(PATH) FS_PATH(FS_FILE_NAME(PATH))
	#define SC_FS_EXT_PATH(PATH) FS_PATH(FS_EXT_NAME(PATH))
	#define SC_FS_STEM_PATH(PATH) FS_PATH(FS_STEM_NAME(PATH))

	// directory entry
	#if BOOST_VERSION <= 103301
		#define SC_FS_DE_PATH(DIRENT) (*DIRENT)
	#else
		#define SC_FS_DE_PATH(DIRENT) DIRENT->path()
	#endif
#endif


namespace Seiscomp {
namespace Core {

SC_SYSTEM_CORE_API std::string getLogin();
SC_SYSTEM_CORE_API std::string getHostname();

SC_SYSTEM_CORE_API void sleep(unsigned long seconds);
SC_SYSTEM_CORE_API void msleep(unsigned long milliseconds);

SC_SYSTEM_CORE_API unsigned int pid();

SC_SYSTEM_CORE_API bool system(const std::string& command);

}
}


#endif
