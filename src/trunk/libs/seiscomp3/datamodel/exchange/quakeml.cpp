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

#define SEISCOMP_COMPONENT QMLHandler

#include "quakeml.h"

#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/io/xml/handler.h>
#include <seiscomp3/logging/log.h>
#include <set>


#define NS_QML           "http://quakeml.org/xmlns/quakeml/1.2"
#define NS_QML_RT        "http://quakeml.org/xmlns/quakeml-rt/1.2"
#define NS_QML_BED       "http://quakeml.org/xmlns/bed/1.2"
#define NS_QML_BED_RT    "http://quakeml.org/xmlns/bed-rt/1.2"
#define RES_REF_PREFIX   "smi:org.gfz-potsdam.de/geofon/"

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace QML {

REGISTER_EXPORTER_INTERFACE(Exporter, "qml1.2");
REGISTER_EXPORTER_INTERFACE(RTExporter, "qml1.2rt");

Amplitude *findAmplitude(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->amplitudeCount(); ++i )
		if ( ep->amplitude(i)->publicID() == id )
			return ep->amplitude(i);
	return NULL;
}

FocalMechanism *findFocalMechanism(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->focalMechanismCount(); ++i )
		if ( ep->focalMechanism(i)->publicID() == id )
			return ep->focalMechanism(i);
	return NULL;
}

Origin *findOrigin(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->originCount(); ++i )
		if ( ep->origin(i)->publicID() == id )
			return ep->origin(i);
	return NULL;
}

Pick *findPick(EventParameters *ep, const std::string& id) {
	for ( size_t i = 0; i < ep->pickCount(); ++i )
		if ( ep->pick(i)->publicID() == id )
			return ep->pick(i);
	return NULL;
}

// Local type map
struct TypeMapCommon : IO::XML::TypeMap {
	TypeMapCommon();
};
struct TypeMap : TypeMapCommon {
	TypeMap();
};
struct RTTypeMap : TypeMapCommon {
	RTTypeMap();
};
TypeMap __typeMap;
RTTypeMap __rtTypeMap;

inline std::string& replaceIDChars(std::string &id) {
	std::string::iterator it;
	for ( it = id.begin(); it != id.end(); ++it )
		if ( *it == ' ' || *it == ':' ) *it = '_';
	return id;
}

struct ResRefFormatter : Formatter {
	bool mandatory;
	ResRefFormatter(bool mandatory = false) : mandatory(mandatory) {}
	void to(std::string& v) {
		if ( v.empty() ) {
			if ( mandatory )
				v.append(RES_REF_PREFIX"NA");
		}
		else {
			// TODO: Use regex and replace every char not matching pattern
			// class: [\w\d\-\.\*\(\)\+\?_~'=,;#/&amp;]
			// If it does not yet start with "smi:" and thus seems
			// to be a valid smi
			if ( v.compare(0, 4, "smi:") != 0 && v.compare(0, 8, "quakeml:") ) {
				replaceIDChars(v);
				v.insert(0, RES_REF_PREFIX);
			}
		}
	}
};
ResRefFormatter __resRef;
ResRefFormatter __resRefMan(true);

struct MaxLenFormatter : Formatter {
	size_t maxLen;
	MaxLenFormatter(size_t maxLen) : maxLen(maxLen) {}
	void to(std::string& v) {
		if ( v.length() > maxLen ) {
			v.resize(maxLen);
			SEISCOMP_WARNING("max length constraint exceeded cutting string to "
			                 "%lu bytes: %s", (unsigned long) maxLen, v.c_str());
		}
	}
};
MaxLenFormatter __maxLen8(8);
MaxLenFormatter __maxLen32(32);
MaxLenFormatter __maxLen64(64);
MaxLenFormatter __maxLen128(128);

struct AmplitudeUnitFormatter : Formatter {
	void to(std::string &v) {
		if ( v != "m" && v != "s" && v != "m/s" && v != "m/(s*s)" &&
		     v != "m*s" && v != "dimensionless" )
			v = "other";
	}
};
static AmplitudeUnitFormatter __amplitudeUnit;

struct EvaluationStatusFormatter : Formatter {
	void to(std::string& v) {
		if ( v == "reported" ) {
			v = "";
			SEISCOMP_WARNING("dropping unsupported EvaluationStatus value: "
			                 "'reported'");
		}
	}
};
EvaluationStatusFormatter __evaluationStatus;

