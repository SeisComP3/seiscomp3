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
#include <seiscomp3/utils/units.h>
#include <seiscomp3/logging/log.h>
#include <set>
#include <map>


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
static AmplitudeUnitFormatter __amplitudeUnitFormatter;

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
					if ( magnitude->originID().empty() )
						magnitude->setOriginID(origin->publicID());
					if ( event->preferredMagnitudeID() == magnitude->publicID() )
						foundPreferredMagnitude = true;
					output->handle(origin->magnitude(mi), "magnitude", "");
				}
				for ( size_t si = 0; si < origin->stationMagnitudeCount(); ++si ) {
					staMag = origin->stationMagnitude(si);
					if ( staMag->originID().empty() )
						staMag->setOriginID(origin->publicID());
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
				Magnitude *magnitude = origin->magnitude(mi);
				if ( magnitude->originID().empty() )
					magnitude->setOriginID(origin->publicID());
				output->handle(magnitude, tag, ns);
			}
			for ( size_t si = 0; si < origin->stationMagnitudeCount(); ++si ) {
				StationMagnitude *staMag = origin->stationMagnitude(si);
				if ( staMag->originID().empty() )
					staMag->setOriginID(origin->publicID());
				output->handle(staMag, "stationMagnitude", "");
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
			catch ( Core::ValueException & ) {}
			try { depth.setUpperUncertainty(depth.upperUncertainty() * 1000); }
			catch ( Core::ValueException & ) {}
			try { depth.setLowerUncertainty(depth.lowerUncertainty() * 1000); }
			catch ( Core::ValueException & ) {}
			output->handle(&depth, tag, ns, &__realQuantityHandler);

			return true;
		}
		catch ( Core::ValueException & ) {}
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
		catch ( Core::ValueException & ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct ArrivalWeightHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Arrival *arrival = Arrival::Cast(object);
		if ( arrival == NULL ) return false;

		bool weightSet = false;
		std::string weight = "1";
		try {
			weight = Core::toString(arrival->weight());
			weightSet = true;
		}
		catch ( Core::ValueException & ) {}

		try {
			const char *twTag = "timeWeight";
			if ( (weightSet || arrival->timeUsed()) &&
			     output->openElement(twTag, "") ) {
				output->put(weight.c_str());
				output->closeElement(twTag, "");
			}
		}
		catch ( Core::ValueException & ) {}

		try {
			const char *hzwTag = "horizontalSlownessWeight";
			if ( arrival->horizontalSlownessUsed() &&
			     output->openElement(hzwTag, "") ) {
				output->put(weight.c_str());
				output->closeElement(hzwTag, "");
			}
		}
		catch ( Core::ValueException & ) {}

		try {
			const char *bawTag = "backazimuthWeight";
			if ( arrival->backazimuthUsed() &&
			     output->openElement(bawTag, "") ) {
				output->put(weight.c_str());
				output->closeElement(bawTag, "");
			}
		}
		catch ( Core::ValueException & ) {}

		return true;
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
		// NA: comment
		addList("timeCorrection, azimuth, distance, timeResidual, "
		        "horizontalSlownessResidual, backazimuthResidual, "
		        "creationInfo");
		add("pickID", &__resRefMan, Mandatory);
		add("phase", NULL, Mandatory);
		addChild("takeOffAngle", "", new TakeOffAngleHandler());
		add("earthModelID", &__resRef);

		// weight, timeUsed,horizontalSlownessUsed and backazimuthUsed is
		// converted to timeWeight, horizontalSlownessWeight backazimuthWeight
		IO::XML::ClassHandler::addMember("", "",
		                                 IO::XML::ClassHandler::Mandatory,
		                                 IO::XML::ClassHandler::Element,
		                                 new ArrivalWeightHandler());
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
			catch ( Core::ValueException & ) {}
			try { ou.setMinHorizontalUncertainty(ou.minHorizontalUncertainty() * 1000); }
			catch ( Core::ValueException & ) {}
			try { ou.setMaxHorizontalUncertainty(ou.maxHorizontalUncertainty() * 1000); }
			catch ( Core::ValueException & ) {}
			try {
				ConfidenceEllipsoid &ce = ou.confidenceEllipsoid();
				ce.setSemiMajorAxisLength(ce.semiMajorAxisLength() * 1000);
				ce.setSemiMinorAxisLength(ce.semiMinorAxisLength() * 1000);
				ce.setSemiIntermediateAxisLength(ce.semiIntermediateAxisLength() * 1000);
			}
			catch ( Core::ValueException & ) {}

			output->handle(&ou, "originUncertainty", ns, &__originUncertaintySecondaryHandler);
			return true;
		}
		catch ( Core::ValueException & ) {}
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
		add("originID", &__resRef);
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


