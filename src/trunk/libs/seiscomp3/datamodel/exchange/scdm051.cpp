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



#include "scdm051.h"

#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/utils.h>

#include <iostream>


namespace Seiscomp {
namespace DataModel {


REGISTER_IMPORTER_INTERFACE(ImporterSCDM051, "scdm0.51");
REGISTER_EXPORTER_INTERFACE(ExporterSCDM051, "scdm0.51");



/* Example XML

xmlns="http://quakeml.org/xmlns/quakeml/0.51"

<?xml version="1.0"?>
<seiscomp>
  <EventParameters>
    <event publicID="gfz2009diri">
      <preferredOriginID>Origin#20090217112058.110174.1189578</preferredOriginID>
      <preferredMagnitudeID>Origin#20090217112058.110174.1189578#netMag.M</preferredMagnitudeID>
      <description/>
      <agencyID/>
      <originReference>Origin#20090217112058.110174.1189578</originReference>
    ...
  </EventParameters>
</seiscomp>

*/


namespace SCDM051 {



struct TypeMap : public IO::XML::TypeMap {
	TypeMap();
};


TypeMap __typeMap;


template <typename T>
class PublicIDSetter : public IO::XML::MemberHandler {
	public:
		PublicIDSetter() {}

		std::string value(Core::BaseObject *obj) {
			T *target = T::Cast(obj);
			if ( !target ) return "";

			return target->publicID();
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			T *target = T::Cast(object);
			if ( !target ) return false;

			target->setPublicID(h->content(n));
			return true;
		}
};


template <typename T>
class CreatedSetter : public IO::XML::MemberHandler {
	public:
		CreatedSetter() {}

		std::string value(Core::BaseObject *obj) {
			T *target = T::Cast(obj);
			if ( !target ) return "";

			try {
				return target->creationInfo().creationTime().iso();
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			T *target = T::Cast(object);
			if ( !target ) return false;

			Core::Time t;
			if ( !Core::fromString(t, h->content(n)) )
				throw Core::ValueException("invalid time value: created");

			try {
				target->creationInfo().setCreationTime(t);
			}
			catch ( ... ) {
				CreationInfo ci;
				ci.setCreationTime(t);
				target->setCreationInfo(ci);
			}

			return true;
		}
};


template <typename T>
class AuthorSetter : public IO::XML::MemberHandler {
	public:
		AuthorSetter() {}

		std::string value(Core::BaseObject *obj) {
			T *target = T::Cast(obj);
			if ( !target ) return "";

			try {
				return target->creationInfo().author();
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			T *target = T::Cast(object);
			if ( !target ) return false;

			std::string v = h->content(n);
			try {
				target->creationInfo().setAuthor(v);
			}
			catch ( ... ) {
				CreationInfo ci;
				ci.setAuthor(v);
				target->setCreationInfo(ci);
			}

			return true;
		}
};


template <typename T>
class AgencyIDSetter : public IO::XML::MemberHandler {
	public:
		AgencyIDSetter() {}

		std::string value(Core::BaseObject *obj) {
			T *target = T::Cast(obj);
			if ( !target ) return "";

			try {
				return target->creationInfo().agencyID();
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			T *target = T::Cast(object);
			if ( !target ) return false;

			std::string v = h->content(n);
			try {
				target->creationInfo().setAgencyID(v);
			}
			catch ( ... ) {
				CreationInfo ci;
				ci.setAgencyID(v);
				target->setCreationInfo(ci);
			}

			return true;
		}
};


GenericHandler::GenericHandler() {
	mapper = &__typeMap;
}



struct NotifierHandler : public IO::XML::TypedClassHandler<Notifier> {
	NotifierHandler() {
		addProperty("parentID", "", Mandatory, Attribute, "parentID");
		addProperty("operation", "", Mandatory, Attribute, "operation");
		addProperty("", "", Mandatory, Element, "object");
	}


	bool get(Core::BaseObject *obj, void *n) {
		isOptional = false;
		GenericHandler gh;
		if ( !gh.get(obj, n) )
			return false;

		(IO::XML::NodeHandler&)(*this) = gh;
		return true;
	}

