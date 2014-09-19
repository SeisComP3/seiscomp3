/***************************************************************************** 
 * serial_plugin.h
 *
 * SeedLink plugin for serial digitizers
 *
 * (c) 2000 Andres Heinloo, GFZ potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef SERIAL_PLUGIN_H
#define SERIAL_PLUGIN_H

#include <iostream>
#include <string>
#include <vector>
#include <cerrno>
#include <csignal>

#include <sys/types.h>

#include "qtime.h"

#include "plugin_module.h"
#include "plugin_exceptions.h"
#include "cppstreams.h"
#include "utils.h"

namespace SeedlinkPlugin_private {

using namespace std;
using namespace PluginModule;
using namespace CPPStreams;
using namespace Utilities;

const char *const SEED_NEWLINE = "\r\n";

//*****************************************************************************
// Exceptions
//*****************************************************************************

class PluginReadError: public PluginError
  {
  public:
    PluginReadError(const string &port_name):
      PluginError(string() + "error reading data from port " + port_name) {}
    PluginReadError(const string &port_name, const string &description):
      PluginError(string() + "error reading data from port " + port_name + " (" + description + ")") {}
  };

class PluginInvalidPort: public PluginError
  {
  public:
    PluginInvalidPort(const string &port):
      PluginError(string() + "invalid port " + port) {}
  };

class PluginInvalidBaudrate: public PluginError
  {
  public:
    PluginInvalidBaudrate(const string &port, int bps):
      PluginError(string() + "cannot set baudrate of port " + port + " to " + to_string(bps)) {}
  };

class PluginCannotOpenPort: public PluginLibraryError
  {
  public:
    PluginCannotOpenPort(const string &port):
      PluginLibraryError(string() + "cannot open port " + port) {}
  };

//*****************************************************************************
// DigitizerConfig
//*****************************************************************************

struct DigitizerConfig
  {
    string proto_name;
    string port_name;
    int port_bps;
    int checksum_used;
    int use_pctime_if_no_gps;
    int lsb;
    int statusinterval;
    int time_offset;
    int zero_sample_limit;
    int default_tq;
    int unlock_tq;
    int nbundles;              // HRD-24
    string soh_log_dir;        // HRD-24
    int baseaddr;              // Modbus

    DigitizerConfig(): port_bps(0), checksum_used(0), use_pctime_if_no_gps(0),
      lsb(8), statusinterval(0), time_offset(0), zero_sample_limit(-1),
      default_tq(0), unlock_tq(10), nbundles(59), baseaddr(0) {}
  };

//*****************************************************************************
// DigitizerTime
//*****************************************************************************

struct DigitizerTime
  {
    INT_TIME it;
    int quality;
    bool valid;
    bool exact;

    DigitizerTime(): quality(0), valid(false), exact(false) {}
  };

//*****************************************************************************
// Proto
//*****************************************************************************

class Proto
  {
  public:
    virtual void attach_output_channel(const string &source_id,
      const string &channel_name, const string &station_name,
      double scale, double realscale, double realoffset,
      const string &realunit, int precision) =0;
    virtual void flush_channels() =0;
    virtual void start() =0;
    virtual ~Proto() {}
  };
    
//*****************************************************************************
// RegisterProto
//*****************************************************************************

template<class U>
class RegisterProto
  {
  private:
    const RegisteredModule<Proto> r;

  public:
    RegisterProto(const string &name):
      r(name, new ModuleSpecImpl<Proto, U>) {}
  };
        
//*****************************************************************************
// Externals
//*****************************************************************************

extern const char *const ident_str;
extern string station_name;
extern ostream seed_log;
extern DigitizerTime digitime;
extern DigitizerConfig dconf;
extern volatile sig_atomic_t terminate_proc;

int open_port(mode_t mode);
void sync_data(int fd);
ssize_t read_port(int fd, void *vptr, size_t n);

} // namespace SeedlinkPlugin_private

namespace SeedlinkPlugin {

using SeedlinkPlugin_private::SEED_NEWLINE;
using SeedlinkPlugin_private::PluginReadError;
using SeedlinkPlugin_private::PluginInvalidBaudrate;
using SeedlinkPlugin_private::PluginCannotOpenPort;
using SeedlinkPlugin_private::DigitizerConfig;
using SeedlinkPlugin_private::Proto;
using SeedlinkPlugin_private::RegisterProto;
using SeedlinkPlugin_private::ident_str;
using SeedlinkPlugin_private::station_name;
using SeedlinkPlugin_private::seed_log;
using SeedlinkPlugin_private::digitime;
using SeedlinkPlugin_private::dconf;
using SeedlinkPlugin_private::terminate_proc;
using SeedlinkPlugin_private::open_port;
using SeedlinkPlugin_private::sync_data;
using SeedlinkPlugin_private::read_port;

} // namespace SeedlinkPlugin

#endif // SERIAL_PLUGIN_H