struct EventTypeFormatter : Formatter {
	void to(std::string &v) {
		if ( v == EEventTypeNames::name(INDUCED_EARTHQUAKE) )
			v = "induced or triggered event";
		else if ( v == EEventTypeNames::name(METEOR_IMPACT) )
			v = "meteorite";
		else if ( v == EEventTypeNames::name(OTHER_EVENT) )
			v = "other event";
		else if ( v == EEventTypeNames::name(NOT_LOCATABLE) ||
		          v == EEventTypeNames::name(OUTSIDE_OF_NETWORK_INTEREST) ||
		          v == EEventTypeNames::name(DUPLICATE) ) {
			SEISCOMP_WARNING("mapping unsupported EventType '%s' to 'other event'",
			                 v.c_str());
			v = "other event";
		}
	}
};
static EventTypeFormatter __eventType;

struct OriginUncertaintyDescriptionFormatter : Formatter {
	void to(std::string &v) {
		if ( v == "probability density function" ) {
			v = "";
			SEISCOMP_WARNING("dropping unsupported OriginUncertaintyDescription"
			                 " value: 'probability density function'");
		}
	}
};
static OriginUncertaintyDescriptionFormatter __originUncertaintyDescription;

// Places FocalMechanisms referenced by current Event as direct child nodes of
// this Event object
struct FocalMechanismConnector : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Event *event = Event::Cast(object);
		if ( event == NULL || event->eventParameters() == NULL ) return false;
		EventParameters *ep = event->eventParameters();
		FocalMechanism *fm;
		for ( size_t i = 0; i < event->focalMechanismReferenceCount(); ++i ) {
			fm = findFocalMechanism(ep, event->focalMechanismReference(i)->focalMechanismID());
			if ( fm != NULL ) {
				output->handle(fm, tag, ns);
			}
		}
		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

// Places Origins, Magnitudes, StationMagnitudes, Amplidues and Picks referenced
// by current Event, its Origins or the Arrivals of the Origins as direct child
// nodes of this Event object
struct OriginConnector : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Event *event = Event::Cast(object);
		if ( event == NULL || event->eventParameters() == NULL ) return false;
		EventParameters *ep = event->eventParameters();
		Origin *origin;
		Magnitude *magnitude;
		StationMagnitude *staMag;
		Amplitude *amplitude;
		Pick *pick;
		bool foundPreferredMagnitude = false;

		std::set<std::string> orgRefs;

		// Collect origin references
		for ( size_t oi = 0; oi < event->originReferenceCount(); ++oi )
			orgRefs.insert(event->originReference(oi)->originID());

		// Check all origins if they should be exported
		for ( size_t i = 0; i < ep->originCount(); ++i ) {
			origin = ep->origin(i);
			if ( orgRefs.find(origin->publicID()) != orgRefs.end() ) {
				for ( size_t mi = 0; mi < origin->magnitudeCount(); ++mi ) {
					magnitude = origin->magnitude(mi);
					if ( event->preferredMagnitudeID() == magnitude->publicID() )
						foundPreferredMagnitude = true;
					output->handle(origin->magnitude(mi), "magnitude", "");
				}
				for ( size_t si = 0; si < origin->stationMagnitudeCount(); ++si ) {
					staMag = origin->stationMagnitude(si);
					amplitude = findAmplitude(ep, staMag->amplitudeID());
					if ( amplitude != NULL) {
						output->handle(amplitude, "amplitude", "");
					}
					output->handle(staMag, "stationMagnitude", "");
				}
				output->handle(origin, tag, ns);
				for ( size_t ai = 0; ai < origin->arrivalCount(); ++ai ) {
					pick = findPick(ep, origin->arrival(ai)->index().pickID);
					if ( pick != NULL ) {
						output->handle(pick, "pick", "");
					}
				}
			}
			else if ( !foundPreferredMagnitude ) {
				// Check for preferred magnitude in unassociated origins if not
				// yet found
				for ( size_t mi = 0; mi < origin->magnitudeCount(); ++mi ) {
					magnitude = origin->magnitude(mi);
					if ( event->preferredMagnitudeID() == magnitude->publicID() ) {
						foundPreferredMagnitude = true;
						output->handle(origin->magnitude(mi), "magnitude", "");
						break;
					}
				}
			}
		}

		if ( !foundPreferredMagnitude )
			SEISCOMP_WARNING("preferred magnitude %s not found", event->preferredMagnitudeID().c_str());

		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

// Places Magnitudes and StationMagnitudes from Origins of all Events as direct
// child nodes of this EventParameters object
struct RTMagnitudeConnector : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		EventParameters *ep = EventParameters::Cast(object);
		if ( ep == NULL ) return false;
		Origin *origin;
		for ( size_t oi = 0; oi < ep->originCount(); ++oi ) {
			origin = ep->origin(oi);
			for ( size_t mi = 0; mi < origin->magnitudeCount(); ++mi ) {
				output->handle(origin->magnitude(mi), tag, ns);
			}
			for ( size_t si = 0; si < origin->stationMagnitudeCount(); ++si ) {
				output->handle(origin->stationMagnitude(si), "stationMagnitude", "");
			}
		}
		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