	bool finalize(Core::BaseObject *obj, IO::XML::ChildList *cl) {
		for ( IO::XML::ChildList::iterator it = cl->begin(); it != cl->end(); ++it ) {
			Object *dmo = Object::Cast(*it);
			if ( dmo ) {
				static_cast<Notifier*>(obj)->setObject(dmo);
				*it = NULL;
				return true;
			}
		}

		throw Core::ValueException("missing object (child element)");
	}
};


struct NotifierMessageHandler : public IO::XML::ClassHandler {
	NotifierMessageHandler() {}

	bool get(Core::BaseObject *obj, void *n) {
		GenericHandler gh;
		if ( !gh.get(obj, n) )
			return false;

		(IO::XML::NodeHandler&)(*this) = gh;
		return true;
	}

	bool finalize(Core::BaseObject *obj, IO::XML::ChildList *cl) {
		for ( IO::XML::ChildList::iterator it = cl->begin(); it != cl->end(); ++it ) {
			Notifier *notifier = Notifier::Cast(*it);
			if ( notifier ) {
				if ( static_cast<NotifierMessage*>(obj)->attach(notifier) )
					*it = NULL;
			}
		}

		return true;
	}

	bool put(Core::BaseObject *obj, const char *tag, const char *ns, IO::XML::OutputHandler *output) {
		output->openElement(tag, ns);

		NotifierMessage *nm = static_cast<NotifierMessage*>(obj);

		for ( NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
			output->handle(it->get(), NULL, NULL);

		output->closeElement(tag, ns);

		return true;
	}
};



struct DataMessageHandler : public IO::XML::ClassHandler {
	DataMessageHandler() {}

	bool get(Core::BaseObject *obj, void *n) {
		GenericHandler gh;
		if ( !gh.get(obj, n) )
			return false;

		(IO::XML::NodeHandler&)(*this) = gh;
		return true;
	}

	bool finalize(Core::BaseObject *obj, IO::XML::ChildList *cl) {
		for ( IO::XML::ChildList::iterator it = cl->begin(); it != cl->end(); ++it ) {
			if ( static_cast<Core::DataMessage*>(obj)->attach(*it) )
				*it = NULL;
		}

		return true;
	}

	bool put(Core::BaseObject *obj, const char *tag, const char *ns, IO::XML::OutputHandler *output) {
		output->openElement(tag, ns);

		Core::DataMessage *nm = static_cast<Core::DataMessage*>(obj);

		for ( Core::DataMessage::iterator it = nm->begin(); it != nm->end(); ++it )
			output->handle(it->get(), NULL, NULL);

		output->closeElement(tag, ns);

		return true;
	}
};



struct CommentHandler : public IO::XML::TypedClassHandler<Comment> {
	CommentHandler() {
		addMember("created", "", Optional, Attribute, new CreatedSetter<Comment>());
		addMember("author", "", Optional, Element, new AuthorSetter<Comment>());
		addProperty("text", "", Mandatory, Element, "text");
	}
};


struct CompositeTimeHandler : public IO::XML::TypedClassHandler<CompositeTime> {
	CompositeTimeHandler() {
		addProperty("year", "", Optional, Element, "year");
		addProperty("month", "", Optional, Element, "month");
		addProperty("day", "", Optional, Element, "day");
		addProperty("hour", "", Optional, Element, "hour");
		addProperty("minute", "", Optional, Element, "minute");
		addProperty("second", "", Optional, Element, "second");
	}
};



struct TimeQuantityHandler : public IO::XML::TypedClassHandler<TimeQuantity> {
	TimeQuantityHandler() {
		addProperty("value", "", Mandatory, Element, "value");
		addProperty("lowerUncertainty", "", Optional, Element, "lowerUncertainty");
		addProperty("upperUncertainty", "", Optional, Element, "upperUncertainty");
	}
};


struct RealQuantityHandler : public IO::XML::TypedClassHandler<RealQuantity> {
	RealQuantityHandler() {
		addProperty("value", "", Mandatory, Element, "value");
		addProperty("lowerUncertainty", "", Optional, Element, "lowerUncertainty");
		addProperty("upperUncertainty", "", Optional, Element, "upperUncertainty");
	}
};


struct TimeWindowHandler : public IO::XML::TypedClassHandler<TimeWindow> {
	TimeWindowHandler() {
		addProperty("reference", "", Mandatory, Element, "reference");
		addProperty("begin", "", Mandatory, Element, "begin");
		addProperty("end", "", Mandatory, Element, "end");
	}
};


struct WaveformStreamIDHandler : public IO::XML::TypedClassHandler<WaveformStreamID> {
	WaveformStreamIDHandler() {
		addProperty("urn", "", Optional, CDATA, "resourceURI");
		addProperty("networkCode", "", Mandatory, Attribute, "networkCode");
		addProperty("stationCode", "", Mandatory, Attribute, "stationCode");
		addProperty("locationCode", "", Optional, Attribute, "locationCode");
		addProperty("channelCode", "", Mandatory, Attribute, "channelCode");
	}
};


struct StationAmplitudeHandler : public IO::XML::TypedClassHandler<Amplitude> {
	StationAmplitudeHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Amplitude>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<Amplitude>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<Amplitude>());

