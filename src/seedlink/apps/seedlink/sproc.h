/*****************************************************************************
 * sproc.h
 *
 * Stream Processor
 *
 * (c) 2000 Andres Heinloo, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/

#ifndef SPROC_H
#define SPROC_H

#include <string>
#include <vector>
#include <list>

#include "qtime.h"

#include "utils.h"
#include "conf_xml.h"
#include "format.h"
#include "encoder.h"
#include "filter.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;
using namespace CfgParser;

//*****************************************************************************
// Abstract base classes
//*****************************************************************************

struct DataPacket
  {
  DataPacket() : flush(false) {}
  void set_data(const int32_t *d, int len, const INT_TIME &time,
                int usec_correction, int timing_quality,
                const int freqn, const int freqd);
  void append_data(const int32_t *d, int len, const int freqn, const int freqd);

  INT_TIME stime;
  INT_TIME etime;
  int usec_correction;
  int timing_quality;
  vector<int32_t> data;
  bool flush;
  };


bool gt(const INT_TIME &it1, const INT_TIME &it2);

template <typename PACKET>
struct Backfilling
  {
  typedef PACKET Packet;
  typedef rc_ptr<Packet> PacketPtr;
  typedef list<PacketPtr> PacketList;

  Backfilling(double cap = -1) : capacity(cap), current(NULL), committed(false) {}
  bool is_enabled() const { return capacity > 0; }
  //! Insert a packet sorted into the buffer
  void insert(PacketPtr packet)
    {
      typename PacketList::iterator it;
      for(it = buffer.begin(); it != buffer.end(); ++it)
        {
          if(gt((*it)->etime,packet->etime))
            {
              buffer.insert(it, packet);
              return;
            }
        }

      buffer.push_back(packet);
    }

  double capacity; // capacity in seconds
  PacketList buffer;
  PacketPtr  current;
  INT_TIME last_commit;
  bool committed;
  };

typedef Backfilling<DataPacket> InputBackfilling;

struct Input
  {
  Input(int freqn, int freqd, double backfill_capacity = -1)
  : clk(freqn, freqd), backfilling(backfill_capacity) {}
  virtual void set_time(const INT_TIME &it, int usec_correction,
    int timing_quality) =0;
  virtual void add_ticks(int n, int usec_correction,
    int timing_quality) =0;
  virtual void send_data(const int32_t *data, int len) =0;
  virtual void flush() =0;
  virtual void reset() =0;
  virtual double time_gap() =0;
  virtual ~Input() {}

  SPClock clk;
  InputBackfilling backfilling;
  };

class StreamProcessor
  {
  protected:
    StreamProcessor(const string &name_init): name(name_init) {}
  
  public:
    struct InputVisitor
      {
        virtual ~InputVisitor() {}
        virtual void visit(const string &channel_name, rc_ptr<Input> input, void *data = NULL) = 0;
      };

    const string name;
    
    virtual rc_ptr<Input> get_input(const string &channel_name) =0;
    virtual void flush() =0;
    virtual void visit_inputs(InputVisitor &, void *data = NULL) =0;
    virtual ~StreamProcessor() {}
  };

class EncoderSpec
  {
  public:
    virtual rc_ptr<Encoder> instance(const string &stream_name,
      const string &location_id, int freqn, int freqd) const =0;
    virtual ~EncoderSpec() {}
  };

class StreamProcessorSpec
  {
  protected:
    StreamProcessorSpec(const string &name_init): name(name_init) {}
    
  public:
    const string name;

    virtual rc_ptr<StreamProcessor> instance(rc_ptr<EncoderSpec> encoder_spec) const =0;
    virtual int number_of_streams() const =0;
    virtual ~StreamProcessorSpec() {}
  };

//*****************************************************************************
// Helper function to specify data format
//*****************************************************************************

template<class _Encoder, class _Format>
rc_ptr<EncoderSpec> make_encoder_spec(rc_ptr<BufferStore> bufs, int rec_length,
  PacketType packtype, const string &station_id, const string &network_id)
  {
    class EncoderSpecHelper: public EncoderSpec
      {
      private:
        const rc_ptr<BufferStore> bufs;
        int rec_length;
        PacketType packtype;
        const string station_id;
        const string network_id;

      public:
        EncoderSpecHelper(rc_ptr<BufferStore> bufs_init, int rec_length_init,
          PacketType packtype_init, const string &station_id_init,
          const string &network_id_init):
          bufs(bufs_init), rec_length(rec_length_init), packtype(packtype_init),
          station_id(station_id_init), network_id(network_id_init) {}

        rc_ptr<Encoder> instance(const string &stream_name,
          const string &location_id, int freqn, int freqd) const
          {
            return new _Encoder(new _Format(bufs, rec_length, packtype, station_id,
              network_id, location_id, stream_name, freqn, freqd), freqn, freqd);
          }
      };

    return new EncoderSpecHelper(bufs, rec_length, packtype, station_id, network_id);
  }

//*****************************************************************************
// Entry point
//*****************************************************************************

rc_ptr<CfgElement> make_stream_proc_cfg(const map<string, rc_ptr<Filter> > &filters,
  map<string, rc_ptr<StreamProcessorSpec> > &specs);

} // namespace SProc_private

namespace SProc {

using SProc_private::Input;
using SProc_private::StreamProcessor;
using SProc_private::EncoderSpec;
using SProc_private::StreamProcessorSpec;
using SProc_private::Backfilling;
using SProc_private::InputBackfilling;
using SProc_private::make_encoder_spec;
using SProc_private::make_stream_proc_cfg;

} // namespace SProc_private

#endif // SPROC_H

