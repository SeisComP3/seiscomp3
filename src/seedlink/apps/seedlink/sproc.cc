/*****************************************************************************
 * sproc.cc
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

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>

#include "utils.h"
#include "conf_xml.h"
#include "confattr.h"
#include "sproc.h"
#include "cbuf.h"
#include "filterimpl.h"
#include "encoder.h"
#include "spclock.h"
#include "diag.h"

namespace SProc_private {

using namespace std;
using namespace Utilities;
using namespace CfgParser;

const int MAX_CBSIZE = 14;
const int SPS_RANGE = 65535;  // 1/65535...65535 sps


//*****************************************************************************
// Data packet
//*****************************************************************************

void DataPacket::set_data(const int32_t *d, int len, const INT_TIME &time,
                          int usec_corr, int timing_qual,
                          const int freqn, const int freqd)
  {
    data.assign(d, d+len);
    usec_correction = usec_corr;
    timing_quality = timing_qual;
    stime = time;
    etime = add_dtime(stime, 1000000 * (double(data.size()) * double(freqd) / double(freqn)));
  }

void DataPacket::append_data(const int32_t *d, int len,
                             const int freqn, const int freqd)
  {
    data.insert(data.end(), d, d+len);
    etime = add_dtime(stime, 1000000 * (double(data.size()) * double(freqd) / double(freqn)));
  }


//*****************************************************************************
// Backfilling
//*****************************************************************************

bool gt(const INT_TIME &it1, const INT_TIME &it2)
  {
    if(it1.year > it2.year) return true;
    if(it1.year < it2.year) return false;
    if(it1.second > it2.second) return true;
    if(it1.second < it2.second) return false;
    if(it1.usec > it2.usec) return true;
    if(it1.usec < it2.usec) return false;
    return false;
  }


//*****************************************************************************
// Node
//*****************************************************************************

class Node: public Input
  {
  private:
    list<rc_ptr<Node> > children;
    rc_ptr<Encoder> encoder;
    rc_ptr<Filter> filter;
    CircularBuffer<double> cbuf;
    CircularBuffer<double>::iterator inp;

    void sync_time(const Node &parent);
    void process_data(const Node &parent);
    void reset_child(const Node &parent);

  public:
    Node(rc_ptr<Filter> filter_init, int cbsize, int freqn, int freqd,
         double backfill_capacity = -1):
    Input(freqn, freqd, backfill_capacity), filter(filter_init), cbuf(cbsize) {}

    void set_time(const INT_TIME &it, int usec_correction,
      int timing_quality);
    void add_ticks(int n, int usec_correction, int timing_quality);
    void send_data(const int32_t *data, int len);
    void flush();
    void reset();
    void attach_node(rc_ptr<Node> node);
    void attach_encoder(rc_ptr<Encoder> enc);

    double time_gap()
      {
        return clk.time_gap();
      }
  };
 
void Node::sync_time(const Node &parent)
  {
    clk.sync_time(parent.clk, parent.cbuf.used(inp), filter->shift());
      
    if(encoder != NULL) encoder->sync_time(clk);

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->sync_time(*this);
  }
  
void Node::process_data(const Node &parent)
  {
    if(parent.cbuf.used(inp) < filter->length()) return;

    double val = filter->apply(inp);
    inp += filter->decimation();
    if(encoder != NULL) encoder->send_data(static_cast<int32_t>(val));
    cbuf.write(val);
    clk.tick();

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->process_data(*this);
  }

void Node::set_time(const INT_TIME &it, int usec_correction,
  int timing_quality)
  {
    clk.set_time(it, usec_correction, timing_quality);
    
    if(encoder != NULL) encoder->sync_time(clk);

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->sync_time(*this);
  }

void Node::add_ticks(int n, int usec_correction, int timing_quality)
  {
    clk.add_ticks(n, usec_correction, timing_quality);
    
    if(encoder != NULL) encoder->sync_time(clk);

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->sync_time(*this);
  }

void Node::send_data(const int32_t *data, int len)
  {
    list<rc_ptr<Node> >::iterator p;
    
    for(int i = 0; i < len; ++i)
      {
        if(encoder != NULL) encoder->send_data(data[i]);
        cbuf.write(data[i]);
        clk.tick();
        
        for(p = children.begin(); p != children.end(); ++p)
           (*p)->process_data(*this);
      }
  }
  
void Node::flush()
  {
    if(encoder != NULL) encoder->flush();

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->flush();
  }

void Node::reset_child(const Node &parent)
  {
    if(encoder != NULL) encoder->flush();

    inp = parent.cbuf.read_ptr();
    sync_time(parent);

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->reset_child(*this);
  }

void Node::reset()
  {
    if(encoder != NULL) encoder->flush();

    list<rc_ptr<Node> >::iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        (*p)->reset_child(*this);
  }

void Node::attach_node(rc_ptr<Node> node)
  {
    node->inp = cbuf.read_ptr();
    children.push_back(node);
  }

void Node::attach_encoder(rc_ptr<Encoder> enc)
  {
    encoder = enc;
  }
 
//*****************************************************************************
// DummyFilter
//*****************************************************************************

class DummyFilter: public Filter
  {
  public:
    DummyFilter(): Filter("(none)", 1)
      {
        len = 1;
      }

    double apply(CircularBuffer<double>::iterator p)
      {
        return *p;
      }

    double shift()
      {
        return 0;
      }
  };

//*****************************************************************************
// NodeSpec, InputSpec -- helper classes to construct stream tree according to
//                        configuration description
//*****************************************************************************

class NodeSpec
  {
  private:
    list<rc_ptr<NodeSpec> > children;
    const string stream_name;
    const rc_ptr<Filter> filter;
    int cbsize;
    int n_streams;

    void normalize_freq(int &freqn, int &freqd) const;

  public:
    NodeSpec(const string &stream, rc_ptr<Filter> filter_init):
      stream_name(stream), filter(filter_init), cbsize(1), n_streams(-1) {}

    rc_ptr<Node> instance(rc_ptr<EncoderSpec> encoder_spec,
      const string &channel_name, const string &location_id,
      int freqn, int freqd) const;
    void attach(rc_ptr<NodeSpec> child);
    int cbsize_required() const;
    bool remove_unused_child_nodes();
    
    int number_of_streams() const
      {
        return n_streams;
      }
  };

void NodeSpec::normalize_freq(int &freqn, int &freqd) const
  {
    int a, b;

    if(freqn > freqd)
      {
        a = freqn;
        b = freqd;
      }
    else
      {
        a = freqd;
        b = freqn;
      }
    
    while(b > 1)
      {
        int tmp = b;
        b = a % b;
        a = tmp;
      }
        
    if(b == 0)
      {
        freqn /= a;
        freqd /= a;
      }
  }

rc_ptr<Node> NodeSpec::instance(rc_ptr<EncoderSpec> encoder_spec,
  const string &channel_name, const string &location_id,
  int freqn, int freqd) const
  {
    rc_ptr<Filter> myfilter;
    
    if(filter == NULL) myfilter = new DummyFilter;
    else myfilter = filter;
    
    rc_ptr<Node> node = new Node(myfilter, cbsize, freqn, freqd);
    
    int cfreqn = freqn;
    int cfreqd = freqd * myfilter->decimation();
    
    normalize_freq(cfreqn, cfreqd);
    
    list<rc_ptr<NodeSpec> >::const_iterator p;
    for(p = children.begin(); p != children.end(); ++p)
        node->attach_node((*p)->instance(encoder_spec, channel_name,
        location_id, cfreqn, cfreqd));
    
    if(stream_name.length() != 0)
        node->attach_encoder(encoder_spec->instance(stream_name + channel_name,
          location_id, cfreqn, cfreqd));

    return node;
  }

void NodeSpec::attach(rc_ptr<NodeSpec> child)
  {
    if(child->cbsize_required() > cbsize)
        cbsize = child->cbsize_required();
    
    children.push_back(child);
  }

int NodeSpec::cbsize_required() const
  {
    if(filter == NULL) return 0;

    int b, n = filter->length();
    
    for(b = 1; b <= MAX_CBSIZE && n >= (1 << b); ++b);

    internal_check(b <= MAX_CBSIZE);

    return b;
  }

bool NodeSpec::remove_unused_child_nodes()
  {
    list<rc_ptr<NodeSpec> >::iterator p = children.begin();

    if(n_streams < 0)
      {
        n_streams = 0;
    
        while(p != children.end())
          {
            if((*p)->remove_unused_child_nodes())
              {
                children.erase(p++);
              }
            else
              {
                n_streams += (*p)->number_of_streams();
                ++p;
              }
          }

        if(stream_name.length() != 0) ++n_streams;
      }
    
    return (n_streams == 0);
  }

class InputSpec
  {
  private:
    const string channel_name;
    const string location_id;
    const int freqn;
    const int freqd;
    rc_ptr<NodeSpec> node_spec;

  public:
    const string name;
  
    InputSpec(const string &name_init, const string &channel_name_init,
      const string &location_id_init, int freqn_init, int freqd_init,
      rc_ptr<NodeSpec> node_spec_init):
      channel_name(channel_name_init), location_id(location_id_init),
      freqn(freqn_init), freqd(freqd_init), node_spec(node_spec_init),
      name(name_init) {}

    bool remove_unused_child_nodes()
      {
        return node_spec->remove_unused_child_nodes();
      }

    int number_of_streams() const
      {
        return node_spec->number_of_streams();
      }
    
    rc_ptr<Node> instance(rc_ptr<EncoderSpec> encoder_spec) const
      {
        return node_spec->instance(encoder_spec, channel_name,
          location_id, freqn, freqd);
      }
  };

//*****************************************************************************
// StreamProcesorImpl
//*****************************************************************************

class StreamProcessorImpl: public StreamProcessor
  {
  private:
    map<string, rc_ptr<Node> > input_nodes;

  public:
    StreamProcessorImpl(const string &name, 
      const map<string, rc_ptr<InputSpec> > input_specs, rc_ptr<EncoderSpec> encoder_spec);
    rc_ptr<Input> get_input(const string &channel_name);
    void flush();
    void visit_inputs(InputVisitor &visitor, void *data = NULL);
  };

StreamProcessorImpl::StreamProcessorImpl(const string &name, 
  const map<string, rc_ptr<InputSpec> > input_specs, rc_ptr<EncoderSpec> encoder_spec):
  StreamProcessor(name)
  {
    map<string, rc_ptr<InputSpec> >::const_iterator p;

    for(p = input_specs.begin(); p != input_specs.end(); ++p)
        input_nodes.insert(make_pair(p->first, p->second->instance(encoder_spec)));
  }

rc_ptr<Input> StreamProcessorImpl::get_input(const string &channel_name)
  {
    return get_object(input_nodes, channel_name);
  }

void StreamProcessorImpl::flush()
  {
    map<string, rc_ptr<Node> >::iterator p;
    for(p = input_nodes.begin(); p != input_nodes.end(); ++p)
        p->second->flush();
  }

void StreamProcessorImpl::visit_inputs(InputVisitor &v, void *data)
  {
    map<string, rc_ptr<Node> >::iterator p;
    for(p = input_nodes.begin(); p != input_nodes.end(); ++p)
      v.visit(p->first, p->second, data);
  }

//*****************************************************************************
// StreamProcessorSpecImpl -- helper class to make stream processors according
//                            to configuration description
//*****************************************************************************

class StreamProcessorSpecImpl: public StreamProcessorSpec
  {
  private:
    map<string, rc_ptr<InputSpec> > input_specs;
    
  public:
    StreamProcessorSpecImpl(const string &name,
      const map<string, rc_ptr<InputSpec> > &input_specs_init,
      const list<rc_ptr<StreamProcessorSpecImpl> > &merge);
    
    int number_of_streams() const;

    rc_ptr<StreamProcessor> instance(rc_ptr<EncoderSpec> encoder_spec) const
      {
        return new StreamProcessorImpl(name, input_specs, encoder_spec);
      }
  };
    
StreamProcessorSpecImpl::StreamProcessorSpecImpl(const string &name,
  const map<string, rc_ptr<InputSpec> > &input_specs_init,
  const list<rc_ptr<StreamProcessorSpecImpl> > &merge):
  StreamProcessorSpec(name), input_specs(input_specs_init)
  {
    list<rc_ptr<StreamProcessorSpecImpl> >::const_iterator i;
    map<string, rc_ptr<InputSpec> >::const_iterator j;
    for(i = merge.begin(); i != merge.end(); ++i)
        for(j = (*i)->input_specs.begin(); j != (*i)->input_specs.end(); ++j)
            insert_object(input_specs, j->second);
  }

int StreamProcessorSpecImpl::number_of_streams() const
  {
    int n_streams = 0;
    map<string, rc_ptr<InputSpec> >::const_iterator p;

    for(p = input_specs.begin(); p != input_specs.end(); ++p)
        n_streams += p->second->number_of_streams();
    
    return n_streams;
  }
 
//*****************************************************************************
// FreqAttribute -- config parameter for sample rate
//*****************************************************************************

class FreqAttribute: public CfgAttribute
  {
  private:
    int &freqn, &freqd;
    int range;

  public:
    FreqAttribute(const string &name, int &freqn_init, int &freqd_init, int range_init):
      CfgAttribute(name), freqn(freqn_init), freqd(freqd_init), range(range_init) {}

    bool assign(ostream &cfglog, const string &value);
  };

bool FreqAttribute::assign(ostream &cfglog, const string &value)
  {
    int arg;
    char *tail;

    arg = strtoul(value.c_str(), &tail, 0);
    
    if(*tail == 0)
      {
        freqn = arg;
        freqd = 1;
      }
    else if(*tail == '/')
      {
        freqn = arg;
        
        arg = strtoul(tail + 1, &tail, 0);

        if(*tail == 0) freqd = arg;
      }
    
    if(*tail || freqn == 0 || freqn > range || freqd == 0 || freqd > range)
      {
        cfglog << "[" << item_name << "] " << value << " is not a valid sample rate" << endl;
        return false;
      }

    return true;
  }

//*****************************************************************************
// XML elements
//*****************************************************************************

class InputElement: public CfgElement
  {
  private:
    string input_name;
    string channel_name;
    string location_id;
    int freqn, freqd;
    map<string, rc_ptr<InputSpec> > &input_specs;
    rc_ptr<NodeSpec> root_node;

  public:
    InputElement(map<string, rc_ptr<InputSpec> > &input_specs_init,
      rc_ptr<NodeSpec> node):
      CfgElement("input"), input_specs(input_specs_init),
      root_node(node) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &)
      {
        input_name = "";
        channel_name = "";
        location_id = "";
        freqn = 0;
        freqd = 0;
        
        rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
        atts->add_item(StringAttribute("name", input_name));
        atts->add_item(StringAttribute("channel", channel_name));
        atts->add_item(StringAttribute("location", location_id));
        atts->add_item(FreqAttribute("rate", freqn, freqd, SPS_RANGE));
        return atts;
      }

    void end_attributes(ostream &cfglog)
      {
        if(input_name.length() == 0)
          {
            cfglog << "input name is not specified" << endl;
            return;
          }

        if(freqn == 0)
          {
            cfglog << "input sample rate is not specified" << endl;
            return;
          }

        insert_object(input_specs, new InputSpec(input_name, channel_name,
          location_id, freqn, freqd, root_node));
      }
  };

class NodeElement: public CfgElement
  {
  private:
    string filter_name;
    string stream_name;
    const map<string, rc_ptr<Filter> > &filters;
    rc_ptr<NodeSpec> parent_node;
    rc_ptr<NodeSpec> this_node;

  public:
    NodeElement(rc_ptr<NodeSpec> node,
      const map<string, rc_ptr<Filter> > &filters_init):
      CfgElement("node"), filters(filters_init), parent_node(node) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &)
      {
        filter_name = "";
        stream_name = "";
        
        rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
        atts->add_item(StringAttribute("filter", filter_name));
        atts->add_item(StringAttribute("stream", stream_name));
        return atts;
      }

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &)
      {
        rc_ptr<Filter> filter;
        if((filter_name.length() != 0) &&
          (filter = get_object(filters, filter_name)) == NULL)
          {
            cfglog << "filter '" << filter_name << "' is not defined" << endl;
            return NULL;
          }
        
        this_node = new NodeSpec(stream_name, filter);

        rc_ptr<CfgElementMap> elms = new CfgElementMap;
        elms->add_item(NodeElement(this_node, filters));
        return elms;
      }
    
    void end_children(ostream &cfglog)
      {
        parent_node->attach(this_node);
      }
  };

class TreeElement: public CfgElement
  {
  private:
    rc_ptr<NodeSpec> root_node;
    const map<string, rc_ptr<Filter> > &filters;
    map<string, rc_ptr<InputSpec> > &input_specs;

  public:
    TreeElement(const map<string, rc_ptr<Filter> > &filters_init,
      map<string, rc_ptr<InputSpec> > &input_specs_init):
      CfgElement("tree"), filters(filters_init), input_specs(input_specs_init) {}

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &)
      {
        root_node = new NodeSpec(string(), NULL);
        
        rc_ptr<CfgElementMap> elms = new CfgElementMap;
        elms->add_item(InputElement(input_specs, root_node));
        elms->add_item(NodeElement(root_node, filters));
        return elms;
      }
  };

class UsingElement: public CfgElement
  {
  private:
    string proc_name;
    const map<string, rc_ptr<StreamProcessorSpec> > &proc_specs;
    list<rc_ptr<StreamProcessorSpecImpl> > &merge;

  public:
    UsingElement(const map<string, rc_ptr<StreamProcessorSpec> > &proc_specs_init,
      list<rc_ptr<StreamProcessorSpecImpl> > &merge_init):
      CfgElement("using"), proc_specs(proc_specs_init), merge(merge_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &)
      {
        proc_name = "";

        rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
        atts->add_item(StringAttribute("proc", proc_name));
        return atts;
      }

    void end_attributes(ostream &cfglog)
      {
        if(proc_name.length() == 0)
          {
            cfglog << "proc name is not specified" << endl;
            return;
          }

        rc_ptr<StreamProcessorSpecImpl> spec;
        if((spec = rc_ptr_cast<StreamProcessorSpecImpl>(get_object(proc_specs, proc_name))) == NULL)
          {
            cfglog << "proc '" << proc_name << "' is not defined" << endl;
            return;
          }

        merge.push_front(spec);
      }
  };

class ProcElement: public CfgElement
  {
  private:
    string proc_name;
    list<rc_ptr<StreamProcessorSpecImpl> > merge;
    map<string, rc_ptr<InputSpec> > input_specs;
    const map<string, rc_ptr<Filter> > &filters;
    map<string, rc_ptr<StreamProcessorSpec> > &proc_specs;
  
  public:
    ProcElement(const map<string, rc_ptr<Filter> > &filters_init,
      map<string, rc_ptr<StreamProcessorSpec> > &proc_specs_init):
      CfgElement("proc"), filters(filters_init), proc_specs(proc_specs_init) {}

    rc_ptr<CfgAttributeMap> start_attributes(ostream &cfglog,
      const string &)
      {
        proc_name = "";
        merge.clear();
        input_specs.clear();

        rc_ptr<CfgAttributeMap> atts = new CfgAttributeMap;
        atts->add_item(StringAttribute("name", proc_name));
        return atts;
      }

    rc_ptr<CfgElementMap> start_children(ostream &cfglog,
      const string &)
      {
        if(proc_name.length() == 0)
          {
            cfglog << "proc name is not specified" << endl;
            return NULL;
          }

        if(has_object(proc_specs, proc_name))
          {
            cfglog << "proc '" << proc_name << "' is already defined" << endl;
            return NULL;
          }

        rc_ptr<CfgElementMap> elms = new CfgElementMap;
        elms->add_item(UsingElement(proc_specs, merge));
        elms->add_item(TreeElement(filters, input_specs));
        return elms;
      }

    void end_children(ostream &cfglog)
      {
        map<string, rc_ptr<InputSpec> >::iterator p = input_specs.begin();

        while(p != input_specs.end())
          {
            if(p->second->remove_unused_child_nodes()) input_specs.erase(p++);
            else ++p;
          }
    
        insert_object<StreamProcessorSpec>(proc_specs, new StreamProcessorSpecImpl(proc_name,
          input_specs, merge));
      }
  };

//*****************************************************************************
// Entry point
//*****************************************************************************

rc_ptr<CfgElement> make_stream_proc_cfg(const map<string, rc_ptr<Filter> > &filters,
  map<string, rc_ptr<StreamProcessorSpec> > &specs)
  {
    return new ProcElement(filters, specs);
  }

} // namespace SProc_private;