		addProperty("pickID", "", Optional, Element, "pickID");
		addProperty("waveformID", "", Mandatory, Element, "waveformID");
		addProperty("filterID", "", Optional, Element, "filterID");
		addProperty("methodID", "", Optional, Element, "methodID");
		addProperty("type", "", Optional, Element, "type");
		addProperty("amplitude", "", Mandatory, Element, "amplitude");
		addProperty("period", "", Optional, Element, "period");
		addProperty("timeWindow", "", Optional, Element, "timeWindow");
	}
};


class AzimuthConverter : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			try {
				return Core::toString(fmod(static_cast<RealQuantity*>(obj)->value() + 180., 360.));
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			RealQuantity *rq = static_cast<RealQuantity*>(object);
			std::string v = h->content(n);
			if ( v.empty() ) return false;

			double value;
			if ( !Core::fromString(value, v) ) return false;

			rq->setValue(fmod(value + 180., 360.));

			return true;
		}
};



struct BackazimuthHandler : public IO::XML::TypedClassHandler<RealQuantity> {
	BackazimuthHandler() {
		addMember("value", "", Mandatory, Element, new AzimuthConverter());
		addProperty("lowerUncertainty", "", Optional, Element, "lowerUncertainty");
		addProperty("upperUncertainty", "", Optional, Element, "upperUncertainty");
	}
};


class PickAzimuthHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			return "";
		}

		bool get(Core::BaseObject *object, void *, IO::XML::NodeHandler *h) {
			Pick *pick = static_cast<Pick*>(object);
			pick->setBackazimuth(RealQuantity());
			h->propagate(&pick->backazimuth(), false, true);
			h->childHandler = &_handler;
			return true;
		}

		bool put(Core::BaseObject *obj, const char *tag, const char *ns,
		         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
			Pick *pick = static_cast<Pick*>(obj);
			try {
				_handler.put(&pick->backazimuth(), tag, ns, output);
			}
			catch ( ... ) {}
			return true;
		}

		bool finalize(Core::BaseObject *parent, Core::BaseObject *member) {
			Pick *pick = static_cast<Pick*>(parent);
			RealQuantity *value = static_cast<RealQuantity*>(member);

			if ( value == NULL )
				pick->setBackazimuth(Core::None);

			return true;
		}

	private:
		BackazimuthHandler _handler;
};


class PickStatusHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			try {
				return Core::toString(static_cast<Pick*>(obj)->evaluationMode());
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) {
			Pick *pick = static_cast<Pick*>(object);

			std::string status = h->content(node);
			if ( status == "manual" )
				pick->setEvaluationMode(EvaluationMode(MANUAL));
			else if ( status == "automatic" )
				pick->setEvaluationMode(EvaluationMode(AUTOMATIC));
			else
				throw Core::ValueException(std::string("invalid enumeration: ") + status);

			return true;
		}
};


class PickPolarityHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			try {
				switch ( static_cast<Pick*>(obj)->polarity() ) {
					case POSITIVE:
						return "up";
					case NEGATIVE:
						return "down";
					case UNDECIDABLE:
						return "undecidable";
					default:
						break;
				}
			}
			catch ( ... ) {}
			return "";
		}

		bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) {
			Pick *pick = static_cast<Pick*>(object);

			std::string polarity = h->content(node);
			if ( polarity == "up" )
				pick->setPolarity(PickPolarity(POSITIVE));
			else if ( polarity == "down" )
				pick->setPolarity(PickPolarity(NEGATIVE));
			else if ( polarity == "undecidable" )
				pick->setPolarity(PickPolarity(UNDECIDABLE));
			else
				throw Core::ValueException(std::string("invalid enumeration: ") + polarity);

			return true;
		}
};


