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



#ifndef __SEISCOMP_APPLICATIONS_FILTER__
#define __SEISCOMP_APPLICATIONS_FILTER__

#define SEISCOMP_COMPONENT ScImport

#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/waveformquality.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>

namespace Seiscomp {
namespace Applications {



/**
 * \class Filter
 * \brief Abstract class to implement a object filter for scimport.
 *
 * To implement a filter subclass some macros have do be called.
 * \code
 * class YourFilter : public Filter {
 *  	DEFINE_MESSAGE_FILTER( DataModel::SpecificObject::ClassName(), YourFilter );
 *
 *	public:
 *		virtual void init(Config* conf, std::string& configPrefix)	{ ..... }
 *		virtual bool filterImpl(Core::BaseObject* object) { .... }
 * };
 * \endcode
 *
 * Additionally you have to place another macro in your implementation file.
 * \code
 * IMPLEMENT_MESSAGE_FILTER(YourFilter);
 * \endcode
 *
 * Once a filter is implemented it will be automatically registered and used
 * in the message filtering process.
 */
class Filter
{
public:
	enum Operator { EQ = 0, LT, GT, ANY, INVALID };
	struct StringList : std::set<std::string> {
		StringList() {}
		StringList(const std::vector<std::string> &source) { *this = source; }
		StringList(const std::string &source) { *this = source; }
		StringList &operator=(const std::vector<std::string> &source) {
			std::vector<std::string>::const_iterator it;
			clear();
			for ( it = source.begin(); it != source.end(); ++it ) insert(*it);
			return *this;
		}
		StringList &operator=(const std::string &source) {
			clear();
			insert(source);
			return *this;
		}
	};

private:
	typedef std::map<std::string, Filter*> Filters;
	typedef std::map<std::string, Operator> Operators;

protected:

	Filter() {
		_Operators.insert(std::make_pair<std::string, Operator>("eq", EQ));
		_Operators.insert(std::make_pair<std::string, Operator>("lt", LT));
		_Operators.insert(std::make_pair<std::string, Operator>("gt", GT));
		_Operators.insert(std::make_pair<std::string, Operator>("*", ANY));
	}

	virtual ~Filter() {
	}

public:
	bool filter(Core::BaseObject* object)
	{
		return filterImpl(object);
	}

	static Filter* GetFilter(const std::string &className)
	{
		Filters::iterator it = _Registry.find(className);
		if (it == _Registry.end())
			return NULL;
		return it->second;
	}

	static bool Init(Client::Application& app) {
		Filters::iterator it = _Registry.begin();
		bool res = true;
		for ( ; it != _Registry.end(); ++it )
			if ( !it->second->init(app) )
				res = false;
		return res;
	}

protected:

	/**
	 * Needs to be implemented by a subclass to add filter functionality for
	 * a specific object. Returns true if the class is eligible to pass the
	 * filter. False otherwise.
	 * @param object
	 * @return bool
	 */
	virtual bool filterImpl(Core::BaseObject* object) = 0;

	/**
	 * Needs to be implemented by a subclass in order to add initalization code
	 * like reading from a configuration file, default assignement to members and
	 * the like.
	 */
	virtual bool init(Client::Application& app) = 0;

	static void RegisterFilter(const std::string& name, Filter* filterObject)
	{
		_Registry[name] = filterObject;
	}

