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


#ifndef __SEISCOMP_IO_RECORDFILTER_DEMUX_H__
#define __SEISCOMP_IO_RECORDFILTER_DEMUX_H__

#include <seiscomp3/io/recordfilter.h>
#include <map>


namespace Seiscomp {
namespace IO {


/**
 * \brief Record demuxer that demultiplexes different channels and applies
 * \brief a given record filter to each of them.
 */
class SC_SYSTEM_CORE_API RecordDemuxFilter : public RecordFilterInterface {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructs a record demuxer with an optional record filter
		//! applied to each channel.
		//! Note: the ownership goes to the record demuxer
		RecordDemuxFilter(RecordFilterInterface *recordFilter = NULL);
		virtual ~RecordDemuxFilter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Note: the ownership goes to the record demuxer
		void setFilter(RecordFilterInterface *recordFilter);


	// ------------------------------------------------------------------
	//  RecordFilter interface
	// ------------------------------------------------------------------
	public:
		virtual Record *feed(const Record *rec);
		virtual Record *flush();
		virtual void reset();
		virtual RecordFilterInterface *clone() const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		typedef std::map<std::string, RecordFilterInterfacePtr> FilterMap;
		RecordFilterInterfacePtr _template;
		FilterMap                _streams;
};


}
}

#endif