struct AmplitudeUnitMap : std::map<std::string, std::string> {
	AmplitudeUnitMap() {
		insert(value_type("MLv", "mm"));
		insert(value_type("MLh", "mm"));
		insert(value_type("ML", "mm"));
		insert(value_type("mb", "nm"));
		insert(value_type("mB", "nm/s"));
		insert(value_type("mBc", "nm/s"));
		insert(value_type("Mjma", "um"));
		insert(value_type("Mwp", "nm*s"));
	}
};

static AmplitudeUnitMap __amplitudeUnits;


struct AmplitudeValueHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Amplitude *amp = Amplitude::Cast(object);
		if ( amp == NULL ) return false;
		try {
			double scale = 1.0;

			if ( amp->unit().empty() ) {
				AmplitudeUnitMap::iterator it = __amplitudeUnits.find(amp->type());
				if ( it != __amplitudeUnits.end() ) {
					const Util::UnitConversion *conv = Util::UnitConverter::get(it->second);
					if ( conv != NULL )
						scale = conv->scale;
				}
			}
			else {
				const Util::UnitConversion *conv = Util::UnitConverter::get(amp->unit());
				if ( conv != NULL )
					scale = conv->scale;
			}

			RealQuantity &amplitude = amp->amplitude();
			amplitude.setValue(amplitude.value() * scale);
			try { amplitude.setUncertainty(amplitude.uncertainty() * scale); }
			catch ( Core::ValueException & ) {}
			try { amplitude.setUpperUncertainty(amplitude.upperUncertainty() * scale); }
			catch ( Core::ValueException & ) {}
			try { amplitude.setLowerUncertainty(amplitude.lowerUncertainty() * scale); }
			catch ( Core::ValueException & ) {}
			output->handle(&amplitude, "genericAmplitude", ns, &__realQuantityHandler);
			return true;
		}
		catch ( Core::ValueException & ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};


struct AmplitudeUnitHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		Amplitude *amp = Amplitude::Cast(object);
		if ( amp == NULL ) return false;

		const std::string *ustr = NULL;

		if ( amp->unit().empty() ) {
			AmplitudeUnitMap::iterator it = __amplitudeUnits.find(amp->type());
			if ( it != __amplitudeUnits.end() ) {
				const Util::UnitConversion *conv = Util::UnitConverter::get(it->second);
				if ( conv != NULL )
					ustr = &conv->toQMLUnit;
			}
		}
		else {
			const Util::UnitConversion *conv = Util::UnitConverter::get(amp->unit());
			if ( conv != NULL )
				ustr = &conv->toQMLUnit;
		}

		if ( !output->openElement(tag, ns) ) return false;;

		if ( ustr != NULL )
			output->put(ustr->c_str());
		else if ( !amp->unit().empty() )
			output->put("other");

		output->closeElement(tag, ns);

		return true;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};


struct AmplitudeHandler : TypedClassHandler<Amplitude> {
	AmplitudeHandler() {
		addPID();
		// NA: category, evaluationStatus
		addList("comment, period, snr, timeWindow, waveformID, "
		        "scalingTime, evaluationMode, creationInfo");
		addChild("amplitude", "", new AmplitudeValueHandler);
		add("type", &__maxLen32);
		addChild("unit", "", new AmplitudeUnitHandler);
		add("methodID", &__resRef);
		add("pickID", &__resRef);
		add("filterID", &__resRef);
		add("magnitudeHint", &__maxLen32);
	}

