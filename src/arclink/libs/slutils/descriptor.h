/***************************************************************************** 
 * descriptor.h
 *
 * StationDescriptor and StreamDescriptor definitions
 *
 * (c) 2004 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <string>

namespace Utilities_private {

using namespace std;

//*****************************************************************************
// StationDescriptor
//*****************************************************************************

// This defines unique ID of a station. operator<() allows to use it
// as a key in associative containers.

class StationDescriptor
  {
  public:
    const string network;
    const string name;

    StationDescriptor() {}
    
    StationDescriptor(const string &network_init, const string &name_init):
      network(network_init), name(name_init) {}

    StationDescriptor &operator=(const StationDescriptor &other)
      {
        if(this != &other)
          {
            this->~StationDescriptor();
            new(this) StationDescriptor(other);
          }

        return *this;
      }

    bool operator<(const StationDescriptor &other) const
      {
        int network_cmp = strcasecmp(network.c_str(), other.network.c_str());
        if(network_cmp < 0) return true;
        else if(network_cmp == 0)
          {
            int name_cmp = strcasecmp(name.c_str(), other.name.c_str());
            if(name_cmp < 0) return true;
          }

        return false;
      }

    bool operator!=(const StationDescriptor &other) const
      {
        return (strcasecmp(network.c_str(), other.network.c_str()) ||
          strcasecmp(name.c_str(), other.name.c_str()));
      }

    bool operator==(const StationDescriptor &other) const
      {
        return !(*this != other);
      }

    bool operator>=(const StationDescriptor &other) const
      {
        return !(*this < other);
      }

    bool operator>(const StationDescriptor &other) const
      {
        return (*this >= other && *this != other);
      }

    bool operator<=(const StationDescriptor &other) const
      {
        return (*this < other || *this == other);
      }

    string to_string() const
      {
        return (network + "." + name);
      }
  };
  
//*****************************************************************************
// StreamDescriptor
//*****************************************************************************

// This defines a unique ID of a SeedLink datastream within the scope of
// a single station. operator<() allows to use it as a key in associative
// containers.

class StreamDescriptor
  {
  public:
    const string location;
    const string seedname;
    const int type;

    StreamDescriptor(): type(-1) {}

    StreamDescriptor(const string &loc, const string &name, int typ):
      location(loc), seedname(name), type(typ) {}

    StreamDescriptor &operator=(const StreamDescriptor &str_desc)
      {
        if(this != &str_desc)
          {
            this->~StreamDescriptor();
            new(this) StreamDescriptor(str_desc);
          }

        return *this;
      }

    bool operator<(const StreamDescriptor &other) const
      {
        int location_cmp = strcasecmp(location.c_str(), other.location.c_str());
        if(location_cmp < 0) return true;
        else if(location_cmp == 0)
          {
            int seedname_cmp = strcasecmp(seedname.c_str(), other.seedname.c_str());
            if(seedname_cmp < 0) return true;
            else if(seedname_cmp == 0)
              {
                if(type < other.type) return true;
              }
          }

        return false;
      }

    bool operator!=(const StreamDescriptor &other) const
      {
        return (strcasecmp(location.c_str(), other.location.c_str()) ||
          strcasecmp(seedname.c_str(), other.seedname.c_str()) ||
          type != other.type);
      }

    bool operator==(const StreamDescriptor &other) const
      {
        return !(*this != other);
      }

    bool operator>=(const StreamDescriptor &other) const
      {
        return !(*this < other);
      }

    bool operator>(const StreamDescriptor &other) const
      {
        return (*this >= other && *this != other);
      }

    bool operator<=(const StreamDescriptor &other) const
      {
        return (*this < other || *this == other);
      }

    string to_string() const
      {
        string stype;
        switch(type)
          {
          case SLDATA: stype = "D"; break;
          case SLDET: stype = "E"; break;
          case SLTIM: stype = "T"; break;
          case SLCAL: stype = "C"; break;
          case SLMSG: stype = "L"; break;
          case SLBLK: stype = "O"; break;
          default: stype = "X";
          }

        return (location + "." + seedname + "." + stype);
      }
  };

} // namespace Utilities_private

namespace Utilities {

using Utilities_private::StationDescriptor;
using Utilities_private::StreamDescriptor;

} // namespace Utilities

#endif // DESCRIPTOR_H