struct PickHandler : public IO::XML::TypedClassHandler<Pick> {
	PickHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Pick>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<Pick>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<Pick>());
		addMember("pickerID", "", Optional, Element, new AuthorSetter<Pick>());

		addProperty("waveformID", "", Mandatory, Element, "waveformID");
		addProperty("filterID", "", Optional, Element, "filterID");
		addProperty("time", "", Mandatory, Element, "time");
		addProperty("phaseHint", "", Optional, Element, "phaseHint");
		addProperty("slowness", "", Optional, Element, "horizontalSlowness");
		addProperty("onset", "", Optional, Element, "onset");

		addMember("status", "", Optional, Element, new PickStatusHandler());
		addMember("polarity", "", Optional, Element, new PickPolarityHandler());
		addMember("azimuth", "", Optional, Element, new PickAzimuthHandler());
	}
};


struct MagnitudeReferenceHandler : public IO::XML::TypedClassHandler<StationMagnitudeContribution> {
	MagnitudeReferenceHandler() {
		addProperty("", "", Mandatory, CDATA, "stationMagnitudeID");
		addProperty("weight", "", Optional, Attribute, "weight");
	}
};


struct StationMagnitudeHandler : public IO::XML::TypedClassHandler<StationMagnitude> {
	StationMagnitudeHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<StationMagnitude>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<StationMagnitude>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<StationMagnitude>());
		addProperty("waveformID", "", Optional, Element, "waveformID");
		addProperty("stationAmplitudeID", "", Optional, Element, "amplitudeID");
		addProperty("type", "", Optional, Element, "type");
		addProperty("magnitude", "", Mandatory, Element, "magnitude");
		addProperty("methodID", "", Optional, Element, "methodID");
	}
};


struct NetworkMagnitudeHandler : public IO::XML::TypedClassHandler<Magnitude> {
	NetworkMagnitudeHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Magnitude>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<Magnitude>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<Magnitude>());
		addProperty("stationCount", "", Optional, Element, "stationCount");
		addProperty("type", "", Optional, Element, "type");
		addProperty("magnitude", "", Mandatory, Element, "magnitude");
		addProperty("methodID", "", Optional, Element, "methodID");

		addChildProperty("magnitudeReference", "", "stationMagnitudeContribution");
	}
};


struct PhaseHandler : public IO::XML::TypedClassHandler<Phase> {
	PhaseHandler() {
		addProperty("code", "", Mandatory, CDATA, "code");
	}
};


struct ArrivalHandler : public IO::XML::TypedClassHandler<Arrival> {
	ArrivalHandler() {
		addProperty("pickID", "", Mandatory, Element, "pickID");
		addProperty("phase", "", Mandatory, Element, "phase");
		addProperty("weight", "", Optional, Attribute, "weight");
		addProperty("residual", "", Optional, Element, "timeResidual");
		addProperty("distance", "", Optional, Element, "distance");
		addProperty("azimuth", "", Optional, Element, "azimuth");
		addProperty("earthModelID", "", Optional, Element, "earthModelID");
	}
};