// Creates MagnitudeReferences from Magniude IDs of all Origins of current Event
struct RTMagnitudeReferenceConnector : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Event *event = Event::Cast(object);
		if ( event == NULL || event->eventParameters() == NULL ) return false;
		EventParameters *ep = event->eventParameters();
		Origin *origin;
		for ( size_t oi = 0; oi < event->originReferenceCount(); ++oi ) {
			origin = findOrigin(ep, event->originReference(oi)->originID());
			if ( origin == NULL ) continue;
			for ( size_t mi = 0; mi < origin->magnitudeCount(); ++mi ) {
				std::string v = origin->magnitude(mi)->publicID();
				__resRefMan.to(v);
				if ( !output->openElement(tag, ns) ) continue;
				output->put(v.c_str());
				output->closeElement(tag, ns);
			}
		}
		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct FormatedPropertyHandler : IO::XML::PropertyHandler {
	Formatter *formatter;
	FormatedPropertyHandler(const Core::MetaProperty *prop, Formatter *format = NULL)
	    : PropertyHandler(prop), formatter(format) {}

	std::string value(Core::BaseObject *obj) {
		std::string v = IO::XML::PropertyHandler::value(obj);
		if ( formatter != NULL ) formatter->to(v);
		return v;
	}
};

template <typename T>
struct PublicIDHandler : IO::XML::MemberHandler {
	std::string value(Core::BaseObject *obj) {
		T *target = T::Cast(obj);
		std::string v = "";
		if ( target ) {
			v = target->publicID();
			__resRef.to(v);
		}
		return v;
		//return RES_REF_PREFIX + target->publicID();
	}
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct StaticHandler : IO::XML::MemberHandler {
	std::string v;
	StaticHandler(const std::string &value) : v(value) {}
	std::string value(Core::BaseObject *obj) { return v; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

// Creates an arrival publicID by combining the pick and origin ID
struct ArrivalPublicIDHandler : IO::XML::MemberHandler {
	std::string value(Core::BaseObject *obj) {
		Arrival *arrival = Arrival::Cast(obj);
		if ( arrival && arrival->origin() ) {
			std::string oid = arrival->origin()->publicID();
			return RES_REF_PREFIX + arrival->pickID() + "_" + replaceIDChars(oid);
		}
		return RES_REF_PREFIX"NA";
	}
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

template <typename T>
void TypedClassHandler<T>::add(const char *property, const char *name,
                               Formatter *format,
                               IO::XML::ClassHandler::Type t,
                               IO::XML::ClassHandler::Location l) {
	const Core::MetaObject *obj = T::Meta();
	if ( obj == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	const Core::MetaProperty *prop = obj->property(property);
	if ( prop == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty: " + property);
	if ( prop->isArray() )
		IO::XML::ClassHandler::addChild(property, "",
		                                new IO::XML::ChildPropertyHandler(prop));
	else
		IO::XML::ClassHandler::addMember(name, "", t, l,
	                                     new FormatedPropertyHandler(prop, format));
}

template <typename T>
void TypedClassHandler<T>::add(const char *property, Formatter *format,
                               IO::XML::ClassHandler::Type t,
                               IO::XML::ClassHandler::Location l) {
	add(property, property, format, t, l);
}

template <typename T>
void TypedClassHandler<T>::addList(const char *properties,
                                   IO::XML::ClassHandler::Type t,
                                   IO::XML::ClassHandler::Location l) {
	std::vector<std::string> toks;
	Core::split(toks, properties, ",");
	const Core::MetaObject *obj = T::Meta();
	if ( obj == NULL )
		throw Core::TypeException(std::string(T::ClassName()) + ": no metaobject");

	std::vector<std::string>::iterator it = toks.begin();
	for ( ; it != toks.end(); ++it) {
		std::string property = Core::trim(*it);
		const Core::MetaProperty *prop = NULL;
		prop = obj->property(property);
		if ( prop == NULL )
			throw Core::TypeException(std::string(T::ClassName()) + ": no metaproperty: " + property);
		if ( prop->isArray() ) {
			IO::XML::ClassHandler::addChild(property.c_str(), "", new IO::XML::ChildPropertyHandler(prop));
		}
		else {
			IO::XML::ClassHandler::addMember(property.c_str(), "", t, l,
			                                 new IO::XML::PropertyHandler(prop));
		}
	}
}

template <typename T>
void TypedClassHandler<T>::addPID() {
	IO::XML::ClassHandler::addMember("publicID", "", IO::XML::ClassHandler::Mandatory,
	          IO::XML::ClassHandler::Attribute, new PublicIDHandler<T>);
}

template <typename T>
void TypedClassHandler<T>::addEmptyPID() {
	IO::XML::ClassHandler::addMember("publicID", "",
	                                 IO::XML::ClassHandler::Mandatory,
	                                 IO::XML::ClassHandler::Attribute,
	                                 new StaticHandler(RES_REF_PREFIX"NA"));
}

struct TimeQuantityHandler : TypedClassHandler<TimeQuantity> {
	TimeQuantityHandler() {
		add("value", NULL, Mandatory);
		addList("uncertainty, lowerUncertainty, upperUncertainty, "
		        "confidenceLevel");
	}
};
static TimeQuantityHandler __timeQuantityHandler;

struct RealQuantityHandler : TypedClassHandler<RealQuantity> {
	RealQuantityHandler() {
		add("value", NULL, Mandatory);
		addList("uncertainty, lowerUncertainty, upperUncertainty, "
		        "confidenceLevel");
	}
};
static RealQuantityHandler __realQuantityHandler;

struct IntegerQuantityHandler : TypedClassHandler<IntegerQuantity> {
	IntegerQuantityHandler() {
		add("value", NULL, Mandatory);
		addList("uncertainty, lowerUncertainty, upperUncertainty, "
		        "confidenceLevel");
	}
};
static IntegerQuantityHandler __integerQuantityHandler;

struct CreationInfoHandler : TypedClassHandler<CreationInfo> {
	CreationInfoHandler() {
		add("agencyID", &__maxLen64);
		add("agencyURI", &__resRef);
		add("author", &__maxLen128);
		add("authorURI", &__resRef);
		add("creationTime");
		add("version", &__maxLen64);
	}
};

struct CommentHandler : TypedClassHandler<Comment> {
	CommentHandler() {
		add("id", &__resRef, Optional, Attribute);
		add("text", NULL, Mandatory);
		add("creationInfo");
	}
};

struct ResourceURIHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		WaveformStreamID *wfsID = WaveformStreamID::Cast(object);
		if ( wfsID == NULL || wfsID->resourceURI().empty() ) return false;
		std::string uri = wfsID->resourceURI();
		__resRef.to(uri);
		output->put(uri.c_str());
		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct WaveformStreamIDHandler : TypedClassHandler<WaveformStreamID> {
	WaveformStreamIDHandler() {
		addList("networkCode, stationCode", Mandatory, Attribute);
		addList("locationCode, channelCode", Optional, Attribute);
		// QuakeML definces resource URI as CDATA of WaveformStreamID
		addChild("resourceURI", "", new ResourceURIHandler());
	}
};

struct PickHandler : TypedClassHandler<Pick> {
	PickHandler() {
		addPID();
		addList("comment, horizontalSlowness, onset, phaseHint, polarity, "
		        "evaluationMode, creationInfo");
		add("time", NULL, Mandatory);
		add("waveformID", NULL, Mandatory);
		add("filterID", &__resRef);
		add("methodID", &__resRef);
		add("slownessMethodID", &__resRef);
		add("evaluationStatus", &__evaluationStatus);
	}
};

struct OriginQualityHandler : TypedClassHandler<OriginQuality> {
	OriginQualityHandler() {
		addList("associatedPhaseCount, usedPhaseCount, associatedStationCount, "
		        "usedStationCount, depthPhaseCount, standardError, "
		        "azimuthalGap, secondaryAzimuthalGap, maximumDistance, "
		        "minimumDistance, medianDistance");
		add("groundTruthLevel", &__maxLen32);
	}
};

// QuakeML requires depth in meter (not kilometer)
struct OriginDepthHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Origin *o = Origin::Cast(object);
		if ( o == NULL ) return false;
		try {
			RealQuantity& depth = o->depth();
			depth.setValue(depth.value() * 1000);
			try { depth.setUncertainty(depth.uncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			try { depth.setUpperUncertainty(depth.upperUncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			try { depth.setLowerUncertainty(depth.lowerUncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			output->handle(&depth, tag, ns, &__realQuantityHandler);

			return true;
		}
		catch ( Core::ValueException ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct TakeOffAngleHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Arrival *arrival = Arrival::Cast(object);
		if ( arrival == NULL ) return false;
		try {
			std::string v = Core::toString(arrival->takeOffAngle());
			const char *tagTA = "takeoffAngle";
			const char *tagV = "value";
			if ( !output->openElement(tagTA, ns) ||
			     !output->openElement(tagV, ns)) {
				SEISCOMP_WARNING("could not open takeoffAngle element");
				return false;
			}
			output->put(v.c_str());
			output->closeElement(tagV, ns);
			output->closeElement(tagTA, ns);
			return true;
		}
		catch ( Core::ValueException ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct ArrivalHandler : TypedClassHandler<Arrival> {
	ArrivalHandler() {
		// public ID is composed of pickID and originID
		IO::XML::ClassHandler::addMember("publicID", "",
		                                 IO::XML::ClassHandler::Mandatory,
		                                 IO::XML::ClassHandler::Attribute,
		                                 new ArrivalPublicIDHandler());
		// NA: comment, backazimuthWeight, horizontalSlownessWeight
		addList("timeCorrection, azimuth, distance, timeResidual, "
		        "horizontalSlownessResidual, backazimuthResidual, "
		        "creationInfo");
		add("pickID", &__resRefMan, Mandatory);
		add("phase", NULL, Mandatory);
		addChild("takeOffAngle", "", new TakeOffAngleHandler());
		add("weight", "timeWeight");
		add("earthModelID", &__resRef);
	}
};

struct PhaseHandler : TypedClassHandler<Phase> {
	PhaseHandler() {
		add("code", NULL, Mandatory, CDATA);
	}
};

struct ConfidenceEllipsoidHandler : TypedClassHandler<ConfidenceEllipsoid> {
	ConfidenceEllipsoidHandler() {
		addList("semiMajorAxisLength, semiMinorAxisLength, "
		        "semiIntermediateAxisLength, majorAxisPlunge, "
		        "majorAxisAzimuth, majorAxisRotation", Mandatory); }
};


struct OriginUncertaintySecondaryHandler : TypedClassHandler<OriginUncertainty> {
	OriginUncertaintySecondaryHandler() {
		addList("horizontalUncertainty, minHorizontalUncertainty, "
		        "maxHorizontalUncertainty, azimuthMaxHorizontalUncertainty, "
		        "confidenceEllipsoid");
		add("preferredDescription", &__originUncertaintyDescription);
	}
};
static OriginUncertaintySecondaryHandler __originUncertaintySecondaryHandler;


// QuakeML requires some uncertainty values in meter (not kilometer)
struct OriginUncertaintyHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Origin *o = Origin::Cast(object);
		if ( o == NULL ) return false;
		try {
			OriginUncertainty &ou = o->uncertainty();
			try { ou.setHorizontalUncertainty(ou.horizontalUncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			try { ou.setMinHorizontalUncertainty(ou.minHorizontalUncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			try { ou.setMaxHorizontalUncertainty(ou.maxHorizontalUncertainty() * 1000); }
			catch ( Core::ValueException ) {}
			try {
				ConfidenceEllipsoid &ce = ou.confidenceEllipsoid();
				ce.setSemiMajorAxisLength(ce.semiMajorAxisLength() * 1000);
				ce.setSemiMinorAxisLength(ce.semiMinorAxisLength() * 1000);
				ce.setSemiIntermediateAxisLength(ce.semiIntermediateAxisLength() * 1000);
			}
			catch ( Core::ValueException ) {}

			output->handle(&ou, "originUncertainty", ns, &__originUncertaintySecondaryHandler);
			return true;
		}
		catch ( Core::ValueException ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct CompositeTimeHandler : TypedClassHandler<CompositeTime> {
	CompositeTimeHandler() {
		addList("year, month, day, hour, minute, second");
	}
};

struct OriginHandler : TypedClassHandler<Origin> {
	OriginHandler() {
		addPID();
		// NA: region
		addList("comment, compositeTime, arrival, time, longitude, latitude, "
		        "depthType, timeFixed, epicenterFixed, quality, type, "
		        "evaluationMode, creationInfo");
		addChild("depth", "", new OriginDepthHandler());
		addChild("uncertainty", "", new OriginUncertaintyHandler());
		add("referenceSystemID", &__resRef);
		add("methodID", &__resRef);
		add("earthModelID", &__resRef);
		add("evaluationStatus", &__evaluationStatus);
	}
};

struct StationMagnitudeHandler : TypedClassHandler<StationMagnitude> {
	StationMagnitudeHandler() {
		addPID();
		addList("comment, waveformID, creationInfo");
		add("magnitude", "mag", NULL, Mandatory);
		add("originID", &__resRef);//, Mandatory);
		add("type", &__maxLen32);
		add("amplitudeID", &__resRef);
		add("methodID", &__resRef);
	}
};

struct StationMagnitudeContributionHandler : TypedClassHandler<StationMagnitudeContribution> {
	StationMagnitudeContributionHandler() {
		add("stationMagnitudeID", &__resRefMan, Mandatory);
		addList("residual, weight");
	}
};

struct MagnitudeHandler : TypedClassHandler<Magnitude> {
	MagnitudeHandler() {
		addPID();
		// NA: evaluationMode
		addList("comment, stationMagnitudeContribution, stationCount, "
		        "azimuthalGap, creationInfo");
		add("magnitude", "mag", NULL, Mandatory);
		add("type", &__maxLen32);
		add("originID", &__resRef);
		add("methodID", &__resRef);
		add("evaluationStatus", &__evaluationStatus);
	}
};

struct TimeWindowHandler : TypedClassHandler<TimeWindow> {
	TimeWindowHandler() {
		addList("begin, end, reference", Mandatory);
	}
};

struct AmplitudeHandler : TypedClassHandler<Amplitude> {
	AmplitudeHandler() {
		addPID();
		// NA: category, evaluationStatus
		addList("comment, period, snr, timeWindow, waveformID, "
		        "scalingTime, evaluationMode, creationInfo");
		add("amplitude", "genericAmplitude");
		add("type", &__maxLen32);
		add("unit", &__amplitudeUnit);
		add("methodID", &__resRef);
		add("pickID", &__resRef);
		add("filterID", &__resRef);
		add("magnitudeHint", &__maxLen32);
	}
};

struct AxisHandler : TypedClassHandler<Axis> {
	AxisHandler() { addList("azimuth, plunge, length", Mandatory); }
};

struct PrincipalAxesHandler : TypedClassHandler<PrincipalAxes> {
	PrincipalAxesHandler() {
		addList("tAxis, pAxis", Mandatory);
		add("nAxis");
	}
};

struct NodalPlaneHandler : TypedClassHandler<NodalPlane> {
	NodalPlaneHandler() { addList("strike, dip, rake", Mandatory); }
};

struct NodalPlanesHandler : TypedClassHandler<NodalPlanes> {
	NodalPlanesHandler() { addList("nodalPlane1, nodalPlane2"); }
};

struct SourceTimeFunctionHandler : TypedClassHandler<SourceTimeFunction> {
	SourceTimeFunctionHandler() {
		addList("type, duration", Mandatory);
		addList("riseTime, decayTime");
	}
};

struct TensorHandler : TypedClassHandler<Tensor> {
	TensorHandler() {
		addList("Mrr, Mtt, Mpp, Mrt, Mrp, Mtp", Mandatory);
	}
};

struct DataUsedHandler : TypedClassHandler<DataUsed> {
	struct DataUsedWaveFormatter : Formatter {
			void to(std::string &v) {
				if ( v == "P body waves" )
					v = "P waves";
				else if ( v == "long-period body waves" )
					v = "body waves";
				else if ( v == "intermediate-period surface waves")
					v = "surface waves";
				else if ( v == "long-period mantle waves" )
					v = "mantle waves";
			}
	};

	DataUsedHandler() {
		static DataUsedWaveFormatter dataUsedWaveFormatter;
		// NA: longestPeriod
		add("waveType", &dataUsedWaveFormatter, Mandatory);
		addList("stationCount, componentCount, shortestPeriod");
	}
};

struct MomentTensorHandler : TypedClassHandler<MomentTensor> {
	MomentTensorHandler() {
		addPID();
		// NA: category, inversionType
		addList("dataUsed, comment, scalarMoment, tensor, variance, "
		    "varianceReduction, doubleCouple, clvd, iso, sourceTimeFunction, "
		    "creationInfo");
		add("derivedOriginID", &__resRef);
		add("momentMagnitudeID", &__resRef);
		add("greensFunctionID", &__resRef);
		add("filterID", &__resRef);
		add("methodID", &__resRef);
	}
};

struct FocalMechanismHandler : TypedClassHandler<FocalMechanism> {
	FocalMechanismHandler() {
		addPID();
		// NA: waveformID
		addList("comment, momentTensor, nodalPlanes, principalAxes, "
		        "azimuthalGap, stationPolarityCount, misfit, "
		        "stationDistributionRatio, creationInfo");
		add("triggeringOriginID", &__resRef);
		add("methodID", &__resRef);
	}
};

struct EventDescriptionHandler : TypedClassHandler<EventDescription> {
	EventDescriptionHandler() {
		add("text", NULL, Mandatory);
		add("type");
	}
};

struct EventHandler : TypedClassHandler<Event> {
	EventHandler() {
		addPID();
		addList("description, comment, typeCertainty, creationInfo");
		addChild("focalMechanism", "", new FocalMechanismConnector());
		// amplitude, magnitude, stationMagnitude, pick: handled by
		// OriginConnector
		addChild("origin", "", new OriginConnector());
		add("preferredOriginID", &__resRef);
		add("preferredMagnitudeID", &__resRef);
		add("preferredFocalMechanismID", &__resRef);
		add("type", &__eventType);
	}
};

struct RTEventHandler : TypedClassHandler<Event> {
	RTEventHandler() {
		addPID();
		addList("description, comment, typeCertainty, creationInfo, "
		        "originReference, focalMechanismReference");
		addChild("magnitudeReference", "", new RTMagnitudeReferenceConnector());
		add("preferredOriginID", &__resRef);
		add("preferredMagnitudeID", &__resRef);
		add("preferredFocalMechanismID", &__resRef);
		add("type", &__eventType);
	}
};

struct EventParametersHandler : TypedClassHandler<EventParameters> {
	EventParametersHandler() {
		addPID();
		// NA: comment, description, creationInfo
		add("event");
	}
};

struct RTOriginReferenceHandler : TypedClassHandler<OriginReference> {
	RTOriginReferenceHandler() {
		add("originID", &__resRef, Mandatory, CDATA);
	}
};

struct RTFocalMechanismReferenceHandler : TypedClassHandler<FocalMechanismReference> {
	RTFocalMechanismReferenceHandler() {
		add("focalMechanismID", &__resRef, Mandatory, CDATA);
	}
};

struct RTPickReferenceHandler : TypedClassHandler<PickReference> {
	RTPickReferenceHandler() {
		add("pickID", &__resRef, Mandatory, CDATA);
	}
};

struct RTAmplitudeReferenceHandler : TypedClassHandler<AmplitudeReference> {
	RTAmplitudeReferenceHandler() {
		add("amplitudeID", &__resRef, Mandatory, CDATA);
	}
};

struct RTReadingHandler : TypedClassHandler<Reading> {
	RTReadingHandler() {
		addPID();
		addList("pickReference, amplitudeReference");
	}
};

struct RTEventParametersHandler : TypedClassHandler<EventParameters> {
	RTEventParametersHandler() {
		addPID();
		// NA: comment, description, creationInfo
		addList("reading, focalMechanism, amplitude, pick, event, origin");
		// stationMagnitude: handled by RTMagnitudeConnector
		addChild("magnitude", "", new RTMagnitudeConnector());
	}
};

struct QuakeMLHandler : IO::XML::NodeHandler {
	virtual bool put(Core::BaseObject *obj, const char *tag, const char *ns,
	                 IO::XML::OutputHandler *output) {
		static EventParametersHandler eventParametersHandler;
		if ( !output->openElement(tag, ns) )
			return false;
		output->handle(obj, "eventParameters", "", &eventParametersHandler);
		output->closeElement(tag, ns);
		return true;
	}
};

struct RTQuakeMLHandler : IO::XML::NodeHandler {
	virtual bool put(Core::BaseObject *obj, const char *tag, const char *ns,
	                 IO::XML::OutputHandler *output) {
		static RTEventParametersHandler eventParametersHandler;
		if ( !output->openElement(tag, ns) )
			return false;
		output->handle(obj, "eventParameters", "", &eventParametersHandler);
		output->closeElement(tag, ns);
		return true;
	}
};

TypeMapCommon::TypeMapCommon() {
	// Generic, used at different locations
	static CreationInfoHandler creationInfoHandler;
	static CommentHandler commentHandler;
	static WaveformStreamIDHandler waveformStreamIDHandler;
	registerMapping<RealQuantity>("", "", &__realQuantityHandler);
	registerMapping<IntegerQuantity>("", "", &__integerQuantityHandler);
	registerMapping<TimeQuantity>("", "", &__timeQuantityHandler);
	registerMapping<CreationInfo>("creationInfo", "", &creationInfoHandler);
	registerMapping<Comment>("comment", "", &commentHandler);
	registerMapping<WaveformStreamID>("waveformID", "", &waveformStreamIDHandler);

	// EventDescription
	static EventDescriptionHandler eventDescriptionHandler;
	registerMapping<EventDescription>("description", "", &eventDescriptionHandler);

	// FocalMechanism
	static FocalMechanismHandler focalMechanismHandler;
	static MomentTensorHandler momentTensorHandler;
	static DataUsedHandler dataUsedHandler;
	static TensorHandler tensorHandler;
	static SourceTimeFunctionHandler sourceTimeFunctionHandler;
	static NodalPlanesHandler nodalPlanesHandler;
	static NodalPlaneHandler nodalPlaneHandler;
	static PrincipalAxesHandler principalAxesHandler;
	static AxisHandler axisHandler;
	registerMapping("focalMechanism", "", "FocalMechanism", &focalMechanismHandler);
	registerMapping("momentTensor", "", "MomentTensor", &momentTensorHandler);
	registerMapping<DataUsed>("dataUsed", "", &dataUsedHandler);
	registerMapping<Tensor>("tensor", "", &tensorHandler);
	registerMapping<SourceTimeFunction>("sourceTimeFunction", "", &sourceTimeFunctionHandler);
	registerMapping<NodalPlanes>("nodalPlanes", "", &nodalPlanesHandler);
	registerMapping<NodalPlane>("", "", &nodalPlaneHandler);
	registerMapping<PrincipalAxes>("pricipalAxes", "", &principalAxesHandler);
	registerMapping<Axis>("", "", &axisHandler);

	// Amplitude
	static AmplitudeHandler amplitudeHandler;
	static TimeWindowHandler timeWindowHandler;
	registerMapping("amplitude", "", "Amplitude", &amplitudeHandler);
	registerMapping<TimeWindow>("timeWindow", "", &timeWindowHandler);

	// Magnitude
	static MagnitudeHandler magnitudeHandler;
	static StationMagnitudeContributionHandler stationMagniutdeContributionHandler;
	registerMapping("magnitude", "", "Magnitude", &magnitudeHandler);
	registerMapping<StationMagnitudeContribution>("stationMagniutdeContribution", "", &stationMagniutdeContributionHandler);

	// StationMagnitude
	static StationMagnitudeHandler stationMagnitudeHandler;
	registerMapping("stationMagnitude", "", "StationMagnitude", &stationMagnitudeHandler);

	// Origin
	static OriginHandler originHandler;
	static CompositeTimeHandler compositeTimeHandler;
	static ConfidenceEllipsoidHandler confidenceEllipsoidHandler;
	static ArrivalHandler arrivalHandler;
	static OriginQualityHandler originQualityHandler;
	static PhaseHandler phaseHandler;
	registerMapping("origin", "", "Origin", &originHandler);
	registerMapping<CompositeTime>("compositeTime", "", &compositeTimeHandler);
	registerMapping<ConfidenceEllipsoid>("confidenceEllipsoid", "", &confidenceEllipsoidHandler);
	registerMapping<Arrival>("arrival", "", &arrivalHandler);
	registerMapping<OriginQuality>("quality", "", &originQualityHandler);
	registerMapping<Phase>("phase", "", &phaseHandler);

	// Pick
	static PickHandler pickHandler;
	registerMapping("pick", "", "Pick", &pickHandler);
	registerMapping<Phase>("phaseHint", "", &phaseHandler);
}

TypeMap::TypeMap() : TypeMapCommon() {
	static QuakeMLHandler quakeMLHandler;
	static EventHandler eventHandler;
	registerMapping<EventParameters>("quakeml", NS_QML, &quakeMLHandler);
	registerMapping("event", "", "Event", &eventHandler);
}

RTTypeMap::RTTypeMap() : TypeMapCommon() {
	static RTQuakeMLHandler rtQuakeMLHandler;
	static RTEventHandler rtEventHandler;
	static RTReadingHandler rtReadingHandler;
	static RTOriginReferenceHandler rtOriginReferenceHandler;
	static RTFocalMechanismReferenceHandler rtFocalMechanismReferenceHandler;
	static RTPickReferenceHandler rtPickReferenceHandler;
	static RTAmplitudeReferenceHandler rtAmplitudeReferenceHandler;
	registerMapping<EventParameters>("quakeml", NS_QML_RT, &rtQuakeMLHandler);
	registerMapping("event", "", "Event", &rtEventHandler);
	registerMapping("originReference", "", "OriginReference", &rtOriginReferenceHandler);
	registerMapping("focalMechanismReferece", "", "FocalMechanismReference", &rtFocalMechanismReferenceHandler);
	registerMapping("reading", "", "Reading", &rtReadingHandler);
	registerMapping("pickReference", "", "PickReference", &rtPickReferenceHandler);
	registerMapping("amplitudeReference", "", "AmplitudeReference", &rtAmplitudeReferenceHandler);
}

Exporter::Exporter() {
	setTypeMap(&__typeMap);
	_defaultNsMap[std::string(NS_QML_BED)] = "";
}

void Exporter::collectNamespaces(Core::BaseObject *obj) {
	// Just copy the defined default namespace map to avoid expensive
	// namespace collections
	_namespaces = _defaultNsMap;

	if ( EventParameters::Cast(obj) ) {
		_namespaces[std::string(NS_QML)] = "q";
	}
}

RTExporter::RTExporter() {
	setTypeMap(&__rtTypeMap);
	_defaultNsMap[std::string(NS_QML_BED_RT)] = "";
}

void RTExporter::collectNamespaces(Core::BaseObject *obj) {
	// Just copy the defined default namespace map to avoid expensive
	// namespace collections
	_namespaces = _defaultNsMap;

	if ( EventParameters::Cast(obj) ) {
		_namespaces[std::string(NS_QML_RT)] = "q";
	}
}


}
}
