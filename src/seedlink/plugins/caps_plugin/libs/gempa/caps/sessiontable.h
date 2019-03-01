/***************************************************************************
 * libcapsclient
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#ifndef __GEMPA_CAPS_SESSIONTABLE_H__
#define __GEMPA_CAPS_SESSIONTABLE_H__

#include "packet.h"

#include <boost/function.hpp>

#include <map>
#include <string>

namespace Gempa {
namespace CAPS {

struct SessionTableItem {
	SessionTableItem() : samplingFrequency(0), samplingFrequencyDivider(0),
	                     fSamplingFrequency(0.0), dataType(DT_Unknown),
	                     dataSize(0), userData(NULL) {}
	std::string           streamID;
	std::string           net;
	std::string           sta;
	std::string           loc;
	std::string           cha;
	int                   samplingFrequency;
	int                   samplingFrequencyDivider;
	double                fSamplingFrequency;
	PacketType            type;
	DataType              dataType;
	int                   dataSize;
	UOM                   uom;
	Time                  startTime;
	Time                  endTime;
	void                 *userData;

	bool splitStreamID();
};


class SessionTable : public std::map<int, SessionTableItem> {
	public:
		enum Status {Success, Error, EOD};

		typedef boost::function<void (SessionTableItem*)> CallbackFunc;

	public:
		//! Default constructor
		SessionTable();
		virtual ~SessionTable() {}

		//! Resets state
		void reset();

		SessionTableItem* getItem(int id) {
			SessionTable::iterator it = find(id);
			if ( it == end() ) return NULL;

			return &it->second;
		}

		Status handleResponse(const char *src_data, int data_len);

		void setItemAddedFunc(const CallbackFunc &func) { _itemAddedFunc = func; }
		void setItemAboutToBeRemovedFunc(const CallbackFunc &func) {
			_itemAboutToBeRemovedFunc = func;
		}

	private:
		enum ResponseState {
			Unspecific,
			Requests
		};

		typedef std::map<std::string, int> StreamIDLookupTable;

	private:
		void registerItem(int id, SessionTableItem &item);
		void removeStream(const std::string & streamID);

	private:
		ResponseState        _state;
		StreamIDLookupTable  _streamIDLookup;
		CallbackFunc         _itemAddedFunc;
		CallbackFunc         _itemAboutToBeRemovedFunc;
};

}
}


#endif