struct OriginQualityHandler : public IO::XML::TypedClassHandler<OriginQuality> {
	OriginQualityHandler() {
		addProperty("stationCount", "", Optional, Element, "usedStationCount");
		addProperty("definingPhaseCount", "", Optional, Element, "usedPhaseCount");
		addProperty("phaseAssociationCount", "", Optional, Element, "associatedPhaseCount");
		addProperty("depthPhaseCount", "", Optional, Element, "depthPhaseCount");
		addProperty("rms", "", Optional, Element, "standardError");
		addProperty("azimuthalGap", "", Optional, Element, "azimuthalGap");
		addProperty("minimumDistance", "", Optional, Element, "minimumDistance");
		addProperty("maximumDistance", "", Optional, Element, "maximumDistance");
		addProperty("medianDistance", "", Optional, Element, "medianDistance");
	}
};


class OriginStatusHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			try {
				return Core::toString(static_cast<Origin*>(obj)->evaluationMode());
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) {
			Origin *org = static_cast<Origin*>(object);

			std::string status = h->content(node);
			if ( status == "manual" )
				org->setEvaluationMode(EvaluationMode(MANUAL));
			else if ( status == "automatic" )
				org->setEvaluationMode(EvaluationMode(AUTOMATIC));
			else if ( status == "preliminary" )
				org->setEvaluationStatus(EvaluationStatus(PRELIMINARY));
			else if ( status == "confirmed" )
				org->setEvaluationStatus(EvaluationStatus(CONFIRMED));
			else if ( status == "rejected" )
				org->setEvaluationStatus(EvaluationStatus(REJECTED));
			else
				throw Core::ValueException(std::string("invalid enumeration: ") + status);

			return true;
		}
};


class OriginHorizontalUncertaintyHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			try {
				return Core::toString(static_cast<Origin*>(obj)->uncertainty().horizontalUncertainty());
			}
			catch ( ... ) {
				return "";
			}
		}

		bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) {
			Origin *org = static_cast<Origin*>(object);

			double value;
			std::string str = h->content(node);
			if ( !str.empty() && !Core::fromString(value, str) )
				throw Core::ValueException(std::string("invalid value: ") + str);

			try {
				org->uncertainty().setHorizontalUncertainty(value);
			}
			catch ( ... ) {
				OriginUncertainty ou;
				ou.setHorizontalUncertainty(value);
				org->setUncertainty(ou);
			}

			return true;
		}
};


struct OriginHandler : public IO::XML::TypedClassHandler<Origin> {
	OriginHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Origin>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<Origin>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<Origin>());
		addProperty("type", "", Optional, Element, "type");

		addProperty("time", "", Mandatory, Element, "time");
		addProperty("longitude", "", Mandatory, Element, "longitude");
		addProperty("latitude", "", Mandatory, Element, "latitude");

		addProperty("depth", "", Optional, Element, "depth");
		addProperty("referenceSystemID", "", Optional, Element, "referenceSystemID");
		addProperty("locationMethodID", "", Optional, Element, "methodID");
		addProperty("earthModelID", "", Optional, Element, "earthModelID");
		addProperty("depthType", "", Optional, Element, "depthType");
		addProperty("quality", "", Optional, Element, "quality");

		addMember("status", "", Optional, Element, new OriginStatusHandler());
		addMember("horizontalUncertainty", "", Optional, Element, new OriginHorizontalUncertaintyHandler());

		addChildProperty("arrival", "", "arrival");
		addChildProperty("stationMagnitude", "", "stationMagnitude");
		addChildProperty("networkMagnitude", "", "magnitude");
		addChildProperty("comment", "", "comment");
		addChildProperty("compositeTime", "", "compositeTime");
	}
};


struct OriginReferenceHandler : public IO::XML::TypedClassHandler<OriginReference> {
	OriginReferenceHandler() {
		addProperty("", "", Mandatory, CDATA, "originID");
	}
};


class EventDescriptionHandler : public IO::XML::MemberHandler {
	public:
		std::string value(Core::BaseObject *obj) {
			Event *evt = static_cast<Event*>(obj);

			return eventRegion(evt);
		}

		bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) {
			Event *evt = static_cast<Event*>(object);

			std::string desc = h->content(node);
			if ( !desc.empty() ) {
				EventDescriptionPtr ed = new EventDescription(desc, REGION_NAME);
				evt->add(ed.get());
			}

			return true;
		}
};