	static Operator GetOperator(const std::string& op)
	{
		Operators::iterator it = _Operators.find(op);
		if (it != _Operators.end())
			return it->second;
		return INVALID;
	}

private:
	static Filters   _Registry;
	static Operators _Operators;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define DEFINE_MESSAGE_FILTER( OBJECTNAME, FILTERCLASS ) \
public: \
	FILTERCLASS() { \
		RegisterFilter(OBJECTNAME, this); \
} \
private: \
	static FILTERCLASS __##FILTERCLASS##_Object__;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define IMPLEMENT_MESSAGE_FILTER( FILTERCLASS ) \
FILTERCLASS FILTERCLASS::__##FILTERCLASS##_Object__;


template <typename T>
const char *evalMode(T *obj) {
	try {
		return obj->evaluationMode().toString();
	}
	catch ( ... ) {
		return "<none>";
	}
}


template <typename T>
const char *evalStat(T *obj) {
	try {
		return obj->evaluationStatus().toString();
	}
	catch ( ... ) {
		return "<none>";
	}
}


template <typename T>
const char *type(T *obj) {
	try {
		return obj->type().toString();
	}
	catch ( ... ) {
		return "<none>";
	}
}


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class PickFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Pick::ClassName(), PickFilter )

public:
	virtual bool init(Client::Application& app)
	{
		try {
			std::string modeStr = app.configGetString("filter.pick.mode");
			DataModel::EvaluationMode mode;
			if ( !Core::fromString(mode, modeStr) ) {
				SEISCOMP_ERROR("PickFilter: invalid mode, expected 'manual' or 'automatic'");
				return false;
			}

			_mode = mode;
		}
		catch( ... ) {}

		try {
			std::string statusStr = app.configGetString("filter.pick.status");
			DataModel::EvaluationStatus status;
			if ( !Core::fromString(status, statusStr) ) {
				SEISCOMP_ERROR("PickFilter: invalid status, expected 'preliminary', 'confirmed', ...");
				return false;
			}

			_status = status;
		}
		catch( ... ) {}

		try { _phase = app.configGetString("filter.pick.phase"); } catch( ... ) {}

		try { _agencyIDs = app.configGetStrings("filter.pick.agencyIDs"); }
		catch( ... ) {
			try { _agencyIDs = app.configGetString("filter.pick.agencyID"); }
			catch( ... ) {}
		}

		try { _networkCode = app.configGetString("filter.pick.networkCode"); } catch( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject* object)
	{
		// std::cout << "Hello, I am a PickFilter" << std::endl;
		DataModel::Pick *pick = DataModel::Pick::Cast(object);
		if ( !pick ) return false;

		bool pass = accepted(pick);

		SEISCOMP_DEBUG("%s %s %s Mode: %s Status: %s Phase: %s AgencyID: %s",
		               pass?"+":"-",
		               pick->className(), pick->publicID().c_str(),
		               evalMode(pick), evalStat(pick),
		               pick->phaseHint().code().c_str(), objectAgencyID(pick).c_str());

		return pass;
	}

	bool accepted(DataModel::Pick *pick) {
		if ( _mode ) {
			try { if ( *_mode != pick->evaluationMode() ) return false; }
			catch ( ... ) { return false; }
		}

		if ( _status ) {
			try { if ( *_status != pick->evaluationStatus() ) return false; }
			catch ( ... ) { return false; }
		}

		if ( _phase ) {
			try { if ( *_phase != pick->phaseHint().code() ) return false; }
			catch ( ... ) { return false; }
		}

		if ( _agencyIDs && _agencyIDs->find(objectAgencyID(pick)) == _agencyIDs->end() )
			return false;

		if ( _networkCode && *_networkCode != pick->waveformID().networkCode() )
			return false;

		return true;
	}


private:
	OPT(DataModel::EvaluationMode)   _mode;
	OPT(DataModel::EvaluationStatus) _status;
	OPT(std::string)                 _phase;
	OPT(StringList)                  _agencyIDs;
	OPT(std::string)                 _networkCode;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class AmplitudeFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Amplitude::ClassName(), AmplitudeFilter )

public:
	virtual bool init(Client::Application& app) {
		_operator = ANY;
		_amplitude = 0;

		try {
			std::string op = app.configGetString("filter.amplitude.operator");
			_operator = GetOperator(op);
			if ( _operator == INVALID ) {
				SEISCOMP_ERROR("Operator: %s is invalid", op.c_str());
				return false;
			}
		}
		catch ( ... ) {}

		try { _amplitude = app.configGetDouble("filter.amplitude.amplitude"); }
		catch ( ... ) {}

		try {
			_agencyIDs = app.configGetStrings("filter.amplitude.agencyIDs");
		}
		catch ( ... ) {
			try {
				_agencyIDs = app.configGetString("filter.amplitude.agencyID");
			}
			catch ( ... ) {}
		}

		return true;
	}


	virtual bool filterImpl(Core::BaseObject* object) {
		// SEISCOMP_DEBUG("Hello, I am a AmplitudeFilter");
		DataModel::Amplitude *amplitude = DataModel::Amplitude::Cast(object);
		if ( !amplitude ) return false;


		bool pass = accepted(amplitude);

		SEISCOMP_DEBUG("%s %s %s Value: %f",
		               pass?"+":"-",
		               amplitude->className(), amplitude->publicID().c_str(),
		               amplitude->amplitude().value());

		return pass;
	}

