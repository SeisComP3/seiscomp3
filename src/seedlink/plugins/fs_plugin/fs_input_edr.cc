/***************************************************************************** 
 * fs_input_edr.cc
 *
 * Input from data files created by Earth Data recorder.
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
#include <cstdio>

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
    const string dir;
    Timestamp tstmp;
    File(const string &name_init, const string &dir_init,
      const Timestamp &tstmp_init):
      name(name_init), dir(dir_init), tstmp(tstmp_init) {}
  };

class FS_Input_EDR: public FS_Input
  {
  private:
    rc_ptr<FS_Decoder> cvt;
    Timestamp last_processed;
    Timestamp last_collected;

    void collect_files2(const string &path, const string &dirname,
      const string &file_pattern, map<string, rc_ptr<File> > &file_map);
      
    void collect_files(const string &path, const string &file_pattern,
      map<string, rc_ptr<File> > &file_map);
      
    void process_files(const string &path,
      const map<string, rc_ptr<File> > &file_map);

  public:
    const string myname;
    
    FS_Input_EDR(const string &myname_init): myname(myname_init) {}

    void init(rc_ptr<FS_Decoder> cvt_init)
      {
        cvt = cvt_init;
      }
      
    void check_for_new_data(const string &path, const string &file_pattern);
    Timestamp get_timestamp();
    void set_timestamp(const Timestamp &tstmp);
  };
    
void FS_Input_EDR::collect_files2(const string &path, const string &dirname,
  const string &file_pattern, map<string, rc_ptr<File> > &file_map)
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
    DIR *dir = opendir((path + "/" + dirname).c_str());
    if (dir == NULL) return;
    while((de = readdir(dir)) != NULL)
      {
        if(de->d_name[0] == '.' && 
          (de->d_name[1] == '.' || de->d_name[1] == 0)) continue;
        
	if((file_pattern.length() != 0 && fnregex != NULL) &&
	   (regexec ( fnregex, de->d_name, (size_t) 0, NULL, 0) != 0))
	  continue;
        
        struct stat st;
        if(stat((path + "/" + dirname + "/" + de->d_name).c_str(), &st) < 0)
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

        char* s = strchr(de->d_name, '.');
        if(s == NULL || (s -= 12) < de->d_name) continue;
        
        int year, month, mday, hour, minute, second;
        if(sscanf(s, "%2d%2d%2d%2d%2d%2d", &year, &month, &mday,
          &hour, &minute, &second) != 6)
            continue;

        struct tm t_parts;
        t_parts.tm_year = year + 100;
        t_parts.tm_mon = month - 1;
        t_parts.tm_mday = mday;
        t_parts.tm_hour = hour;
        t_parts.tm_min = minute;
        t_parts.tm_sec = second;
        
        if(verbosity > 1)
          {
            printf("%s %s\n", de->d_name, asctime(&t_parts));
            fflush(stdout);
          }
        
        Timestamp tstmp(mktime(&t_parts), 0);
        
        if(tstmp > last_collected)
            last_collected = tstmp;
        
        if(tstmp > last_processed)
            insert_object(file_map, new File(de->d_name, dirname, tstmp));
      }

    closedir(dir);
  }
    
void FS_Input_EDR::collect_files(const string &path, const string &file_pattern,
  map<string, rc_ptr<File> > &file_map) 
  {
    DIR *dir;
    dirent *de;
    
    if((dir = opendir(path.c_str())) == NULL) return;

    while((de = readdir(dir)) != NULL)
      {
        if(de->d_name[0] == '.' && 
          (de->d_name[1] == '.' || de->d_name[1] == 0)) continue;
        
        int yday;
        if(sscanf(de->d_name, "%d", &yday) != 1 || yday < 1 || yday > 366)
            continue;
        
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

        collect_files2(path, de->d_name, file_pattern, file_map);
      }

    closedir(dir);
  }

void FS_Input_EDR::process_files(const string &path,
  const map<string, rc_ptr<File> > &file_map)
  {
    map<string, rc_ptr<File> >::const_iterator p;
    for(p = file_map.begin(); p != file_map.end(); ++p)
      {
        if(p->second->tstmp >= last_collected)
          {
//            printf("active file: %s\n", p->first.c_str());
//            fflush(stdout);
            continue;
          }
        
        cvt->process_file(path + "/" + p->second->dir + "/" + p->second->name, 0, -1);

        if(p->second->tstmp > last_processed)
            last_processed = p->second->tstmp;
      }
  }
 
void FS_Input_EDR::check_for_new_data(const string &path,
  const string &file_pattern)
  {
    map<string, rc_ptr<File> > file_map;
    collect_files(path, file_pattern, file_map);
    process_files(path, file_map);
  }

void FS_Input_EDR::set_timestamp(const Timestamp &tstmp)
  {
    last_processed = tstmp;
  }

Timestamp FS_Input_EDR::get_timestamp()
  {
    return last_processed;
  }

RegisterInput<FS_Input_EDR> input("edr");

} // unnamed namespace

