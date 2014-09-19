/***************************************************************************** 
 * fs_plugin.h
 *
 * SeedLink plugin that reads data from filesystem
 *
 * (c) 2002 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef FS_PLUGIN_H
#define FS_PLUGIN_H

#include <string>
#include <set>

#include <signal.h>
#include <sys/types.h>

#include "qtime.h"

#include "plugin_module.h"
#include "plugin_exceptions.h"
#include "utils.h"

namespace SeedlinkPlugin_private {

using namespace std;
using namespace PluginModule;
using namespace Utilities;

//*****************************************************************************
// Timestamp
//*****************************************************************************

class Timestamp
  {
  public:
    time_t sec;
    unsigned long usec;

    Timestamp(time_t sec_init, unsigned long usec_init):
      sec(sec_init), usec(usec_init) {}

    Timestamp(): sec(0), usec(0) {}
    
    bool operator==(const Timestamp &other) const
      {
        return (sec == other.sec) && (usec == other.usec);
      }
    
    bool operator<(const Timestamp &other) const
      {
        return (sec < other.sec) || ((sec == other.sec) && (usec < other.usec));
      }

    bool operator>(const Timestamp &other) const
      {
        return (sec > other.sec) || ((sec == other.sec) && (usec > other.usec));
      }

    bool operator<=(const Timestamp &other) const
      {
        return !(*this > other);
      }

    bool operator>=(const Timestamp &other) const
      {
        return !(*this < other);
      }
  };

//*****************************************************************************
// FS_Decoder
//*****************************************************************************

class FS_Decoder
  {
  public:
    virtual void attach_output_channel(const string &source_id,
      const string &channel_name) =0;
    virtual void flush_channels() =0;
    virtual void process_file(const string &file, off_t offset,
      ssize_t len) =0;
    virtual ~FS_Decoder() {}
  };

//*****************************************************************************
// FS_Input
//*****************************************************************************

class FS_Input
  {
  public:
    virtual void init(rc_ptr<FS_Decoder> cvt_init) =0;
    virtual void check_for_new_data(const string &path,
        const string &file_pattern) =0;
    virtual Timestamp get_timestamp() =0;
    virtual void set_timestamp(const Timestamp &tstmp) =0;
    virtual ~FS_Input() {}
  };

//*****************************************************************************
// RegisterDecoder
//*****************************************************************************

template<class U>
class RegisterDecoder
  {
  private:
    const RegisteredModule<FS_Decoder> r;

  public:
    RegisterDecoder(const string &name):
      r(name, new ModuleSpecImpl<FS_Decoder, U>) {}
  };

//*****************************************************************************
// RegisterInput
//*****************************************************************************

template<class U>
class RegisterInput
  {
  private:
    const RegisteredModule<FS_Input> r;

  public:
    RegisterInput(const string &name):
      r(name, new ModuleSpecImpl<FS_Input, U>) {}
  };

//*****************************************************************************
// DigitizerTime
//*****************************************************************************

struct DigitizerTime
  {
    INT_TIME it;
    bool valid;
    bool exact;

    DigitizerTime(): valid(false), exact(false) {}
  };

//*****************************************************************************
// Externals
//*****************************************************************************

extern const char *const ident_str;
extern string station_name;
extern string network_name;
extern set<string> accepted_stations;
extern int default_timing_quality;
extern int zero_sample_limit;
extern int file_read_delay;
extern int verbosity;
extern int scan_level;
extern bool move_files;
extern bool delete_files;
extern bool use_timestamp;
extern ostream seed_log;
extern DigitizerTime digitime;
extern volatile sig_atomic_t terminate_proc;

} // namespace SeedlinkPlugin_private

namespace SeedlinkPlugin {
using SeedlinkPlugin_private::Timestamp;
using SeedlinkPlugin_private::FS_Decoder;
using SeedlinkPlugin_private::FS_Input;
using SeedlinkPlugin_private::RegisterDecoder;
using SeedlinkPlugin_private::RegisterInput;
using SeedlinkPlugin_private::DigitizerTime;
using SeedlinkPlugin_private::ident_str;
using SeedlinkPlugin_private::station_name;
using SeedlinkPlugin_private::network_name;
using SeedlinkPlugin_private::accepted_stations;
using SeedlinkPlugin_private::default_timing_quality;
using SeedlinkPlugin_private::zero_sample_limit;
using SeedlinkPlugin_private::file_read_delay;
using SeedlinkPlugin_private::verbosity;
using SeedlinkPlugin_private::scan_level;
using SeedlinkPlugin_private::move_files;
using SeedlinkPlugin_private::delete_files;
using SeedlinkPlugin_private::use_timestamp;
using SeedlinkPlugin_private::seed_log;
using SeedlinkPlugin_private::digitime;
using SeedlinkPlugin_private::terminate_proc;

} // namespace SeedlinkPlugin

#endif // FS_PLUGIN_H