	bool accepted(DataModel::Amplitude *amplitude) {
		if ( _agencyIDs && _agencyIDs->find(objectAgencyID(amplitude)) == _agencyIDs->end() )
			return false;

		switch ( _operator ) {
			case EQ:
				if ( _amplitude != amplitude->amplitude() )
					return false;
				break;
			case LT:
				if ( _amplitude > amplitude->amplitude() )
					return false;
				break;
			case GT:
				if ( _amplitude < amplitude->amplitude() )
					return false;
				break;
			default:
				break;
		}

		return true;
	}

private:
	Operator        _operator;
	double          _amplitude;
	OPT(StringList) _agencyIDs;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class OriginFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Origin::ClassName(), OriginFilter )

public:
	static bool WasAccepted() { return _wasAccepted; }

	virtual bool init(Client::Application& app) {
		_latitude = _longitude = _depth = false;

		try {
			std::vector<std::string> tokens;

			// latitude
			std::string lat = app.configGetString("filter.origin.latitude");
			if ( Core::split(tokens, lat.c_str(), ":") == 2 ) {
				if ( !Core::fromString(_lat0, tokens[0]) )
					return false;
				if ( !Core::fromString(_lat1, tokens[1]) )
					return false;
				_latitude = true;
			}
			else {
				SEISCOMP_ERROR("OriginFilter: invalid latitude range");
				return false;
			}
		}
		catch ( ... ) {}

		try {
			std::vector<std::string> tokens;

			// latitude
			std::string lon = app.configGetString("filter.origin.longitude");
			if ( Core::split(tokens, lon.c_str(), ":") == 2 ) {
				if ( !Core::fromString(_lon0, tokens[0]) )
					return false;
				if ( !Core::fromString(_lon1, tokens[1]) )
					return false;
				_longitude = true;
			}
			else {
				SEISCOMP_ERROR("OriginFilter: invalid longitude range");
				return false;
			}
		}
		catch ( ... ) {}

		try {
			std::vector<std::string> tokens;

			// latitude
			std::string dep = app.configGetString("filter.origin.depth");
			if ( Core::split(tokens, dep.c_str(), ":") == 2 ) {
				if ( !Core::fromString(_depth0, tokens[0]) )
					return false;
				if ( !Core::fromString(_depth1, tokens[1]) )
					return false;
				_depth = true;
			}
			else {
				SEISCOMP_ERROR("OriginFilter: invalid depth range");
				return false;
			}
		}
		catch ( ... ) {}

		// agencyId
		try { _agencyIDs = app.configGetStrings("filter.origin.agencyIDs"); }
		catch ( ... ) {
			try { _agencyIDs = app.configGetString("filter.origin.agencyID"); }
			catch ( ... ) {}
		}

		// status
		try {
			std::string statusStr = app.configGetString("filter.origin.status");
			DataModel::EvaluationStatus status;
			if ( !Core::fromString(status, statusStr) ) {
				SEISCOMP_ERROR("OriginFilter: invalid status, expected 'preliminary', 'confirmed', ...");
				return false;
			}

			_status = status;
		}
		catch( ... ) {}

		// mode
		try {
			std::string modeStr = app.configGetString("filter.origin.mode");
			DataModel::EvaluationMode mode;
			if ( !Core::fromString(mode, modeStr) ) {
				SEISCOMP_ERROR("OriginFilter: invalid mode, expected 'manual' or 'automatic'");
				return false;
			}

			_mode = mode;
		}
		catch( ... ) {}

		// phases
		try { _arrivalCount = app.configGetInt("filter.origin.arrivalcount"); }
		catch( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject* object) {
		// SEISCOMP_DEBUG("Hello, I am a OriginFilter");
		DataModel::Origin *origin = DataModel::Origin::Cast(object);
		if ( origin == NULL ) return false;

		bool pass = accepted(origin);

		SEISCOMP_DEBUG("%s %s %s Mode: %s Status: %s AgencyID: %s",
		               pass?"+":"-",
		               origin->className(), origin->publicID().c_str(),
		               evalMode(origin), evalStat(origin), objectAgencyID(origin).c_str());

		_wasAccepted = pass;
		return pass;
	}


	bool accepted(DataModel::Origin *origin) {
		if ( _latitude )
			if ( origin->latitude() < _lat0 || origin->latitude() > _lat1 )
				return false;

		if ( _longitude )
			if ( origin->longitude() < _lon0 || origin->longitude() > _lon1 )
				return false;

		if ( _depth )
			if ( origin->depth() < _depth0 || origin->depth() > _depth1 )
				return false;

		if ( _agencyIDs )
			if ( _agencyIDs->find(objectAgencyID(origin)) == _agencyIDs->end() )
				return false;

		if ( _status ) {
			try {
				if ( origin->evaluationStatus() != *_status )
					return false;
			}
			catch ( ... ) { return false; }
		}

		if ( _mode ) {
			try {
				if ( origin->evaluationMode() != *_mode )
					return false;
			}
			catch ( ... ) { return false; }
		}

		if ( _arrivalCount )
			if ( origin->arrivalCount() < *_arrivalCount )
				return false;

		return true;
	}

private:
	bool                             _latitude;
	double                           _lat0, _lat1;

	bool                             _longitude;
	double                           _lon0, _lon1;

	bool                             _depth;
	double                           _depth0, _depth1;

	OPT(StringList)                  _agencyIDs;

	OPT(DataModel::EvaluationMode)   _mode;

	OPT(DataModel::EvaluationStatus) _status;

	OPT(size_t)                      _arrivalCount;

	static bool                      _wasAccepted;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ArrivalFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Arrival::ClassName(), ArrivalFilter )

public:
	virtual bool init(Client::Application& app) {
		return true;
	}