struct EventHandler : public IO::XML::TypedClassHandler<Event> {
	EventHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Event>());
		addMember("created", "", Optional, Attribute, new CreatedSetter<Event>());
		addMember("agencyID", "", Optional, Element, new AgencyIDSetter<Event>());
		addProperty("preferredOriginID", "", Optional, Element, "preferredOriginID");
		addProperty("preferredMagnitudeID", "", Optional, Element, "preferredMagnitudeID");
		addProperty("type", "", Optional, Element, "type");
		addMember("description", "", Optional, Element, new EventDescriptionHandler());

		addChildProperty("originReference", "", "originReference");
		addChildProperty("comment", "", "comment");
	}
};


struct EventParametersHandler : public IO::XML::TypedClassHandler<EventParameters> {
	EventParametersHandler() {
		addChildProperty("pick", "", "pick");
		addChildProperty("stationAmplitude", "", "amplitude");
		addChildProperty("origin", "", "origin");
		addChildProperty("event", "", "event");
	}
};



TypeMap::TypeMap() {
	static NotifierHandler notifierHandler;
	static NotifierMessageHandler notifierMessageHandler;
	static DataMessageHandler dataMessageHandler;

	static ArrivalHandler arrivalHandler;
	static EventParametersHandler eventParametersHandler;
	static EventHandler eventHandler;
	static CommentHandler commentHandler;
	static CompositeTimeHandler compositeTimeHandler;
	static MagnitudeReferenceHandler magnitudeReferenceHandler;
	static NetworkMagnitudeHandler networkMagnitudeHandler;
	static OriginReferenceHandler originReferenceHandler;
	static OriginQualityHandler originQualityHandler;
	static OriginHandler originHandler;
	static PickHandler pickHandler;
	static RealQuantityHandler realQuantityHandler;
	static TimeQuantityHandler timeQuantityHandler;
	static TimeWindowHandler timeWindowHandler;
	static PhaseHandler phaseHandler;
	static StationAmplitudeHandler stationAmplitudeHandler;
	static StationMagnitudeHandler stationMagnitudeHandler;
	static WaveformStreamIDHandler waveformStreamIDHandler;

	registerMapping("notifier", "", "notifier", &notifierHandler);
	registerMapping<NotifierMessage>("notifier_message", "", &notifierMessageHandler);
	registerMapping<Core::DataMessage>("data_message", "", &dataMessageHandler);

	registerMapping<Arrival>("Arrival", "", &arrivalHandler);
	registerMapping<Comment>("Comment", "", &commentHandler);
	registerMapping<CompositeTime>("CompositeTime", "", &compositeTimeHandler);
	registerMapping("Event", "", "Event", &eventHandler);
	registerMapping("EventParameters", "", "EventParameters", &eventParametersHandler);
	registerMapping<StationMagnitudeContribution>("MagnitudeReference", "", &magnitudeReferenceHandler);
	registerMapping("NetworkMagnitude", "", "Magnitude", &networkMagnitudeHandler);
	registerMapping("Origin", "", "Origin", &originHandler);
	registerMapping<OriginQuality>("OriginQuality", "", &originQualityHandler);
	registerMapping<OriginReference>("OriginReference", "", &originReferenceHandler);
	registerMapping<Phase>("Phase", "", &phaseHandler);
	registerMapping("Pick", "", "Pick", &pickHandler);
	registerMapping<RealQuantity>("RealQuantity", "", &realQuantityHandler);
	registerMapping("StationAmplitude", "", "Amplitude", &stationAmplitudeHandler);
	registerMapping("StationMagnitude", "", "StationMagnitude", &stationMagnitudeHandler);
	registerMapping<TimeQuantity>("TimeQuantity", "", &timeQuantityHandler);
	registerMapping<TimeWindow>("TimeWindow", "", &timeWindowHandler);
	registerMapping<WaveformStreamID>("WaveformStreamID", "", &waveformStreamIDHandler);
}


} // namespace SCDM051
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImporterSCDM051::ImporterSCDM051() {
	setRootName("seiscomp");
	setTypeMap(&SCDM051::__typeMap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExporterSCDM051::ExporterSCDM051() {
	setRootName("seiscomp");
	setTypeMap(&SCDM051::__typeMap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
