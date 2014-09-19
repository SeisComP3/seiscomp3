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


#ifndef __EVENTDATA_H___
#define __EVENTDATA_H___

#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>

#include <seiscomp3/datamodel/databasearchive.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <seiscomp3/datamodel/publicobjectcache.h>


template <typename T>
class Data {
	public:
		Data(T* object)
		 : _objectPtr(object),
		   _containerCreationTime(Seiscomp::Core::Time::GMT()) {
		}
		virtual ~Data() {}

	public:
		virtual const std::string& id() const = 0;

		const T* object() const {
			return _objectPtr.get();
		}

		const Seiscomp::Core::Time& containerCreationTime() const {
			return _containerCreationTime;
		}

		void setContainerCreationTime(const Seiscomp::Core::Time& time) {
			_containerCreationTime = time;
		}

	private:
		typename Seiscomp::Core::SmartPointer<T>::Impl _objectPtr;
		Seiscomp::Core::Time                           _containerCreationTime;

};




class EventData : public Data<Seiscomp::DataModel::Event> {
	public:
		EventData(Seiscomp::DataModel::Event* event, Seiscomp::Gui::OriginSymbol* originSymbol)
		 : Data<Seiscomp::DataModel::Event>(event),
		   _originSymbolRef(originSymbol),
		   _isActive(false),
		   _isSelected(false) {
		}

	public:
		virtual const std::string& id() const {
			return object()->publicID();
		}

		const std::string& preferredOriginId() const {
			return object()->preferredOriginID();
		}

		bool isActive() const {
			return _isActive;
		}

		void setActive(bool val) {
			_isActive = val;
		}

		bool isSelected() const {
			return _isSelected;
		}

		void setSelected(bool val) {
			_isSelected = val;
		}

		Seiscomp::Gui::OriginSymbol* originSymbol() const {
			return _originSymbolRef;
		}

	private:
		Seiscomp::Gui::OriginSymbol*  _originSymbolRef;

		bool _isActive;
		bool _isSelected;
};




class ArrivalData : public Data<Seiscomp::DataModel::Arrival> {
	public:
		ArrivalData(Seiscomp::DataModel::Arrival* arrival)
		 : Data<Seiscomp::DataModel::Arrival>(arrival) {
		}

	public:
		virtual const std::string& id() const {
			return object()->pickID();
		}
};




typedef std::list<ArrivalData> ArrivalDataCollection;
typedef std::list<EventData>   EventDataCollection;




class EventDataRepository {
	public:
		typedef EventDataCollection::iterator event_iterator;
		typedef EventDataCollection::const_iterator const_event_iterator;

	public:
		EventDataRepository();
		EventDataRepository(Seiscomp::DataModel::DatabaseArchive* databaseArchive,
		                   const Seiscomp::Core::TimeSpan& timeSpan);

	public:
		event_iterator eventsBegin();
		event_iterator eventsEnd();

		const_event_iterator eventsBegin() const;
		const_event_iterator eventsEnd() const;

		int eventCount() const;

		void setDatabaseArchive(Seiscomp::DataModel::DatabaseArchive* dataBaseArchive);
		void setEventDataLifeSpan(const Seiscomp::Core::TimeSpan& timeSpan);

		bool addEvent(Seiscomp::DataModel::Event* event, Seiscomp::Gui::OriginSymbol* originSymbol);
		void addOrigin(Seiscomp::DataModel::Origin* origin);
		void addMagnitude(Seiscomp::DataModel::Magnitude* magnitude);
		void addAmplitude(Seiscomp::DataModel::Amplitude* amplitude);
		void addArrival(Seiscomp::DataModel::Arrival* arrival);
		void addPick(Seiscomp::DataModel::Pick* pick);

		EventData* findEvent(const std::string& id);
		EventData* findNextExpiredEvent();
		EventData* findLatestEvent();
		ArrivalData* findArrivalwithPickId(const std::string& pickId);
		Seiscomp::DataModel::Origin* findOrigin(const std::string& id);
		Seiscomp::DataModel::Magnitude* findMagnitude(const std::string& id);

		void removeEvent(const std::string& id);

	private:
		ArrivalDataCollection                           _arrivalDataCollection;
		EventDataCollection                             _eventDataCollection;
		Seiscomp::DataModel::PublicObjectTimeSpanBuffer _objectTimeSpanBuffer;

		Seiscomp::Core::TimeSpan _eventDataObjectLifeSpan;
};


#endif
