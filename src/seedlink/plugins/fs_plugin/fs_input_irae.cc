/***************************************************************************** 
 * fs_input_irae.cc
 *
 * Input from IRAE-style local directory hierarchy
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * Modified:
 *   2003.9.4: changed to regular expression matching with the file
 *             pattern option, Chad Trabant
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#include <string>
#include <map>
#include <cstdlib>

#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>

#include "cppstreams.h"
#include "fs_plugin.h"
#include "utils.h"

namespace {

using namespace std;
using namespace SeedlinkPlugin;
using namespace CPPStreams;
using namespace Utilities;

class File
  {
  public:
    const string name;
    Timestamp tstmp;
    File(const string &name_init, const Timestamp &tstmp_init):
      name(name_init), tstmp(tstmp_init) {}
  };

class Directory
  {
  public:
    const string name;
    Timestamp tstmp;
    map<string, rc_ptr<File> > file_map;
    Directory(const string &name_init, const Timestamp &tstmp_init):
      name(name_init), tstmp(tstmp_init) {}
  };
    
class FS_Input_IRAE: public FS_Input
  {
  private:
    rc_ptr<FS_Decoder> cvt;
    Timestamp last_dir_time;
    Timestamp last_file_time;

    void read_files(const string &path, const string &file_pattern,
      map<string, rc_ptr<File> > &file_map);
      
    void read_dirs(const string &path, const string &file_pattern, 
      map<string, rc_ptr<Directory> > &dir_map);
      
    void process_files(const string &path,
      const map<string, rc_ptr<File> > &file_map);
      
    void process_dirs(const string &path,
      const map<string, rc_ptr<Directory> > &dir_map);

  public:
    const string myname;
    
    FS_Input_IRAE(const string &myname_init): myname(myname_init) {}

    void init(rc_ptr<FS_Decoder> cvt_init)
      {
        cvt = cvt_init;
      }
      
    void check_for_new_data(const string &path, const string &file_pattern);
    Timestamp get_timestamp();
    void set_timestamp(const Timestamp &tstmp);
  };
    
void FS_Input_IRAE::read_files(const string &path, const string &file_pattern,
  map<string, rc_ptr<File> > &file_map)
  {
    static regex_t *fnregex = NULL;
    
    // Allocate and compile regular expression "pattern" if specified
    if (file_pattern.length() != 0 && fnregex == NULL)
      {
	fnregex = (regex_t *) malloc (sizeof (regex_t));
	
	if ( regcomp(fnregex, file_pattern.c_str(), REG_EXTENDED|REG_NOSUB ) != 0)
	  {
            logs(LOG_WARNING) << "cannot compile regular expression '" << 
	      file_pattern << "'" << endl;
	  }
      }

    dirent *de;
    DIR *dir = opendir(path.c_str());
    if (dir == NULL) return;
    while((de = readdir(dir)) != NULL)
      {
        if(de->d_name[0] == '.' && 
          (de->d_name[1] == '.' || de->d_name[1] == 0)) continue;
        
        if((file_pattern.length() != 0 && fnregex != NULL) &&
	   (regexec ( fnregex, de->d_name, (size_t) 0, NULL, 0) != 0))
	  continue;
        
        struct stat st;
        if(stat((path + "/" + de->d_name).c_str(), &st) < 0)
          {
            logs(LOG_WARNING) << "cannot stat '" << de->d_name << "'" << endl;
            continue;
          }

        if(!S_ISREG(st.st_mode))
          {
            logs(LOG_WARNING) << "'" << de->d_name << "' is not a "
              "regular file" << endl;
            continue;
          }

#if MTIME_USEC
        Timestamp tstmp(st.st_mtime, st.st_mtime_usec);
#else
        Timestamp tstmp(st.st_mtime, 0);
#endif

//        if(tstmp > last_file_time)
        if(tstmp > last_file_time && time(NULL) - tstmp.sec >= file_read_delay)
            insert_object(file_map, new File(de->d_name, tstmp));
      }

    closedir(dir);
  }
    
void FS_Input_IRAE::read_dirs(const string &path, const string &file_pattern,
  map<string, rc_ptr<Directory> > &dir_map) 
  {
    DIR *dir;
    dirent *de;
    
    if((dir = opendir(path.c_str())) == NULL) return;

    while((de = readdir(dir)) != NULL)
      {
        if(de->d_name[0] == '.' && 
          (de->d_name[1] == '.' || de->d_name[1] == 0)) continue;
        
        struct stat st;
        if(stat((path + "/" + de->d_name).c_str(), &st) < 0)
          {
            logs(LOG_WARNING) << "cannot stat '" << de->d_name << "'" << endl;
            continue;
          }

        if(!S_ISDIR(st.st_mode))
          {
            logs(LOG_WARNING) << "'" << de->d_name << "' is not a directory"
              << endl;
            continue;
          }

#if MTIME_USEC
        Timestamp tstmp(st.st_mtime, st.st_mtime_usec);
#else
        Timestamp tstmp(st.st_mtime, 0);
#endif

        if(tstmp > last_dir_time)
          {
            rc_ptr<Directory> dp = new Directory(de->d_name, tstmp);
            read_files(path + "/" + de->d_name, file_pattern, dp->file_map);

            if(!dp->file_map.empty()) insert_object(dir_map, dp);
          }
      }

    closedir(dir);
  }

void FS_Input_IRAE::process_files(const string &path,
  const map<string, rc_ptr<File> > &file_map)
  {
    map<string, rc_ptr<File> >::const_iterator p;
    for(p = file_map.begin(); p != file_map.end(); ++p)
      {
        cvt->process_file(path + "/" + p->second->name, 0, -1);

        if(p->second->tstmp > last_file_time)
            last_file_time = p->second->tstmp;
      }
  }
 
void FS_Input_IRAE::process_dirs(const string &path,
  const map<string, rc_ptr<Directory> > &dir_map)
  {
    map<string, rc_ptr<Directory> >::const_iterator p;
    for(p = dir_map.begin(); p != dir_map.end(); ++p)
      {
        process_files(path + "/" + p->second->name, p->second->file_map);
        
        if(p->second->tstmp > last_dir_time)
            last_dir_time = p->second->tstmp;
      }
  }

void FS_Input_IRAE::check_for_new_data(const string &path,
  const string &file_pattern)
  {
    map<string, rc_ptr<Directory> > dir_map;
    read_dirs(path, file_pattern, dir_map);
    process_dirs(path, dir_map);
  }

void FS_Input_IRAE::set_timestamp(const Timestamp &tstmp)
  {
    last_file_time = tstmp;
  }

Timestamp FS_Input_IRAE::get_timestamp()
  {
    return last_file_time;
  }

RegisterInput<FS_Input_IRAE> input("irae");

} // unnamed namespace

