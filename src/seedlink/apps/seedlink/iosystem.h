/*****************************************************************************
 * iosystem.h
 *
 * Module "IOSystem"
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef IOSYSTEM_H
#define IOSYSTEM_H

#include <string>
#include <cstring>
#include <cerrno>
#include <cstdlib>

#include <sys/types.h>
#include <sys/time.h>

// fix broken FD_ZERO
#ifdef FD_ZERO_BUG
#include <cstdlib>
#undef FD_ZERO
#define FD_ZERO(p) memset((p), 0, sizeof(fd_set))
#endif

#include "monitor.h"
#include "cppstreams.h"
#include "utils.h"
#include "buffer.h"

namespace {

const int         MSEED_RECLEN      = 9;          // 512 bytes
const int         MAX_HEADER_LEN    = 512;

}

namespace IOSystem_private {

using namespace std;
using namespace SeedlinkMonitor;
using namespace CPPStreams;
using namespace Utilities;

//*****************************************************************************
// Exceptions
//*****************************************************************************

class Error: public GenericException
  {
  public:
    Error(const string &message):
      GenericException("IOSystem", message) {}
  };

class LibraryError: public Error
  {
  public:
    LibraryError(const string &message):
      Error(message + " (" + strerror(errno) + ")") {}
  };

class CannotCreateFile: public LibraryError
  {
  public:
    CannotCreateFile(const string &name):
      LibraryError("cannot create file '" + name + "'") {}
  };

class CannotOpenFile: public LibraryError
  {
  public:
    CannotOpenFile(const string &name):
      LibraryError("cannot open file '" + name + "'") {}
  };

class CannotReadFile: public LibraryError
  {
  public:
    CannotReadFile(const string &name):
      LibraryError("cannot read file '" + name + "'") {}
  };

class CannotWriteFile: public LibraryError
  {
  public:
    CannotWriteFile(const string &name):
      LibraryError("cannot write file '" + name + "'") {}
  };

class CannotDeleteFile: public LibraryError
  {
  public:
    CannotDeleteFile(const string &name):
      LibraryError("cannot delete file '" + name + "'") {}
  };

class CannotStatFile: public LibraryError
  {
  public:
    CannotStatFile(const string &name):
      LibraryError("cannot stat file '" + name + "'") {}
  };

class CannotOpenDir: public LibraryError
  {
  public:
    CannotOpenDir(const string &name):
      LibraryError("cannot open directory '" + name + "'") {}
  };

class BadFileFormat: public Error
  {
  public:
    BadFileFormat(const string &name):
      Error("file '" + name + "' has wrong format") {}
  };

class BadFileName: public Error
  {
  public:
    BadFileName(const string &name):
      Error("file '" + name + "' has incorrect name") {}
  };

class CannotAllocateFD: public Error
  {
  public:
    CannotAllocateFD():
      Error("cannot allocate file descriptor") {}
  };

class FDSetsizeExceeded: public Error
  {
  public:
    FDSetsizeExceeded(int fd):
      Error("FD_SETSIZE exceeded (" + to_string(fd) + ")") {}
  };

//*****************************************************************************
// Fdset
//*****************************************************************************

class Fdset
  {
  private:
    fd_set read_set, write_set, write_set2, read_active, write_active;
    int select_status;

    void check_fd(int fd, const char *file, int line) const
      {
        if(fd >= FD_SETSIZE)
          {
            clog << file << ":" << line << ": " << fd << " >= FD_SETSIZE" << endl;
            exit(1);
          }
      }

  public:
    Fdset(): select_status(0)
      {
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&write_set2);
        FD_ZERO(&read_active);
        FD_ZERO(&write_active);
      }

    void set_read(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_SET(fd, &read_set);
      }

    void set_write(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_SET(fd, &write_set);
        FD_SET(fd, &write_set2);
      }

    void set_write2(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_SET(fd, &write_set2);
      }

    void clear_read(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_CLR(fd, &read_set);
      }

    void clear_write(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_CLR(fd, &write_set);
        FD_CLR(fd, &write_set2);
      }

    void clear_write2(int fd)
      {
        check_fd(fd, __FILE__, __LINE__);
        FD_CLR(fd, &write_set);  // NB: write_set, not write_set2
      }

    void sync()
      {
        write_set = write_set2;
      }

    bool isactive_read(int fd) const
      {
        check_fd(fd, __FILE__, __LINE__);
        return FD_ISSET(fd, &read_active);
      }

    bool isactive_write(int fd) const
      {
        check_fd(fd, __FILE__, __LINE__);
        return FD_ISSET(fd, &write_active);
      }

    int select(timeval *ptv)
      {
        read_active = read_set;
        write_active = write_set;

        select_status = ::select(FD_SETSIZE, &read_active, &write_active, NULL, ptv);
        return select_status;
      }

    int status()
      {
        return select_status;
      }
  };

//*****************************************************************************
// ConnectionHandler
//*****************************************************************************

class ConnectionHandler
  {
  public:
    virtual bool operator()(Fdset &fds)
      {
        return true;
      }

    virtual ~ConnectionHandler() {}
  };

template<class T>
class ConnectionHandlerAdapter: public ConnectionHandler
  {
  private:
    T rep;

  public:
    ConnectionHandlerAdapter(const T &rep_init): rep(rep_init) {}

    bool operator()(Fdset &fds)
      {
        return rep(fds);
      }
  };

//*****************************************************************************
// ConnectionManager
//*****************************************************************************

class ConnectionManager
  {
  protected:
    rc_ptr<ConnectionHandler> handler;

  public:
    virtual rc_ptr<BufferStore> register_station(const string &station_key,
      const string &station_name, const string &network_id,
      const string &description, bool rlog, const string &station_dir_init,
      int nbufs, int blank_bufs_init, int filesize_init, int nfiles,
      int seq_gap_limit, bool stream_check, const string &gap_check_pattern,
      int gap_treshold, const IPACL &ip_access) =0;

    virtual void start(int port) =0;
    virtual void save_state() =0;
    virtual void restore_state() =0;
    virtual ~ConnectionManager() {}

    template<class T>
    void set_handler(const T &h)
      {
        handler = new ConnectionHandlerAdapter<T>(h);
      }
  };

//*****************************************************************************
// Entry Point
//*****************************************************************************

rc_ptr<ConnectionManager> make_conn_manager(const string &daemon_name,
  const string &software_ident, const string &default_network_id,
  rc_ptr<MasterMonitor> monitor, bool rlog, int max_conn,
  int max_conn_per_ip, int trusted_info_level, int untrusted_info_level,
  bool trusted_window_extraction, bool untrusted_window_extraction,
  int max_bps, int to_sec, int to_usec);

} // namespace IOSystem_private

namespace IOSystem {

using IOSystem_private::Error;
using IOSystem_private::LibraryError;
using IOSystem_private::CannotCreateFile;
using IOSystem_private::CannotOpenFile;
using IOSystem_private::CannotReadFile;
using IOSystem_private::CannotWriteFile;
using IOSystem_private::CannotDeleteFile;
using IOSystem_private::BadFileFormat;
using IOSystem_private::Fdset;
using IOSystem_private::ConnectionManager;
using IOSystem_private::make_conn_manager;

} // namespace IOSystem

#endif // IOSYSTEM_H