	// remove amplitude if amplitude value is not set since genericAmplitude
	// is mandatory in QuakeML
	bool put(Core::BaseObject *obj, const char *tag, const char *ns,
	         IO::XML::OutputHandler *output) {
		Amplitude *amplitude = Amplitude::Cast(obj);
		if ( amplitude == NULL ) return false;

		try {
			amplitude->amplitude();
		}
		catch ( Core::ValueException & ) {
			SEISCOMP_WARNING("skipping amplitude %s: amplitude value not set",
			                 amplitude->publicID().c_str());
			return false;
		}
		return TypedClassHandler<Amplitude>::put(obj, tag, ns, output);
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

struct MomentTensorMethodHandler : IO::XML::MemberHandler {
	bool put(Core::BaseObject *object, const char *tag, const char *ns,
	         bool opt, IO::XML::OutputHandler *output, IO::XML::NodeHandler *h) {
		MomentTensor *mt = MomentTensor::Cast(object);
		try {
			MomentTensorMethod method = mt->method();
			const char *tagCategory = "category";
			if ( (method == TELESEISMIC || method == REGIONAL) &&
			     output->openElement(tagCategory, "") ) {
				output->put(EMomentTensorMethodNames::name(method));
				output->closeElement(tagCategory, "");
				return true;
			}
		}
		catch ( Core::ValueException & ) {}
		return false;
	}
	std::string value(Core::BaseObject *obj) { return ""; }
	bool get(Core::BaseObject *object, void *node, IO::XML::NodeHandler *h) { return false; }
};

struct MomentTensorHandler : TypedClassHandler<MomentTensor> {
	MomentTensorHandler() {
		addPID();
		//NA: inversionType
		addList("dataUsed, comment, scalarMoment, tensor, variance, "
		    "varianceReduction, doubleCouple, clvd, iso, sourceTimeFunction, "
		    "creationInfo");

		add("derivedOriginID", &__resRef);
		add("momentMagnitudeID", &__resRef);
		add("greensFunctionID", &__resRef);
		add("filterID", &__resRef);
		add("methodID", &__resRef);
		addChild("method", "", new MomentTensorMethodHandler());
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
	registerMapping<RealQuantity>("RealQuantity", "", &__realQuantityHandler);
	registerMapping<IntegerQuantity>("IntegerQuantity", "", &__integerQuantityHandler);
	registerMapping<TimeQuantity>("TimeQuantity", "", &__timeQuantityHandler);
	registerMapping<CreationInfo>("CreationInfo", "", &creationInfoHandler);
	registerMapping<Comment>("Comment", "", &commentHandler);
	registerMapping<WaveformStreamID>("WaveformStreamID", "", &waveformStreamIDHandler);

	// EventDescription
	static EventDescriptionHandler eventDescriptionHandler;
	registerMapping<EventDescription>("EventDescription", "", &eventDescriptionHandler);

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
	registerMapping("FocalMechanism", "", "FocalMechanism", &focalMechanismHandler);
	registerMapping("MomentTensor", "", "MomentTensor", &momentTensorHandler);
	registerMapping<DataUsed>("DataUsed", "", &dataUsedHandler);
	registerMapping<Tensor>("Tensor", "", &tensorHandler);
	registerMapping<SourceTimeFunction>("SourceTimeFunction", "", &sourceTimeFunctionHandler);
	registerMapping<NodalPlanes>("NodalPlanes", "", &nodalPlanesHandler);
	registerMapping<NodalPlane>("NodalPlane", "", &nodalPlaneHandler);
	registerMapping<PrincipalAxes>("PrincipalAxes", "", &principalAxesHandler);
	registerMapping<Axis>("Axis", "", &axisHandler);

	// Amplitude
	static AmplitudeHandler amplitudeHandler;
	static TimeWindowHandler timeWindowHandler;
	registerMapping("Amplitude", "", "Amplitude", &amplitudeHandler);
	registerMapping<TimeWindow>("TimeWindow", "", &timeWindowHandler);

	// Magnitude
	static MagnitudeHandler magnitudeHandler;
	static StationMagnitudeContributionHandler stationMagniutdeContributionHandler;
	registerMapping("Magnitude", "", "Magnitude", &magnitudeHandler);
	registerMapping<StationMagnitudeContribution>("StationMagniutdeContribution", "", &stationMagniutdeContributionHandler);

	// StationMagnitude
	static StationMagnitudeHandler stationMagnitudeHandler;
	registerMapping("StationMagnitude", "", "StationMagnitude", &stationMagnitudeHandler);

	// Origin
	static OriginHandler originHandler;
	static CompositeTimeHandler compositeTimeHandler;
	static ConfidenceEllipsoidHandler confidenceEllipsoidHandler;
	static ArrivalHandler arrivalHandler;
	static OriginQualityHandler originQualityHandler;
	static PhaseHandler phaseHandler;
	registerMapping("Origin", "", "Origin", &originHandler);
	registerMapping<CompositeTime>("CompositeTime", "", &compositeTimeHandler);
	registerMapping<ConfidenceEllipsoid>("ConfidenceEllipsoid", "", &confidenceEllipsoidHandler);
	registerMapping<Arrival>("Arrival", "", &arrivalHandler);
	registerMapping<OriginQuality>("Quality", "", &originQualityHandler);
	registerMapping<Phase>("Phase", "", &phaseHandler);

	// Pick
	static PickHandler pickHandler;
	registerMapping("Pick", "", "Pick", &pickHandler);
}

TypeMap::TypeMap() : TypeMapCommon() {
	static QuakeMLHandler quakeMLHandler;
	static EventHandler eventHandler;
	registerMapping<EventParameters>("quakeml", NS_QML, &quakeMLHandler);
	registerMapping("Event", "", "Event", &eventHandler);
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
	registerMapping("Event", "", "Event", &rtEventHandler);
	registerMapping("OriginReference", "", "OriginReference", &rtOriginReferenceHandler);
	registerMapping("FocalMechanismReference", "", "FocalMechanismReference", &rtFocalMechanismReferenceHandler);
	registerMapping("Reading", "", "Reading", &rtReadingHandler);
	registerMapping("PickReference", "", "PickReference", &rtPickReferenceHandler);
	registerMapping("AmplitudeReference", "", "AmplitudeReference", &rtAmplitudeReferenceHandler);
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