	virtual bool filterImpl(Core::BaseObject* object) {
		DataModel::Arrival* arrival = DataModel::Arrival::Cast(object);
		if ( !arrival ) return false;

		SEISCOMP_DEBUG("%s %s Pick: %s",
		               OriginFilter::WasAccepted()?"+":"-",
		               arrival->className(), arrival->pickID().c_str());

		// Only pass arrivals if the last origin was accepted
		return OriginFilter::WasAccepted();
	}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class EventFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Event::ClassName(), EventFilter )

public:
	virtual bool init(Client::Application& app) {
		try {
			std::string typeStr = app.configGetString("filter.event.type");
			DataModel::EventType type;
			if ( !Core::fromString(type, typeStr) ) {
				SEISCOMP_ERROR("EventFilter: invalid type, expected 'earthquake', 'not existing', ...");
				return false;
			}

			_type = type;
		}
		catch( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject* object) {
		// SEISCOMP_DEBUG("Hello, I am a EventFilter");
		DataModel::Event *event = DataModel::Event::Cast(object);
		if ( event == NULL ) return false;

		bool pass = accepted(event);

		SEISCOMP_DEBUG("%s %s %s Type: %s",
		               pass?"+":"-",
		               event->className(), event->publicID().c_str(),
		               type(event));

		return pass;
	}

	bool accepted(DataModel::Event *event) {
		if ( _type ) {
			try {
				if ( *_type != event->type() )
					return false;
			}
			catch ( ... ) {
				return false;
			}
		}

		return true;
	}

	OPT(DataModel::EventType) _type;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class StationMagnitudeFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::StationMagnitude::ClassName(), StationMagnitudeFilter )

public:
	virtual bool init(Client::Application& app) {
		try	{
			_type = app.configGetString("filter.stationMagnitude.type");
		}
		catch ( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject* object) {
		// SEISCOMP_DEBUG("Hello, I am a StationMagnitudeFilter");
		DataModel::StationMagnitude *magnitude = DataModel::StationMagnitude::Cast(object);
		if ( magnitude == NULL ) return false;

		if ( _type )
			if ( *_type != magnitude->type() )
				return false;

		return true;
	}

private:
	OPT(std::string) _type;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class MagnitudeFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::Magnitude::ClassName(), MagnitudeFilter )

public:
	virtual bool init(Client::Application &app) {
		try {
			_type = app.configGetString("filter.magnitude.type");
			// SEISCOMP_DEBUG("MagnitudeFilter: enabled");
		}
		catch ( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject *object) {
		// SEISCOMP_DEBUG("Hello, I am a MagnitudeFilter");
		DataModel::Magnitude *magnitude = DataModel::Magnitude::Cast(object);
		if ( magnitude == NULL )
			return false;

		if ( _type )
			if ( *_type != magnitude->type() )
				return false;

		return true;
	}

private:
	OPT(std::string) _type;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class WaveformQualityFilter : public Filter
{
	DEFINE_MESSAGE_FILTER( DataModel::WaveformQuality::ClassName(), WaveformQualityFilter )

public:
	virtual bool init(Client::Application &app) {
		try {
			_type = app.configGetString("filter.qc.type");
		}
		catch ( ... ) {}

		return true;
	}

	virtual bool filterImpl(Core::BaseObject *object) {
		// SEISCOMP_DEBUG("Hello, I am a WaveformQulaityFilter");
		DataModel::WaveformQuality *wq = DataModel::WaveformQuality::Cast(object);
		if ( wq == NULL )
			return false;

		if ( _type )
			if ( *_type != wq->type() )
				return false;

		return true;
	}
private:
	OPT(std::string) _type;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Applications
} // namespace Seiscomp

#endif
